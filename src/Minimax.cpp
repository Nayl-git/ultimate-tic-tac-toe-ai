#include "Minimax.h"
#include <algorithm> // Pour std::sort, std::max et std::min
#include <iostream>

//---------- Heuristique ---------------
int Minimax::obtenirScorePriorite(const GameMove& coup) {
    int ligneCel = coup.row % TAILLE;
    int colonneCel = coup.col % TAILLE;

    // Priorité 1 : Le centre du sous-morpion
    if (ligneCel == 1 && colonneCel == 1) return 3;

    // Priorité 2 : Les coins
    if ((ligneCel == 0 || ligneCel == 2) && (colonneCel == 0 || colonneCel == 2)) return 2;

    // Priorité 3 : Les bords (croix)
    return 1;
}

int Minimax::evaluerPlateau(const Plateau& plateau, EtatCase monJoueur) {
    // L'Heuristique : Donne une note au plateau actuel
    int score = 0;
    EtatCase adversaire = (monJoueur == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;

    for (int i = 0; i < TAILLE; ++i) {
        for (int j = 0; j < TAILLE; ++j) {
            EtatPartie statut = plateau.getStatutSousMorpion(i, j);

            if (statut == static_cast<EtatPartie>(monJoueur)) {
                score += 10;
                if (i == 1 && j == 1) score += 5; // Bonus stratégique pour le centre
            } else if (statut == static_cast<EtatPartie>(adversaire)) {
                score -= 10;
                if (i == 1 && j == 1) score -= 5; // Malus pour le centre
            }
        }
    }
    return score;
}

//---------- Minimax avec Alpha-Beta -----------
int Minimax::minimax(Plateau& plateau, int profondeur, int alpha, int beta, bool isMaximizing, EtatCase monJoueur) {
    //fonction récursive Minimax avec Alpha-Beta
    EtatPartie statutGlobal = plateau.getGagnantGlobal();
    EtatCase adversaire = (monJoueur == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;

    // Conditions d'arrêt : Victoire, Défaite, Nul, ou limite de profondeur atteinte
    if (statutGlobal == static_cast<EtatPartie>(monJoueur)) return 10000 + profondeur; // ajout profondeur --> IA AlphaDestroyer va preferer une victoire rapide
    if (statutGlobal == static_cast<EtatPartie>(adversaire)) return -10000 - profondeur;// soustraction profondeur --> IA AlphaDestroyer va preferer une défaite longue
    if (statutGlobal == EtatPartie::Egalite) return 0;
    if (profondeur == 0) return evaluerPlateau(plateau, monJoueur);

    std::vector<GameMove> coups = plateau.getCoupValide();
    if (coups.empty()) return 0;

    // 1. Tri des coups (Move Ordering)
    std::sort(coups.begin(), coups.end(), [](const GameMove& a, const GameMove& b) {
        return obtenirScorePriorite(a) > obtenirScorePriorite(b);
    });

    // 2. Récupération des résultats
    if (isMaximizing) {
        int meilleurScore = -9999999;
        for (const GameMove& coup : coups) {
            plateau.appliquerCoup(coup, monJoueur);
            int score = minimax(plateau, profondeur - 1, alpha, beta, false, monJoueur);
            plateau.annulerDernierCoup(); // Restauraton état initial

            meilleurScore = std::max(meilleurScore, score);
            alpha = std::max(alpha, score);
            if (beta <= alpha) break; // Élagage Beta : l'adversaire a une meilleure option ailleurs, on coupe.
        }
        return meilleurScore;
    } else {
        int pireScore = 9999999;
        for (const GameMove& coup : coups) {
            plateau.appliquerCoup(coup, adversaire);
            int score = minimax(plateau, profondeur - 1, alpha, beta, true, monJoueur);
            plateau.annulerDernierCoup();

            pireScore = std::min(pireScore, score);
            beta = std::min(beta, score);
            if (beta <= alpha) break; // Élagage Alpha : on a une meilleure option ailleurs, on coupe.
        }
        return pireScore;
    }
}

//-------------- Multithreading ---------------
GameMove Minimax::getMeilleurCoup(Plateau& plateau, int profondeurMax, EtatCase monJoueur) {
    std::vector<GameMove> coups = plateau.getCoupValide();
    if (coups.empty()) return {-1, -1};
    if (coups.size() == 1) return coups[0]; // Coup forcé

    // 1. Tri des coups (Move Ordering)
    std::sort(coups.begin(), coups.end(), [](const GameMove& a, const GameMove& b) {
        return obtenirScorePriorite(a) > obtenirScorePriorite(b);
    });

    // Vecteur pour stocker les promesses de résultats (Score, Coup)
    std::vector<std::future<std::pair<int, GameMove>>> resultatsFutures;


    // 2. Lancement des threads
    for (const GameMove& coup : coups) {
        // std::async lance la recherche sur un thread séparé, plateau --> (copie locale)
        resultatsFutures.push_back(std::async(std::launch::async, [coup, plateau, profondeurMax, monJoueur]() {
            int alpha = -9999999;
            int beta = 9999999;

            Plateau copieLocale = plateau; // 'copieLocale' unique à ce thread.
            copieLocale.appliquerCoup(coup, monJoueur);

            // On lance le Minimax sur cette branche
            int score = minimax(copieLocale, profondeurMax - 1, alpha, beta, false, monJoueur);

            return std::make_pair(score, coup);
        }));
    }

    int meilleurScore = -9999999;
    GameMove meilleurCoup = coups[0];

    // 3. Récupération des résultats
    for (size_t i = 0; i < resultatsFutures.size(); ++i) {
        // .get() bloque l'exécution jusqu'à ce que le thread ait fini son calcul
        std::pair<int, GameMove> resultat = resultatsFutures[i].get();
        int scoreDuCoup = resultat.first;
        GameMove coupEvalue = resultat.second;

        if (scoreDuCoup > meilleurScore) {
            meilleurScore = scoreDuCoup;
            meilleurCoup = coupEvalue;
        }
    }

    std::cerr << "--- Minimax (d=" << profondeurMax << ") Score: " << meilleurScore << std::endl;
    return meilleurCoup;
}
