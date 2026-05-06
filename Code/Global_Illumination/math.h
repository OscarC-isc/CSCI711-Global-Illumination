#pragma once
#include "globals.h"

float dotProduct(float dx1, float dy1, float dz1,
    float dx2, float dy2, float dz2);

void normalize(float& dx, float& dy, float& dz);

float vectorLength(float dx, float dy, float dz);

void crossProduct(float ax, float ay, float az,
    float bx, float by, float bz,
    float& cx, float& cy, float& cz);

float randf();