#ifndef ZONE_HEADER
#define ZONE_HEADER
#include <Arduino.h>
#include <OneButton.h> // on utilise OneButton et pas OneButtonTiny pour les callbacks dans la classe elle-même
#include <FastLED.h>

#include "debug.hpp"

// constantes
#define RELAIS_ON true
#define RELAIS_OFF false

#define INCREMENTER_LUM_LED true
#define DECREMENTER_LUM_LED false

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
        uint16_t* m_settingsModbus = nullptr;
        uint32_t ms(uint16_t secondes) const { return secondes * 1000; } // convertir des secondes en millisecondes
        uint32_t getTempsFonctionnement() const { return ms(*m_settingsModbus); } // 1ère case, c'est l'index passé au début
        uint32_t getTempsAvantExtinction() const { return ms(*(m_settingsModbus + 1)); } // on va chercher la valeur à la case suivante
        unsigned long tempsPrecedentClique;
        void resetMinuteur() { tempsPrecedentClique = millis(); }
        etatsZoneEclairage etat = REPOS;
        // relais vers l'éclairage
        uint8_t m_pinRelais;
        bool etatCourantRelais;
        void setEtatRelais(bool);
        // led rgb
        CRGB* m_led = nullptr;
        CRGB m_couleur;
        uint8_t luminosite;
        bool sensClignotement = INCREMENTER_LUM_LED;
        unsigned long tempsPrecedentClignotement;
        // fonctions gestion de la led
        void ledClignoterDoucement();
        void ledClignoterRapidement();
        void ledAllumerCompletement() { if(*m_led != m_couleur) *m_led = m_couleur; }
        void ledAppliquerLum();

    public:
        /// @param pinBouton
        /// @param pinRelais
        /// @param settingsModbus pointeur vers l'index de la zone dans le holdingRegisters modbus
        /// @param led pointeur vers l'index de la led dans le tableau CRGB
        /// @param couleur (CRGB) de la led associée à la zone
        void begin(const uint8_t, const uint8_t, uint16_t *, CRGB *, CRGB);
        
        /// @brief fonction de mise à jour de la machine à état
        /// @warning A appeler le plus souvent possible!
        void update();

        /// @brief fonction appelée quand un click simple est effectué
        void callbackClick();

        /// @brief fonction appelée quand un click long est effectué
        void callbackClickLong();
};

#endif // ZONE_HEADER