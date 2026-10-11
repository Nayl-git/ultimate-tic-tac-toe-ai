#ifndef PLATEAU_H
#define PLATEAU_H

#include "main.h"
#include "Morpion.h"

#include <array>
#include <vector>


class Plateau {
private:
    std::array<std::array<Morpion, TAILLE>, TAILLE> grille; // 9 morpions composant le plateau
    std::array<std::array<EtatPartie, TAILLE>, TAILLE> resultat = {};
    std::vector<GameMove> historique;

public:
    Plateau() = default;
    void jouerCoup(int ligneMorpion, int colonneMorpion, int ligneCel, int colonneCel, EtatCase joueur);
    bool caseVide(int ligneMorpion, int colonneMorpion, int ligneCel, int colonneCel) {
        return (grille[ligneMorpion][colonneMorpion].getValeurCellule(ligneCel, colonneCel) == EtatCase::Vide);
    }

    void appliquerCoup(GameMove coup, EtatCase joueur);
    void annulerDernierCoup();

    std::vector<GameMove> getCoupValide();
    EtatPartie getGagnantGlobal() const;
    EtatPartie getStatutSousMorpion(int ligne, int colonne) const { return resultat[ligne][colonne]; }
};


#endif
