#include <iostream>
#include "main.h"
#include <vector>
#include "Plateau.h"
#include "Minimax.h"
#include "MonteCarlo.h"
#include <cstdlib>
#include <ctime>

int main()
{
    // Game initialization
    game.initialize(10, Level::MEDIUM_1, Mode::ARENA, false, "AlphaDestroyer");
    std::srand(std::time(nullptr));

    while (!game.isAllGameFinish())
    {
        Plateau plateau;

        while (!game.isFinish())
        {
            // Récupérer coup IA (Adversaire)
            GameMove gameMove;
            game.getMove(gameMove);
            std::cerr << "IA move " << gameMove.row << " " << gameMove.col << std::endl;

            // Application du coup adverse (Joueur 2)
            if(gameMove.row >= 0 && gameMove.col >= 0) plateau.appliquerCoup(gameMove, EtatCase::J2);

            // Sécurité : Vérifier si la partie est déjà terminée globalement
            if(plateau.getGagnantGlobal() != EtatPartie::EnCours) break;

            // Déclenchement de l'IA (Joueur 1)
            std::vector<GameMove> coupsValide = plateau.getCoupValide();
            GameMove myMove = {-1, -1};

            if(!coupsValide.empty()){
                myMove = MonteCarlo::getMeilleurCoup(plateau, EtatCase::J1, 2600);
//                myMove = Minimax::getMeilleurCoup(plateau, 6, EtatCase::J1);
                plateau.appliquerCoup(myMove, EtatCase::J1);
            }

            std::cerr << "Send move " << myMove.row << " " << myMove.col << std::endl;
            game.setMove(myMove);
        }
    }

    return 0;
}
