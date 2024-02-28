#pragma once
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

// �change les valeurs des 2 param�tres donn�s
void permuter(int& nombre1, int& nombre2);

// affiche tout le contenu du vecteur donn�
void afficher_vecteur_entiers(vector<int>& nombre);
void afficher_vecteur_reels(vector<double>& reel);
void afficher_vecteur_textes(vector<string>& texte);

// retourne un vecteur d'entiers de la taille donn�e, rempli de nombres al�atoires entre min et (max - 1) car max est exclu
vector<int> remplir_aleatoire(int nbDeValeursAleatoires, int min, int max);

// Retourne l'index de la plus grande valeur du vecteur nomm�.
// Si cette valeur est pr�sente plusieurs fois, le premier index est retoun�
int index_plus_grand(vector<int> max);

// Convertit tout un mot en majuscule
void convertir_en_majuscule(string& mot);

// Ouvre le fichier portant le nom donn� et initialise la variable [i/o]fstream
// Affiche un message d'erreur si le fichier ne peut pas �tre ouvert
// Retourne true si le fichier est ouvert, false sinon
bool ouvrir_fichier_en_lecture(ifstream& canalLecture, string nomDuFichier);
// Si ajouter � la fin est vrai, on �crase pas un fichier existant
bool ouvrir_fichier_en_�criture(ofstream& canalEcriture, string nomDuFichier, bool ajouterALaFin);