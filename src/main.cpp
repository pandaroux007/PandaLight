/*
lib_deps = 
	; Dlloydev/Toggle@^3.1.8
	fastled/FastLED@^3.6.0
	mathertel/OneButton@^2.5.0
	; CMB27/ModbusRTUSlave@^2.0.5
*/

#include <Arduino.h>
// fichiers programmes
#include "ZoneEclairage.h"
#include "definitions.h"

CRGB leds[NBR_LEDS];

ZoneEclairage eclairageSpotGarage(PIN_BOUTON_GARAGE, PIN_RELAIS_GARAGE, leds[INDEX_LED_GARAGE]);
ZoneEclairage eclairageSpotVelo(PIN_BOUTON_VELO, PIN_RELAIS_VELO, leds[INDEX_LED_VELO]);
ZoneEclairage eclairageSpotGuirlande(PIN_BOUTON_GUIRLANDE, PIN_RELAIS_GUIRLANDE, leds[INDEX_LED_GUIRLANDE]);

void setup(void)
{
  // Initialisation du moniteur série
  Serial.begin(115200);
  Serial.println(F("Démarrage!\n-------------------------------------------------------------------"));
  Serial.print(F("Temps de fonctionnement d'une lampe en mode court : ")); Serial.print(TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT); Serial.println(F("ms"));
  Serial.print(F("Temps de fonctionnement d'une lampe en mode long  : ")); Serial.print(TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG); Serial.println(F("ms"));
  // initialisation des leds WS2812B
  FastLED.addLeds<WS2812B, PIN_LEDS, GRB /*RGB par défaut, ce qui inversait le vert et le rouge sur mon bandeau!*/>(leds, NBR_LEDS);
  Serial.println(F("LEDs WS2812B initialisées!\n"));
  delay(1000); // stop 1s
}

void loop(void)
{
  eclairageSpotGarage.update();
  eclairageSpotVelo.update();
  eclairageSpotGuirlande.update();
}