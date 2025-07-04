#ifndef ZONE_HEADER
#define ZONE_HEADER
#include <Arduino.h> // on définit Arduino ici donc on ne le redéclare pas dans le cpp
#include <OneButton.h> // on utilise OneButton et pas OneButtonTiny pour les callbacks dans la classe elle-même
#include <FastLED.h>

#include "debug.hpp"

// constantes
#define RELAIS_ON true
#define RELAIS_OFF false

#define INCREMENTER_LUM_LED true // sens incrémentation de la luminosité de la led
#define DECREMENTER_LUM_LED false // sens décrémentation de la luminosité de la led

#define ACTIVE_LOW true

// machine à états
enum etatsZoneEclairage : uint8_t
{
    REPOS,
    ALLUME,
    ALLUME_VERS_REPOS
};

class ZoneEclairage : public OneButton
{
    private:
        // minuteur d'extinction
        uint16_t* settingsModbus = nullptr;
        inline uint32_t ms(uint16_t secondes) const { return secondes * 1000; }
        inline uint32_t getTempsFonctionnement() const { return ms(*settingsModbus); } // 1ère case, c'est l'index passé au début
        inline uint32_t getTempsAvantExtinction() const { return ms(*(settingsModbus + 1)); } // on va chercher la valeur à la case suivante
        unsigned long tempsPrecedentClique;
        etatsZoneEclairage etat = REPOS;
        // relais vers l'éclairage
        uint8_t pinRelais;
        bool etatCourantRelais;
        void setEtatRelais(bool);
        // led rgb
        CRGB* led = nullptr;
        CRGB couleur;
        uint8_t luminosite;
        bool sensClignotement = INCREMENTER_LUM_LED;
        unsigned long tempsPrecedentClignotement;
        // fonctions gestion de la led
        void ledClignoterDoucement();
        void ledClignoterRapidement();
        inline void ledAllumerCompletement() { if(*led != couleur) *led = couleur; }
        void ledAppliquerLum();

    public:
        /// @param uint8_t pin du bouton
        /// @param uint8_t pin du relais
        /// @param uint16_t pointeur vers l'index de la zone dans le holdingRegisters modbus
        /// @param CRGB pointeur vers l'index de la led dans le tableau
        /// @param CRGB couleur de la led associée à la zone
        void begin(const uint8_t, const uint8_t, uint16_t *, CRGB *, CRGB);
        // fonctions principales
        void update();
        void callbackClick();
        void callbackClickLong();
        // fonctions getter + secondaires
        inline void eteindreZone() { etat = REPOS; }
        inline void resetMinuteur() { tempsPrecedentClique = millis(); }
};

#endif // ZONE_HEADER