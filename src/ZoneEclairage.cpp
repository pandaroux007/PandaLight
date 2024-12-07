#include "ZoneEclairage.hpp"

// --------------------------------------------- public

/// @brief fonction d'initialisation, à appeler dans le setup normalement
/// @param none ne prend pas de paramètres
void ZoneEclairage::begin(const uint8_t passedPinBouton, const uint8_t passedPinRelais, CRGB * passedLed, CRGB passedCouleur) {
  couleur = passedCouleur;
  pinRelais = passedPinRelais;
  pinMode(pinRelais, OUTPUT);
  bouton.setup(passedPinBouton, INPUT_PULLUP, true);
  // on attache le clique simple à la gestion d'un événement
  bouton.attachClick([](void *instance) {
    ((ZoneEclairage *)instance)->checkEventClic();
  }, this);
  // et on fait pareil pour le clique long
  bouton.attachLongPressStart([](void *instance) {
    ((ZoneEclairage *)instance)->checkEventClicLong();
  }, this);
}

/// @brief fonction de mise à jour de la machine à état
/// @warning À appeller le plus souvent possible!
void ZoneEclairage::update(void)
{
  bouton.tick(); // on met à jour l'état du bouton
  // gestion relais & led
  switch (etat)
  {
  case REPOS:
    setRelais(RELAIS_OFF);
    ledClignoterDoucement();
    break;
  case ALLUME_COURT:
    setRelais(RELAIS_ON);
    *led = couleur;
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT - TEMPS_AVANT_EXTENCTION))
    {
      DEBUG_PRINTLN(F(">> Fin proche du temps court !"));
      etat = ALLUME_VERS_REPOS;
    }
    break;
  case ALLUME_VERS_REPOS:
    setRelais(RELAIS_ON);
    ledClignoterRapidement();
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT)) // Si le temps "long" est écoulé,
    {
      DEBUG_PRINTLN(F(">> Fin du temps court, on passe en mode REPOS!"));
      etat = REPOS; // alors on passe en mode repos
    }
    break;
  case ALLUME_LONG:
    setRelais(RELAIS_ON);
    *led = couleur;
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG) // si le temps long est passé
    {
      DEBUG_PRINTLN(F(">> Fin du temps long, on passe en mode REPOS!"));
      etat = REPOS; // alors on éteint tout (ouais là on s'embête pas à faire un rappel, le temps est suffisament long)
    }
    break;
  default:
    DEBUG_PRINTLN(F("ERREUR >> État inconnu dans update! Mode REPOS par défaut"));
    etat = REPOS;
    break;
  }

  FastLED.show(); // on affiche les changement sur la led
}

/// @brief fonction appelée quand un clique simple est effectué
void ZoneEclairage::checkEventClic(void)
{
  DEBUG_PRINT(F("\tClique ! >> "));
  switch (etat)
  {
  case ALLUME_COURT:
    DEBUG_PRINTLN(F("On était en ALLUME_COURT, on relance le décompte"));
    tempsPrecedentClique = millis();
    break;
  case ALLUME_LONG:
    DEBUG_PRINTLN(F("On était en ALLUME_LONG, on relance le décompte"));
    tempsPrecedentClique = millis();
    break;
  case REPOS: // Si on était au repos, alors on passe en mode allumé court (géré dans update)
    DEBUG_PRINTLN(F("On était au REPOS, on passe en mode ALLUME_COURT"));
    tempsPrecedentClique = millis();
    etat = ALLUME_COURT; // on attribue le nouvel état
    break;
  case ALLUME_VERS_REPOS: // Si on était au repos, alors on passe en mode allumé court (géré dans update)
    DEBUG_PRINTLN(F("On était en ALLUME_VERS_REPOS, on repasse en mode ALLUME_COURT"));
    tempsPrecedentClique = millis();
    etat = ALLUME_COURT; // on attribue le nouvel état
    break;
  default:
    DEBUG_PRINTLN(F("\t\tERREUR >> Etat inconnu dans checkEventClic !, on bascule en mode REPOS"));
    etat = REPOS;
    break;
  }
}

/// @brief fonction appelée quand un clique long est effectué
void ZoneEclairage::checkEventClicLong(void)
{
  DEBUG_PRINT(F("\tClic Long ! >> "));
  switch (etat)
  {
  case ALLUME_COURT: // si on était allumé sur un temps court,
    DEBUG_PRINTLN(F("On était en ALLUME_COURT, on passe en mode REPOS"));
    etat = REPOS; // on éteint tout.
    break;
  case ALLUME_LONG: // si on était en mode allumé longtemps,
    DEBUG_PRINTLN(F("On était au ALLUME_LONG, on bascule en mode REPOS"));
    etat = REPOS; // alors on éteint tout en passant en mode REPOS
    break;
  case REPOS: // si on était tout éteint,
    DEBUG_PRINTLN(F("On était au REPOS, on bascule en mode ALLUME_LONG"));
    etat = ALLUME_LONG; // alors on passe en mode allumé longtemps
    break;
  case ALLUME_VERS_REPOS: // si on était sur le point de s'éteindre alors que l'on était allumé,
    DEBUG_PRINTLN(F("On était en ALLUME_VERS_REPOS, on repasse en mode ALLUME_COURT"));
    tempsPrecedentClique = millis();
    etat = ALLUME_COURT; // alors on relance un temps court
    break;
  default:
    DEBUG_PRINTLN(F("ERREUR >> Etat inconnu dans checkEventClicLong! Mode REPOS par défaut"));
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
    digitalWrite(pinRelais, etatSouhaite); // on applique les changements, puis on le print à la ligne suivante
    DEBUG_PRINT(F(">> relais défini sur ")); DEBUG_PRINTLN(etatSouhaite);
  }
}

void ZoneEclairage::ledClignoterDoucement(void)
{
  if((millis() - tempsPrecedentClignotement) >= 1ul) // une milliseconde
  {
    if(sensIncrementation == incrementationLumLed)
    {
      if(luminosite == 100)
      {
        sensIncrementation = false;
        luminosite = 99;
      }
      else luminosite++;
    }
    else // si le sens est en décrémentation
    {
      if(luminosite == 0)
      {
        sensIncrementation = true;
        luminosite = 1;
      }
      else luminosite--;
    }

    if(*led != couleur) *led = couleur;
    led->nscale8(luminosite);

    tempsPrecedentClignotement = millis();
  }
}

void ZoneEclairage::ledClignoterRapidement(void)
{
  if((millis() - tempsPrecedentClignotement) >= 500) // une demie seconde
  {
    luminosite >= 1 ? luminosite = 0 : luminosite = 255;
    if(*led != couleur) *led = couleur;
    led->nscale8(luminosite);

    tempsPrecedentClignotement = millis();
  }
}