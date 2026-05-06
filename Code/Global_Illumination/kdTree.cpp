#include "kdTree.h"
#include "globals.h"
#include "photonMap.h"
#include "math.h"
#include <vector>
#include <algorithm>

using namespace std;

KDNode* photonRoot = nullptr;


KDNode* buildKDTree(vector<Photon>& photons, int depth)
{
    if (photons.empty())
        return nullptr;

    int axis = depth % 3;

    sort(photons.begin(), photons.end(),
        [axis](const Photon& a, const Photon& b)
        {
            if (axis == 0) return a.position.x < b.position.x;
            if (axis == 1) return a.position.y < b.position.y;
            return a.position.z < b.position.z;
        });

    size_t median = photons.size() / 2;

    KDNode* node = new KDNode;
    node->photon = photons[median];
    node->axis = axis;

    vector<Photon> leftPhotons(
        photons.begin(), photons.begin() + median);
    vector<Photon> rightPhotons(
        photons.begin() + median + 1, photons.end());

    node->left = buildKDTree(leftPhotons, depth + 1);
    node->right = buildKDTree(rightPhotons, depth + 1);

    return node;
}

void searchKDTree(KDNode* node,
    Point hitPoint,
    float radius2,
    float nx, float ny, float nz,
    Color& result,
    int& photonCount)
{
    if (!node) return;

    float dx = node->photon.position.x - hitPoint.x;
    float dy = node->photon.position.y - hitPoint.y;
    float dz = node->photon.position.z - hitPoint.z;

    float dist2 = dx * dx + dy * dy + dz * dz;

    if (dist2 <= radius2)
    {
        float weight = dotProduct(nx, ny, nz,
            -node->photon.dirX,
            -node->photon.dirY,
            -node->photon.dirZ);

        if (weight > 0) {
            result.red += node->photon.power.red * weight;
            result.green += node->photon.power.green * weight;
            result.blue += node->photon.power.blue * weight;
            photonCount++;
        }
    }

    float axisDiff;

    if (node->axis == 0)
        axisDiff = hitPoint.x - node->photon.position.x;
    else if (node->axis == 1)
        axisDiff = hitPoint.y - node->photon.position.y;
    else
        axisDiff = hitPoint.z - node->photon.position.z;

    KDNode* nearChild = axisDiff < 0 ? node->left : node->right;
    KDNode* farChild = axisDiff < 0 ? node->right : node->left;

    searchKDTree(nearChild, hitPoint, radius2, nx, ny, nz, result, photonCount);

    if (axisDiff * axisDiff < radius2)
        searchKDTree(farChild, hitPoint, radius2, nx, ny, nz, result, photonCount);
}