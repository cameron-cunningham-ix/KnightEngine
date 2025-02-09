# Syrinx Engine Changelog

## Syrinx Engine V1.03
- Doing alpha-beta cutoffs based on TT; fairly significant speedup


## Syrinx Engine V1.02
- Changed maximum search depth to 8
- Changed king PST to try and discourage its attempts
at early suicide by walking to castle square instead of castling


## Syrinx Engine V1.01
- Changed Rook and King PSTs to try to promote
castling more and avoid king moving early
- Added airyKing penalty

## Syrinx Engine V1.0



## Syrinx Engine V0.431
- Changed bishop pair score to only score for actual pair (light and dark)

## Syrinx Engine V0.42 (2025-01-13)
- Some evaluation constants changed

## Syrinx Engine V0.41 (2025-01-12)
- Added piece-square tables for early game and endgame, and 
ENDGAME_LERP for interpolating between early and endgame

## Syrinx Engine V0.4 (2025-01-12)
- Changed alpha beta function again, it seems to be working better
Issues:
- Doesn't account for three-fold repetition, so it makes those draws
a lot. Seems like Zobrist hash keys could help to solve this as well
as evaluating the same position multiple times

## Syrinx Engine V0.321 (2025-01-12)
- Changed supported pawn bonus to 90

## Syrinx Engine V0.32 (2025-01-12)
- Added SupportedPawnBonus
- Flipped return value for side in check back to what it should be for 
alpha beta (INF_NEG for white, INF_POS for black)
Note: This makes it worse for some reason :( might have to refactor
alphaBeta and findBestMove functions

## Syrinx Engine V0.315 (2025-01-11)
- Added sendInfo functionality; engine now sends info back to UCI
about currmove, currmovenumber, depth, score (centipawns), time (to
get move), and nodes searched along with nodes/second
- Flipped score for checkmate in alphaBeta:
    If alphaBeta is maximizing, return 999999, else - 999999.
    I'm not sure why this is working better, since the check is for
    it's own side in checkmate (as far as I can tell).
- Won 88 / 100 games against RandomEngine V1.0 in first test

## Syrinx Engine V0.31 (2025-01-11) 
- Changed attacks to opposite king bonus to 3000
- It seems like the engine is trying to maximize score WITHOUT
checkmating the king

## Syrinx Engine V0.3 (2025-01-10)
- Changed alphaBeta function {
    if maximizing, eval >= beta break
    if minimizing, eval <= alpha break
}

## Syrinx Engine V0.2 (2025-01-10)
Features:
- Basic UCI implementation, successful communication with 
CuteChess 
- Basic attempt of evaluation

Known Issues:
- Evaluation is quite possibly not statistically any better than playing random moves.
It drew 9 out of 10 games against RandomEngine V1.0.

Goals:
- Make evaluation statistically better than playing random moves.