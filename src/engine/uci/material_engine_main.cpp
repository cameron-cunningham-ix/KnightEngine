#include "../material_engine.hpp"
#include "uci_handler.hpp"
#include <memory>

int main() {
    // Create engine instance
    auto engine = std::make_unique<MaterialEngine>();
    auto player = std::make_unique<EnginePlayer>(std::move(engine));

    // Set EnginePlayer settings
    player->setSearchDepth(8);
    
    // Start UCI loop
    uciLoop(player);
    
    return 0;
}