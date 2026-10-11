#ifndef MONTECARLO_H
#define MONTECARLO_H

#include "Plateau.h"
#include "main.h"

class MonteCarlo {
public:
    // Évalue tous les coups possibles et retourne le plus prometteur
    static GameMove getMeilleurCoup(const Plateau& plateauActuel, EtatCase monJoueur, int nbSimulations);

private:
    // Joue des coups aléatoires jusqu'à la fin du jeu
    static EtatPartie simulerPartieAleatoire(Plateau plateauCopie, EtatCase joueurCourant);
};

#endif
