#pragma once

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

struct modbus_t
{
    uint16_t* modbusAddr = nullptr;

    uint16_t getTempsFonctionnement(void) const { return *modbusAddr; } // 1ère case, c'est l'index passé au début
    uint16_t getTempsAvantExtinction(void) const { return *(modbusAddr + 1); } // on va chercher la valeur suivante à la case 2
    uint16_t getTempsRestant(void) const { return *(modbusAddr + 2); } // même chose à la case 3

    void setTempsFonctionnement(uint16_t t) { *modbusAddr = t; }
    void setTempsAvantExtinction(uint16_t t) { *(modbusAddr + 1) = t; }
    void setTempsRestant(uint16_t t) { *(modbusAddr + 2) = t; }
};

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
        // modbus
        modbus_t m_modbus;
        uint16_t tempsRestantPrecedent = 0;
        //uint32_t
        void updateModbus();
        // minuteur d'extinction
        uint32_t ms(uint16_t secondes) const { return secondes * 1000; } // convertir des secondes en millisecondes
        uint32_t tempsFonctionnement() const { return ms(m_modbus.getTempsFonctionnement()); }
        uint32_t tempsAvantExtinction() const { return ms(m_modbus.getTempsAvantExtinction()); }
        unsigned long tempsPrecedentClick = 0;
        void resetMinuteur() { tempsPrecedentClick = millis(); }
        etatsZoneEclairage etat = REPOS;
        // relais vers l'éclairage
        uint8_t m_pinRelais = 0;
        bool etatCourantRelais;
        void setEtatRelais(bool);
        // led rgb
        CRGB* m_led = nullptr;
        CRGB m_couleur = 0x000000;
        uint8_t luminosite = 0;
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
        /// @param configModbus pointeur vers l'index de la zone dans le holdingRegisters modbus
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