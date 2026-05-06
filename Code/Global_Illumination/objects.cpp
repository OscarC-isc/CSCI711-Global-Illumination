#include "objects.h"


// Ray-sphere intersection
bool intersectSphere(Point rayOrigin, float rayDx, float rayDy, float rayDz,
    int objIndex, float& t) {
    float ox = rayOrigin.x - object[objIndex].center.x;
    float oy = rayOrigin.y - object[objIndex].center.y;
    float oz = rayOrigin.z - object[objIndex].center.z;

    float a = rayDx * rayDx + rayDy * rayDy + rayDz * rayDz;
    float b = 2.0f * (ox * rayDx + oy * rayDy + oz * rayDz);
    float c = ox * ox + oy * oy + oz * oz - object[objIndex].radius * object[objIndex].radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0) {
        return false;
    }

    float sqrtDisc = sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);

    if (t1 > 0.001f) {
        t = t1;
        return true;
    }
    else if (t2 > 0.001f) {
        t = t2;
        return true;
    }

    return false;
}

// Ray-floor intersection
bool intersectFloor(Point rayOrigin, float rayDx, float rayDy, float rayDz,
    int objIndex, float& t) {
    float nx = object[objIndex].normal.dx;
    float ny = object[objIndex].normal.dy;
    float nz = object[objIndex].normal.dz;

    float denominator = rayDx * nx + rayDy * ny + rayDz * nz;

    if (fabs(denominator) < 0.0001f) {
        return false;
    }

    float px = object[objIndex].vertex[0].x;
    float py = object[objIndex].vertex[0].y;
    float pz = object[objIndex].vertex[0].z;

    t = ((px - rayOrigin.x) * nx + (py - rayOrigin.y) * ny + (pz - rayOrigin.z) * nz) / denominator;

    if (t < 0.001f) {
        return false;
    }

    float hitX = rayOrigin.x + t * rayDx;
    float hitY = rayOrigin.y + t * rayDy;
    float hitZ = rayOrigin.z + t * rayDz;

    if (hitX < object[objIndex].xmin || hitX > object[objIndex].xmax ||
        hitZ < object[objIndex].zmin || hitZ > object[objIndex].zmax) {
        return false;
    }

    return true;
}

int getFloorColorIndex(float x, float z, int objIndex) {
    float checkSize = object[objIndex].checksize;

    float relX = x - object[objIndex].xmin;
    float relZ = z - object[objIndex].zmin;

    int checkX = (int)(relX / checkSize);
    int checkZ = (int)(relZ / checkSize);

    return (checkX + checkZ) % 2;
}

// New wall intersection function
bool intersectWall(Point rayOrigin, float rayDx, float rayDy, float rayDz,
    int objIndex, float& t) {
    // Plane intersection
    float nx = object[objIndex].normal.dx;
    float ny = object[objIndex].normal.dy;
    float nz = object[objIndex].normal.dz;

    float denominator = rayDx * nx + rayDy * ny + rayDz * nz;
    if (fabs(denominator) < 0.0001f) return false;

    float px = object[objIndex].vertex[0].x;
    float py = object[objIndex].vertex[0].y;
    float pz = object[objIndex].vertex[0].z;

    t = ((px - rayOrigin.x) * nx + (py - rayOrigin.y) * ny + (pz - rayOrigin.z) * nz) / denominator;
    if (t < 0.001f) return false;

    float hitX = rayOrigin.x + t * rayDx;
    float hitY = rayOrigin.y + t * rayDy;
    float hitZ = rayOrigin.z + t * rayDz;

    // Check if inside quad (using bounding box for simplicity)
    if (hitX < object[objIndex].xmin || hitX > object[objIndex].xmax ||
        hitY < object[objIndex].ymin || hitY > object[objIndex].ymax ||
        hitZ < object[objIndex].zmin || hitZ > object[objIndex].zmax) {
        return false;
    }
    return true;
}