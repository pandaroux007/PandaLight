#ifndef ZONE_HEADER
#define ZONE_HEADER
#include <Arduino.h> // on définit Arduino ici donc on ne le redéclare pas dans le cpp
#include <OneButton.h> // on utilise OneButton et pas OneButtonTiny pour les callbacks dans la classe elle-même
#include <FastLED.h>

#include "debug.hpp"

// constantes
#define RELAIS_ON true
#define RELAIS_OFF false

#define INCREMENT_LUM_LED true // sens incrémentation de la luminosité de la led
#define DECREMENT_LUM_LED false // sens décrémentation de la luminosité de la led

#define ACTIVE_LOW false

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
        unsigned long tempsPrecedentClique;
        etatsZoneEclairage etat = REPOS;
        // relais vers l'éclairage
        uint8_t pinRelais;
        bool etatCourantRelais;
        void setRelais(bool);
        // led rgb + gestion de son clignotement
        CRGB * led;
        CRGB couleur;
        uint8_t luminosite;
        bool sensIncrementation = INCREMENT_LUM_LED;
        unsigned long tempsPrecedentClignotement;
        void ledClignoterDoucement();
        void ledClignoterRapidement();
        inline void ledAllumerCompletement() { *led = couleur; }

    public:
        // fonctions principales
        void begin(const uint8_t, const uint8_t, CRGB *, CRGB);
        void update();
        void callbackClick();
        void callbackClickLong();
        // fonctions getter + secondaires
        inline etatsZoneEclairage getStateMachine() const { return(etat); }
        inline void resetMinuteur() { tempsPrecedentClique = millis(); }
};

#endif // ZONE_HEADER