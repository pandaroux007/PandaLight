[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

# Présentation & fonctionnement
- Date de création : 2024/02/01.
- Auteur : @pandaroux007
- Licence : voir la [licence MIT](LICENCE.txt)
- Dépendances : voir le [platformio.ini](platformio.ini)

## Objectifs
- Le garage est divisé en plusieurs zones (par exemple, le spot principal, le spot du local à vélo, la guirlande électrique...), et la cave en plusieurs pièce. Le but est d'avoir un bouton pour chaque zone pour allumer la lumière qui l'éteigne au bout d'un certain temps si on ne rappuie pas dessus.
- Un bouton général, si appuyé, allume toutes les zones 3 à 5 qui ne sont pas allumées et redémarre le minuteur de celles qui le sont. Si toutes les zones sont allumées, un appui long va toutes les éteindre. Point particulier ? Pour ne pas faire un appel de courant trop fort au niveau du disjoncteur lorsque les 3 zones s'allument en même temps, le bouton général doit les allumer une à une, successivement, à 1s d'interval.

## Matériel
- boutons tactiles de type `TTP223`
- ruban de led de type `WS2812B`
- gaines + boitier de dérivation IP65

## Quelques détails importants :
- **Les leds RGB sont en série, c'est-à-dire que c'est en réalité un ruban de led avec une grande distance entre chacune.** Cela permet de n'avoir à gérer qu'une seule instance de FastLED, pour gérer la led correspondant à un bouton, on utilise son index ([tableau CRGB dans le code](https://github.com/FastLED/FastLED/blob/master/examples/Blink/Blink.ino#L18-18)). Cela permet également de n'utiliser qu'une seule pin de l'Arduino pour toutes les leds.

- **Les leds RGB indiquent l'état actuel dans lequel se trouve l'éclairage.** Par exemple, si la led clignote doucement (ce que l'on peut obtenir via PWM avec une simple led classique), c'est que la machine à état est au repos, qu'elle attend un événement (un clique ou un clique long). Si elle est complètement allumée, cela signifie que l'éclairage est allumé également. Enfin, si elle clignote rapidement, cela indique que l'éclairage s'éteindra dans peu de temps, et qu'il faut relancer le minuteur d'économie d'énergie avec un clique sur le bouton.

- **Les appuis sur les boutons ne font pas la même chose en fonction de l'état actuel de l'éclairage ainsi que du type de clique effectué.** Si on était en repos par exemple, et que l'on fait un clique simple ou long, cela allume l'éclairage. Inversement, si on était allumé, si on fait un clique long cela éteint l'éclairage - on passe en mode repos. Une image vaut mille mots, vous trouverez un schéma illustrant la machine à état du programme [dans le fichier schema.odg](<schema machine a etat.odg>).