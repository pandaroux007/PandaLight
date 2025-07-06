#include "ZoneEclairage.hpp"

void ZoneEclairage::begin(const uint8_t pinBouton, const uint8_t pinRelais, uint16_t* settingsModbus, CRGB* led, CRGB couleur)
{
    m_couleur = couleur;
    m_led = led;
    m_pinRelais = pinRelais;
    m_settingsModbus = settingsModbus;
    
    pinMode(pinRelais, OUTPUT);
    setup(pinBouton, INPUT_PULLUP, ACTIVE_LOW);

    // on attache le click simple à la gestion d'un événement
    attachClick([](void *instance) {
        ((ZoneEclairage *)instance)->callbackClick();
    }, this);
    // et on fait pareil pour le click long
    attachLongPressStart([](void *instance) {
        ((ZoneEclairage *)instance)->callbackClickLong();
    }, this);
}

void ZoneEclairage::update()
{
    tick(); // màj état bouton

    // gestion relais & led
    switch (etat)
    {
    case REPOS:
        setEtatRelais(RELAIS_OFF);
        ledClignoterDoucement();
        break;
    case ALLUME:
        setEtatRelais(RELAIS_ON);
        ledAllumerCompletement();
        // gestion minuteur
        if((millis() - tempsPrecedentClique) >= (getTempsFonctionnement() - getTempsAvantExtinction())) // si temps presque écoulé
        {
            DEBUG_PRINTLN(">> Fin du temps proche!");
            etat = ALLUME_VERS_REPOS;
        }
        break;
    case ALLUME_VERS_REPOS:
        setEtatRelais(RELAIS_ON);
        ledClignoterRapidement();
        // gestion minuteur
        if((millis() - tempsPrecedentClique) >= getTempsFonctionnement()) // si temps complètement écoulé
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

void ZoneEclairage::callbackClick()
{
    DEBUG_PRINT("click >> ");
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

void ZoneEclairage::callbackClickLong()
{
    DEBUG_PRINT("click long >> ");
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

void ZoneEclairage::setEtatRelais(bool etatSouhaite)
{
    if(etatSouhaite != etatCourantRelais)
    {
        etatCourantRelais = etatSouhaite;
        digitalWrite(m_pinRelais, etatSouhaite); // on applique les changements
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
    if(m_led != nullptr)
    {
        if(*m_led != m_couleur) *m_led = m_couleur;
        m_led->nscale8(luminosite);
    }
}