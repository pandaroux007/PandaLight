#ifndef constants_h
#define constants_h
#include <Arduino.h>

#define VITESSE_MONITEUR_SERIE 115200

// écran LCD I2C 1602
#define ADRESSE_ECRAN 0x27
#define NBR_COLONNES 16
#define NBR_LIGNES 2

byte rond_en_exposant_symbole_degres_celsius[8] =
{
  0b01100,
  0b10010,
  0b10010,
  0b01100,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// couleurs des leds en fontion de chaque zone
#define COULEUR_ZONE_GARAGE 0xFF0000 // rouge
#define COULEUR_ZONE_VELO 0x00FF00 // vert
#define COULEUR_ZONE_GUIRLANDE 0x0000FF // bleu

// nombre de leds sur le bandeau
#define NBR_LEDS 3
// index des leds
#define INDEX_LED_GARAGE 0
#define INDEX_LED_VELO 1
#define INDEX_LED_GUIRLANDE 2

// modbus RTU slave RS485
#define ID_SLAVE 200
//#define VIRTUAL_ADDRESS 0x7000
#define VITESSE_MODBUS 38400
#define TAILLE_BUFFER_MODBUS 6
// index des valeurs dans le tableau
#define INDEX_ETAT_COURANT_GARAGE 0
#define INDEX_ETAT_COURANT_VELO 1
#define INDEX_ETAT_COURANT_GUIRLANDE 2
#define INDEX_VALEUR_TEMPERATURE 3
#define INDEX_VALEUR_HUMIDITE 4
#define INDEX_VALEUR_PRESSION 5

#endif //constants_h