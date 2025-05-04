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
// fichier programmes
#include "ZoneEclairage.hpp"

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

// instances des zones d'éclairage
ZoneEclairage eclairages[NBR_ZONES];

enum indexZones : uint8_t
{
    //   nom  -  index
    INDEX_ZONE_1, // 0
    INDEX_ZONE_2, // 1
    INDEX_ZONE_3, // 2
    INDEX_ZONE_4, // 3
    INDEX_ZONE_5, // 4
};

// bouton général (voir README.md)
OneButton boutonGeneral;

enum class etatBpGeneral : uint8_t
{
    HORS_ALLUMAGE = 0,
    PREMIERE_ZONE = INDEX_ZONE_3,
    DEUXIEME_ZONE = INDEX_ZONE_4,
    TROISIEME_ZONE = INDEX_ZONE_5
} etatCourantGeneral;

void callbackGeneralClick();
void callbackGeneralClickLong();
void updateGeneral();

void setup()
{
    // Initialisation des leds WS2812B
    // (RGB par défaut, ce qui inversait le vert et le rouge sur le bandeau - avec GRB c'est OK)
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
    DEBUG_PRINTLN("Démarrage!\n");
    delay(1000);
}

void loop()
{
    // màj machine à état zones
    for(uint8_t index = 0; index < NBR_ZONES; index++)
    {
        eclairages[index].update();
    }

    updateGeneral();
}

// --------------------------------------------- callbacks du bouton général
inline void callbackGeneralClick()
{
    etatCourantGeneral = etatBpGeneral::PREMIERE_ZONE;
}

void callbackGeneralClickLong()
{
    etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;

    for(uint8_t index = INDEX_ZONE_3; index <= INDEX_ZONE_5; index++)
    {
        DEBUG_PRINT("GENERAL > Click long sur la zone "); DEBUG_VALUE_PRINT_TAB(index);
        eclairages[index].callbackClickLong();
    }
}

void updateGeneral()
{
    boutonGeneral.tick();

    static uint32_t tempsPrecedent = 0;
    uint32_t tempsMaintenant = millis();

    if((tempsMaintenant - tempsPrecedent) >= 1000) // 1 seconde
    {
        tempsPrecedent = tempsMaintenant;

        switch (etatCourantGeneral)
        {
        case etatBpGeneral::HORS_ALLUMAGE:
            // quand on passe ici, on ne fait rien
            break;

        case etatBpGeneral::PREMIERE_ZONE:
            etatCourantGeneral = etatBpGeneral::DEUXIEME_ZONE;
            DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::PREMIERE_ZONE);
            eclairages[(int)etatBpGeneral::PREMIERE_ZONE].callbackClick();
            break;

        case etatBpGeneral::DEUXIEME_ZONE:
            etatCourantGeneral = etatBpGeneral::TROISIEME_ZONE;
            DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::DEUXIEME_ZONE);
            eclairages[(int)etatBpGeneral::DEUXIEME_ZONE].callbackClick();
            break;

        case etatBpGeneral::TROISIEME_ZONE:
            etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;
            DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::TROISIEME_ZONE);
            eclairages[(int)etatBpGeneral::TROISIEME_ZONE].callbackClick();
            break;

        default:
            etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;
            DEBUG_PRINTLN("GENERAL > État inconnu, passage à HORS_ALLUMAGE");
            break;
        }
    }
}