#include "types.hpp"

/// @brief Board Utility class
class BUTIL {
public:

    inline static const int indexToRank(int index) { return index / 8; }
    inline static const int squareToFileIndex(int index) { return index % 8; }
    inline static const U64 indexRankMask(int index) { return RankMask << ((index / 8)*8); }
    inline static const U64 indexFileMask(int index) { return FileMask << (index % 8); }

    // Rank masks


    static const U64 RankMask = 0xFFULL;
    static const U64 Rank1 = 0xFFULL;
    static const U64 Rank2 = 0xFF00ULL;
    static const U64 Rank3 = 0xFF0000ULL;
    static const U64 Rank4 = 0xFF000000ULL;
    static const U64 Rank5 = 0xFF00000000ULL;
    static const U64 Rank6 = 0xFF0000000000ULL;
    static const U64 Rank7 = 0xFF000000000000ULL;
    static const U64 Rank8 = 0xFF00000000000000ULL;

    // File masks

    static const U64 FileMask = 0x0101010101010101ULL;
    static const U64 FileA =    0x0101010101010101ULL;
    static const U64 FileB =    0x0202020202020202ULL;
    static const U64 FileC =    0x0404040404040404ULL;
    static const U64 FileD =    0x0808080808080808ULL;
    static const U64 FileE =    0x1010101010101010ULL;
    static const U64 FileF =    0x2020202020202020ULL;
    static const U64 FileG =    0x4040404040404040ULL;
    static const U64 FileH =    0x8080808080808080ULL;

    // Castling masks

    static const U64 W_ShortCastleMask =    0x0000000000000060;
    static const U64 W_LongCastleMask =     0x000000000000000E;
    static const U64 B_ShortCastleMask =    0x6000000000000000;
    static const U64 B_LongCastleMask =     0x0E00000000000000;

    // Square indices

    static const int A1 = 0;
    static const int B1 = 1;
    static const int C1 = 2;
    static const int D1 = 3;
    static const int E1 = 4;
    static const int F1 = 5;
    static const int G1 = 6;
    static const int H1 = 7;
    static const int A2 = 8;
    static const int B2 = 9;
    static const int C2 = 10;
    static const int D2 = 11;
    static const int E2 = 12;
    static const int F2 = 13;
    static const int G2 = 14;
    static const int H2 = 15;
    static const int A3 = 16;
    static const int B3 = 17;
    static const int C3 = 18;
    static const int D3 = 19;
    static const int E3 = 20;
    static const int F3 = 21;
    static const int G3 = 22;
    static const int H3 = 23;
    static const int A4 = 24;
    static const int B4 = 25;
    static const int C4 = 26;
    static const int D4 = 27;
    static const int E4 = 28;
    static const int F4 = 29;
    static const int G4 = 30;
    static const int H4 = 31;
    static const int A5 = 32;
    static const int B5 = 33;
    static const int C5 = 34;
    static const int D5 = 35;
    static const int E5 = 36;
    static const int F5 = 37;
    static const int G5 = 38;
    static const int H5 = 39;
    static const int A6 = 40;
    static const int B6 = 41;
    static const int C6 = 42;
    static const int D6 = 43;
    static const int E6 = 44;
    static const int F6 = 45;
    static const int G6 = 46;
    static const int H6 = 47;
    static const int A7 = 48;
    static const int B7 = 49;
    static const int C7 = 50;
    static const int D7 = 51;
    static const int E7 = 52;
    static const int F7 = 53;
    static const int G7 = 54;
    static const int H7 = 55;
    static const int A8 = 56;
    static const int B8 = 57;
    static const int C8 = 58;
    static const int D8 = 59;
    static const int E8 = 60;
    static const int F8 = 61;
    static const int G8 = 62;
    static const int H8 = 63;
};