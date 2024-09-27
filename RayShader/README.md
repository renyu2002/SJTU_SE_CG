# notice
this repo is copied and slightly modified from https://github.com/DomNomNom/RayShader for education usage

## build prerequest
only tested on ubuntu
* sudo apt-get install libpng-dev
* sudo apt-get install freeglut3-dev

RayShader
=========

A realtime ray tracer using GLSL

![Screenshot](https://raw.github.com/DomNomNom/RayShader/master/screenshot2.png)

![Screenshot](https://raw.github.com/DomNomNom/RayShader/master/screenshot.png)


External dependencies:
----------------------
 * OpenGL2.0+
 * glut
 * libpng


How to run:
-----------
 > make && ./build/RayShader


Controls:
----------
 * To go into Path Tracing mode (top screenshot) press q then 6
 * Mouse drag - change camera angle
 * Mouse right drag - change ball position
 * Scrollwheel - zoom
 * [1-5] - change scene
 * Z - create water ripple
 * V - create vortex (positioned under ball)
 * W - toggle water
 * E - toggle objects
 * R - toggle whether one sphere is refractive (applies only to scene 1 and 3)
 * ] / [ -  increase/decrease shadow samples
 * P - reset shadow samples to 0
 * L - toggle environment
 * A / S / D - Switch rendering mode (S is back to RayShading)

