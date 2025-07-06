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
enum class EtatGeneral : uint8_t
{
    ATTENTE,
    ZONE_2,
    ZONE_3,
    ZONE_4
};

EtatGeneral etatGeneral = EtatGeneral::ATTENTE;

OneButton boutonGeneral;

void clickGeneral();
void clickLongGeneral();
void updateGeneral();

// ------------------------------ modbus
// transmission des données et paramétrage des zones
SoftwareSerial SerialModbus(9, 8); // RX/TX
ModbusRTUSlave modbus(SerialModbus, 7);

constexpr uint16_t MODBUS_BAUDRATE = 38400;
constexpr uint8_t SLAVE_ADDR = 200;

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
    boutonGeneral.attachClick(clickGeneral);
    boutonGeneral.attachLongPressStart(clickLongGeneral);

    // Initialisation communication modbus
    modbus.configureHoldingRegisters(holdingRegisters, NBR_HOLDING_REGISTERS);
    modbus.configureInputRegisters(inputRegisters, NBR_INPUT_REGISTERS);
    SerialModbus.begin(MODBUS_BAUDRATE);
    modbus.begin(SLAVE_ADDR, MODBUS_BAUDRATE);
    for(size_t zone = 0; zone < NBR_ZONES; ++zone)
    {
        holdingRegisters[zone * 2]     = TEMPS_FONCTIONNEMENT_TOTAL_DEFAUT;
        holdingRegisters[zone * 2 + 1] = TEMPS_AVANT_EXTINCTION_DEFAUT;
    }

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

// --------------------------------------------- callbacks du bouton général
inline void clickGeneral()
{
    etatGeneral = EtatGeneral::ZONE_2;
}

void clickLongGeneral()
{
    etatGeneral = EtatGeneral::ATTENTE;
    for(uint8_t i = 2; i <= NBR_ZONES; i++)
    {
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
        switch (etatGeneral)
        {
        case EtatGeneral::ATTENTE:
            // rien à faire ici
            break;
        case EtatGeneral::ZONE_2:
            etatGeneral = EtatGeneral::ZONE_3;
            zones[2].callbackClick();
            break;
        case EtatGeneral::ZONE_3:
            etatGeneral = EtatGeneral::ZONE_4;
            zones[3].callbackClick();
            break;
        case EtatGeneral::ZONE_4:
            etatGeneral = EtatGeneral::ATTENTE;
            zones[4].callbackClick();
            break;
        default:
            etatGeneral = EtatGeneral::ATTENTE;
        }
    }
}