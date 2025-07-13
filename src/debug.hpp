#pragma once

// ligne à commenter pour désactiver le debug et/ou les temps raccourcis
#define PRINT_DEBUG_ACTIVE
#define TEMPS_DEBUG_ACTIVE

#ifdef PRINT_DEBUG_ACTIVE
    #define DEBUG_INIT(vitesse) Serial.begin(vitesse)
    #define DEBUG_PRINTLN(texte) Serial.println(F(texte))
    #define DEBUG_PRINT(texte) Serial.print(F(texte))
    #define DEBUG_VALUE_PRINT_TAB(valeur) Serial.print(valeur); Serial.print(F("\t"))
    #define DEBUG_VALUE_PRINT(valeur) Serial.print(valeur)
#else
    #define DEBUG_INIT(vitesse)
    #define DEBUG_PRINTLN(texte)
    #define DEBUG_PRINT(texte)
    #define DEBUG_VALUE_PRINT_TAB(valeur)
    #define DEBUG_VALUE_PRINT(valeur)
#endif

#ifdef TEMPS_DEBUG_ACTIVE
    // temps courts pour les tests
    constexpr uint16_t TEMPS_FONCTIONNEMENT_TOTAL_DEFAUT = 30; // 30s
    constexpr uint16_t TEMPS_AVANT_EXTINCTION_DEFAUT = 10; // 10s
#else
    // 4h × 60mn × 60s × 1000ms, temps avant que la zone s'éteigne toute seule
    constexpr uint32_t TEMPS_FONCTIONNEMENT_TOTAL_DEFAUT = 14400; // 4h
    // 5mn × 60s × 1000ms, temps pendant lequel un clique (simple ou long) relance le minuteur
    constexpr uint32_t TEMPS_AVANT_EXTINCTION_DEFAUT = 300; // 5 minutes
#endif