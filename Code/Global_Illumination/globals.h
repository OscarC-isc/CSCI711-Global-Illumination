#pragma once

#include <cmath>
#include <limits>
#include <iostream>

struct Point { float x, y, z; };
struct Vec3 { Point start; float dx, dy, dz; };
struct Color { float red, green, blue; };

struct Camera {
    Point position;
    Point lookAt;
    Vec3 up;
    float focalLength;
};

struct Light {
    Point position;
    Color color;
    Color ambientColor;
};

struct Object {
    char letter;
    Color ambient[2], diffuse[2], specular[2];
    float refractionIndex;
    float Ka, Kd, Ks, Ke, Kr, Kt;

    Point center;
    float radius;

    Point vertex[4];
    float checksize;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    Vec3 normal;
    float F;
};

enum RenderMode {
    WHITTED,
    PHOTON
};

extern RenderMode renderMode;

// globals
extern Camera camera;
extern Light light;
extern Object object[3];
extern Color background;

extern float worldWidth, worldHeight;
extern int screenWidth, screenHeight, numberOfLevels;