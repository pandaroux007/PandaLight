# Présentation & fonctionnement
Ce projet commence à être imaginé en 2023, mais n'est réellement entamé d'un point de vu programmation que le 2024/02/01. Il a pour but de créer un système de gestion de l'éclairage dans un garage.

Des boutons tactiles de type `TTP223` seront disposés un peu partout, dans des petits boîtiers étanches imprimé en 3D, et associés à une LED RGB de type `WS2812B` également dans le boîtier. Les câbles sortant de celui-ci passeront par des gaines cachées dans la dalle de béton du garage pour rejoindre un boitier de dérivation IP65, dans lequel sera incrusté un écran OLED. Chaque bouton contrôle un éclairage en particulier, par exemple le spot principal du garage, le spot du local à vélo, la guirlande électique...

## Quelques détails importants :
**Les leds RGB sont en série, c'est à dire que c'est en réalité un ruban de led avec une grande distance entre chacune.** Cela me permet de n'avoir à gérer qu'une seul instance FastLED, pour gérer la led correspondant à un bouton, j'utilise sont index ([tableau CRGB dans le code](https://github.com/FastLED/FastLED/blob/master/examples/Blink/Blink.ino#L18-18)). Cela permet également de n'utiliser qu'une seule pin de l'Arduino pour toutes les leds.

**Les leds RGB indiquent l'état actuel dans lequel se trouve l'éclairage.** Vous trouverez un schéma expliquant la machine à état du programme dans le fichier [schema.odg](<schema machine a etat.odg>). Par exemple, si la led clignote doucement (ce que l'on peut obtenir via PWM avec une simple led classique), c'est que la machine à état est au repos, qu'elle attend un événement (un clique ou un clique long). Si elle est complètement allumée, cela signifie que l'éclairage est allumé également. Enfin, si elle clignote rapidement, cela indique que l'éclairage s'éteindra dans 5 minutes ou moins, et qu'il faut relancer le minuteur d'économie d'énergie avec un clique sur le bouton.

**Les appuis sur les boutons ne font pas la même chose en fonction de l'état actuel de l'éclairage ainsi que le types de cliques.** Si on était en repos, et que l'on fait un clique long, cela allume l'éclairage en mode long (pas de rappel, s'éteint automatiquement 5h plus tard). Inversement, si on était allumé (court ou long), cela éteint l'éclairage - on passe en mode repos. Un clique court active le mode allumé court (rappel possible, s’éteint 1H plus tard si pas de rappel)

> Dés que je projet sera terminé, j'ajouterai dans le README.md un lien vers son topic de présentation sur le forum Arduino francophone. Vous y trouverez des photos du projet installé et fonctionnel ainsi que des détails sur le code.

## Licence & Dépendances
Ce projet est sous [Licence MIT](LICENCE.txt) - [The MIT License (MIT)](https://choosealicense.com/licenses/mit/).
Retrouvez les détails de cette licence sur le site officiel : https://opensource.org/licenses/MIT.

Voir le fichier [platformio.ini](platformio.ini) pour les dépendances.

## Potentielles améliorations
J'envisage l'ajout d'une communication `RS485` avec le - futur - système domotique Arduino de chez moi, dont l'un des terminaux de commande sera situé à la cave (bah oui, mon petit espace de travail ne sera pas épargné!). Une partie des gaines sous la dalle du garage sortant dans celle-ci, il est parfaitement possible de relier le système de gestion du garage avec le système domotique, via une liaison `MODBUS`.

Cette liaison amène une autre amélioration possible : l'ajout de l'envoie de données météorologiques ! Effectivement, je compte ajouter un capteur de température, pression et humidité de type `BME280` (**et pas BMP280 comme je le croyais avant! En effet ce module ne capte pas l'humidité contrairement à son grand frère... J'en avais acheté 5 sans prendre la peine de regarder la différence entre les deux!**) au niveau du toit du garage ou directement derrière une grille dans le boîtier de dérivation (ce qui est dommage car cela rendrai l'étanchéité inutile). Ces données seraient affichées sur l'écran (sinon il n'afficherai que l'état courant des éclairages, ce qui, soyons honnêtes, ne sert strictement à rien.), et transmise aux réseau domotique de la maison via l'amélioration évoquée précédemment pour traitement et affichage.

# Crédits
1. Merci à @J-M-L pour son aide sur le forum arduino francophone,
2. Merci à @petitours pour ses conseils sur la manière de structurer mon programme,
3. Merci aux concepteurs des éléments électroniques utilisés dans ce projet,
4. Merci aux développeurs des bibliothèques utilisées dans ce programme, pour leurs travaux et leurs contributions à la communauté,
5. Enfin, merci à tous les créateurs de contenus techniques sur Internet qui me permettent d'en apprendre un peu plus chaque jour.