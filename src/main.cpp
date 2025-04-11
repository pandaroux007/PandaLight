/*
  Nome : PandaLight
  Version : 1-cave
  Auteur : Pandaroux007
  Lien : https://github.com/pandaroux007/PandaLight
  Licence : MIT (voir LICENCE.txt)

  NOTE : Pour plus d'information à propos de ce programme et du projet,
  référez-vous au fichier README.md
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
// fichiers programmes
#include "ZoneEclairage.hpp"
#include "constantes.hpp"

// instances des zones d'éclairage + général
ZoneEclairage eclairages[NBR_ZONES];

enum indexZones : uint8_t {
    INDEX_ZONE_1 = 0,
    INDEX_ZONE_2 = 1,
    INDEX_ZONE_3 = 2,
    INDEX_ZONE_4 = 3,
    INDEX_ZONE_5 = 4
};

void setup()
{
    // Initialisation du moniteur série
    Serial.begin(VITESSE_MONITEUR_SERIE);
    DEBUG_PRINTLN("Démarrage!");
    DEBUG_PRINT("Temps de fonctionnement : "); DEBUG_VALUE_PRINTLN(TEMPS_FONCTIONNEMENT_TOTAL);
    DEBUG_PRINT("Temps de rappel avant extinction : "); DEBUG_VALUE_PRINTLN(TEMPS_AVANT_EXTINCTION);
    DEBUG_PRINTLN("\n");
    /*
    Initialisation des leds WS2812B
    RGB par défaut, ce qui inversait le vert et le rouge sur le bandeau - passage en GRB
    */
    FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NBR_ZONES);
    FastLED.setBrightness(100);
    // Initialisation des zones d'éclairage
    eclairages[INDEX_ZONE_1].begin(12, A3, &leds[INDEX_ZONE_1], HEX_ROUGE);
    eclairages[INDEX_ZONE_2].begin(11, A2, &leds[INDEX_ZONE_2], HEX_VERT);
    eclairages[INDEX_ZONE_3].begin(10, A1, &leds[INDEX_ZONE_3], HEX_BLEU);
    eclairages[INDEX_ZONE_4].begin(4, A0, &leds[INDEX_ZONE_4], HEX_JAUNE);
    eclairages[INDEX_ZONE_5].begin(3, 13, &leds[INDEX_ZONE_5], HEX_CYAN);

    delay(1000);
}

void loop()
{
    // màj machine à état
    for(uint8_t index = 0; index < NBR_ZONES; index++)
    {
        eclairages[index].update();
    }
}