# Arena entry point (original)

`main.cpp` is the team's original program for the school arena, kept as it was (only converted from
Latin-1 to UTF-8). It declares itself as `AlphaDestroyer`, plays a series of games against the arena
AI at the `MEDIUM_1` level and answers each arena move with
`MonteCarlo::getMeilleurCoup(plateau, EtatCase::J1, 2600)`. The Minimax alternative (depth 6) is left
commented out.

It does not build on its own: the arena interface it uses (`game`, levels, modes) comes from the
school's template header and arena library (`libUTTTLib.a`, which also needs the Allegro 5 DLLs).
They belong to the school and are **not included** in this repository.

To build and run the engines without them, use the standalone match runner in
[`../standalone`](../standalone) (see the main [README](../README.md)).
