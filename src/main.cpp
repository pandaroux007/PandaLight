#include <Arduino.h>
#include <Bme280.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
// fichiers programmes
#include "ZoneEclairage.h"
#include "constants.h"
#include "pinout.h"

CRGB leds[NBR_LEDS];
LiquidCrystal_I2C lcd(0x27,16,2);
Bme280TwoWire bme;

ZoneEclairage eclairageSpotGarage(PIN_BOUTON_GARAGE, PIN_RELAIS_GARAGE, leds[INDEX_LED_GARAGE], COULEUR_ZONE_GARAGE);
ZoneEclairage eclairageSpotVelo(PIN_BOUTON_VELO, PIN_RELAIS_VELO, leds[INDEX_LED_VELO], COULEUR_ZONE_VELO);
ZoneEclairage eclairageSpotGuirlande(PIN_BOUTON_GUIRLANDE, PIN_RELAIS_GUIRLANDE, leds[INDEX_LED_GUIRLANDE], COULEUR_ZONE_GUIRLANDE);

void setup(void)
{
  // Initialisation du moniteur série
  Serial.begin(115200);
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
  lcd.clear();
  lcd.home();
  lcd.createChar(1, rond_en_exposant_symbole_degres_celsius);
  Serial.println(F(">> Écran LCD 1602 I2C initialisé!"));
  // Initialisation du capteur BME280
  bme.begin(Bme280TwoWireAddress::Primary);
  bme.setSettings(Bme280Settings::weatherMonitoring());
  Serial.println(F(">> Capteur BME280 I2C initialisé!"));
  Serial.println(F("-----------------------------------------------------------\nFin des initalisations!"));
  delay(1000);
}

void loop(void)
{
  // gestion machine à état
  eclairageSpotGarage.update();
  eclairageSpotVelo.update();
  eclairageSpotGuirlande.update();
  // gestion affichage température
  EVERY_N_SECONDS(1) // en phase de test
  { // affichage données sur moniteur série
    Serial.print(F("T: ")); Serial.print(bme.getTemperature()); Serial.print(F("°C\t"));
    Serial.print(F("H: ")); Serial.print(bme.getHumidity()); Serial.print(F("%\t"));
    Serial.print(F("P: ")); Serial.print(bme.getPressure() / 100.0); Serial.println(F("hPa\t"));
    // affichage données sur écran lcd
    lcd.clear(); lcd.home();
    lcd.print(F("T: ")); lcd.print(bme.getTemperature()); lcd.print(char(1)); lcd.print(F("C"));
    lcd.print(F("H: ")); lcd.print(bme.getHumidity()); lcd.print(F("%"));
    lcd.print(F("P: ")); lcd.print(bme.getPressure() / 100.0); lcd.print(F("hPa"));
  }
}