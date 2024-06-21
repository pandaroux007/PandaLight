#include <ZoneEclairage.h>

// définition du constructeur
ZoneEclairage::ZoneEclairage(byte pinBouton, byte pinRelais, CRGB & led)
: bouton(pinBouton, false /*bp actif sur HIGH puisque pas de PULLUP (LOW par defaut)*/),
led(led)
{
  // on met la pin du relais en sortie et on enregistre la pin en question
  this->pinRelais = pinRelais;
  pinMode(this->pinRelais, OUTPUT);
  // on attache le clique à la gestion de l'événement,
  // c'est à dire que fait-on dans quel état (utilisation fonction lambda)
  this->bouton.attachClick([](void *instance) {
      ZoneEclairage * cible= (ZoneEclairage *)instance;
      cible->checkEventClic();
  }, this);
  // et on fait pareil pour le clique long!
  this->bouton.attachLongPressStart([](void *instance) {
      ZoneEclairage * cible= (ZoneEclairage *)instance;
      cible->checkEventClicLong();
  }, this);
}

// définition des fonctions de gestion des événements pour les transitions
void ZoneEclairage::checkEventClic() // fonction appelée quand un clique simple est effectué
{
  Serial.print(F("Clique ! >> "));
  switch (etats)
  {
  case ZoneEclairage::ALLUME_COURT: Serial.println(F("On était en ALLUME_COURT, on ne fait rien")); break; // Si la lampe est déjà allumée, alors on ne fait rien, on sort...
  case ZoneEclairage::ALLUME_LONG: Serial.println(F("On était en ALLUME_LONG, on ne fait rien")); break; // pareil.
  case ZoneEclairage::REPOS: // Si on était au repos, alors on passe en mode allumé court (géré dans update)
    Serial.println(F("On était au REPOS, on passe en mode ALLUME_COURT"));
    tempsPrecedentClique = millis(); // on stocke le temps au moment du clique
    this->etats = ZoneEclairage::ALLUME_COURT; // on attribue le nouvel état
    break;
  case ZoneEclairage::ALLUME_VERS_REPOS: // Si on était au repos, alors on passe en mode allumé court (géré dans update)
    Serial.println(F("On était en ALLUME_VERS_REPOS, on repasse en mode ALLUME_COURT"));
    tempsPrecedentClique = millis(); // on stocke le temps au moment du clique
    this->etats = ZoneEclairage::ALLUME_COURT; // on attribue le nouvel état
    break;
  default: // Si état inconnu, bah on le print et on sort.
    Serial.println(F("Erreur dans switch/case checkEventClic !"));
    break;
  }
}

void ZoneEclairage::checkEventClicLong() // fonction appelée quand un clique long est effectué
{
  Serial.print(F("Clique Long ! >> "));
  switch (etats)
  {
  case ZoneEclairage::ALLUME_COURT: // si on était allumé sur un temps court,
    Serial.println(F("On était en ALLUME_COURT, on passe en mode REPOS"));
    this->etats = ZoneEclairage::REPOS; // bah on éteint tout.
    break;
  case ZoneEclairage::ALLUME_LONG: // si on était en mode allumé longtemps,
    Serial.println(F("On était au ALLUME_LONG, on bascule en mode REPOS"));
    this->etats = ZoneEclairage::REPOS; // alors on éteint tout en passant en mode REPOS
    break;
  case ZoneEclairage::REPOS: // si on était tout éteint,
    Serial.println(F("On était au REPOS, on bascule en mode ALLUME_LONG"));
    this->etats = ZoneEclairage::ALLUME_LONG; // alors on passe en mode allumé longtemps
    break;
  case ZoneEclairage::ALLUME_VERS_REPOS: // si on était sur le point de s'éteindre alors que l'on était allumé,
    Serial.println(F("On était en ALLUME_VERS_REPOS, on repasse en mode ALLUME_COURT"));
    tempsPrecedentClique = millis(); // on stocke le temps au moment du clique
    this->etats = ZoneEclairage::ALLUME_COURT; // alors on relance un temps court
    break;
  default: // Si état inconnu, bah on le print et on sort.
    Serial.println(F("Erreur dans switch/case checkEventClicLong !"));
    break;
  }
}

void ZoneEclairage::update()
{
  bouton.tick(); // on met à jour l'état du bouton
  // gestion relais
  switch (etats)
  {
  case ZoneEclairage::REPOS:
    led.setRGB(0, 0, 0);
    setRelais(RELAIS_OFF); // on éteint le relais
    break;
  case ZoneEclairage::ALLUME_COURT:
    setRelais(RELAIS_ON); // on allume le relais
    led.setRGB(255, 0, 0);
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT - TEMPS_AVANT_EXTENCTION))
    {
      Serial.println(F("Fin proche du temps court !"));
      this->etats = ZoneEclairage::ALLUME_VERS_REPOS;
      break;
    }
    break;
  case ZoneEclairage::ALLUME_VERS_REPOS:
    setRelais(RELAIS_ON); // on allume le relais
    led.setRGB(0, 255, 0);
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= (TEMPS_FONCTIONNEMENT_SANS_RAPPEL_COURT)) // Si le temps "long" est écoulé,
    {
      Serial.println(F("Fin du temps court, on passe en mode REPOS!"));
      this->etats = ZoneEclairage::REPOS; // alors on passe en mode repos
      break;
    }
    break;
  case ZoneEclairage::ALLUME_LONG:
    setRelais(RELAIS_ON); // on allume le relais
    led.setRGB(0, 0, 255);
    // gestion minuteur
    if((millis() - tempsPrecedentClique) >= TEMPS_FONCTIONNEMENT_SANS_RAPPEL_LONG) // si le temps long est passé
    {
      Serial.println(F("Fin du temps long, on passe en mode REPOS!"));
      this->etats = ZoneEclairage::REPOS; // alors on éteint tout (ouais là on s'embête pas à faire un rappel, le temps est suffisament long)
      break;
    }
    break;
  default:
    Serial.println(F("État inconnu dans update! Mode REPOS par défaut"));
    // setRelais(RELAIS_OFF); // pas besoin ici puis qu'au prochain tour de loop on verra qu'on est en REPOS et on coupera le relais et la led
    etats = REPOS;
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
    Serial.print(F("Relais défini sur ")); Serial.println(etatSouhaite);
  }
}