#include "math.h"
#include <cmath>


float dotProduct(float dx1, float dy1, float dz1,
    float dx2, float dy2, float dz2)
{
    return dx1 * dx2 + dy1 * dy2 + dz1 * dz2;
}

void normalize(float& dx, float& dy, float& dz)
{
    float len = sqrt(dx * dx + dy * dy + dz * dz);
    if (len > 0.0001f) {
        dx /= len; dy /= len; dz /= len;
    }
}

float vectorLength(float dx, float dy, float dz)
{
    return sqrt(dx * dx + dy * dy + dz * dz);
}

void crossProduct(float ax, float ay, float az,
    float bx, float by, float bz,
    float& cx, float& cy, float& cz)
{
    cx = ay * bz - az * by;
    cy = az * bx - ax * bz;
    cz = ax * by - ay * bx;
}

float randf() {
    return (rand() + 0.5f) / (RAND_MAX + 1.0f);
}