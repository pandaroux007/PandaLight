#ifndef DEBUG_HEADER
#define DEBUG_HEADER

// ligne à commenter pour désactiver le debug + les temps raccourcis
#define MODE_TEST_ACTIVE

#ifdef MODE_TEST_ACTIVE
    #define DEBUG_PRINTLN(x) Serial.println(F(x))
    #define DEBUG_PRINT(x) Serial.print(F(x))
    #define DEBUG_VALUE_PRINTLN(x) Serial.println(x)
    #define DEBUG_VALUE_PRINT(x) Serial.print(x)
#else
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINT(x)
    #define DEBUG_VALUE_PRINTLN(x)
    #define DEBUG_VALUE_PRINT(x)
#endif

#ifdef MODE_TEST_ACTIVE
    // temps courts pour les tests
    constexpr uint16_t TEMPS_FONCTIONNEMENT_TOTAL = 30000; // 30s
    constexpr uint16_t TEMPS_AVANT_EXTINCTION = 10000; // 10s
#else
    // 4h × 60mn × 60s × 1000ms, temps avant que la zone s'éteigne toute seule
    constexpr uint32_t TEMPS_FONCTIONNEMENT_TOTAL = 14400000; // 4h
    // 5mn × 60s × 1000ms, temps pendant lequel un clique (simple ou long) relance le minuteur
    constexpr uint32_t TEMPS_AVANT_EXTINCTION = 300000; // 5 minutes
#endif

#endif // DEBUG_HEADER