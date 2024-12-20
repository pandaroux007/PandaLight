/*
PandaLight version cave
*/

#include <Arduino.h> // https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Arduino.h
#include <SoftwareSerial.h> // https://github.com/arduino/ArduinoCore-avr/tree/master/libraries/SoftwareSerial
// fichiers programmes
#include "ZoneEclairage.hpp"
#include "constantes.hpp"

// instances des zones d'éclairage
ZoneEclairage eclairages[NBR_ZONES];
OneButton boutonGeneral;

void eventClicGeneral();
void eventClicLongGeneral();

void setup(void)
{
  // Initialisation du moniteur série
  Serial.begin(VITESSE_MONITEUR_SERIE);
  /*
  Initialisation des leds WS2812B
  RGB par défaut, ce qui inversait le vert et le rouge sur le bandeau - passage en GRB
  */
  FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NBR_ZONES);
  FastLED.setBrightness(100);
  // Initialisation des zones d'éclairage
  eclairages[0].begin(12, A3, &leds[0], 0xFF0000); // rouge
  eclairages[1].begin(11, A2, &leds[1], 0x00FF00); // vert
  eclairages[2].begin(10, A1, &leds[2], 0x0000FF); // bleu
  eclairages[3].begin(4, A0, &leds[3], 0xFFFF00); // jaune
  eclairages[4].begin(3, 13, &leds[4], 0x00FFFF); // cyan

  boutonGeneral.setup(5, INPUT_PULLUP, true);
  boutonGeneral.attachClick(eventClicGeneral);
  boutonGeneral.attachLongPressStart(eventClicLongGeneral);

  DEBUG_PRINTLN(F("Démarrage!"));
  delay(1000);
}

void loop(void)
{
  // mise à jour machine à état (à appeller très régulièrement)
  for(uint8_t index = 0; index < NBR_ZONES; index++)
  {
    eclairages[index].update();
  }
}

void eventClicGeneral()
{
  uint8_t conteurValidation = 0;
  for (uint8_t index = 2; index < 5; index++)
  {
    if (eclairages[index].getStateMachine() == REPOS)
    {
      conteurValidation++;
    }
  }

  // si toutes les zones sont éteintes, alors on les allumes une à une
  if(conteurValidation == 3)
  {
    static unsigned long tempsPrecedent = 0;
    unsigned long maintenant = millis();

    uint8_t indexAllumageZone = 2;
    if((maintenant - tempsPrecedent) >= 500)
    {
      tempsPrecedent = maintenant;
      eclairages[indexAllumageZone].checkEventClic();
      indexAllumageZone++;
      if(indexAllumageZone == 5) return;
    }
  }
  // sinon si certaines ne sont pas au REPOS, allumer uniquement celles au REPOS
  else
  {
    Serial.print("conteurValidation: "); Serial.print(conteurValidation);
    for(uint8_t index = 2; index < 5; index++)
    {
      if(eclairages[index].getStateMachine() == REPOS)
      {
        /* pour l'instant pas de gestion du temps ici,
        plus tard à intégrer comme plus haut dans la fonction */
        eclairages[index].checkEventClic();
      }
    }
  }
}

void eventClicLongGeneral(void)
{
  for (uint8_t index = 2; index < 5; index++)
  {
    if (eclairages[index].getStateMachine() != REPOS)
    {
      eclairages[index].rebootConteurEtatCourant();
    }
  }
}