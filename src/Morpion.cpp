#include "Morpion.h"
#include <iostream>
#include <array>


void Morpion::verifierVictoire() {
    if (statut != EtatPartie::EnCours) return;

    for (int i = 0; i < TAILLE; ++i) {
        // verif ligne
        if (cel[i][0] != EtatCase::Vide && cel[i][0] == cel[i][1] && cel[i][1] == cel[i][2]) {
            statut = static_cast<EtatPartie>(cel[i][0]);
            return;
        }
        // verif colonne
        if (cel[0][i] != EtatCase::Vide && cel[0][i] == cel[1][i] && cel[1][i] == cel[2][i]) {
            statut = static_cast<EtatPartie>(cel[0][i]);
            return;
        }
    }

    // verif diagonales
    if (cel[0][0] != EtatCase::Vide && cel[0][0] == cel[1][1] && cel[1][1] == cel[2][2]) {
        statut = static_cast<EtatPartie>(cel[0][0]);
        return;
    }
    if (cel[0][2] != EtatCase::Vide && cel[0][2] == cel[1][1] && cel[1][1] == cel[2][0]) {
        statut = static_cast<EtatPartie>(cel[0][2]);
        return;
    }

    bool estPlein = true;
    for (int i = 0; i < TAILLE; ++i) {
        for (int j = 0; j < TAILLE; ++j) {
            if (cel[i][j] == EtatCase::Vide) { estPlein = false; }
        }
    }
    if (estPlein) { statut = EtatPartie::Egalite; }
}

std::vector<std::pair<int, int>> Morpion::getCasesDispo() const{
    std::vector<std::pair<int, int>> lst;

    for(int i=0;i<TAILLE;i++){
        for(int j=0;j<TAILLE;j++){
            if( cel[i][j] == EtatCase::Vide){ lst.push_back({i,j}); }
        }
    }

    return lst;
}
