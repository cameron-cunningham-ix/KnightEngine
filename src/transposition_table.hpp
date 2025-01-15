#include "types.hpp"

/// @brief Transposition Table Entry
struct TTEntry {
    static constexpr int EXACT = 0;
    static constexpr int ALPHA = 1;
    static constexpr int BETA = 1;

    U64 key;
    int depth;
    int flag;
    int score;
    DenseMove bestMove;
    
};