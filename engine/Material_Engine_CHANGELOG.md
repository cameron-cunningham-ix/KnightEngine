# Material Engine Changelog

## Material Engine V0.315 (2025-01-11)
- Added sendInfo functionality; engine now sends info back to UCI
about currmove, currmovenumber, depth, score (centipawns), time (to
get move), and nodes searched along with nodes/second
- Flipped score for checkmate in alphaBeta:
    If alphaBeta is maximizing, return 999999, else - 999999.
    I'm not sure why this is working better, since the check is for
    it's own side in checkmate (as far as I can tell).
- Won 88 / 100 games against RandomEngine V1.0 in first test

## Material Engine V0.31 (2025-01-11) 
- Changed attacks to opposite king bonus to 3000
- It seems like the engine is trying to maximize score WITHOUT
checkmating the king

## Material Engine V0.3 (2025-01-10)
- Changed alphaBeta function {
    if maximizing, eval >= beta break
    if minimizing, eval <= alpha break
}

## Material Engine V0.2 (2025-01-10)
Features:
- Basic UCI implementation, successful communication with 
CuteChess 
- Basic attempt of evaluation

Known Issues:
- Evaluation is quite possibly not statistically any better than playing random moves.
It drew 9 out of 10 games against RandomEngine V1.0.

Goals:
- Make evaluation statistically better than playing random moves.