#pragma once
#include "globals.h"

Color traceRay(Point rayOrigin, float rayDx, float rayDy, float rayDz, int depth);

bool refract(float inDx, float inDy, float inDz,
    float nx, float ny, float nz,
    float eta,
    float& tx, float& ty, float& tz);

void reflect(float inDx, float inDy, float inDz,
    float nx, float ny, float nz,
    float& rx, float& ry, float& rz);

Color computePhongShading(Point hitPoint, float normalX, float normalY, float normalZ,
    Point viewPoint, Color ambient, Color diffuse, Color specular,
    float Ka, float Kd, float Ks, float Ke);

void output_ppm();
