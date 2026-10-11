#ifndef MORPION_H
#define MORPION_H

#include "main.h"

#include <array>
#include <vector>
#include <utility> // bibliothèque pour std::pair

// Définition des états possibles
enum class EtatCase { Vide = 0, J1 = 1, J2 = 2 };
enum class EtatPartie { EnCours = 0, VictoireJ1 = 1, VictoireJ2 = 2, Egalite = 3 };


class Morpion {
private:
    std::array<std::array<EtatCase,TAILLE>, TAILLE> cel = {}; // 9 cases composant le morpion
//    int gagnant;
    EtatPartie statut = EtatPartie::EnCours;

public:
    Morpion() = default;

    EtatPartie getStatut() const { return statut; };
    EtatCase getValeurCellule(int ligne, int colonne) const { return cel[ligne][colonne]; };
    void setValeurCellule(int ligne, int colonne, EtatCase val) { cel[ligne][colonne] = val; } ;

    void verifierVictoire();
    std::vector<std::pair<int, int>> getCasesDispo() const; // positions(x,y) libres du morpion

    void annulerCoup(int ligne, int colonne) {
        cel[ligne][colonne] = EtatCase::Vide;
        statut = EtatPartie::EnCours; // On réinitialise par défaut
        verifierVictoire();           // et on revérifie pour mettre à jour l'état
    }
};


#endif
