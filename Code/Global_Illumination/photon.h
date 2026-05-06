#pragma once

#include "globals.h"

struct Photon {
    Point position;
    float dirX, dirY, dirZ;   // incoming direction
    Color power;
};