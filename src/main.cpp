/*
    Nome : PandaLight
    Auteur : Pandaroux007
    Source : https://github.com/pandaroux007/PandaLight
    Licence : MIT (LICENCE.txt)
    Documentation : README.md
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>
#include <SparkFunBME280.h>

#include "ZoneEclairage.hpp"

// ------------------------------ zones d'éclairage
struct ZoneConfig {
    uint8_t pinRelais;
    uint8_t pinBouton;
    uint32_t couleur;
};

constexpr ZoneConfig zonesConfig[] = {
    {12, A3, 0xFF0000}, // Rouge
    {11, A2, 0x00FF00}, // Vert
    {10, A1, 0x0000FF}, // Bleu
    {4,  A0, 0xFFFF00}, // Jaune
    {3,  13, 0x00FFFF}  // Cyan
};

constexpr size_t NBR_ZONES = sizeof(zonesConfig) / sizeof(zonesConfig[0]);
ZoneEclairage zones[NBR_ZONES];

// ------------------------------ leds & BME280
CRGB leds[NBR_ZONES];
BME280 bme280;

// ------------------------------ bouton général (voir README.md)
OneButton boutonGeneral;

enum class etatBpGeneral : uint8_t
{
    HORS_ALLUMAGE = 0,
    PREMIERE_ZONE = 2,
    DEUXIEME_ZONE = 3,
    TROISIEME_ZONE = 4
} etatCourantGeneral;

void callbackGeneralClick();
void callbackGeneralClickLong();
void updateGeneral();

// ------------------------------ modbus
// transmission des données et paramétrage des zones
//                       RX/TX pins
SoftwareSerial SerialModbus(9, 8);
ModbusRTUSlave modbus(SerialModbus, 7);

constexpr uint16_t MODBUS_BAUDRATE = 38400;
constexpr uint8_t SLAVE_ADDR = 200;

void configParDefautHoldingRegisters();
const uint8_t NBR_HOLDING_REGISTERS = NBR_ZONES * 2; // x2 car deux paramètres/zone : temps de fonctionnement ET temps avant extinction
uint16_t holdingRegisters[NBR_HOLDING_REGISTERS]; // R/W de la part du maitre modbus (config)
constexpr uint8_t NBR_INPUT_REGISTERS = 2;
uint16_t inputRegisters[NBR_INPUT_REGISTERS]; // R seulement de la part du maitre (capteur)

// ------------------------------ setup & loop
void setup()
{
    // Initialisation du moniteur série
    DEBUG_INIT(115200);
    DEBUG_PRINTLN("Démarrage!\n");
    delay(500);

    // Initialisation des leds WS2812B
    // (RGB par défaut, ce qui inversait le vert et le rouge sur le bandeau - avec GRB c'est OK)
    FastLED.addLeds<WS2812B, 5, GRB>(leds, NBR_ZONES);
    FastLED.setBrightness(100);

    // Initialisation des zones d'éclairage
    for(size_t i = 0; i < NBR_ZONES; ++i)
    {
        zones[i].begin(zonesConfig[i].pinRelais, zonesConfig[i].pinBouton, &holdingRegisters[i * 2], &leds[i], zonesConfig[i].couleur);
    }
    // Initialisation du bouton général
    boutonGeneral.setup(2, INPUT_PULLUP, ACTIVE_LOW);
    boutonGeneral.attachClick(callbackGeneralClick);
    boutonGeneral.attachLongPressStart(callbackGeneralClickLong);

    // Initialisation communication modbus
    modbus.configureHoldingRegisters(holdingRegisters, NBR_HOLDING_REGISTERS);
    modbus.configureInputRegisters(inputRegisters, NBR_INPUT_REGISTERS);
    SerialModbus.begin(MODBUS_BAUDRATE);
    modbus.begin(SLAVE_ADDR, MODBUS_BAUDRATE);
    configParDefautHoldingRegisters();

    // Initialisation bme280
    Wire.begin();
    bme280.beginI2C();
    bme280.setI2CAddress(0x76);
}

void loop()
{
    // màj machine à état zones
    for(size_t i = 0; i < NBR_ZONES; ++i)
    {
        zones[i].update();
    }

    updateGeneral();

    // màj communication modbus
    float temperature = bme280.readTempC();
    float humidite = bme280.readFloatHumidity();
    inputRegisters[0] = (int16_t)(temperature * 100);
    inputRegisters[1] = (uint16_t)(humidite * 100);

    modbus.poll();
}

void configParDefautHoldingRegisters()
{
    for(size_t zone = 0; zone < NBR_ZONES; ++zone)
    {
        // DEBUG_PRINT("Paramètrage zone "); DEBUG_VALUE_PRINT(zone); DEBUG_PRINTLN("");
        holdingRegisters[zone * 2]     = TEMPS_FONCTIONNEMENT_TOTAL_DEFAUT;
        // DEBUG_PRINT("holdingRegisters[1]: "); DEBUG_VALUE_PRINT(holdingRegisters[zone * 2]); DEBUG_PRINTLN("");
        holdingRegisters[zone * 2 + 1] = TEMPS_AVANT_EXTINCTION_DEFAUT;
        // DEBUG_PRINT("holdingRegisters[2]: "); DEBUG_VALUE_PRINT(holdingRegisters[zone * 2 + 1]); DEBUG_PRINTLN("");
    }
}

// --------------------------------------------- callbacks du bouton général
inline void callbackGeneralClick()
{
    etatCourantGeneral = etatBpGeneral::PREMIERE_ZONE;
}

void callbackGeneralClickLong()
{
    etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;

    for(uint8_t i = 2; i <= NBR_ZONES; i++)
    {
        // DEBUG_PRINT("GENERAL > Click long sur la zone "); DEBUG_VALUE_PRINT_TAB(i);
        zones[i].callbackClickLong();
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
            // DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::PREMIERE_ZONE);
            zones[(int)etatBpGeneral::PREMIERE_ZONE].callbackClick();
            break;

        case etatBpGeneral::DEUXIEME_ZONE:
            etatCourantGeneral = etatBpGeneral::TROISIEME_ZONE;
            // DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::DEUXIEME_ZONE);
            zones[(int)etatBpGeneral::DEUXIEME_ZONE].callbackClick();
            break;

        case etatBpGeneral::TROISIEME_ZONE:
            etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;
            // DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::TROISIEME_ZONE);
            zones[(int)etatBpGeneral::TROISIEME_ZONE].callbackClick();
            break;

        default:
            etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;
            // DEBUG_PRINTLN("GENERAL > État inconnu, passage à HORS_ALLUMAGE");
            break;
        }
    }
}