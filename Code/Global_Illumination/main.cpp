#include "readScene.h"
#include "rayTracer.h"
#include "toneReproduction.h"
#include "photonMap.h"
#include "math.h"

#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

int main()
{
    ifstream inputFile("cornell_box.txt");
    if (!inputFile.is_open()) {
        cerr << "\nERROR \n";
        cerr << "Could not open scene_input.txt\n";
        cerr << "Current directory contents:\n";
        system("dir");
        cerr << "\nMake sure scene_input.txt is in the same directory as the .exe\n";
        cout << "\nPress Enter to exit...";
        cin.get();
        return 1;
    }

    streambuf* cinbuf = cin.rdbuf();
    cin.rdbuf(inputFile.rdbuf());

    srand(static_cast<unsigned>(time(nullptr)));
    readCamera();
    readLight();
    readParameters();
    readBackground();
    readRenderMode();

    char objType;
    int objCount = 0;


    while (cin >> objType && objCount < 3)
    {
        if (objType == 'S') readSphere(objCount);
        else if (objType == 'F') readFloor(objCount);
        objCount++;
    }

    cin.rdbuf(cinbuf);
    inputFile.close();

    cout << "\n\n";
    cout << "Starting ray tracing...\n";
    cout << "Resolution: " << screenWidth << "x" << screenHeight << "\n";
    cout << "\n\n";

    if (renderMode == PHOTON) {
        cout << "Emitting photons...\n";
        emitPhotons();
    }

    cout << "Rendering image...\n";
    output_ppm();

    cout << "\n===========================================\n";
    cout << "SUCCESS!\n";
    cout << "Output file: raytraced_scene.ppm\n";
    cout << "\n\n";
    cout << "Press Enter to exit...";
    cin.get();

    return 0;
}