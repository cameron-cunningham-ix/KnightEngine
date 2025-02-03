#include "../syrinx_engine.hpp"
#include "uci_handler.hpp"
#include <memory>

int main() {
    // Create engine instance
    auto engine = std::make_unique<Syrinx>();
    auto player = std::make_unique<EnginePlayer>(std::move(engine));

    // Start UCI loop
    uciLoop(player);
    
    return 0;
}