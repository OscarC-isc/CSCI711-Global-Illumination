#include "readScene.h"
#include "globals.h"
#include "math.h"
#include <iostream>
#include <algorithm>

using namespace std;


void readCamera() {
    cout << "DEBUG: About to read camera parameters...\n";

    if (!(cin >> camera.position.x >> camera.position.y >> camera.position.z
        >> camera.lookAt.x >> camera.lookAt.y >> camera.lookAt.z
        >> camera.up.dx >> camera.up.dy >> camera.up.dz
        >> camera.focalLength >> worldWidth >> worldHeight)) {
        cerr << "ERROR: Failed to read camera parameters!\n";
        cerr << "Stream state - fail: " << cin.fail() << ", bad: " << cin.bad() << ", eof: " << cin.eof() << "\n";
        return;
    }

    cout << "Camera:\n position ("
        << camera.position.x << ", "
        << camera.position.y << ", "
        << camera.position.z << ")\n";
    cout << " lookAt (" << camera.lookAt.x << ", " << camera.lookAt.y << ", " << camera.lookAt.z << ")\n";
    cout << " focal length: " << camera.focalLength << "\n";
    cout << " world size: " << worldWidth << " x " << worldHeight << "\n";
}

void readLight() {
    cout << "\nDEBUG: About to read light parameters...\n";

    if (!(cin >> light.position.x >> light.position.y >> light.position.z
        >> light.color.red >> light.color.green >> light.color.blue
        >> light.ambientColor.red >> light.ambientColor.green >> light.ambientColor.blue)) {
        cerr << "ERROR: Failed to read light parameters!\n";
        return;
    }

    cout << "Light:\n position("
        << light.position.x << ", "
        << light.position.y << ", "
        << light.position.z << ")\n";
    cout << " color (" << light.color.red << ", " << light.color.green << ", " << light.color.blue << ")\n";
}

void readParameters() {
    cout << "\nDEBUG: About to read parameters...\n";

    if (!(cin >> screenWidth >> screenHeight >> numberOfLevels)) {
        cerr << "ERROR: Failed to read parameters!\n";
        return;
    }

    cout << "screen width = " << screenWidth
        << ", screen height = " << screenHeight
        << ", number of levels = " << numberOfLevels << "\n";
}

void readBackground() {
    cout << "\nDEBUG: About to read background...\n";

    if (!(cin >> background.red >> background.green >> background.blue)) {
        cerr << "ERROR: Failed to read background!\n";
        return;
    }

    cout << "Background color ("
        << background.red << ", "
        << background.green << ", "
        << background.blue << ")\n";
}

void readSphere(int i) {
    object[i].letter = 'S';

    cout << "\nDEBUG: Reading sphere " << i << "...\n";

    if (!(cin >> object[i].center.x >> object[i].center.y >> object[i].center.z
        >> object[i].radius >> object[i].refractionIndex)) {
        cerr << "ERROR: Failed to read sphere position!\n";
        return;
    }

    if (!(cin >> object[i].ambient[0].red >> object[i].ambient[0].green >> object[i].ambient[0].blue
        >> object[i].diffuse[0].red >> object[i].diffuse[0].green >> object[i].diffuse[0].blue
        >> object[i].specular[0].red >> object[i].specular[0].green >> object[i].specular[0].blue
        >> object[i].Ka >> object[i].Kd >> object[i].Ks >> object[i].Ke
        >> object[i].Kr >> object[i].Kt)) {
        cerr << "ERROR: Failed to read sphere materials!\n";
        return;
    }

    cout << "  Center: (" << object[i].center.x << ", " << object[i].center.y << ", " << object[i].center.z << ")\n";
    cout << "  Radius: " << object[i].radius << "\n";
}

void readFloor(int i) {
    object[i].letter = 'F';
    object[i].normal = { {}, 0.0f, 1.0f, 0.0f };
    object[i].F = 0.0f;

    cout << "\nDEBUG: Reading floor " << i << "...\n";

    for (int j = 0; j < 4; j++) {
        if (!(cin >> object[i].vertex[j].x
            >> object[i].vertex[j].y
            >> object[i].vertex[j].z)) {
            cerr << "ERROR: Failed to read floor vertex " << j << "!\n";
            return;
        }

        if (j == 0) {
            object[i].xmin = object[i].xmax = object[i].vertex[j].x;
            object[i].ymin = object[i].ymax = object[i].vertex[j].y;
            object[i].zmin = object[i].zmax = object[i].vertex[j].z;
        }
        else {
            object[i].xmin = min(object[i].xmin, object[i].vertex[j].x);
            object[i].xmax = max(object[i].xmax, object[i].vertex[j].x);
            object[i].ymin = min(object[i].ymin, object[i].vertex[j].y);
            object[i].ymax = max(object[i].ymax, object[i].vertex[j].y);
            object[i].zmin = min(object[i].zmin, object[i].vertex[j].z);
            object[i].zmax = max(object[i].zmax, object[i].vertex[j].z);
        }
    }

    if (!(cin >> object[i].checksize)) {
        cerr << "ERROR: Failed to read checksize!\n";
        return;
    }

    for (int j = 0; j < 2; j++) {
        if (!(cin >> object[i].ambient[j].red >> object[i].ambient[j].green >> object[i].ambient[j].blue
            >> object[i].diffuse[j].red >> object[i].diffuse[j].green >> object[i].diffuse[j].blue
            >> object[i].specular[j].red >> object[i].specular[j].green >> object[i].specular[j].blue)) {
            cerr << "ERROR: Failed to read floor color " << j << "!\n";
            return;
        }
    }

    if (!(cin >> object[i].Ka >> object[i].Kd >> object[i].Ks
        >> object[i].Ke >> object[i].Kr >> object[i].Kt)) {
        cerr << "ERROR: Failed to read floor materials!\n";
        return;
    }

    cout << "  Bounds: X(" << object[i].xmin << " to " << object[i].xmax << ")\n";
    cout << "  Checksize: " << object[i].checksize << "\n";
}


void readRenderMode() {
    char mode;
    if (!(cin >> mode)) {
        cerr << "ERROR: Failed to read render mode!\n";
        return;
    }

    if (mode == 'P') renderMode = PHOTON;
    else renderMode = WHITTED;

    cout << "Render mode: " << (mode == 'P' ? "Photon Mapping" : "Raytracing") << "\n";
}

void readWall(int i) {
    object[i].letter = 'W';
    object[i].normal = { {}, 0.0f, 0.0f, 0.0f }; // Will compute below

    cout << "\nDEBUG: Reading wall " << i << "...\n";

    for (int j = 0; j < 4; j++) {
        if (!(cin >> object[i].vertex[j].x
            >> object[i].vertex[j].y
            >> object[i].vertex[j].z)) {
            cerr << "ERROR: Failed to read wall vertex " << j << "!\n";
            return;
        }
    }

    // Compute normal (cross product of two edges)
    float ux = object[i].vertex[1].x - object[i].vertex[0].x;
    float uy = object[i].vertex[1].y - object[i].vertex[0].y;
    float uz = object[i].vertex[1].z - object[i].vertex[0].z;
    float vx = object[i].vertex[2].x - object[i].vertex[0].x;
    float vy = object[i].vertex[2].y - object[i].vertex[0].y;
    float vz = object[i].vertex[2].z - object[i].vertex[0].z;
    crossProduct(ux, uy, uz, vx, vy, vz,
        object[i].normal.dx, object[i].normal.dy, object[i].normal.dz);
    normalize(object[i].normal.dx, object[i].normal.dy, object[i].normal.dz);

    // Set bounds for intersection
    object[i].xmin = object[i].xmax = object[i].vertex[0].x;
    object[i].ymin = object[i].ymax = object[i].vertex[0].y;
    object[i].zmin = object[i].zmax = object[i].vertex[0].z;
    for (int j = 1; j < 4; j++) {
        object[i].xmin = min(object[i].xmin, object[i].vertex[j].x);
        object[i].xmax = max(object[i].xmax, object[i].vertex[j].x);
        object[i].ymin = min(object[i].ymin, object[i].vertex[j].y);
        object[i].ymax = max(object[i].ymax, object[i].vertex[j].y);
        object[i].zmin = min(object[i].zmin, object[i].vertex[j].z);
        object[i].zmax = max(object[i].zmax, object[i].vertex[j].z);
    }

    if (!(cin >> object[i].ambient[0].red >> object[i].ambient[0].green >> object[i].ambient[0].blue
        >> object[i].diffuse[0].red >> object[i].diffuse[0].green >> object[i].diffuse[0].blue
        >> object[i].specular[0].red >> object[i].specular[0].green >> object[i].specular[0].blue
        >> object[i].Ka >> object[i].Kd >> object[i].Ks
        >> object[i].Ke >> object[i].Kr >> object[i].Kt)) {
        cerr << "ERROR: Failed to read wall materials!\n";
        return;
    }
}