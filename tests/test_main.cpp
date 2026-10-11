// Unit tests for the Ultimate Tic-Tac-Toe rules and engines.
//
// ADDED FOR THE PUBLIC VERSION OF THE REPOSITORY (not part of the original school
// project). No external framework: each CHECK prints the failing expression and
// the test executable returns a non-zero exit code if anything fails.
//
// Coordinates: GameMove{row, col} is a cell of the 9x9 grid. The small board is
// (row / 3, col / 3) and the cell inside it is (row % 3, col % 3).

#include "Minimax.h"
#include "MonteCarlo.h"
#include "Morpion.h"
#include "Plateau.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

int g_checks = 0;
int g_failures = 0;

#define CHECK(expr)                                                                       \
    do {                                                                                  \
        ++g_checks;                                                                       \
        if (!(expr)) {                                                                    \
            ++g_failures;                                                                 \
            std::cout << "  FAILED: " #expr " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        }                                                                                 \
    } while (0)

void runTest(const char* name, void (*test)()) {
    int before = g_failures;
    test();
    std::cout << (g_failures == before ? "[PASS] " : "[FAIL] ") << name << "\n";
}

// ---- helpers ---------------------------------------------------------------

// Plays a cell of small board (bi, bj) without any legality check (test setup only).
void put(Plateau& p, int bi, int bj, int ci, int cj, EtatCase who) {
    p.appliquerCoup(GameMove{bi * 3 + ci, bj * 3 + cj}, who);
}

// Gives small board (bi, bj) to `who` with a top-row line.
void winBoard(Plateau& p, int bi, int bj, EtatCase who) {
    put(p, bi, bj, 0, 0, who);
    put(p, bi, bj, 0, 1, who);
    put(p, bi, bj, 0, 2, who);
}

// Fills small board (bi, bj) with a drawn pattern:  X O X / X O O / O X X
void drawBoard(Plateau& p, int bi, int bj) {
    const EtatCase X = EtatCase::J1, O = EtatCase::J2;
    const EtatCase pattern[3][3] = {{X, O, X}, {X, O, O}, {O, X, X}};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) put(p, bi, bj, i, j, pattern[i][j]);
}

bool contains(const std::vector<GameMove>& moves, GameMove m) {
    return std::any_of(moves.begin(), moves.end(),
                       [&](const GameMove& x) { return x.row == m.row && x.col == m.col; });
}

bool allInBoard(const std::vector<GameMove>& moves, int bi, int bj) {
    return std::all_of(moves.begin(), moves.end(),
                       [&](const GameMove& m) { return m.row / 3 == bi && m.col / 3 == bj; });
}

// Snapshot of everything observable through the public interface of Plateau.
struct Snapshot {
    std::vector<bool> empty;
    std::vector<EtatPartie> small;
    EtatPartie global;
    std::vector<std::pair<int, int>> legal;

    bool operator==(const Snapshot& o) const {
        return empty == o.empty && small == o.small && global == o.global && legal == o.legal;
    }
};

Snapshot snapshot(Plateau& p) {
    Snapshot s;
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) s.empty.push_back(p.caseVide(r / 3, c / 3, r % 3, c % 3));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) s.small.push_back(p.getStatutSousMorpion(i, j));
    s.global = p.getGagnantGlobal();
    for (const GameMove& m : p.getCoupValide()) s.legal.emplace_back(m.row, m.col);
    return s;
}

// ---- small board (Morpion) -------------------------------------------------

EtatPartie smallResult(const std::vector<std::pair<int, int>>& cells, EtatCase who) {
    Morpion m;
    for (auto [i, j] : cells) m.setValeurCellule(i, j, who);
    m.verifierVictoire();
    return m.getStatut();
}

void testSmallBoardRows() {
    for (int r = 0; r < 3; ++r) {
        CHECK(smallResult({{r, 0}, {r, 1}, {r, 2}}, EtatCase::J1) == EtatPartie::VictoireJ1);
        CHECK(smallResult({{r, 0}, {r, 1}, {r, 2}}, EtatCase::J2) == EtatPartie::VictoireJ2);
    }
}

void testSmallBoardColumns() {
    for (int c = 0; c < 3; ++c) {
        CHECK(smallResult({{0, c}, {1, c}, {2, c}}, EtatCase::J1) == EtatPartie::VictoireJ1);
        CHECK(smallResult({{0, c}, {1, c}, {2, c}}, EtatCase::J2) == EtatPartie::VictoireJ2);
    }
}

void testSmallBoardDiagonals() {
    CHECK(smallResult({{0, 0}, {1, 1}, {2, 2}}, EtatCase::J1) == EtatPartie::VictoireJ1);
    CHECK(smallResult({{0, 2}, {1, 1}, {2, 0}}, EtatCase::J2) == EtatPartie::VictoireJ2);
}

void testSmallBoardInProgressAndDraw() {
    CHECK(smallResult({{0, 0}, {0, 1}}, EtatCase::J1) == EtatPartie::EnCours);
    CHECK(smallResult({{0, 0}, {1, 1}, {2, 1}}, EtatCase::J1) == EtatPartie::EnCours);

    Morpion m;
    const EtatCase X = EtatCase::J1, O = EtatCase::J2;
    const EtatCase pattern[3][3] = {{X, O, X}, {X, O, O}, {O, X, X}};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) m.setValeurCellule(i, j, pattern[i][j]);
    m.verifierVictoire();
    CHECK(m.getStatut() == EtatPartie::Egalite);
    CHECK(m.getCasesDispo().empty());
}

void testSmallBoardFreeCellsAndUndo() {
    Morpion m;
    CHECK(m.getCasesDispo().size() == 9);
    m.setValeurCellule(0, 0, EtatCase::J1);
    m.setValeurCellule(0, 1, EtatCase::J1);
    m.setValeurCellule(0, 2, EtatCase::J1);
    m.verifierVictoire();
    CHECK(m.getStatut() == EtatPartie::VictoireJ1);
    CHECK(m.getCasesDispo().size() == 6);
    m.annulerCoup(0, 2);
    CHECK(m.getStatut() == EtatPartie::EnCours);
    CHECK(m.getValeurCellule(0, 2) == EtatCase::Vide);
    CHECK(m.getCasesDispo().size() == 7);
}

// ---- big board (Plateau) ---------------------------------------------------

void testGlobalWinRowsColumnsDiagonals() {
    for (int r = 0; r < 3; ++r) {
        Plateau p;
        for (int c = 0; c < 3; ++c) winBoard(p, r, c, EtatCase::J1);
        CHECK(p.getGagnantGlobal() == EtatPartie::VictoireJ1);
    }
    for (int c = 0; c < 3; ++c) {
        Plateau p;
        for (int r = 0; r < 3; ++r) winBoard(p, r, c, EtatCase::J2);
        CHECK(p.getGagnantGlobal() == EtatPartie::VictoireJ2);
    }
    Plateau d1;
    for (int k = 0; k < 3; ++k) winBoard(d1, k, k, EtatCase::J1);
    CHECK(d1.getGagnantGlobal() == EtatPartie::VictoireJ1);
    Plateau d2;
    for (int k = 0; k < 3; ++k) winBoard(d2, k, 2 - k, EtatCase::J2);
    CHECK(d2.getGagnantGlobal() == EtatPartie::VictoireJ2);
}

void testGlobalInProgressAndDraw() {
    Plateau p;
    CHECK(p.getGagnantGlobal() == EtatPartie::EnCours);
    winBoard(p, 0, 0, EtatCase::J1);
    winBoard(p, 0, 1, EtatCase::J1);
    CHECK(p.getGagnantGlobal() == EtatPartie::EnCours);

    // Drawn small boards never count towards a line.
    Plateau withDraws;
    drawBoard(withDraws, 0, 0);
    drawBoard(withDraws, 0, 1);
    drawBoard(withDraws, 0, 2);
    CHECK(withDraws.getGagnantGlobal() == EtatPartie::EnCours);

    // All nine small boards finished without a line of three: global draw.
    //   X O X / X O O / O X X  at the macro level
    const EtatCase X = EtatCase::J1, O = EtatCase::J2;
    const EtatCase pattern[3][3] = {{X, O, X}, {X, O, O}, {O, X, X}};
    Plateau full;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) winBoard(full, i, j, pattern[i][j]);
    CHECK(full.getGagnantGlobal() == EtatPartie::Egalite);
}

void testMoveGenerationCounts() {
    Plateau p;
    CHECK(p.getCoupValide().size() == 81);  // first move: anywhere

    // Centre cell of the centre board sends the opponent back to the centre board.
    p.appliquerCoup({4, 4}, EtatCase::J1);
    std::vector<GameMove> moves = p.getCoupValide();
    CHECK(moves.size() == 8);
    CHECK(allInBoard(moves, 1, 1));
    CHECK(!contains(moves, {4, 4}));

    // Cell (0, 2) of the centre board sends the opponent to the top-right board.
    p.appliquerCoup({3, 5}, EtatCase::J2);
    moves = p.getCoupValide();
    CHECK(moves.size() == 9);
    CHECK(allInBoard(moves, 0, 2));
}

void testSentToBoardRule() {
    // Every cell (ci, cj) of any board sends the next player to board (ci, cj).
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            Plateau p;
            p.appliquerCoup({r, c}, EtatCase::J1);
            std::vector<GameMove> moves = p.getCoupValide();
            int bi = r % 3, bj = c % 3;
            bool sameBoard = (r / 3 == bi && c / 3 == bj);
            CHECK(allInBoard(moves, bi, bj));
            CHECK(moves.size() == (sameBoard ? 8u : 9u));
        }
    }
}

void testFreeChoiceWhenTargetBoardIsFinished() {
    // Board (0, 0) is won by J1; then J1 plays cell (0, 0) of board (1, 1),
    // which would send J2 to the finished board (0, 0): J2 may play anywhere open.
    Plateau won;
    winBoard(won, 0, 0, EtatCase::J1);
    put(won, 1, 1, 0, 0, EtatCase::J1);
    std::vector<GameMove> moves = won.getCoupValide();
    CHECK(moves.size() == 8u * 9u - 1u);  // 8 open boards, one cell taken in board (1, 1)
    CHECK(!contains(moves, {0, 0}) && !contains(moves, {1, 1}) && !contains(moves, {2, 2}));
    for (const GameMove& m : moves) CHECK(!(m.row / 3 == 0 && m.col / 3 == 0));

    // Same with a drawn target board.
    Plateau drawn;
    drawBoard(drawn, 2, 2);
    put(drawn, 0, 0, 2, 2, EtatCase::J2);  // sends to board (2, 2), which is full
    moves = drawn.getCoupValide();
    CHECK(moves.size() == 8u * 9u - 1u);
    for (const GameMove& m : moves) CHECK(!(m.row / 3 == 2 && m.col / 3 == 2));
}

void testApplyUndoConsistency() {
    std::mt19937 rng(42);
    for (int game = 0; game < 50; ++game) {
        Plateau p;
        std::vector<Snapshot> history;
        EtatCase who = EtatCase::J1;
        while (p.getGagnantGlobal() == EtatPartie::EnCours) {
            Snapshot before = snapshot(p);
            std::vector<GameMove> moves = p.getCoupValide();
            CHECK(!moves.empty());
            if (moves.empty()) break;
            // Every legal move must be undoable back to the exact same state.
            for (const GameMove& m : moves) {
                p.appliquerCoup(m, who);
                CHECK(!p.caseVide(m.row / 3, m.col / 3, m.row % 3, m.col % 3));
                p.annulerDernierCoup();
                CHECK(snapshot(p) == before);
            }
            history.push_back(before);
            std::uniform_int_distribution<std::size_t> pick(0, moves.size() - 1);
            p.appliquerCoup(moves[pick(rng)], who);
            who = (who == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;
        }
        // Unwind the whole game: each undo restores the state seen before that move.
        while (!history.empty()) {
            p.annulerDernierCoup();
            CHECK(snapshot(p) == history.back());
            history.pop_back();
        }
        CHECK(p.getCoupValide().size() == 81);
    }
}

void testUndoRestoresWonSmallBoard() {
    Plateau p;
    put(p, 1, 1, 0, 0, EtatCase::J1);
    put(p, 1, 1, 0, 1, EtatCase::J1);
    put(p, 1, 1, 0, 2, EtatCase::J1);
    CHECK(p.getStatutSousMorpion(1, 1) == EtatPartie::VictoireJ1);
    p.annulerDernierCoup();
    CHECK(p.getStatutSousMorpion(1, 1) == EtatPartie::EnCours);
    CHECK(p.caseVide(1, 1, 0, 2));
}

// ---- engines ---------------------------------------------------------------

// J1 owns boards (0,0) and (0,1) and two cells of board (0,2). J1 is to move and
// is sent to board (0,2): cell (0,2) of that board, i.e. GameMove{0, 8}, wins the
// whole game.
Plateau positionWithImmediateWin() {
    Plateau p;
    winBoard(p, 0, 0, EtatCase::J1);
    winBoard(p, 0, 1, EtatCase::J1);
    put(p, 0, 2, 0, 0, EtatCase::J1);
    put(p, 0, 2, 0, 1, EtatCase::J1);
    put(p, 2, 2, 0, 2, EtatCase::J2);  // J2's last move: cell (0,2) sends J1 to board (0,2)
    return p;
}

// J2 owns boards (2,0) and (2,1) and cells (0,0), (0,1) of board (2,2): it wins the
// game by taking cell (0,2) of board (2,2), i.e. GameMove{6, 8}. Every other board
// is finished (drawn), so J1 can only play in board (2,2), and any J1 move other
// than {6, 8} hands J2 either board (2,2) or a free choice: an immediate loss.
Plateau positionWithImmediateThreat() {
    Plateau p;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j) drawBoard(p, i, j);
    winBoard(p, 2, 0, EtatCase::J2);
    winBoard(p, 2, 1, EtatCase::J2);
    put(p, 2, 2, 0, 0, EtatCase::J2);
    put(p, 2, 2, 0, 1, EtatCase::J2);  // last move sends J1 to board (0,1): drawn, so free choice
    return p;
}

void testPositionsAreWhatTheyClaim() {
    Plateau win = positionWithImmediateWin();
    CHECK(win.getGagnantGlobal() == EtatPartie::EnCours);
    std::vector<GameMove> moves = win.getCoupValide();
    CHECK(moves.size() == 7);
    CHECK(allInBoard(moves, 0, 2));

    Plateau threat = positionWithImmediateThreat();
    CHECK(threat.getGagnantGlobal() == EtatPartie::EnCours);
    moves = threat.getCoupValide();
    CHECK(moves.size() == 7);
    CHECK(allInBoard(moves, 2, 2));
    threat.appliquerCoup({6, 8}, EtatCase::J2);
    CHECK(threat.getGagnantGlobal() == EtatPartie::VictoireJ2);
}

void testMinimaxTakesImmediateWin() {
    for (int depth : {1, 2, 4}) {
        Plateau p = positionWithImmediateWin();
        GameMove m = Minimax::getMeilleurCoup(p, depth, EtatCase::J1);
        CHECK(m.row == 0 && m.col == 8);
    }
}

void testMinimaxBlocksImmediateLoss() {
    for (int depth : {2, 3, 4}) {
        Plateau p = positionWithImmediateThreat();
        GameMove m = Minimax::getMeilleurCoup(p, depth, EtatCase::J1);
        CHECK(m.row == 6 && m.col == 8);
    }
}

void testEnginesOnForcedAndFinishedPositions() {
    // Board (0,0) has a single empty cell (2,2) and no winner yet:
    //   X O X / X O O / O X _
    const EtatCase X = EtatCase::J1, O = EtatCase::J2;
    const EtatCase pattern[3][3] = {{X, O, X}, {X, O, O}, {O, X, X}};
    Plateau forced;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (!(i == 2 && j == 2)) put(forced, 0, 0, i, j, pattern[i][j]);
    put(forced, 1, 1, 0, 0, EtatCase::J2);  // sends J1 to board (0,0)
    CHECK(forced.getStatutSousMorpion(0, 0) == EtatPartie::EnCours);
    CHECK(forced.getCoupValide().size() == 1);
    GameMove mm = Minimax::getMeilleurCoup(forced, 4, EtatCase::J1);
    GameMove mc = MonteCarlo::getMeilleurCoup(forced, EtatCase::J1, 2600);
    CHECK(mm.row == 2 && mm.col == 2);
    CHECK(mc.row == 2 && mc.col == 2);

    // Every small board finished: no legal move, both engines return {-1, -1}.
    Plateau finished;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) winBoard(finished, i, j, pattern[i][j]);
    CHECK(finished.getGagnantGlobal() == EtatPartie::Egalite);
    CHECK(finished.getCoupValide().empty());
    mm = Minimax::getMeilleurCoup(finished, 4, EtatCase::J1);
    mc = MonteCarlo::getMeilleurCoup(finished, EtatCase::J1, 2600);
    CHECK(mm.row == -1 && mm.col == -1);
    CHECK(mc.row == -1 && mc.col == -1);
}

void testMonteCarloReturnsLegalMoves() {
    std::srand(7);
    std::mt19937 rng(7);
    for (int game = 0; game < 3; ++game) {
        Plateau p;
        EtatCase who = EtatCase::J1;
        while (p.getGagnantGlobal() == EtatPartie::EnCours) {
            std::vector<GameMove> moves = p.getCoupValide();
            GameMove m;
            if (who == EtatCase::J1) {
                m = MonteCarlo::getMeilleurCoup(p, who, 300);
            } else {
                std::uniform_int_distribution<std::size_t> pick(0, moves.size() - 1);
                m = moves[pick(rng)];
            }
            CHECK(contains(moves, m));
            if (!contains(moves, m)) return;
            p.appliquerCoup(m, who);
            who = (who == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;
        }
    }
}

void testMonteCarloTakesImmediateWin() {
    Plateau p = positionWithImmediateWin();
    GameMove m = MonteCarlo::getMeilleurCoup(p, EtatCase::J1, 2600);
    CHECK(m.row == 0 && m.col == 8);
}

}  // namespace

int main() {
    std::cout << "Ultimate Tic-Tac-Toe unit tests\n";
    runTest("small board: rows", testSmallBoardRows);
    runTest("small board: columns", testSmallBoardColumns);
    runTest("small board: diagonals", testSmallBoardDiagonals);
    runTest("small board: in progress and draw", testSmallBoardInProgressAndDraw);
    runTest("small board: free cells and undo", testSmallBoardFreeCellsAndUndo);
    runTest("global: rows, columns, diagonals", testGlobalWinRowsColumnsDiagonals);
    runTest("global: in progress and draw", testGlobalInProgressAndDraw);
    runTest("moves: generation counts", testMoveGenerationCounts);
    runTest("moves: sent-to-board rule (all 81 cells)", testSentToBoardRule);
    runTest("moves: free choice when the target board is finished", testFreeChoiceWhenTargetBoardIsFinished);
    runTest("apply/undo: 50 random games, every legal move", testApplyUndoConsistency);
    runTest("apply/undo: undo restores a won small board", testUndoRestoresWonSmallBoard);
    runTest("engines: test positions are set up correctly", testPositionsAreWhatTheyClaim);
    runTest("minimax: takes an immediate win (depth 1, 2, 4)", testMinimaxTakesImmediateWin);
    runTest("minimax: blocks an immediate loss (depth 2, 3, 4)", testMinimaxBlocksImmediateLoss);
    runTest("engines: forced move and no legal move", testEnginesOnForcedAndFinishedPositions);
    runTest("monte carlo: returns legal moves for whole games", testMonteCarloReturnsLegalMoves);
    runTest("monte carlo: takes an immediate win", testMonteCarloTakesImmediateWin);

    std::cout << "\n" << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
