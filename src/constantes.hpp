#ifndef constantes_hpp
#define constantes_hpp

#include <Arduino.h>
#include <FastLED.h>

// constantes
#define VITESSE_MONITEUR_SERIE 115200
#define NBR_ZONES 5
#define PIN_LEDS 5

constexpr uint8_t PIN_BPS[NBR_ZONES] = {12, 11, 10, 4, 3};
constexpr uint8_t PIN_RELAIS[NBR_ZONES] = {A3, A2, A1, A0, 13};
// --------------------------   rouge  //  vert  // bleu   //  jaune  //  cyan
constexpr uint32_t COULEURS[NBR_ZONES] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0x00FFFF};

CRGB leds[NBR_ZONES];

#endif // constantes_hpp