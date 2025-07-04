[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

# ✨ Présentation & fonctionnement
- Date de création : 2024/02/01.
- Auteur : @pandaroux007
- Licence : voir la [licence MIT](LICENCE.txt)
- Dépendances : voir le [platformio.ini](platformio.ini)

## 🎯 Objectifs
Mon garage est divisé en plusieurs zones (un grand espace principal + un petit local à vélo), et la cave en plusieurs pièce. Le principe de `PandaLight` est d'arriver à un système permettant de contrôler normalement les éclairages de chaque zones via un coupte optotriac + triac (un équivalent aux relais), séparément, avec un système de rappel. Si on ne rappuie pas sur le bouton commandant l'éclairage d'une zone, au bout d'un moment, celle-ci s'éteint. Cela permet d'éviter les étourderies quand on part en vacances, ou juste de faire des économies lorsqu'on oublie d'éteindre la lumière 💡.

Une carte `PandaLight` est installée dans chaque espace subdivisé en zones à gérer que l'on souhaite contrôler, ici la cave et le garage. On dispose plusieurs boutons aux endroits stratégiques, ainsi qu'un ou plusieurs "bouton général". Si appuyé, ce dernier allume toutes les zones (3 à 5) qui ne sont pas allumées et redémarre le minuteur de celles qui le sont. Si toutes les zones sont allumées, un appui long va toutes les éteindre. ***Point particulier ?*** Pour ne pas faire un appel de courant trop fort et éviter des coupures intempestives au niveau du disjoncteur lorsque les 3 zones s'allument en même temps, le bouton général doit les allumer une à une, successivement, à 1s d'interval.

Chaque zone est contrôlée par un bouton, et celui-ci est toujours accompagné d'une led RGB, qui permet d'indiquer en temps réel l'état actuel de l'éclairage. Par exemple, lorsque la zone est éteinte, la led RGB correspondante clignote doucement, à la manière des leds classiques commandée en PWM. Si l'on clique si le bouton, la zone s'allume, au même titre que la led. Enfin, lorsque la fin du temps de fonctionnement de la zone est proche, la led clignote rapidement, pour indiquer à l'utilisateur de relancer le minuteur d'extinction. En sachant que la couleur est paramétrable, vu qu'il y a plusieurs zones.

Dernier point, chaque carte `PandaLight` est équipée d'un driver RS485 pour pouvoir communiquer en modbus et d'un port I2C pour y brancher un capteur de luminosité et un BME280 (et pourquoi pas, un écran OLED ou LCD).

## ⚙️ Fonctionnement
### Dépendances :
> [!NOTE]
> **Attention!** Les dépendances présentées ici, risquent de changer dans le code avec le temps sans que cette section ne soit actualisée!

J'utilise la bibliothèque `OneButton` pour gérer les appuis longs et simples sur les boutons. Je n'utilise pas `OneButtonTiny` malheureusement, car celle-ci ne propose pas de moyen de binder (entendez "lier") un click sur le bouton, avec une fonction membre de la classe `ZoneEclairage`, et encore, avec OneButton je suis obligé de passer par une fonction lambda dans la fonction `begin` de [ZoneEclairage.cpp](src/ZoneEclairage.cpp):
```cpp
void ZoneEclairage::begin(...)
{
    ...

    // on attache le click simple à la gestion d'un événement
    attachClick([](void *instance) {
        ((ZoneEclairage *)instance)->callbackClick();
    }, this);
    // et on fait pareil pour le click long
    attachLongPressStart([](void *instance) {
        ((ZoneEclairage *)instance)->callbackClickLong();
    }, this);
}
```

Ensuite, `FastLED`, qui me permet de gérer le bandeau de led simplement via un tableau `CRGB`, de faire varier la luminosité, etc. Pour le capteur BME280 (car oui, chaque carte `PandaLight` en compte un), j'utilise la bibliothèque de Sparkfun, `SparkFunBME280`, bien que mon module ne soit pas de cette marque. Ce capteur, de la marque BOSH, permet de mesurer la température, l'humidité ainsi que la pression (moins utile dans mon cas). Le miens est relié en I2C

Enfin, les cartes `PandaLight` utilise un driver RS485, de type N65HVD72D, afin de pouvoir communiquer *via* le protocole modbus. Pour utiliser le protocole facilement et communiquer à un serveur Modbus TCP maître qui récoltera les données et paramétrera les cartes, via la gateway TCP/RTU, j'utilise la bibliothèque `ModbusRTUSlave`.