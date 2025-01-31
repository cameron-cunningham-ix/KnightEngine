#pragma once

#include "../../engine_player.hpp"
#include <memory>

// Main UCI loop function
void uciLoop(std::unique_ptr<EnginePlayer>& player);

// Helper function to parse UCI moves
std::vector<DenseMove> parseUCIMoves(const std::string& fen, std::istringstream& iss);