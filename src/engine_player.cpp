#include "engine_player.hpp"
#include "../engine/material_engine.hpp"
#include <sstream>
#include <string>

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
    // Start UCI thread for engine player
    uciThread = std::thread(&EnginePlayer::uciLoop, this);
    // Create default clock
    currentClock = ChessClock();
    currentClock.setInfinite(true);
}

EnginePlayer::~EnginePlayer() {
    quit();
    if (uciThread.joinable()) {
        uciThread.join();
    }
    if (searchThread && searchThread->joinable()) {
        searchThread->join();
    }
}

DenseMove EnginePlayer::getMove(ChessBoard& board, 
                                const ChessClock& clock) {
    int depth = calculateSearchDepth(clock);
    thinking = true;
    bestMove = engine->findBestMove(board, depth);
    thinking = false;
    return bestMove;
}

void EnginePlayer::notifyOpponentMove(const DenseMove& move) {
    // Notify engine about opponent's move for any internal state updates
    std::string moveStr = moveToUCI(move);
    position("", moveStr);  // Empty FEN means use current position
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

void EnginePlayer::setOption(const std::string &name, const std::string &value) {
    std::unique_lock<std::mutex> lock(mutex);
    commandQueue.push(std::format("setoption name {} value {}", name, value));
    cv.notify_one();
}

void EnginePlayer::uciNewGame() {
    engine->resetSearchDepth();  // Reset to default depth
    // Reset any engine-specific state
    currentClock = ChessClock();
    currentClock.setInfinite(true);
    currentBoard = ChessBoard();
}

void EnginePlayer::position(const std::string& fen, const std::string& moves) {
    std::stringstream ss;
    ss << "position ";
    if (fen.empty()) {
        ss << "startpos ";
    } else {
        ss << "fen " << fen;
    }
    if (!moves.empty()) {
        ss << " moves " << moves;
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

bool EnginePlayer::isInitialized() const {
    std::lock_guard<std::mutex> lock(mutex);
    return initialized;
}

int EnginePlayer::calculateSearchDepth(const ChessClock& clock) const {
    // If clock is set to infinite, we can return the set search depth
    if (clock.isInfinite()) return engine->getSearchDepth();
    // Get remaining time for current player
    auto remainingTime = (clock.getActiveColor() == WHITE) ? 
                        clock.getWhiteTime() : 
                        clock.getBlackTime();
    
    // Calculate maximum time we can spend on this move (10th of remaining time)
    auto timePerMove = remainingTime / 10;
    
    auto maxMoveTime = std::min(maxTime, timePerMove);    
    auto actualMoveTime = std::max(maxMoveTime, minTime);
    
    // Convert time to depth using a simple heuristic
    int depth = 1;
    auto timeForDepth = std::chrono::milliseconds(20);
    
    while (timeForDepth * 2 < actualMoveTime) {
        depth++;
        timeForDepth *= 2;
    }
    
    int finalDepth = std::clamp(depth, 1, engine->getSearchDepth());
    return finalDepth;
}

/// @brief 
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
        // Ensure that PEXT is initialized
        if (!PEXT::initialized) {
            PEXT::initialize();
        }
        // Send engine identification
        sendResponse("id name " + engine->getName() + " " + engine->getVersion());
        sendResponse("id author " + engine->getAuthor());
        
        // Send all available options
        const auto& engineOptions = engine->getOptions();
        for (const auto& [name, option] : engineOptions) {
            sendResponse(option.toUCIString());
        }
        
        sendResponse("uciok");
        initialized = true;
    }
    else if (token == "setoption") {
        std::string nameToken, name, valueToken, value;
        iss >> nameToken; // Skip "name"
        iss >> name;
        iss >> valueToken; // Should be "value"
        
        if (valueToken == "value") {
            iss >> value;
            if (!engine->setOption(name, value)) {
                // Log error if option setting failed
                std::cerr << "Failed to set option " << name << " to value " << value << std::endl;
            }
        }
    }
    else if (token == "ucinewgame") {
        uciNewGame();
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
                searchParams.insert({token, value});
            }
        }
    
        // Start search in a controlled thread
        std::lock_guard<std::mutex> lock(boardMutex);
        if (searchThread && searchThread->joinable()) {
            searchThread->join();
        }
        searchThread = std::make_unique<std::thread>([this, searchParams] {
            // thinking = true;
            currentClock.setInfinite(true);
            // Set search parameters
            // Depth
            auto it = searchParams.find("depth");
            if (it != searchParams.end()) {
                if (it->second == "infinite") {
                    engine->setSearchDepth(128);   // Very large depth
                } 
                // If not infinite, there should only be digits
                else if (it->second.find_first_not_of("0123456789") == std::string::npos) {
                    engine->setSearchDepth(std::stoi(it->second));
                }
                else {
                    engine->resetSearchDepth();
                }
            } else {
                engine->resetSearchDepth();
            }
            // Clock time
            it = searchParams.find("wtime");
            if (it != searchParams.end()) {
                currentClock.setTime(WHITE, std::chrono::milliseconds(std::stoi(it->second)));
                currentClock.setInfinite(false);
            }
                
            it = searchParams.find("btime");
            if (it != searchParams.end()) {
                currentClock.setTime(BLACK, std::chrono::milliseconds(std::stoi(it->second)));
                currentClock.setInfinite(false);
            } 
            
            // Use current board position
            std::lock_guard<std::mutex> lock(boardMutex);
            bestMove = getMove(currentBoard, currentClock);
            
            // thinking = false;
            sendResponse("bestmove " + moveToUCI(bestMove));
        });
    } 
    else if (token == "position") {
        std::lock_guard<std::mutex> lock(boardMutex);
        // Parse position command and update currentBoard

        iss >> token;   // Get next token

        // Set up initial position
        if (token == "startpos") {
            currentBoard = ChessBoard();
            iss >> token;   // Consume "moves" if present
        }
        else if (token == "fen") {
            // Collect all parts of the FEN string
            std::string fen;
            // FEN has 6 fields separated by spaces
            for (int i = 0; i < 6; i++) {
                std::string fenPart;
                if (!(iss >> fenPart)) break;   // 
                fen += (i == 0 ? "" : " ") + fenPart;
            }
            currentBoard = ChessBoard();
            currentBoard.setupPositionFromFEN(fen);
            iss >> token;   // Consume "moves" if present
        }

        // If there's any moves to make
        if (token == "moves") {
            std::string moveStr;
            while (iss >> moveStr) {
                DenseMove move = uciToMove(moveStr, currentBoard);
                currentBoard.makeMove(move, false);
            }
        }
    }
    else if (token == "stop") {
        engine->stopSearch();
        if (searchThread && searchThread->joinable()) {
            searchThread->join();
        }
    }
    /// @todo Handle other UCI commands
}

void EnginePlayer::sendResponse(const std::string& response) {
    std::cout << response << std::endl;
}

std::string EnginePlayer::moveToUCI(const DenseMove& move) const {
    std::string from = indexToAlgebraic(move.getFrom());
    std::string to = indexToAlgebraic(move.getTo());
    std::string promotion = "";
    
    if (move.getPromoteDense() != D_EMPTY) {
        switch (move.getPromotePiece()) {
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
/// @brief This function assumes that the UCI string is a valid move on the board
/// @param uciMove 
/// @param board 
/// @return 
DenseMove EnginePlayer::uciToMove(const std::string& uciMove, 
                                  const ChessBoard& board) const {
    if (uciMove.length() < 4) return DenseMove();
    
    int from = algebraicToIndex(uciMove.substr(0, 2));
    int to = algebraicToIndex(uciMove.substr(2, 2));
    
    if (from == -1 || to == -1) return DenseMove();
    
    PieceType piece = board.getPieceAt(from);
    DenseType capturedPiece = board.getDenseTypeAt(to);
    bool isPromotion = uciMove.length() > 4;
    
    DenseMove move(piece, from, to, capturedPiece);
    
    if (isPromotion && uciMove.length() > 4) {
        // Set promotion piece
        char prom = uciMove[4];
        DenseType promoteTo;
        switch (prom) {
            case 'q': promoteTo = D_QUEEN; break;
            case 'r': promoteTo = D_ROOK; break;
            case 'b': promoteTo = D_BISHOP; break;
            case 'n': promoteTo = D_KNIGHT; break;
            default: return DenseMove();
        }
        move.setPromoteTo(promoteTo);
    }
    // Castling
    if ((piece == W_KING && from == 4 && (to == 2 || to == 6)) || 
        (piece == B_KING && from == 60 && (to == 58 || to == 62))) {
            move.setCastle(true);
    }
    // En passant
    // Pawn moving to en passant square has to be capturing en passant
    if ((piece == W_PAWN || piece == B_PAWN) && to == board.currentGameState.enPassantSquare) {
        move.setEnPass(true);
    }
    
    return move;
}