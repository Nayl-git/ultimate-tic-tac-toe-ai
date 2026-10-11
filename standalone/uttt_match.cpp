// uttt_match: standalone match runner for the Ultimate Tic-Tac-Toe engines.
//
// ADDED FOR THE PUBLIC VERSION OF THE REPOSITORY. This file is not part of the
// original school project: the original entry point (arena/main.cpp) only runs
// inside the school's arena library, which is not distributed here. This runner
// lets anyone play the team's engines (src/) against each other or against a
// uniform-random player, using the team's own Plateau/Morpion classes for the
// rules. The engines themselves are used unchanged.
//
// Usage:
//   uttt_match --a montecarlo --b random --games 200 --sims 2600 --depth 4 --seed 1
//
// Engines: montecarlo | minimax | random
// Players alternate who moves first: engine A starts games 1, 3, 5, ...
// --opening N plays the first N plies of every game uniformly at random (seeded),
// which gives varied games between two deterministic engines.

#include "Minimax.h"
#include "MonteCarlo.h"
#include "Plateau.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>

namespace {

// The team's Minimax prints a debug line to std::cerr on every move; we silence
// std::cerr while an engine is thinking so that the match output stays readable.
class NullBuffer : public std::streambuf {
protected:
    int overflow(int c) override { return c; }
};

struct Options {
    std::string a = "montecarlo";
    std::string b = "random";
    int games = 100;
    int sims = 2600;
    int depth = 4;
    unsigned seed = 1;
    int opening = 0;
    bool verbose = false;
};

struct EngineStats {
    long long moves = 0;
    double totalMs = 0.0;
    double maxMs = 0.0;
};

class Engine {
public:
    Engine(std::string kind, const Options& opt, std::mt19937& rng)
        : kind_(std::move(kind)), sims_(opt.sims), depth_(opt.depth), rng_(rng) {}

    std::string label() const {
        if (kind_ == "montecarlo") return "montecarlo(sims=" + std::to_string(sims_) + ")";
        if (kind_ == "minimax") return "minimax(depth=" + std::to_string(depth_) + ")";
        return kind_;
    }

    GameMove chooseMove(Plateau& board, EtatCase me) {
        if (kind_ == "montecarlo") return MonteCarlo::getMeilleurCoup(board, me, sims_);
        if (kind_ == "minimax") return Minimax::getMeilleurCoup(board, depth_, me);
        std::vector<GameMove> moves = board.getCoupValide();
        if (moves.empty()) return {-1, -1};
        std::uniform_int_distribution<std::size_t> pick(0, moves.size() - 1);
        return moves[pick(rng_)];
    }

    EngineStats stats;

private:
    std::string kind_;
    int sims_;
    int depth_;
    std::mt19937& rng_;
};

bool isKnownEngine(const std::string& kind) {
    return kind == "montecarlo" || kind == "minimax" || kind == "random";
}

void printUsage() {
    std::cout << "Usage: uttt_match [--a ENGINE] [--b ENGINE] [--games N] [--sims N]\n"
                 "                  [--depth N] [--seed N] [--opening N] [--verbose]\n"
                 "  ENGINE: montecarlo | minimax | random   (defaults: --a montecarlo --b random)\n"
                 "  --games   number of games, players alternate who moves first (default 100)\n"
                 "  --sims    Monte Carlo playout budget per decision (default 2600, as in the arena)\n"
                 "  --depth   Minimax search depth (default 4)\n"
                 "  --seed    seed for the random player, random openings and std::srand (default 1)\n"
                 "  --opening play the first N plies of each game at random (default 0)\n"
                 "  --verbose print every game and its final board\n";
}

bool parseArgs(int argc, char** argv, Options& opt) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto next = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::cout << "Missing value for " << name << "\n";
                return nullptr;
            }
            return argv[++i];
        };
        const char* v = nullptr;
        if (arg == "--help" || arg == "-h") {
            printUsage();
            std::exit(0);
        } else if (arg == "--verbose") {
            opt.verbose = true;
        } else if (arg == "--a") {
            if (!(v = next("--a"))) return false;
            opt.a = v;
        } else if (arg == "--b") {
            if (!(v = next("--b"))) return false;
            opt.b = v;
        } else if (arg == "--games") {
            if (!(v = next("--games"))) return false;
            opt.games = std::atoi(v);
        } else if (arg == "--sims") {
            if (!(v = next("--sims"))) return false;
            opt.sims = std::atoi(v);
        } else if (arg == "--depth") {
            if (!(v = next("--depth"))) return false;
            opt.depth = std::atoi(v);
        } else if (arg == "--opening") {
            if (!(v = next("--opening"))) return false;
            opt.opening = std::atoi(v);
        } else if (arg == "--seed") {
            if (!(v = next("--seed"))) return false;
            opt.seed = static_cast<unsigned>(std::strtoul(v, nullptr, 10));
        } else {
            std::cout << "Unknown argument: " << arg << "\n";
            return false;
        }
    }
    if (!isKnownEngine(opt.a) || !isKnownEngine(opt.b)) {
        std::cout << "Unknown engine (expected montecarlo, minimax or random)\n";
        return false;
    }
    if (opt.games <= 0 || opt.sims <= 0 || opt.depth <= 0 || opt.opening < 0) {
        std::cout << "--games, --sims and --depth must be positive, --opening non-negative\n";
        return false;
    }
    return true;
}

// Prints the 9x9 grid (X = first player, O = second player) with the small-board
// results underneath.
void printBoard(const std::vector<std::vector<char>>& cells, const Plateau& board) {
    for (int r = 0; r < 9; ++r) {
        if (r % 3 == 0 && r != 0) std::cout << "    ------+-------+------\n";
        std::cout << "    ";
        for (int c = 0; c < 9; ++c) {
            if (c % 3 == 0 && c != 0) std::cout << "| ";
            std::cout << cells[r][c] << ' ';
        }
        std::cout << "\n";
    }
    std::cout << "    small boards: ";
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            EtatPartie s = board.getStatutSousMorpion(i, j);
            std::cout << (s == EtatPartie::VictoireJ1 ? 'X' : s == EtatPartie::VictoireJ2 ? 'O' : s == EtatPartie::Egalite ? '=' : '.');
        }
        if (i < 2) std::cout << '/';
    }
    std::cout << "\n";
}

// Plays one game. `first` moves as J1, `second` as J2. The first `opening` plies
// are random moves (not timed, not attributed to an engine). Returns the final
// state, or exits with an error if an engine plays an illegal move.
EtatPartie playGame(Engine& first, Engine& second, int opening, std::mt19937& rng, bool verbose) {
    Plateau board;
    std::vector<std::vector<char>> cells(9, std::vector<char>(9, '.'));
    EtatCase toMove = EtatCase::J1;
    NullBuffer nullBuffer;
    int ply = 0;

    while (board.getGagnantGlobal() == EtatPartie::EnCours) {
        if (ply < opening) {
            std::vector<GameMove> legal = board.getCoupValide();
            std::uniform_int_distribution<std::size_t> pick(0, legal.size() - 1);
            GameMove move = legal[pick(rng)];
            board.appliquerCoup(move, toMove);
            cells[move.row][move.col] = (toMove == EtatCase::J1) ? 'X' : 'O';
            toMove = (toMove == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;
            ++ply;
            continue;
        }
        Engine& engine = (toMove == EtatCase::J1) ? first : second;
        std::vector<GameMove> legal = board.getCoupValide();

        std::streambuf* saved = std::cerr.rdbuf(&nullBuffer);
        auto start = std::chrono::steady_clock::now();
        GameMove move = engine.chooseMove(board, toMove);
        auto end = std::chrono::steady_clock::now();
        std::cerr.rdbuf(saved);

        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        engine.stats.moves++;
        engine.stats.totalMs += ms;
        engine.stats.maxMs = std::max(engine.stats.maxMs, ms);

        bool isLegal = std::any_of(legal.begin(), legal.end(), [&](const GameMove& m) {
            return m.row == move.row && m.col == move.col;
        });
        if (!isLegal) {
            std::cout << "ERROR: " << engine.label() << " played an illegal move (" << move.row << ", "
                      << move.col << ")\n";
            std::exit(2);
        }

        board.appliquerCoup(move, toMove);
        cells[move.row][move.col] = (toMove == EtatCase::J1) ? 'X' : 'O';
        toMove = (toMove == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;
        ++ply;
    }

    if (verbose) printBoard(cells, board);
    return board.getGagnantGlobal();
}

std::string percent(int n, int total) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << (100.0 * n / total) << "%";
    return out.str();
}

}  // namespace

int main(int argc, char** argv) {
    Options opt;
    if (!parseArgs(argc, argv, opt)) {
        printUsage();
        return 1;
    }

    // The random player uses its own generator. std::srand also seeds the C
    // generator used by the Monte Carlo playouts on platforms where rand() state
    // is shared between threads (glibc); on Windows each thread has its own
    // rand() state, so the playouts there do not depend on --seed.
    std::mt19937 rng(opt.seed);
    std::srand(opt.seed);

    Engine a(opt.a, opt, rng);
    Engine b(opt.b, opt, rng);
    const std::string labelA = "A = " + a.label();
    const std::string labelB = "B = " + b.label();

    std::cout << "Ultimate Tic-Tac-Toe match\n"
              << "  " << labelA << "\n"
              << "  " << labelB << "\n"
              << "  games=" << opt.games << " seed=" << opt.seed << " random-opening-plies=" << opt.opening
              << "\n  (A moves first in odd-numbered games, B in even-numbered games)\n\n";

    int wins = 0, draws = 0, losses = 0;
    int winsFirst = 0, drawsFirst = 0, lossesFirst = 0;
    auto matchStart = std::chrono::steady_clock::now();

    for (int g = 1; g <= opt.games; ++g) {
        bool aFirst = (g % 2 == 1);
        Engine& first = aFirst ? a : b;
        Engine& second = aFirst ? b : a;

        if (opt.verbose) std::cout << "Game " << g << " (" << (aFirst ? "A" : "B") << " = X moves first)\n";
        EtatPartie result = playGame(first, second, opt.opening, rng, opt.verbose);

        // Translate the result to A's point of view.
        EtatPartie aWins = aFirst ? EtatPartie::VictoireJ1 : EtatPartie::VictoireJ2;
        EtatPartie bWins = aFirst ? EtatPartie::VictoireJ2 : EtatPartie::VictoireJ1;
        const char* outcome = "draw";
        if (result == aWins) {
            wins++;
            if (aFirst) winsFirst++;
            outcome = "A wins";
        } else if (result == bWins) {
            losses++;
            if (aFirst) lossesFirst++;
            outcome = "B wins";
        } else {
            draws++;
            if (aFirst) drawsFirst++;
        }
        if (opt.verbose) std::cout << "  -> " << outcome << "\n\n";
    }

    double totalSeconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - matchStart).count();
    int gamesFirst = (opt.games + 1) / 2;
    int gamesSecond = opt.games / 2;

    std::cout << "Results from A's point of view (" << opt.games << " games)\n"
              << "  wins:   " << std::setw(5) << wins << "  (" << percent(wins, opt.games) << ")\n"
              << "  draws:  " << std::setw(5) << draws << "  (" << percent(draws, opt.games) << ")\n"
              << "  losses: " << std::setw(5) << losses << "  (" << percent(losses, opt.games) << ")\n"
              << "  A moving first  (" << gamesFirst << " games): W/D/L = " << winsFirst << "/" << drawsFirst
              << "/" << lossesFirst << "\n"
              << "  A moving second (" << gamesSecond << " games): W/D/L = " << (wins - winsFirst) << "/"
              << (draws - drawsFirst) << "/" << (losses - lossesFirst) << "\n\n";

    std::cout << std::fixed << std::setprecision(2) << "Time per move\n";
    for (Engine* e : {&a, &b}) {
        const std::string& label = (e == &a) ? labelA : labelB;
        double avg = e->stats.moves ? e->stats.totalMs / static_cast<double>(e->stats.moves) : 0.0;
        std::cout << "  " << label << ": avg " << avg << " ms, max " << e->stats.maxMs << " ms over "
                  << e->stats.moves << " moves\n";
    }
    std::cout << std::setprecision(1) << "Total wall time: " << totalSeconds << " s\n";
    return 0;
}
