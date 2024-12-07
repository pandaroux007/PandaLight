#ifndef ZoneEclairage_h
#define ZoneEclairage_h
#include <Arduino.h> // on définit Arduino ici donc on ne le redéclare pas dans le cpp
#include <OneButton.h> // on utilise OneButton et pas OneButtonTiny pour les callbacks dans la classe elle-même
#include <FastLED.h>

// directive de préprocesseur pour le debug
#define DEBUG_ACTVE // ligne à commenter pour desactiver le debug
#ifdef DEBUG_ACTVE
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
#endif

#define RELAIS_ON true
#define RELAIS_OFF false

#define incrementationLumLed true
#define decrementationLumLed false

#define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT 3600000ul // 1h (60mn × 60s × 1000ms) >> Temps avant que l'éclairage s'éteigne tout seul en mode 1H.
#define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG 120000ul // 5h (5 × 60mn × 60s × 1000ms) >> Temps avant que l'éclairage s'éteigne tout seul en mode long.
#define TEMPS_AVANT_EXTENCTION 300000ul // 5mn (5mn × 60s × 1000ms) >> Temps ou l'on attend un nouveau clique sur le bouton pour relancer le minuteur.

/* // temps courts pour les testes du montage !
#define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT 30000ul // 30 secondes
#define TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG 60000 // 1 minutes
#define TEMPS_AVANT_EXTENCTION 10000 // 10s */

// machine à état
enum etatsZoneEclairage : uint8_t
{
  ALLUME_COURT,
  ALLUME_LONG,
  ALLUME_VERS_REPOS,
  REPOS
};

/// @brief constructeur d'une zone d'éclairage
/// @param uint8_t pin du bouton relié à la zone
/// @param uint8_t pin du relais contrôlant l'éclairage de la zone
/// @param CRGB référence vers l'index du tableau de led correspondant à la led de la zone
/// @param CRGB valeur RGB de la couleur de la led associée à la zone
class ZoneEclairage
{
  private:
    OneButton bouton;
    CRGB * led;
    // minuteur d'extinction
    uint32_t tempsPrecedentClique;
    // variable pour gérer le clignotement des leds
    uint32_t tempsPrecedentClignotement;
    etatsZoneEclairage etat = REPOS;
    // fonctions et pin gestion relais
    void setRelais(bool);
    uint8_t pinRelais;
    bool etatCourantRelais;
    // variables et fonction des clignotements des leds rgb
    CRGB couleur;
    bool sensIncrementation = incrementationLumLed;
    uint8_t luminosite;
    void ledClignoterDoucement(void);
    void ledClignoterRapidement(void);

  public:
    explicit ZoneEclairage() = default; //constructeur
    void begin(const uint8_t, const uint8_t, CRGB *, CRGB);
    void update(void);
    /// @brief fonction getter pour l'état courant de la machine à état de la zone
    /// @return etatsZoneEclairage
    inline etatsZoneEclairage getStateMachine(void) const {return(etat);}
    /// @brief permet de remettre à 0 le temps depuis le dernier click
    inline void rebootConteurEtatCourant(void) {tempsPrecedentClique = millis();}
    void checkEventClic(void);
    void checkEventClicLong(void);
};

#endif //ZoneEclairage_h