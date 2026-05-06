#pragma once
#include "photon.h"
#include "globals.h"
#include "kdTree.h"
#include <vector>


void sampleHemisphere(float nx, float ny, float nz,
    float& dx, float& dy, float& dz);

void tracePhoton(Point origin,
    float dx, float dy, float dz,
    float pr, float pg, float pb,
    int depth,
    bool specularPath);

void emitPhotons();

Color estimateRadiance(KDNode* root, Point hitPoint,
    float nx, float ny, float nz);


extern std::vector<Photon> causticMap;
extern std::vector<Photon> globalMap;

extern KDNode* causticRoot;
extern KDNode* globalRoot;