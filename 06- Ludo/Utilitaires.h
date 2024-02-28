#pragma once
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

// Échange les valeurs des 2 paramètres donnés
void permuter(int& nombre1, int& nombre2);

// affiche tout le contenu du vecteur donné
void afficher_vecteur_entiers(vector<int>& nombre);
void afficher_vecteur_reels(vector<double>& reel);
void afficher_vecteur_textes(vector<string>& texte);

// retourne un vecteur d'entiers de la taille donnée, rempli de nombres aléatoires entre min et (max - 1) car max est exclu
vector<int> remplir_aleatoire(int nbDeValeursAleatoires, int min, int max);

// Retourne l'index de la plus grande valeur du vecteur nommé.
// Si cette valeur est présente plusieurs fois, le premier index est retouné
int index_plus_grand(vector<int> max);

// Convertit tout un mot en majuscule
void convertir_en_majuscule(string& mot);

// Ouvre le fichier portant le nom donné et initialise la variable [i/o]fstream
// Affiche un message d'erreur si le fichier ne peut pas être ouvert
// Retourne true si le fichier est ouvert, false sinon
bool ouvrir_fichier_en_lecture(ifstream& canalLecture, string nomDuFichier);
// Si ajouter à la fin est vrai, on écrase pas un fichier existant
bool ouvrir_fichier_en_écriture(ofstream& canalEcriture, string nomDuFichier, bool ajouterALaFin);