#ifndef CONSTANTES_HEADER
#define CONSTANTES_HEADER

#include <Arduino.h>
#include <FastLED.h>

// couleurs
constexpr uint32_t HEX_ROUGE = 0xFF0000;
constexpr uint32_t HEX_VERT = 0x00FF00;
constexpr uint32_t HEX_BLEU = 0x0000FF;
constexpr uint32_t HEX_JAUNE = 0xFFFF00;
constexpr uint32_t HEX_CYAN = 0x00FFFF;

// constantes
constexpr uint32_t VITESSE_MONITEUR_SERIE = 115200;
constexpr uint8_t NBR_ZONES = 5;
constexpr uint8_t PIN_LEDS = 5;

CRGB leds[NBR_ZONES];

#endif // CONSTANTES_HEADER