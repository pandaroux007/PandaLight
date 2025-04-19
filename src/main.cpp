/*
  Nome : PandaLight
  Version : 1-cave
  Auteur : Pandaroux007
  Lien : https://github.com/pandaroux007/PandaLight
  Licence : MIT (voir LICENCE.txt)

  NOTE : Pour plus d'informations à propos de ce programme et du projet,
  référez-vous au fichier README.md
*/

#include <Arduino.h>
// fichiers programmes
#include "ZoneEclairage.hpp"
#include "constantes.hpp"

// instances des zones d'éclairage
ZoneEclairage eclairages[NBR_ZONES];

enum indexZones : uint8_t
{
    INDEX_ZONE_1 = 0,
    INDEX_ZONE_2 = 1,
    INDEX_ZONE_3 = 2,
    INDEX_ZONE_4 = 3,
    INDEX_ZONE_5 = 4
};

enum etatsBpGeneral : uint8_t
{
    GENERAL_REPOS,
    GENERAL_ALLUMAGE,
    GENERAL_ALLUME,
    GENERAL_EXTINCTION
} etatGeneral;

// bouton général (voir README.md)
OneButton boutonGeneral;
void callbackGeneralClick();
void callbackGeneralClickLong();
bool allumerZonesGeneral();
void eteindreZonesGeneral();
void reinitialiserZonesGeneral();

#define ZONES_TOUTES_ALLUMEES true

void setup()
{
    // Initialisation des leds WS2812B
    // (RGB par défaut, ce qui inversait le vert et le rouge sur le bandeau - passage en GRB)
    FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NBR_ZONES);
    FastLED.setBrightness(100);
    // Initialisation des zones d'éclairage
    eclairages[INDEX_ZONE_1].begin(12, A3, &leds[INDEX_ZONE_1], HEX_ROUGE);
    eclairages[INDEX_ZONE_2].begin(11, A2, &leds[INDEX_ZONE_2], HEX_VERT);
    eclairages[INDEX_ZONE_3].begin(10, A1, &leds[INDEX_ZONE_3], HEX_BLEU);
    eclairages[INDEX_ZONE_4].begin(4, A0, &leds[INDEX_ZONE_4], HEX_JAUNE);
    eclairages[INDEX_ZONE_5].begin(3, 13, &leds[INDEX_ZONE_5], HEX_CYAN);
    // Initialisation du bouton général
    boutonGeneral.setup(2, INPUT_PULLUP, ACTIVE_LOW);
    boutonGeneral.attachClick(callbackGeneralClick);
    boutonGeneral.attachLongPressStart(callbackGeneralClickLong);
    // Initialisation du moniteur série
    DEBUG_INIT(VITESSE_MONITEUR_SERIE);
    DEBUG_PRINTLN("Démarrage!");
    DEBUG_PRINT("Temps de fonctionnement : "); DEBUG_VALUE_PRINTLN(TEMPS_FONCTIONNEMENT_TOTAL);
    DEBUG_PRINT("Temps de rappel avant extinction : "); DEBUG_VALUE_PRINTLN(TEMPS_AVANT_EXTINCTION);
    DEBUG_PRINTLN("\n");
    delay(1000);
}

void loop()
{
    // màj machine à état zones
    for(uint8_t index = 0; index < NBR_ZONES; index++)
    {
        eclairages[index].update();
    }

    boutonGeneral.tick();
    // màj machine à état général
    switch (etatGeneral)
    {
    case GENERAL_REPOS:
        // rien dans ce cas
        break;
    case GENERAL_ALLUMAGE:
        if(allumerZonesGeneral() == ZONES_TOUTES_ALLUMEES)
        {
            etatGeneral = GENERAL_ALLUME;
        }
        break;
    case GENERAL_ALLUME:
        // rien ici non plus, le compteur de chaque zone
        // est géré directement dans son instance dans la
        // boucle for au dessus.
        break;
    case GENERAL_EXTINCTION:
        eteindreZonesGeneral();
        DEBUG_PRINT("GENERAL > Passage de GENERAL_EXTINCTION à GENERAL_REPOS");
        etatGeneral = GENERAL_REPOS;
        break;
    
    default:
        etatGeneral = GENERAL_EXTINCTION;
        break;
    }
}

// --------------------------------------------- callbacks du bouton général
void callbackGeneralClick()
{
    DEBUG_PRINT("GENERAL clique > ");
    switch (etatGeneral)
    {
    case GENERAL_REPOS:
        DEBUG_PRINTLN("On était à GENERAL_REPOS, on passe à GENERAL_ALLUMAGE");
        etatGeneral = GENERAL_ALLUMAGE;
        break;
    case GENERAL_ALLUMAGE:
        // rien dans ce cas, on passe
        DEBUG_PRINTLN("On était à GENERAL_ALLUMAGE, on ne fait rien");
        break;
    case GENERAL_ALLUME:
        DEBUG_PRINTLN("On est à GENERAL_ALLUME, on reset les minuteurs");
        reinitialiserZonesGeneral();
        break;
    case GENERAL_EXTINCTION:
        // rien ici non plus
        DEBUG_PRINTLN("On était à GENERAL_EXTINCTION, on ne fait rien");
        break;
    default:
        DEBUG_PRINTLN("Etat imprévu, extinction!");
        etatGeneral = GENERAL_EXTINCTION;
        break;
    }
}

void callbackGeneralClickLong()
{
    DEBUG_PRINT("GENERAL clique long > ");
    switch (etatGeneral)
    {
    case GENERAL_REPOS:
        DEBUG_PRINTLN("On était à GENERAL_REPOS, on passe à GENERAL_ALLUMAGE");
        etatGeneral = GENERAL_ALLUMAGE;
        break;
    case GENERAL_ALLUMAGE:
        // rien dans ce cas, on passe
        DEBUG_PRINTLN("On était à GENERAL_ALLUMAGE, on ne fait rien");
        break;
    case GENERAL_ALLUME:
        DEBUG_PRINTLN("On était à GENERAL_ALLUME, on passe à GENERAL_EXTINCTION");
        etatGeneral = GENERAL_EXTINCTION;
        break;
    case GENERAL_EXTINCTION:
        // rien ici non plus
        DEBUG_PRINTLN("On était à GENERAL_EXTINCTION, on ne fait rien");
        break;
    default:
        DEBUG_PRINTLN("Etat imprévu, extinction!");
        etatGeneral = GENERAL_EXTINCTION;
        break;
    }
}

// --------------------------------------------- fonctions de contrôle des zones
bool allumerZonesGeneral()
{
    static uint8_t index = INDEX_ZONE_3;

    static uint32_t tempsPrecedent = 0;
    uint32_t tempsMaintenant = millis();
    if((tempsMaintenant - tempsPrecedent) >= 1000) // 1 seconde
    {
        tempsPrecedent = tempsMaintenant;
        eclairages[index].callbackClick();
        DEBUG_PRINT("GENERAL > Zone "); DEBUG_VALUE_PRINT(index); DEBUG_PRINTLN(" allumée!");
        index++;
        if (index > INDEX_ZONE_5)
        {
            DEBUG_PRINTLN("GENERAL > Toutes les zones de 3 à 5 ont été allumées!");
            index = INDEX_ZONE_3;
            return true;
        }
    }

    return false;
}

void eteindreZonesGeneral()
{
    for(uint8_t index = INDEX_ZONE_3; index < INDEX_ZONE_5; index++)
    {
        eclairages[index].eteindreZone();
    }
    DEBUG_PRINTLN("GENERAL > Toutes les zones de 3 à 5 ont été éteintes!");
}

void reinitialiserZonesGeneral()
{
    for(uint8_t index = INDEX_ZONE_3; index < INDEX_ZONE_5; index++)
    {
        eclairages[index].resetMinuteur();
    }
    DEBUG_PRINTLN("GENERAL > Toutes les zones de 3 à 5 ont été réinitialisées!");
}