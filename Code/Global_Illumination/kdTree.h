#pragma once

#include "globals.h"
#include "photon.h"
#include <vector>

struct KDNode {
    Photon photon;
    KDNode* left;
    KDNode* right;
    int axis;   // 0=x, 1=y, 2=z
};

KDNode* buildKDTree(std::vector<Photon>& photons, int depth);

void searchKDTree(KDNode* node,
    Point hitPoint,
    float radius2,
    float nx, float ny, float nz,
    Color& result,
    int& photonCount);