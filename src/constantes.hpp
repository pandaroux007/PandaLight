#ifndef constantes_hpp
#define constantes_hpp

#include <Arduino.h>

// constantes
#define VITESSE_MONITEUR_SERIE 115200
#define NBR_ZONES 5
#define PIN_LEDS 5

// couleurs
constexpr uint32_t COULEUR_ZONE_1 = 0xFF0000; // rouge
constexpr uint32_t COULEUR_ZONE_2 = 0x00FF00; // vert
constexpr uint32_t COULEUR_ZONE_3 = 0x0000FF; // bleu
constexpr uint32_t COULEUR_ZONE_4 = 0xFFFF00; // jaune
constexpr uint32_t COULEUR_ZONE_5 = 0x00FFFF; // cyan

#endif // constantes_hpp