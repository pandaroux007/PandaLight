#ifndef ZoneEclairage_h
#define ZoneEclairage_h
#include <Arduino.h> // on définit Arduino ici donc on ne le redéclare pas dans le cpp
#include <OneButton.h> // on utilise OneButton et pas OneButtonTiny pour les callbacks dans la classe elle-même
#include <FastLED.h>

#define RELAIS_ON true
#define RELAIS_OFF false

#define incrementationLumLed true
#define decrementationLumLed false

/* #define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT 3600000ul // 1h (60mn × 60s × 1000ms) >> Temps avant que l'éclairage s'éteigne tout seul en mode 1H.
#define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG 120000ul // 5h (5 × 60mn × 60s × 1000ms) >> Temps avant que l'éclairage s'éteigne tout seul en mode long.
#define TEMPS_AVANT_EXTENCTION 300000ul // 5mn (5mn × 60s × 1000ms) >> Temps ou l'on attend un nouveau clique sur le bouton pour relancer le minuteur. */

// temps courts pour les testes du montage !
#define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT 30000ul // 30 secondes
#define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG 60000 // 1 minutes
#define TEMPS_AVANT_EXTENCTION 10000 // 10s

class ZoneEclairage
{
  private:
    const char * nom;
    // déclaration de l'instance du bouton et de la led
    OneButton bouton;
    CRGB & led;
    // minuteur d'extinction
    uint32_t tempsPrecedentClique;
    // variable pour gérer le clignotement des leds
    uint32_t tempsPrecedentClignotement;
    // machine à état
    enum etatsZoneEclairage : uint8_t
    {
      ALLUME_COURT,
      ALLUME_LONG,
      ALLUME_VERS_REPOS,
      REPOS
    } etats = REPOS;
    // fonctions et pin gestion relais
    void setRelais(bool);
    byte pinRelais;
    bool etatCourantRelais;
    // variables et fonction des clignotements des leds rgb
    CRGB couleur;
    bool sensIncrementation = incrementationLumLed;
    byte luminosite;
    void ledClignoterDoucement(void);
    void ledClignoterRapidement(void);
    // fonction de gestion des événements du bouton
    void checkEventClic(void);
    void checkEventClicLong(void);

  public:
    ZoneEclairage(const char *, byte, byte, CRGB &, CRGB); // Constructeur
    void update(void); // fonction a appeller à chaque loop, pour gérer le btn et la machine à état
    void begin(void);
    bool getEtatCourant(void);
};

#endif //ZoneEclairage_h