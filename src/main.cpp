#include <Arduino.h> // https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Arduino.h
#include <Bme280.h> // https://github.com/malokhvii-eduard/arduino-bme280
#include <LiquidCrystal_I2C.h> // https://github.com/marcoschwartz/LiquidCrystal_I2C
#include <SoftwareSerial.h> // https://github.com/arduino/ArduinoCore-avr/tree/master/libraries/SoftwareSerial
#include <ModbusRTUSlave.h> // https://github.com/CMB27/ModbusRTUSlave
#include <Wire.h> // https://github.com/arduino/ArduinoCore-avr/tree/master/libraries/Wire
// fichiers programmes
#include "ZoneEclairage.h"
#include "constants.h"
#include "pinout.h"

LiquidCrystal_I2C lcd(0x27,16,2);
enum etatsAffichage : uint8_t {METEO, ETAT_ZONES};
etatsAffichage etatAffichage = METEO;
// LEDs RGB et BME280
Bme280TwoWire bme;
CRGB leds[NBR_LEDS];
// Émulation software de liaison série et Modbus
SoftwareSerial communicationSerieLogicielle(PIN_RX, PIN_TX);
ModbusRTUSlave modbus(communicationSerieLogicielle, PIN_DE);
uint16_t holdingRegisters[TAILLE_BUFFER_MODBUS];
// Instances de ma classe pour chaque zone d'éclairage (boutons, relais et LED RGB)
ZoneEclairage eclairageSpotGarage("garage", PIN_BOUTON_GARAGE, PIN_RELAIS_GARAGE, leds[INDEX_LED_GARAGE], COULEUR_ZONE_GARAGE);
ZoneEclairage eclairageSpotVelo("velo" ,PIN_BOUTON_VELO, PIN_RELAIS_VELO, leds[INDEX_LED_VELO], COULEUR_ZONE_VELO);
ZoneEclairage eclairageSpotGuirlande("guirlande", PIN_BOUTON_GUIRLANDE, PIN_RELAIS_GUIRLANDE, leds[INDEX_LED_GUIRLANDE], COULEUR_ZONE_GUIRLANDE);

void gestionMachineAEtatAffichage(void)
{
  switch (etatAffichage)
  {
  case METEO:
    Serial.println(F("Mode METEO"));
    etatAffichage = ETAT_ZONES;
    // affichage données météo sur écran lcd
    lcd.clear(); lcd.home();
    lcd.print(F("T: ")); lcd.print(bme.getTemperature()); lcd.print(char(1)); lcd.print(F("C"));
    lcd.setCursor(0,1);
    lcd.print(F("H: ")); lcd.print(bme.getHumidity()); lcd.print(F("%"));
    break;
  case ETAT_ZONES:
    Serial.println(F("Mode ETAT_ZONES"));
    etatAffichage = METEO;
    // affichage de l'état (allumé ou éteint) de chacune des trois zone (vélo, garage, et guirlande)
    lcd.clear(); lcd.home();
    lcd.print(F("VELO:")); lcd.print(eclairageSpotVelo.getEtatCourant());
    lcd.print(F(", GARAGE:")); lcd.print(eclairageSpotGarage.getEtatCourant());
    lcd.setCursor(0, 1);
    lcd.print(F("  GUIRLANDE:")); lcd.print(eclairageSpotGuirlande.getEtatCourant());
    break;
  default:
    Serial.println(F("ERREUR >> etat inconnu dans loop affichage! Mode METEO par défaut"));
    etatAffichage = METEO;
    break;
  }
}

void setup(void)
{
  // Initialisation du moniteur série
  Serial.begin(VITESSE_MONITEUR_SERIE);
  Serial.println(F("\n\nDémarrage!\n-----------------------------------------------------------"));
  Serial.print(F("Temps de fonctionnement d'une lampe en mode court : ")); Serial.print(TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT); Serial.println(F("ms"));
  Serial.print(F("Temps de fonctionnement d'une lampe en mode long  : ")); Serial.print(TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG); Serial.println(F("ms"));
  Serial.println(F("-----------------------------------------------------------"));
  // Initialisation des leds WS2812B
  FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(leds, NBR_LEDS); //RGB par défaut, ce qui inversait le vert et le rouge sur mon bandeau!
  FastLED.setBrightness(100);
  Serial.println(F(">> LEDs WS2812B initialisées!"));
  // Initialisation des zones d'éclairage
  eclairageSpotGarage.begin();
  eclairageSpotVelo.begin();
  eclairageSpotGuirlande.begin();
  Serial.println(F(">> Zones d'éclairages initialisées!"));
  // Initialisation de l'écran LCD
  lcd.init();
  lcd.backlight();
  lcd.setContrast(255);
  lcd.clear();
  lcd.home();
  lcd.createChar(1, rond_en_exposant_symbole_degres_celsius);
  Serial.println(F(">> Écran LCD 1602 I2C initialisé!"));
  // Initialisation du capteur BME280
  bme.begin(Bme280TwoWireAddress::Primary);
  bme.setSettings(Bme280Settings::weatherMonitoring());
  Serial.println(F(">> Capteur BME280 I2C initialisé!"));
  // Configuration du modbus
  modbus.configureHoldingRegisters(holdingRegisters, TAILLE_BUFFER_MODBUS);
  modbus.begin(ID_SLAVE, VITESSE_MODBUS);
  Serial.println(F("-----------------------------------------------------------\nFin des initalisations!"));
  delay(1000);
  gestionMachineAEtatAffichage();
}

void loop(void)
{
  // gestion machine à état des boutons et des éclairage
  eclairageSpotGarage.update();
  eclairageSpotVelo.update();
  eclairageSpotGuirlande.update();
  // gestion machine à état de l'affichage des données
  EVERY_N_SECONDS(5) gestionMachineAEtatAffichage(); // 5s au lieu de 3s - pour le débug
  // gestion modbus
  modbus.poll();
  holdingRegisters[INDEX_ETAT_COURANT_GARAGE] = eclairageSpotGarage.getEtatCourant();
  holdingRegisters[INDEX_ETAT_COURANT_VELO] = eclairageSpotVelo.getEtatCourant();
  holdingRegisters[INDEX_ETAT_COURANT_GUIRLANDE] = eclairageSpotGuirlande.getEtatCourant();
  holdingRegisters[INDEX_VALEUR_TEMPERATURE] = (int8_t)(bme.getTemperature()*100);
  holdingRegisters[INDEX_VALEUR_HUMIDITE] = (uint8_t)(bme.getHumidity()*100);
  holdingRegisters[INDEX_VALEUR_PRESSION] = (uint16_t)(bme.getPressure()*10000); //hPa
}