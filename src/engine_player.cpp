// src/engine_player.cpp
#include "engine_player.hpp"
#include <sstream>

EnginePlayer::EnginePlayer(std::unique_ptr<ChessEngineBase> engineImpl, 
                         bool acceptDrawOffers,
                         std::chrono::milliseconds minTimePerMove,
                         std::chrono::milliseconds maxTimePerMove)
    : engine(std::move(engineImpl))
    , acceptDraws(acceptDrawOffers)
    , minTime(minTimePerMove)
    , maxTime(maxTimePerMove)
    , initialized(false)
    , thinking(false)
    , shouldQuit(false) {
    // Start UCI thread
    uciThread = std::thread(&EnginePlayer::uciLoop, this);
}

EnginePlayer::~EnginePlayer() {
    quit();
    if (uciThread.joinable()) {
        uciThread.join();
    }
}

Move EnginePlayer::getMove(const ChessBoard& board, 
                          const GameState& state,
                          const ChessClock& clock) {
    int depth = calculateSearchDepth(clock);
    thinking = true;
    bestMove = engine->findBestMove(board, state, depth);
    thinking = false;
    return bestMove;
}

void EnginePlayer::notifyOpponentMove(const Move& move) {
    // Notify engine about opponent's move for any internal state updates
    std::vector<Move> moves = {move};
    position("", moves);  // Empty FEN means use current position
}

void EnginePlayer::onGameEnd() {
    uciNewGame();  // Reset engine state for new game
}

// UCI Interface Implementation
void EnginePlayer::uci() {
    std::unique_lock<std::mutex> lock(mutex);
    commandQueue.push("uci");
    cv.notify_one();
}

void EnginePlayer::isReady() {
    std::unique_lock<std::mutex> lock(mutex);
    commandQueue.push("isready");
    cv.notify_one();
}

void EnginePlayer::setOption(const std::string& name, const std::string& value) {
    engine->setOption(name, value);
    options[name].currentValue = value;
}

void EnginePlayer::uciNewGame() {
    engine->setSearchDepth(4);  // Reset to default depth
    // Reset any engine-specific state
}

void EnginePlayer::position(const std::string& fen, const std::vector<Move>& moves) {
    std::stringstream ss;
    ss << "position ";
    if (fen.empty()) {
        ss << "startpos";
    } else {
        ss << "fen " << fen;
    }
    if (!moves.empty()) {
        ss << " moves";
        for (const Move& move : moves) {
            ss << " " << moveToUCI(move);
        }
    }
    
    std::unique_lock<std::mutex> lock(mutex);
    commandQueue.push(ss.str());
    cv.notify_one();
}

void EnginePlayer::go(const std::map<std::string, std::string>& searchParams) {
    std::stringstream ss;
    ss << "go";
    for (const auto& [param, value] : searchParams) {
        ss << " " << param << " " << value;
    }
    
    std::unique_lock<std::mutex> lock(mutex);
    commandQueue.push(ss.str());
    cv.notify_one();
}

void EnginePlayer::stop() {
    engine->stopSearch();
    thinking = false;
}

void EnginePlayer::quit() {
    shouldQuit = true;
    cv.notify_all();
}

bool EnginePlayer::hasOption(const std::string& name) const {
    return options.count(name) > 0;
}

int EnginePlayer::calculateSearchDepth(const ChessClock& clock) const {
    // Get remaining time for current player
    auto remainingTime = (clock.getActiveColor() == WHITE) ? 
                        clock.getWhiteTime() : 
                        clock.getBlackTime();
    
    // Calculate maximum time we can spend on this move
    auto maxTimeForMove = std::min(maxTime, remainingTime / 30);
    maxTimeForMove = std::max(maxTimeForMove, minTime);
    
    // Convert time to depth using a simple heuristic
    int depth = 1;
    auto timeForDepth = std::chrono::milliseconds(50);
    
    while (timeForDepth * 5 < maxTimeForMove) {
        depth++;
        timeForDepth *= 5;
    }
    
    return std::clamp(depth, 1, engine->getSearchDepth());
}

void EnginePlayer::uciLoop() {
    while (!shouldQuit) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !commandQueue.empty() || shouldQuit; });
        
        if (shouldQuit) break;
        
        std::string cmd = commandQueue.front();
        commandQueue.pop();
        lock.unlock();
        
        processCommand(cmd);
    }
}

void EnginePlayer::processCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string token;
    iss >> token;
    
    if (token == "uci") {
        // Send engine identification
        sendResponse("id name " + engine->getName() + " " + engine->getVersion());
        sendResponse("id author " + engine->getAuthor());
        
        // Send options
        for (const auto& [name, option] : options) {
            std::string optionStr = "option name " + name;
            switch (option.type) {
                case Option::Type::Check:
                    optionStr += " type check default " + option.defaultValue;
                    break;
                case Option::Type::Spin:
                    optionStr += " type spin default " + option.defaultValue + 
                                " min " + std::to_string(option.minValue) +
                                " max " + std::to_string(option.maxValue);
                    break;
                // ... handle other option types
            }
            sendResponse(optionStr);
        }
        
        sendResponse("uciok");
        initialized = true;
    }
    else if (token == "isready") {
        sendResponse("readyok");
    }
    else if (token == "go") {
        // Parse search parameters
        std::map<std::string, std::string> searchParams;
        while (iss >> token) {
            std::string value;
            if (iss >> value) {
                searchParams[token] = value;
            }
        }
        
        // Start search in a separate thread
        std::thread([this, searchParams] {
            thinking = true;
            // Find best move using parameters
            // For now, just use base engine search
            ChessBoard board;  // Need to track current position
            GameState state;
            bestMove = engine->findBestMove(board, state);
            thinking = false;
            sendResponse("bestmove " + moveToUCI(bestMove));
        }).detach();
    }
    // ... handle other UCI commands
}

void EnginePlayer::sendResponse(const std::string& response) {
    std::cout << response << std::endl;
}

std::string EnginePlayer::moveToUCI(const Move& move) const {
    std::string from = indexToAlgebraic(move.from);
    std::string to = indexToAlgebraic(move.to);
    std::string promotion = "";
    
    if (move.isPromotion) {
        switch (move.promoteTo) {
            case W_QUEEN:
            case B_QUEEN: promotion = "q"; break;
            case W_ROOK:
            case B_ROOK: promotion = "r"; break;
            case W_BISHOP:
            case B_BISHOP: promotion = "b"; break;
            case W_KNIGHT:
            case B_KNIGHT: promotion = "n"; break;
            default: break;
        }
    }
    
    return from + to + promotion;
}

Move EnginePlayer::uciToMove(const std::string& uciMove, 
                            const ChessBoard& board,
                            const GameState& state) const {
    if (uciMove.length() < 4) return Move();
    
    int from = algebraicToIndex(uciMove.substr(0, 2));
    int to = algebraicToIndex(uciMove.substr(2, 2));
    
    if (from == -1 || to == -1) return Move();
    
    PieceType piece = board.getPieceAt(from);
    bool isCapture = board.getPieceAt(to) != static_cast<PieceType>(-1);
    bool isPromotion = uciMove.length() > 4;
    
    Move move(piece, from, to, isCapture, isPromotion);
    
    if (isPromotion && uciMove.length() > 4) {
        // Set promotion piece
        char prom = uciMove[4];
        PieceType promoteTo;
        switch (prom) {
            case 'q': promoteTo = state.sideToMove == WHITE ? W_QUEEN : B_QUEEN; break;
            case 'r': promoteTo = state.sideToMove == WHITE ? W_ROOK : B_ROOK; break;
            case 'b': promoteTo = state.sideToMove == WHITE ? W_BISHOP : B_BISHOP; break;
            case 'n': promoteTo = state.sideToMove == WHITE ? W_KNIGHT : B_KNIGHT; break;
            default: return Move();
        }
        move.promoteTo = promoteTo;
    }
    
    return move;
}