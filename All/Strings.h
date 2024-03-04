#pragma once
// Bibliothèque de manipulation de chaines de caractères
#include <string>
#include <vector>


// Supprime toutes les espaces au début et à la fin de la chaine donnée
// Ne modifie pas les espaces entre d'autres caractères
void rogner_espaces(std::string& chaine);


// Retourne le nombre donné converti en chaine de caractères
std::string entier_en_chaine(int entier);
std::string reel_en_chaine(double reel);


// Convertit une chaine de caractères en nombre entier
//
// chaine est une référence sur la chaine à convertir.
// entier est une référence sur la variable dans laquelle mettre la valeur convertie.
//
// Si la chaine débute par un nombre entier (il peut y avoir des espaces au début de la chaine),
// ce nombre est extrait de la chaine et assigné au paramètre 'entier'. La chaine est modifiée
// pour enlever le nombre, et les espaces avant ce nombre si présentes. La fonction retourne alors vrai.
//
// Si la chaine ne débute pas par un nombre entier, 'chaine' et 'entier' sont inchangés.
// La fonction retourne alors faux.
bool extraire_entier(std::string& chaine, int& entier);


// Même comportement que convertirEntier ci-dessus, mais pour un nombre réel.
bool extraire_reel(std::string& chaine, double& reel);


// Sépare la chaine donnée en une suite de sous-chaines selon le séparateur donné
// Retourne un vecteur contenant toutes les sous-chaines séparées
std::vector<std::string> separer(std::string chaine, char sep);
