#pragma once
// G�n�ration de nombres al�atoires


// Retourne un nombre entier choisi al�atoirement
int aleatoire();


// Retourne un nombre entier choisi al�atoirement dans l'intervalle [min, max)
// ATTENTION: min doit �tre plus petit que max, sinon le programme va planter
int aleatoire_dans_intervalle(int min, int max);
