#ifndef MINIMAX_H_INCLUDED
#define MINIMAX_H_INCLUDED

#include "Plateau.h"
#include "main.h"

#include <future> //Pour std::async et std::future
#include <utility> // Pour std::pair

class Minimax {
public:
    // profondeurMax définit combien de coups à l'avance l'IA calcule
    static GameMove getMeilleurCoup(Plateau& plateau, int profondeurMax, EtatCase monJoueur);

private:
    // Fonction récursive avec élagage Alpha-Beta
    static int minimax(Plateau& plateau, int profondeur, int alpha, int beta, bool isMaximizing, EtatCase monJoueur);

    // Fonction d'évaluation (Heuristique)
    static int evaluerPlateau(const Plateau& plateau, EtatCase monJoueur);
    static int obtenirScorePriorite(const GameMove& coup);
};

#endif
