// Génération de nombres aléatoires

#include <random>
#include "Aleatoire.h"


std::mt19937& generateur_aleatoire()
{
   static std::random_device rd;
   static std::mt19937 mt(rd());
   return mt;
}


int aleatoire()
{
   return generateur_aleatoire()();
}


int aleatoire_dans_intervalle(int min, int max)
{
   std::uniform_int_distribution<int> distribution(min, max - 1);
   return distribution(generateur_aleatoire());
}
