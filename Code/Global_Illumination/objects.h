#pragma once
#include "globals.h"

bool intersectSphere(Point origin, float dx, float dy, float dz,
    int objIndex, float& t);

bool intersectFloor(Point origin, float dx, float dy, float dz,
    int objIndex, float& t);

int getFloorColorIndex(float x, float z, int objIndex);

bool intersectWall(Point origin, float dx, float dy, float dz,
	int objIndex, float& t);