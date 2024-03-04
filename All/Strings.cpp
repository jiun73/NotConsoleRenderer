// Bibliothèque de manipulation de chaines de caractères
#include "pch.h"
#include <sstream>
#include "Strings.h"

using namespace std;


template <typename T>
string en_chaine(T v)
{
    ostringstream oss;
    oss << v;
    return oss.str();
}


template <typename T>
bool extraire(string& chaine, T& v)
{
    istringstream iss(chaine);
    iss >> v;
    if (iss)
    {
        chaine.clear();
        getline(iss, chaine);
        return true;
    }
    return false;
}


void rogner_espaces(string& chaine)
{
    size_t pos1 = chaine.find_first_not_of(" ");

    if (pos1 != string::npos)
    {
        chaine = chaine.substr(pos1, chaine.find_last_not_of(" ") - pos1 + 1);
    }
    else
    {
        chaine.clear();
    }
}


string entier_en_chaine(int entier)
{
    return en_chaine(entier);
}


string reel_en_chaine(double reel)
{
    return en_chaine(reel);
}


bool extraire_entier(string& chaine, int& entier)
{
    return extraire(chaine, entier);
}


bool extraire_reel(string& chaine, double& reel)
{
    return extraire(chaine, reel);
}


vector<string> separer(string chaine, char sep)
{
    vector<string> v;
    size_t debut = 0;
    size_t pos = chaine.find(sep);

    while (pos != string::npos)
    {
        v.push_back(chaine.substr(debut, pos - debut));
        debut = pos + 1;
        pos = chaine.find(sep, debut);
    }

    v.push_back(debut < chaine.size() ? chaine.substr(debut) : string());
    return v;
}
