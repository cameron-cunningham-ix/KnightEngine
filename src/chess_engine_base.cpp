// src/chess_engine_base.cpp
#include "chess_engine_base.hpp"

bool ChessEngineBase::setOption(const std::string& name, const std::string& value) {
    // Store the option value if it exists
    auto it = options.find(name);
    if (it != options.end()) {
        it->second = value;
        return true;
    }
    return false;  // Option not found
}