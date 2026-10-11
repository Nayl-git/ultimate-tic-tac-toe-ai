#include "Plateau.h"
#include <iostream>


void Plateau::jouerCoup(int ligneMorpion, int colonneMorpion, int ligneCel, int colonneCel, EtatCase joueur){
    if (!caseVide(ligneMorpion, colonneMorpion, ligneCel, colonneCel)) return;

    grille[ligneMorpion][colonneMorpion].setValeurCellule(ligneCel, colonneCel, joueur);
    grille[ligneMorpion][colonneMorpion].verifierVictoire();
    resultat[ligneMorpion][colonneMorpion] = grille[ligneMorpion][colonneMorpion].getStatut();
}

void Plateau::appliquerCoup(GameMove coup, EtatCase joueur){
    int ligneMorpion = coup.row / TAILLE; // nb : comme le type est int, l'opérateur fait la division Euclidienne il me semble
    int colonneMorpion = coup.col / TAILLE;
    int ligneCel = coup.row % TAILLE;
    int colonneCel = coup.col % TAILLE;

    jouerCoup(ligneMorpion, colonneMorpion, ligneCel, colonneCel, joueur);
    historique.push_back(coup);
}

void Plateau::annulerDernierCoup() {
    if (historique.empty()) return;

    GameMove dernier = historique.back();
    historique.pop_back();

    int lM = dernier.row / TAILLE;
    int cM = dernier.col / TAILLE;
    int lC = dernier.row % TAILLE;
    int cC = dernier.col % TAILLE;

    grille[lM][cM].annulerCoup(lC, cC); // Rétablit le sous-morpion
    resultat[lM][cM] = grille[lM][cM].getStatut(); // Met à jour la macro-grille
}

std::vector<GameMove> Plateau::getCoupValide(){
    std::vector<GameMove> coupValide;

    // Si aucun coup n'a été joué, on peut jouer partout
    if(historique.empty()) {
        for(int i = 0; i < TAILLE; i++) {
            for(int j = 0; j < TAILLE; j++) {
                std::vector<std::pair<int, int>> casesDispo = grille[i][j].getCasesDispo();
                for(const auto& coord : casesDispo) {
                    coupValide.push_back({(i * TAILLE) + coord.first, (j * TAILLE) + coord.second});
                }
            }
        }
        return coupValide;
    }

    // Sinon, on regarde le dernier coup de l'historique
    GameMove dernierCoup = historique.back();
    int ligneMorpion = dernierCoup.row % TAILLE;
    int colonneMorpion = dernierCoup.col % TAILLE;

    if(resultat[ligneMorpion][colonneMorpion] != EtatPartie::EnCours){
        for(int i=0; i<TAILLE; i++){
            for(int j=0; j<TAILLE; j++){
                if(resultat[i][j] == EtatPartie::EnCours){
                    std::vector<std::pair<int, int>> casesDispo = grille[i][j].getCasesDispo();
                    for(std::pair<int,int>& coord : casesDispo){
                        GameMove coup;
                        coup.row = (i*TAILLE) + coord.first;
                        coup.col = (j*TAILLE) + coord.second;
                        coupValide.push_back(coup);
                    }
                }
            }
        }
    }else{
        std::vector<std::pair<int, int>> casesDispo = grille[ligneMorpion][colonneMorpion].getCasesDispo();
        for(std::pair<int,int>& coord : casesDispo){
            GameMove coup;
            coup.row = (ligneMorpion*TAILLE) + coord.first;
            coup.col = (colonneMorpion*TAILLE) + coord.second;
            coupValide.push_back(coup);
        }
    }

    return coupValide;
}

EtatPartie Plateau::getGagnantGlobal() const {
    for (int i = 0; i < TAILLE; ++i) {
        // Vérification des Lignes (on ignore les 0=vide et les 3=match nul)
        if (resultat[i][0] != EtatPartie::EnCours && resultat[i][0] != EtatPartie::Egalite && resultat[i][0] == resultat[i][1] && resultat[i][1] == resultat[i][2])
            return resultat[i][0];
        // Vérification des Colonnes
        if (resultat[0][i] != EtatPartie::EnCours && resultat[0][i] != EtatPartie::Egalite && resultat[0][i] == resultat[1][i] && resultat[1][i] == resultat[2][i])
            return resultat[0][i];
    }

    // Vérification des Diagonales
    if (resultat[0][0] != EtatPartie::EnCours && resultat[0][0] != EtatPartie::Egalite && resultat[0][0] == resultat[1][1] && resultat[1][1] == resultat[2][2])
        return resultat[0][0];
    if (resultat[0][2] != EtatPartie::EnCours && resultat[0][2] != EtatPartie::Egalite && resultat[0][2] == resultat[1][1] && resultat[1][1] == resultat[2][0])
        return resultat[0][2];

    // Vérification de match nul global (si toutes les cases de 'resultat' sont remplies)
    bool estPlein = true;
    for (int i = 0; i < TAILLE; ++i) {
        for (int j = 0; j < TAILLE; ++j) {
            if (resultat[i][j] == EtatPartie::EnCours) estPlein = false;
        }
    }

    if (estPlein) return EtatPartie::Egalite; // Code pour Match Nul

    return EtatPartie::EnCours; // Partie toujours en cours
}
