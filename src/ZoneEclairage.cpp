#include "ZoneEclairage.hpp"

// --------------------------------------------- public

void ZoneEclairage::begin(const uint8_t passedPinBouton, const uint8_t passedPinRelais, CRGB * passedLed, CRGB passedCouleur)
{
    couleur = passedCouleur;
    led = passedLed;
    pinRelais = passedPinRelais;
    
    pinMode(pinRelais, OUTPUT);
    setup(passedPinBouton, INPUT_PULLUP, ACTIVE_LOW);

    // on attache le clique simple à la gestion d'un événement
    attachClick([](void *instance) {
        ((ZoneEclairage *)instance)->callbackClick();
    }, this);
    // et on fait pareil pour le clique long
    attachLongPressStart([](void *instance) {
        ((ZoneEclairage *)instance)->callbackClickLong();
    }, this);
}

/// @brief fonction de mise à jour de la machine à état
/// @warning A appeler le plus souvent possible!
void ZoneEclairage::update()
{
    tick(); // màj état bouton

    // gestion relais & led
    switch (etat)
    {
    case REPOS:
        setRelais(RELAIS_OFF);
        ledClignoterDoucement();
        break;
    case ALLUME:
        setRelais(RELAIS_ON);
        ledAllumerCompletement();
        // gestion minuteur
        if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_TOTAL - TEMPS_AVANT_EXTINCTION)) // si temps presque écoulé
        {
            DEBUG_PRINTLN(">> Fin du temps proche!");
            etat = ALLUME_VERS_REPOS;
        }
        break;
    case ALLUME_VERS_REPOS:
        setRelais(RELAIS_ON);
        ledClignoterRapidement();
        // gestion minuteur
        if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_TOTAL)) // si temps complètement écoulé
        {
            DEBUG_PRINTLN(">> Fin du temps, passage à REPOS!");
            etat = REPOS;
        }
        break;
    // en cas de situation wtf, on revient en mode repos
    default:
        etat = REPOS;
        break;
    }

    FastLED.show(); // on affiche les changement sur la led
}

// --------------------------------------------------------------------------------- callbacks

/// @brief fonction appelée quand un clique simple est effectué
void ZoneEclairage::callbackClick()
{
    DEBUG_PRINT("Clique >> ");
    switch (etat)
    {
    case REPOS:
        DEBUG_PRINTLN("On était au REPOS, on passe à ALLUME");
        resetMinuteur();
        etat = ALLUME;
        break;
    case ALLUME:
        DEBUG_PRINTLN("On était ALLUME, on relance le décompte");
        resetMinuteur();
        break;
    case ALLUME_VERS_REPOS:
        DEBUG_PRINTLN("On était en ALLUME_VERS_REPOS, on repasse en mode ALLUME");
        resetMinuteur();
        etat = ALLUME;
        break;
    default:
        etat = REPOS;
        break;
    }
}

/// @brief fonction appelée quand un clique long est effectué
void ZoneEclairage::callbackClickLong()
{
    DEBUG_PRINT("Clique long >> ");
    switch (etat)
    {
    case REPOS:
        DEBUG_PRINTLN("On était au REPOS, on ne fait rien");
        // ici on ne fait rien
        break;
    case ALLUME:
        DEBUG_PRINTLN("On était ALLUME, on passe à REPOS");
        etat = REPOS;
        break;
    case ALLUME_VERS_REPOS:
        DEBUG_PRINTLN("On était en ALLUME_VERS_REPOS, on passe à REPOS");
        etat = REPOS;
        break;
    default:
        etat = REPOS;
        break;
    }
}

// --------------------------------------------- private

// Comme on utilise souvent la gestion de l'état du relais, une petite fonction ne mange pas de pain ;)
void ZoneEclairage::setRelais(bool etatSouhaite)
{
    if(etatSouhaite != etatCourantRelais)
    {
        etatCourantRelais = etatSouhaite;
        digitalWrite(pinRelais, etatSouhaite); // on applique les changements
    }
}

void ZoneEclairage::ledClignoterDoucement()
{
    unsigned long maintenant = millis();
    if((maintenant - tempsPrecedentClignotement) >= 1) // 1 milliseconde
    {
        if(sensClignotement == INCREMENTER_LUM_LED)
        {
            if(luminosite == 100)
            {
                sensClignotement = DECREMENTER_LUM_LED;
                luminosite = 99;
            }
            else luminosite++;
        }
        else // si le sens est en décrémentation
        {
            if(luminosite == 0)
            {
                sensClignotement = INCREMENTER_LUM_LED;
                luminosite = 1;
            }
            else luminosite--;
        }

        ledAppliquerLum();

        tempsPrecedentClignotement = maintenant;
    }
}

void ZoneEclairage::ledClignoterRapidement()
{
    unsigned long maintenant = millis();
    if((maintenant - tempsPrecedentClignotement) >= 500) // 1/2 seconde
    {
        luminosite = (luminosite >= 1) ? 0 : 255;
        ledAppliquerLum();

        tempsPrecedentClignotement = maintenant;
    }
}

void ZoneEclairage::ledAppliquerLum()
{
    if(led != nullptr)
    {
        if(*led != couleur) *led = couleur;
        led->nscale8(luminosite);
    }
}