# seeds

## Introduction
**seeds** is a game with its own engine written in C++ using OpenGL.

The game concept is strongly inspired by the game [Dyson](https://www.dyson-game.com/), which I used to play in my childhood. 

The main goal of the project was to develop an engine that is able to handle a slightly complicated game. It was never the goal to develop an entertaining game. That's why the gameplay is certainly in need of improvement. 


The following libraries were used:
* [loki](http://loki-lib.sourceforge.net/)
* [freetype](https://www.freetype.org/)
* [log4cpp](http://log4cpp.sourceforge.net/)
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
* [GLEW](http://glew.sourceforge.net/)
* [GLFW](https://www.glfw.org/)
* [glm](https://github.com/g-truc/glm)

## Description
The game takes place in a grey void somewhere in space.
The only things in this void are planets, on which trees may grow. These trees in turn spawn seeds, who grow new trees, fight enemy seeds and conquer enemey planets.

![planet.png](https://github.com/halpersim/seeds/blob/master/readme/planet.png)

It is possible to grow new trees and send seeds to enemy planets via the head up display.

![create_tree.gif](https://github.com/halpersim/seeds/blob/master/readme/create_tree.gif) ![move_soldiers.gif](https://github.com/halpersim/seeds/blob/master/readme/move_soldiers.gif)

Once seeds arrive on a foreign planet, they start to attack the enemy seeds and will try to take over the planet by removing one tree and getting into the planets interior.

![planets_conquering.gif](https://github.com/halpersim/seeds/blob/master/readme/planets_conquering.gif)
