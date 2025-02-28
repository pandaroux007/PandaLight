[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

# Présentation & fonctionnement
Ce projet commence à être imaginé en 2023, mais n'est réellement entamé d'un point de vue programmation que le 2024/02/01. Il a pour but de créer un système de gestion de l'éclairage dans une cave tout d'abord, puis dans un garage.

Des boutons tactiles de type `TTP223` seront disposés un peu partout, dans des petits boîtiers étanches imprimé en 3D, et associés à une LED RGB de type `WS2812B` également dans le boîtier. Les câbles sortant de celui-ci passeront par des gaines cachées dans la dalle de béton au sol pour rejoindre un boitier de dérivation IP65, dans lequel sera incrusté un écran OLED. Chaque bouton contrôle un éclairage en particulier, par exemple pour le garage, le spot principal, le spot du local à vélo, la guirlande électique...

## Quelques détails importants :
**Les leds RGB sont en série, c'est à dire que c'est en réalité un ruban de led avec une grande distance entre chacune.** Cela me permet de n'avoir à gérer qu'une seul instance FastLED, pour gérer la led correspondant à un bouton, j'utilise sont index ([tableau CRGB dans le code](https://github.com/FastLED/FastLED/blob/master/examples/Blink/Blink.ino#L18-18)). Cela permet également de n'utiliser qu'une seule pin de l'Arduino pour toutes les leds.

**Les leds RGB indiquent l'état actuel dans lequel se trouve l'éclairage.** Vous trouverez un schéma expliquant la machine à état du programme dans le fichier [schema.odg](<schema machine a etat.odg>). Par exemple, si la led clignote doucement (ce que l'on peut obtenir via PWM avec une simple led classique), c'est que la machine à état est au repos, qu'elle attend un événement (un clique ou un clique long). Si elle est complètement allumée, cela signifie que l'éclairage est allumé également. Enfin, si elle clignote rapidement, cela indique que l'éclairage s'éteindra dans 5 minutes ou moins, et qu'il faut relancer le minuteur d'économie d'énergie avec un clique sur le bouton.

**Les appuis sur les boutons ne font pas la même chose en fonction de l'état actuel de l'éclairage ainsi que le types de cliques.** Si on était en repos, et que l'on fait un clique long, cela allume l'éclairage en mode long (pas de rappel, s'éteint automatiquement 5h plus tard). Inversement, si on était allumé (court ou long), cela éteint l'éclairage - on passe en mode repos. Un clique court active le mode allumé court (rappel possible, s’éteint 1H plus tard si pas de rappel)

> Dès que je projet sera terminé, j'ajouterai dans le README.md un lien vers son topic de présentation sur le forum Arduino francophone. Vous y trouverez des photos du projet installé et fonctionnel ainsi que des détails sur le code.

## Licence & Dépendances
Ce projet est sous [Licence MIT](LICENCE.txt) - [The MIT License (MIT)](https://choosealicense.com/licenses/mit/).
Retrouvez les détails de cette licence sur le site officiel : https://opensource.org/licenses/MIT.

Voir le fichier [platformio.ini](platformio.ini) pour les dépendances.