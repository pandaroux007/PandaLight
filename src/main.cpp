/*
    Nome : PandaLight
    Version : 1.1
    Auteur : Pandaroux007
    Lien : https://github.com/pandaroux007/PandaLight
    Licence : MIT (voir LICENCE.txt)

    NOTE : Pour plus d'informations à propos de ce programme et du projet,
    référez-vous au fichier README.md
*/

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>
#include <SparkFunBME280.h>
// fichier programmes
#include "ZoneEclairage.hpp"

BME280 bme280;

// ------------------------------ leds
constexpr uint8_t NBR_ZONES = 5;

CRGB leds[NBR_ZONES];

constexpr uint32_t HEX_ROUGE = 0xFF0000;
constexpr uint32_t HEX_VERT = 0x00FF00;
constexpr uint32_t HEX_BLEU = 0x0000FF;
constexpr uint32_t HEX_JAUNE = 0xFFFF00;
constexpr uint32_t HEX_CYAN = 0x00FFFF;

// ------------------------------ zones d'éclairage
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

// ------------------------------ bouton général (voir README.md)
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

// ------------------------------ modbus
// transmission des données et paramétrage des zones
//                       RX/TX pins
SoftwareSerial SerialModbus(9, 8);
ModbusRTUSlave modbus(SerialModbus, 7);

constexpr uint16_t MODBUS_BAUDRATE = 38400;
constexpr uint8_t SLAVE_ADDR = 200;

void configParDefautHoldingRegisters();
constexpr uint8_t NBR_HOLDING_REGISTERS = NBR_ZONES * 2; // x2 car deux paramètres/zone : temps de fonctionnement ET temps avant extinction
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
    eclairages[INDEX_ZONE_1].begin(12, A3, &holdingRegisters[INDEX_ZONE_1 * 2], &leds[INDEX_ZONE_1], HEX_ROUGE);
    eclairages[INDEX_ZONE_2].begin(11, A2, &holdingRegisters[INDEX_ZONE_2 * 2], &leds[INDEX_ZONE_2], HEX_VERT);
    eclairages[INDEX_ZONE_3].begin(10, A1, &holdingRegisters[INDEX_ZONE_3 * 2], &leds[INDEX_ZONE_3], HEX_BLEU);
    eclairages[INDEX_ZONE_4].begin(4, A0, &holdingRegisters[INDEX_ZONE_4 * 2], &leds[INDEX_ZONE_4], HEX_JAUNE);
    eclairages[INDEX_ZONE_5].begin(3, 13, &holdingRegisters[INDEX_ZONE_5 * 2], &leds[INDEX_ZONE_5], HEX_CYAN);
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
    for(uint8_t index = 0; index < NBR_ZONES; index++)
    {
        eclairages[index].update();
    }

    updateGeneral();

    // màj communication modbus
    float temperature = bme280.readTempC();
    float humidite = bme280.readFloatHumidity();
    inputRegisters[0] = (int16_t)(temperature * 100);
    inputRegisters[1] = (uint16_t)(humidite * 100);

    modbus.poll();
    /*
    on recoit depuis la gateway 200 4 00 02 9682 toutes les 3 secondes: c'est l'ADU
    au début on a 200 (l'adresse du slave) et à la fin on a 9682 un CRC qui permet de valider l'ADU
    si le slave ou le CRC ne sont pas bon on NE répond PAS pour ne pas encombrer le bus puisque soit le master parle à quelqu'un d'autre, soit la trame est fausse
    la lib utilise n'ensuite que le PDU avec :
    - 4 le numéro de fonction pour lire des input register
    - 00 2 octets pou donner l'adresse de depart dans le tableau des input register que l'on veut lire
    - 02 2 octets qui donnent le nomnre de registrent que l'on veut lire (ici 2 donc le registrer d'adresse 0 et celui d'adresse 1)
    */
}

void configParDefautHoldingRegisters()
{
    for(uint8_t zone = 0; zone < NBR_ZONES; ++zone)
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

    for(uint8_t index = INDEX_ZONE_3; index <= INDEX_ZONE_5; index++)
    {
        // DEBUG_PRINT("GENERAL > Click long sur la zone "); DEBUG_VALUE_PRINT_TAB(index);
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
            // DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::PREMIERE_ZONE);
            eclairages[(int)etatBpGeneral::PREMIERE_ZONE].callbackClick();
            break;

        case etatBpGeneral::DEUXIEME_ZONE:
            etatCourantGeneral = etatBpGeneral::TROISIEME_ZONE;
            // DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::DEUXIEME_ZONE);
            eclairages[(int)etatBpGeneral::DEUXIEME_ZONE].callbackClick();
            break;

        case etatBpGeneral::TROISIEME_ZONE:
            etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;
            // DEBUG_PRINT("GENERAL > Click sur la zone "); DEBUG_VALUE_PRINT_TAB((int)etatBpGeneral::TROISIEME_ZONE);
            eclairages[(int)etatBpGeneral::TROISIEME_ZONE].callbackClick();
            break;

        default:
            etatCourantGeneral = etatBpGeneral::HORS_ALLUMAGE;
            // DEBUG_PRINTLN("GENERAL > État inconnu, passage à HORS_ALLUMAGE");
            break;
        }
    }
}