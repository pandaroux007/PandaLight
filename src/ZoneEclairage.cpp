#include "ZoneEclairage.h"

ZoneEclairage::ZoneEclairage(const byte passedPinBouton, const byte passedPinRelais, CRGB & passedLed, CRGB passedCouleur)
: bouton(passedPinBouton), led(passedLed)
{
  couleur = passedCouleur;
  pinRelais = passedPinRelais;
  pinBouton = passedPinBouton;
}

// définition des fonctions de gestion des événements pour les transitions
void ZoneEclairage::checkEventClic(void) // fonction appelée quand un clique simple est effectué
{
  DEBUG_PRINT(F("\tClique ! >> "));
  switch (etats)
  {
  case ZoneEclairage::ALLUME_COURT: DEBUG_PRINTLN(F("On était en ALLUME_COURT, on ne fait rien")); break; // Si la lampe est déjà allumée, alors on ne fait rien, on sort...
  case ZoneEclairage::ALLUME_LONG: DEBUG_PRINTLN(F("On était en ALLUME_LONG, on ne fait rien")); break; // pareil.
  case ZoneEclairage::REPOS: // Si on était au repos, alors on passe en mode allumé court (géré dans update)
    DEBUG_PRINTLN(F("On était au REPOS, on passe en mode ALLUME_COURT"));
    tempsPrecedentClique = millis(); // on stocke le temps au moment du clique
    etats = ZoneEclairage::ALLUME_COURT; // on attribue le nouvel état
    break;
  case ZoneEclairage::ALLUME_VERS_REPOS: // Si on était au repos, alors on passe en mode allumé court (géré dans update)
    DEBUG_PRINTLN(F("On était en ALLUME_VERS_REPOS, on repasse en mode ALLUME_COURT"));
    tempsPrecedentClique = millis(); // on stocke le temps au moment du clique
    etats = ZoneEclairage::ALLUME_COURT; // on attribue le nouvel état
    break;
  default: // Si état inconnu, bah on le print et on sort.
    DEBUG_PRINTLN(F("\t\tERREUR >> Etat inconnu dans checkEventClic !, on bascule en mode REPOS"));
    etats = ZoneEclairage::REPOS;
    break;
  }
}

void ZoneEclairage::checkEventClicLong(void) // fonction appelée quand un clique long est effectué
{
  DEBUG_PRINT(F("\tClic Long ! >> "));
  switch (etats)
  {
  case ZoneEclairage::ALLUME_COURT: // si on était allumé sur un temps court,
    DEBUG_PRINTLN(F("On était en ALLUME_COURT, on passe en mode REPOS"));
    etats = ZoneEclairage::REPOS; // bah on éteint tout.
    break;
  case ZoneEclairage::ALLUME_LONG: // si on était en mode allumé longtemps,
    DEBUG_PRINTLN(F("On était au ALLUME_LONG, on bascule en mode REPOS"));
    etats = ZoneEclairage::REPOS; // alors on éteint tout en passant en mode REPOS
    break;
  case ZoneEclairage::REPOS: // si on était tout éteint,
    DEBUG_PRINTLN(F("On était au REPOS, on bascule en mode ALLUME_LONG"));
    etats = ZoneEclairage::ALLUME_LONG; // alors on passe en mode allumé longtemps
    break;
  case ZoneEclairage::ALLUME_VERS_REPOS: // si on était sur le point de s'éteindre alors que l'on était allumé,
    DEBUG_PRINTLN(F("On était en ALLUME_VERS_REPOS, on repasse en mode ALLUME_COURT"));
    tempsPrecedentClique = millis(); // on stocke le temps au moment du clique
    etats = ZoneEclairage::ALLUME_COURT; // alors on relance un temps court
    break;
  default: // Si état inconnu, bah on le print et on sort.
    DEBUG_PRINTLN(F("ERREUR >> Etat inconnu dans checkEventClicLong! Mode REPOS par défaut"));
    etats = ZoneEclairage::REPOS;
    break;
  }
}

void ZoneEclairage::begin() {
  // init du relais
  pinMode(pinRelais, OUTPUT);
  // paramètrage du bouton
  bouton.setup(pinBouton, INPUT_PULLUP, true);
  // on attache le clique simple à la gestion d'un événement
  bouton.attachClick([](void *instance) {
    ((ZoneEclairage *)instance)->checkEventClic();
  }, this);
  // et on fait pareil pour le clique long
  bouton.attachLongPressStart([](void *instance) {
    ( (ZoneEclairage *)instance)->checkEventClicLong();
  }, this);
}

void ZoneEclairage::update(void) // fonction à appeller le plus souvent possible
{
  bouton.tick(); // on met à jour l'état du bouton
  // gestion relais
  switch (etats)
  {
  case ZoneEclairage::REPOS:
    setRelais(RELAIS_OFF); // on éteint le relais
    ledClignoterDoucement();
    break;
  case ZoneEclairage::ALLUME_COURT:
    setRelais(RELAIS_ON); // on allume le relais
    led = couleur;
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT - TEMPS_AVANT_EXTENCTION))
    {
      DEBUG_PRINTLN(F(">> Fin proche du temps court !"));
      etats = ZoneEclairage::ALLUME_VERS_REPOS;
    }
    break;
  case ZoneEclairage::ALLUME_VERS_REPOS:
    setRelais(RELAIS_ON); // on allume le relais
    ledClignoterRapidement();
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT)) // Si le temps "long" est écoulé,
    {
      DEBUG_PRINTLN(F(">> Fin du temps court, on passe en mode REPOS!"));
      etats = ZoneEclairage::REPOS; // alors on passe en mode repos
    }
    break;
  case ZoneEclairage::ALLUME_LONG:
    setRelais(RELAIS_ON); // on allume le relais
    led = couleur;
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG) // si le temps long est passé
    {
      DEBUG_PRINTLN(F(">> Fin du temps long, on passe en mode REPOS!"));
      etats = ZoneEclairage::REPOS; // alors on éteint tout (ouais là on s'embête pas à faire un rappel, le temps est suffisament long)
    }
    break;
  default:
    DEBUG_PRINTLN(F("ERREUR >> État inconnu dans update! Mode REPOS par défaut"));
    // setRelais(RELAIS_OFF); // pas besoin ici puis qu'au prochain tour de loop on verra qu'on est en REPOS et on coupera le relais et la led
    etats = ZoneEclairage::REPOS;
    break;
  }

  FastLED.show(); // on affiche les changement sur la led
}

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

bool ZoneEclairage::getEtatCourant(void)
{
  switch (etats)
  {
  case ZoneEclairage::REPOS:
    return(false); //0
    break;
  case ZoneEclairage::ALLUME_COURT:
    return(true); //1
    break;
  case ZoneEclairage::ALLUME_LONG:
    return(true); //1
    break;
  case ZoneEclairage::ALLUME_VERS_REPOS:
    return(true); //1
    break;
  default:
    DEBUG_PRINT(F("ERREUR >> Etat inconnu dans getEtatCourant, impossible de retourner l'état courant - 0 par défaut"));
    return(false);
    break;
  }
}

// Fonctions de gestion des Leds rgb
void ZoneEclairage::ledClignoterDoucement(void)
{
  if((millis() - tempsPrecedentClignotement) >= 1ul) // une milliseconde
  {
    if (sensIncrementation == incrementationLumLed)
    {
      if (luminosite == 255)
      {
        sensIncrementation = false;
        luminosite = 254;
      }
      else luminosite++;
    }
    else // si le sens est en décrémentation
    {
      if (luminosite == 0)
      {
        sensIncrementation = true;
        luminosite = 1;
      }
      else luminosite--;
    }

    if(led != couleur) led = couleur;
    led.nscale8(luminosite);

    tempsPrecedentClignotement = millis();
  }
}

void ZoneEclairage::ledClignoterRapidement(void)
{
  if((millis() - tempsPrecedentClignotement) >= 500) // une demie seconde
  {
    luminosite >= 1 ? luminosite = 0 : luminosite = 255;
    if(led != couleur) led = couleur;
    led.nscale8(luminosite);

    tempsPrecedentClignotement = millis();
  }
}