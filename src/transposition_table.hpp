#pragma once

#include "types.hpp"

/// @brief Transposition Table Entry
/// Each entry is 24 bytes 
struct TTEntry {
    // Constant values for 'flag' to indicate what the score
    // attribute is determining for this entry

    // If the score of the node was exact
    static constexpr int EXACT = 0;
    // If the score of the node was at most 'score'
    static constexpr int ALPHA = 1;
    // If the score of the node was at least 'score'
    static constexpr int BETA = 2;
    // If the score of the node is unknown
    static constexpr int UNKNOWN = 3;

    // Stored Zobrist key for this entry in the transposition table
    U64 key;
    // The search depth of this entry
    // Recorded as we don't want to trust values from depths that
    // are shallower than the current depth we're searching
    int depth;
    // Flag for indicating what score means for this entry;
    // Set to either EXACT, ALPHA, BETA, or UNKNOWN
    int flag;
    // 
    int score;
    DenseMove bestMove;
    
};