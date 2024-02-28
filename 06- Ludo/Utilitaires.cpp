#include <iostream>
#include "Utilitaires.h"
#include <vector>
#include "Aleatoire.h"

using namespace std;

void permuter(int& nombre1, int& nombre2)
{
	int nbAttente = nombre1;
	nombre1 = nombre2;
	nombre2 = nbAttente;
}

void afficher_vecteur_entiers(vector<int>& nombre)
{
	for (int i = 0; i < nombre.size(); i++)
	{
		cout << " [" << i << "] : " << nombre.at(i) << endl;
	}
}

void afficher_vecteur_reels(vector<double>& reel)
{
	for (int i = 0; i < reel.size(); i++)
	{
		cout << " [" << i << "] : " << reel.at(i) << endl;
	}
}

void afficher_vecteur_textes(vector<string>& texte)
{
	for (int i = 0; i < texte.size(); i++)
	{
		cout << " [" << i << "] : " << texte.at(i) << endl;
	}
}

vector<int> remplir_aleatoire(int nbDeValeursAleatoires, int min, int max)
{
	vector<int> val;
	for (int i = 1; i <= nbDeValeursAleatoires; i++)
	{
		val.push_back(aleatoire_dans_intervalle(min, max));
	}
	return val;
}

int index_plus_grand(vector<int> max)
{
	// Au départ, la première valeur est considérée la plus grande
	int index_le_plus_grand = 0;

	// On teste tooutes les autres valeurs
	for (int i = 0; i < max.size(); i++)
	{
		if (max.at(i) > max.at(index_le_plus_grand))
		{
			index_le_plus_grand = i;
		}
	}
	return index_le_plus_grand;
}

void convertir_en_majuscule(string& mot)
{
	for (int i = 0; i < mot.size(); i++)
	{
		mot.at(i) = toupper(mot.at(i));
	}
}


bool ouvrir_fichier_en_lecture(ifstream& canalLecture, string nomDuFichier)
{
	canalLecture.open(nomDuFichier);
	if (canalLecture.is_open())
	{
		return true;
	}
	cout << "Le fichier ne peut ne peut pas être ouvert" << endl;
	return false;
}

bool ouvrir_fichier_en_écriture(ofstream& canalEcriture, string nomDuFichier, bool ajouterALaFin)
{
	if (ajouterALaFin)
	{
		canalEcriture.open(nomDuFichier, ofstream::app);
	}
	else
	{
		canalEcriture.open(nomDuFichier);
	}
	if (canalEcriture.is_open())
	{
		return true;
	}
	cout << "Le fichier ne peut ne peut pas être ouvert" << endl;
	return false;
}

