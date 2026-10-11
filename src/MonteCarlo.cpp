#include "MonteCarlo.h"

#include <cstdlib>
#include <vector>
#include <future> //Pour std::future

EtatPartie MonteCarlo::simulerPartieAleatoire(Plateau plateauCopie, EtatCase joueurCourant) {
    EtatPartie gagnant = plateauCopie.getGagnantGlobal();

    // Tant que le jeu n'est pas terminé
    while (gagnant == EtatPartie::EnCours) {
        std::vector<GameMove> coups = plateauCopie.getCoupValide();
        if (coups.empty()) return EtatPartie::Egalite; // Sécurité : Match nul si la partie est bloquée

        EtatCase adversaire = (joueurCourant == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;
        GameMove coupChoisi = {-1, -1};

        // 1. Chercher un coup de victoire immédiate
        for (const GameMove& coup : coups) {
            plateauCopie.appliquerCoup(coup, joueurCourant);

            if (plateauCopie.getGagnantGlobal() == static_cast<EtatPartie>(joueurCourant)) {
                coupChoisi = coup;
                plateauCopie.annulerDernierCoup(); // On nettoie avant de sortir
                break; // On a trouvé la victoire, on arrête de chercher
            }
            plateauCopie.annulerDernierCoup(); // Ce n'est pas gagnant, on annule et on teste le coup suivant
        }

        // 2. Si pas de victoire, chercher un coup de blocage immédiat
        if (coupChoisi.row == -1) {
            for (const GameMove& coup : coups) {
                plateauCopie.appliquerCoup(coup, adversaire);

                // On simule un coup de l'adversaire sur cette case
                if (plateauCopie.getGagnantGlobal() == static_cast<EtatPartie>(adversaire)) {
                    coupChoisi = coup;
                    plateauCopie.annulerDernierCoup();
                    break; // On a trouvé la menace, on va jouer ici pour bloquer
                }
                plateauCopie.annulerDernierCoup();
            }
        }

        // 3. Si aucun coup critique, jouer un coup aléatoire
        if (coupChoisi.row == -1) {
            int index = std::rand() % coups.size();
            coupChoisi = coups[index];
        }

        // Appliquer le coup sélectionné
        plateauCopie.appliquerCoup(coupChoisi, joueurCourant);

        // Alternance des joueurs
        joueurCourant = adversaire;
        gagnant = plateauCopie.getGagnantGlobal();
    }

    return gagnant;
}

GameMove MonteCarlo::getMeilleurCoup(const Plateau& plateauActuel, EtatCase monJoueur, int nbSimulations) {
    std::vector<GameMove> coups = (const_cast<Plateau&>(plateauActuel)).getCoupValide();

    if (coups.empty()) return {-1, -1};
    if (coups.size() == 1) return coups[0]; // Mouvement forcé, pas besoin de calculer

    int simulationsParCoup = ( nbSimulations == 0 ) ? 1 :  nbSimulations / coups.size();

    // Vecteur pour stocker les "promesses" de résultats asynchrones
    std::vector<std::future<int>> resultatsFutures;

    // 1. Lancement des threads en parallèle pour chaque coup possible
    for (const GameMove& coup : coups) {
        // std::async lance la fonction lambda (anonyme) sur un thread séparé
        resultatsFutures.push_back(std::async(std::launch::async, [coup, plateauActuel, monJoueur, simulationsParCoup]() {
            int scoreCoup = 0;
            EtatCase adversaire = (monJoueur == EtatCase::J1) ? EtatCase::J2 : EtatCase::J1;

            // Ce bloc tourne en parallèle pour chaque coup
            for (int i = 0; i < simulationsParCoup; ++i) {
                Plateau copie = plateauActuel;
                copie.appliquerCoup(coup, monJoueur);

                EtatPartie gagnantSim = simulerPartieAleatoire(copie, adversaire);

                if (gagnantSim == static_cast<EtatPartie>(monJoueur)) {
                    scoreCoup += 2;
                } else if (gagnantSim == EtatPartie::Egalite) {
                    scoreCoup += 1; // Nul
                }
            }
            return scoreCoup;
        }));
    }

    // 2. Récupération des résultats et sélection du meilleur coup
    int meilleurScore = -1;
    GameMove meilleurCoup = coups[0];

    for (size_t i = 0; i < coups.size(); ++i) {
        // La méthode .get() bloque l'exécution jusqu'à ce que le thread ait terminé son calcul
        int scoreCoup = resultatsFutures[i].get();

        if (scoreCoup > meilleurScore) {
            meilleurScore = scoreCoup;
            meilleurCoup = coups[i];
        }
    }

    return meilleurCoup;
}
