# KnightEngine

A C++20 chess engine development framework featuring the **Syrinx** UCI-compatible chess engine, a full bitboard-based chess library, testing infrastructure, and automated development tooling.

## Overview

KnightEngine is a chess engine project built around a clean, reusable chess library. It currently ships two engines:

| Engine | Version | Description |
|--------|---------|-------------|
| **Syrinx** | v1.25 | Main engine with alpha-beta search, PVS, LMR, and advanced evaluation |
| **RandomEngine** | v1.0 | Baseline random-move engine used for testing |

Both engines are fully UCI-compliant and work with standard chess GUIs (e.g., CuteChess, Arena, Banksia).

## Features

### Search
- Alpha-beta pruning with iterative deepening
- Principal Variation Search (PVS)
- Late Move Reduction (LMR)
- Null move pruning
- Transposition table (~44.7M entries)
- Killer move heuristic
- History table for move ordering

### Evaluation
- Material scoring with tunable piece values
- Piece-square tables with early/endgame interpolation
- Pawn structure (passed, doubled, isolated, supported pawns)
- King safety (pawn shield, open file penalties)
- Bishop pair bonus
- Rook on open/semi-open file bonus
- 17+ UCI options for live parameter tuning

### Chess Library
- Bitboard-based board representation (7 bitboards per color)
- Compact 32-bit `DenseMove` struct (from/to, piece type, capture type, flags)
- Full legal move generation: all piece types, castling, en passant, promotion
- Zobrist hashing for position identification
- PEXT bitboard intrinsics for fast attack lookups

## Project Structure

```
KnightEngine/
├── src/
│   ├── chess_board.hpp/cpp       # Bitboard board state
│   ├── moves.hpp/cpp             # Move generation
│   ├── utility.hpp/cpp           # Evaluation and move scoring
│   ├── types.hpp/cpp             # Core types and enums
│   ├── transposition_table.hpp   # TT implementation
│   ├── zobrist.hpp/cpp           # Zobrist hashing
│   ├── pext_bitboard.hpp/cpp     # Bitboard utilities
│   ├── chess_clock.hpp/cpp       # Time management
│   ├── engine_player.hpp/cpp     # UCI player wrapper
│   └── engine/
│       ├── syrinx_engine.hpp     # Syrinx engine
│       ├── random_engine.hpp     # Random engine
│       ├── uci/                  # UCI entry points
│       └── *_CHANGELOG.md        # Per-engine changelogs
├── test/                         # Google Test suite
├── benchmarks/                   # Google Benchmark suite
├── meta/                         # Development tooling (Python)
│   ├── engine_parameter_tuning.py
│   ├── engine_version_manager.py
│   ├── sprt_test.py
│   └── run_benchmarks.py
├── stored_engines/               # Pre-built engine binaries
│   └── engine_info.json
└── CMakeLists.txt
```

## Building

**Requirements:** CMake 3.16+, C++20-capable compiler (MSVC or GCC/Clang), internet access for dependency fetch.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Build outputs are placed in `build/engines/`. The two engine executables are:
- `syrinx_engine.exe`
- `random_engine.exe`

Dependencies (fetched automatically via CMake FetchContent):
- [Google Test](https://github.com/google/googletest) v1.12.1
- [Google Benchmark](https://github.com/google/benchmark) v1.8.3

## Running

Both engines communicate over stdin/stdout using the [UCI protocol](https://www.wbec-ridderkerk.nl/html/UCIProtocol.html).

```bash
./build/engines/syrinx_engine.exe
```

To use with a GUI, point the GUI's engine path to the built executable.

### UCI Options (Syrinx)

Syrinx exposes its evaluation and search parameters as UCI options, allowing tuning without recompilation. Key options include piece values, piece-square table weights, pawn structure bonuses, king safety penalties, and search depth limits.

## Testing

```bash
cmake --build build --target KnightEngineTests
./build/KnightEngineTests
```

The test suite includes:
- Board state and move generation (including perft verification)
- Engine correctness (mate-in-2, mate-in-3 positions)
- Time management
- UCI player interface

## Development Tooling

All tooling lives in [meta/](meta/).

| Script | Purpose |
|--------|---------|
| `engine_version_manager.py` | Build, tag, and archive engine versions |
| `engine_parameter_tuning.py` | Automated parameter optimization via self-play |
| `sprt_test.py` | SPRT statistical significance testing between versions |
| `run_benchmarks.py` | Run and compare Google Benchmark results |
| `pgn_to_uci.py` | Convert PGN game files to UCI move sequences |

Fast-chess is used for rapid engine-vs-engine games during development and is included in [meta/fastchess/](meta/fastchess/).

## Engine History

Syrinx has been developed through 25+ tracked versions. Major milestones:

- **v0.2**: Initial material-based engine
- **v1.00**: Transposition table integration
- **v1.03**: TT cutoffs
- **v1.20**: Principal Variation Search
- **v1.22**: Late Move Reduction
- **v1.23**: Null move pruning
- **v1.25**: Current release

See [src/engine/Syrinx_Engine_CHANGELOG.md](src/engine/Syrinx_Engine_CHANGELOG.md) for the full history.

## Author

Cameron Cunningham
