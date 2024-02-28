#pragma once
// Génération de nombres aléatoires


// Retourne un nombre entier choisi aléatoirement
int aleatoire();


// Retourne un nombre entier choisi aléatoirement dans l'intervalle [min, max)
// ATTENTION: min doit être plus petit que max, sinon le programme va planter
int aleatoire_dans_intervalle(int min, int max);
