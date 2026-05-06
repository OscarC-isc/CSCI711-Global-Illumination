#include "photonMap.h"
#include "globals.h"
#include "math.h"
#include "objects.h"
#include "kdTree.h"
#include "rayTracer.h"
#include <limits>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iostream>

#define M_PI 3.1415926535898f
#define PHOTON_RADIUS 0.5f
#define NUM_PHOTONS 100000
#define POWERSCALE (1.0f / NUM_PHOTONS)

using namespace std;

// Two photon maps:
// - causticMap: photons that underwent specular interactions before hitting diffuse
// - globalMap: general indirect illumination photons
vector<Photon> causticMap;
vector<Photon> globalMap;

// KD-tree roots for fast spatial lookup during rendering
KDNode* causticRoot = nullptr;
KDNode* globalRoot = nullptr;

// Debug counters for validating photon tracing behavior
static int photonsHit = 0;
static int photonsStored = 0;
static int photonsMissed = 0;


/*
    Cosine-weighted hemisphere sampling.

    This function generates a random direction over a hemisphere oriented
    around the surface normal (nx, ny, nz). The cosine-weighting ensures
    physically correct Lambertian reflection (more energy near the normal).

    Steps:
    1. Sample spherical coordinates (theta, phi)
    2. Convert to local coordinates
    3. Build tangent space (T, B, N)
    4. Transform sample into world space
*/
void sampleHemisphere(float nx, float ny, float nz,
    float& dx, float& dy, float& dz)
{
    float u = (float)rand() / RAND_MAX;
    float v = (float)rand() / RAND_MAX;

    float theta = 2.0f * M_PI * u;
    float phi = acos(sqrt(1.0f - v)); // cosine-weighted distribution

    // Local hemisphere sample (Y-up)
    float x = sin(phi) * cos(theta);
    float y = cos(phi);
    float z = sin(phi) * sin(theta);

    // build tangent space
    Vec3 tangent;
    if (fabs(nx) < 0.9f)
        tangent = Vec3{ Point{0,0,0}, 1, 0, 0 };
    else
        tangent = Vec3{ Point{0,0,0}, 0, 1, 0 };

    float tx, ty, tz;
    crossProduct(nx, ny, nz, tangent.dx, tangent.dy, tangent.dz, tx, ty, tz);
    normalize(tx, ty, tz);

    float bx, by, bz;
    crossProduct(nx, ny, nz, tx, ty, tz, bx, by, bz);

    // Transform from local → world space
    dx = x * tx + y * nx + z * bx;
    dy = x * ty + y * ny + z * by;
    dz = x * tz + y * nz + z * bz;

    normalize(dx, dy, dz);
}

/*
    Core photon tracing function.

    This implements recursive photon propagation using:
    - Ray-scene intersection
    - Photon storage at diffuse surfaces
    - Russian roulette for bounce type selection
    - Energy conservation via probability scaling

    Parameters:
    - origin, (dx,dy,dz): photon ray
    - pr,pg,pb: photon power (RGB energy)
    - depth: recursion depth
    - specularPath: whether photon has bounced specularly
    - hasDiffuseBounce: used to separate caustics from global photons
*/

void tracePhoton(Point origin,
    float dx, float dy, float dz,
    float pr, float pg, float pb,
    int depth,
    bool specularPath, bool hasDiffuseBounce)
{   
    // Limit recursion depth to prevent infinite loops
    if (depth > 5) return;
      
    // Find closest intersection (standard ray tracing step)
    float closestT = numeric_limits<float>::max();
    int hitObject = -1;

    for (int i = 0; i < size(object); i++) {
        float t;
        bool hit = false;

        if (object[i].letter == 'S') {
            hit = intersectSphere(origin, dx, dy, dz, i, t);
        }
        else if (object[i].letter == 'F') {
            hit = intersectFloor(origin, dx, dy, dz, i, t);
        }
        else if (object[i].letter == 'W') {
            hit = intersectWall(origin, dx, dy, dz, i, t);
        }
        else {
            continue;
        }

        if (hit && t < closestT) {
            closestT = t;
            hitObject = i;
        }
    }

    // Photon escaped scene
    if (hitObject == -1) {
        if (depth == 0) photonsMissed++;
        return;
    }

    if (depth == 0) photonsHit++;

    Point hitPoint = {
        origin.x + dx * closestT,
        origin.y + dy * closestT,
        origin.z + dz * closestT
    };

    // Compute surface normal
    float nx, ny, nz;
    if (object[hitObject].letter == 'S') {
        nx = hitPoint.x - object[hitObject].center.x;
        ny = hitPoint.y - object[hitObject].center.y;
        nz = hitPoint.z - object[hitObject].center.z;
        normalize(nx, ny, nz);
    }
    else if (object[hitObject].letter == 'F') {
        nx = object[hitObject].normal.dx;
        ny = object[hitObject].normal.dy;
        nz = object[hitObject].normal.dz;
    }
    else if (object[hitObject].letter == 'W') {
        nx = object[hitObject].normal.dx;
        ny = object[hitObject].normal.dy;
        nz = object[hitObject].normal.dz;
    }

    float Kd = object[hitObject].Kd; // diffuse coefficient
    float Ks = object[hitObject].Ks; // specular coefficient
    float Kt = object[hitObject].Kt; // transmission coefficient

    /*
        PHOTON STORAGE RULE:

        Only store photons at diffuse surfaces because:
        - Diffuse surfaces scatter light in all directions
        - This approximates the rendering equation via density estimation

        Caustics:
        - Stored only if photon came through specular chain and hits diffuse

        Global:
        - Stored if no specular chain involved
    */
    if (Kd > 0.0f)
    {
        Photon p;
        p.position = hitPoint;
        p.dirX = -dx;
        p.dirY = -dy;
        p.dirZ = -dz;
        p.power = { pr, pg, pb };

        if (specularPath && !hasDiffuseBounce)
            causticMap.push_back(p);
        else if (!specularPath)
            globalMap.push_back(p);

        photonsStored++;
    }

    /*
        Russian Roulette:

        Instead of spawning multiple rays, we probabilistically choose ONE:
        - Diffuse
        - Specular
        - Refraction

        This keeps computation manageable while preserving unbiased energy.
    */
    float probDiffuse = Kd;
    float probSpecular = Ks;
    float probRefract = Kt;

    float sum = probDiffuse + probSpecular + probRefract;
    if (sum == 0) return;

    probDiffuse /= sum;
    probSpecular /= sum;
    probRefract /= sum;

    float r = (float)rand() / RAND_MAX;

    // DIFFUSE
    if (r < probDiffuse)
    {
        float newDx, newDy, newDz;
        sampleHemisphere(nx, ny, nz, newDx, newDy, newDz);

        tracePhoton(
            { hitPoint.x + nx * 0.001f,
              hitPoint.y + ny * 0.001f,
              hitPoint.z + nz * 0.001f },
            newDx, newDy, newDz,
            pr *= Kd / probDiffuse,
            pg *= Kd / probDiffuse,
            pb *= Kd / probDiffuse,
            depth + 1,
            false, true // breaks caustic chain
        );
    }

    // SPECULAR REFLECTION 
    else if (r < probDiffuse + probSpecular)
    {
        float rx, ry, rz;
        reflect(dx, dy, dz, nx, ny, nz, rx, ry, rz);

        tracePhoton(
            { hitPoint.x + nx * 0.001f,
              hitPoint.y + ny * 0.001f,
              hitPoint.z + nz * 0.001f },
            rx, ry, rz,
            pr * Ks / probSpecular,
            pg * Ks / probSpecular,
            pb * Ks / probSpecular,
            depth + 1,
            true, hasDiffuseBounce
        );
    }

    // REFRACTION 
    else
    {
        float tx, ty, tz;
        float eta = 1.0f / object[hitObject].refractionIndex;

        if (refract(dx, dy, dz, nx, ny, nz, eta, tx, ty, tz))
        {
            tracePhoton(
                { hitPoint.x + tx * 0.001f,
                  hitPoint.y + ty * 0.001f,
                  hitPoint.z + tz * 0.001f },
                tx, ty, tz,
                pr * Kt / probRefract,
                pg * Kt / probRefract,
                pb * Kt / probRefract,
                depth + 1,
                true, hasDiffuseBounce
            );
        }
    }
}

/*
    Radiance estimation using photon density estimation.

    Steps:
    1. Query KD-tree for nearby photons
    2. Accumulate their power
    3. Divide by area (πr²) to estimate irradiance

    This approximates:
        L ≈ (1 / (πr²)) * Σ photon_power
*/

Color estimateRadiance(KDNode* root, Point hitPoint,
    float nx, float ny, float nz)
{
    if (!root) return { 0, 0, 0 };

    Color result = { 0, 0, 0 };
    int photonCount = 0;
    bool isCaustic = false;
    float radius2 = (isCaustic ? (PHOTON_RADIUS / 10) : PHOTON_RADIUS);
    float area = M_PI * radius2;

    searchKDTree(root, hitPoint, radius2, nx, ny, nz, result, photonCount);

    if (photonCount > 0)
    {
        float area = M_PI * radius2;
        float scale = 1.0f / (area * photonCount);

        result.red *= scale;
        result.green *= scale;
        result.blue *= scale;
    }

    result.red = max(0.0f, result.red);
    result.green = max(0.0f, result.green);
    result.blue = max(0.0f, result.blue);

    return result;
}

/*
    Photon emission phase (preprocessing step).

    This shoots photons from the light source into the scene:
    - Directions sampled uniformly on sphere
    - Each photon carries a fraction of total light energy
    - Results stored in photon maps

    After emission:
    - KD-trees are built for fast rendering queries
*/
void emitPhotons()
{
    causticMap.clear();
    globalMap.clear();
    photonsHit = 0;
    photonsStored = 0;
    photonsMissed = 0;

    cout << "\nLight position: (" << light.position.x << ", "
        << light.position.y << ", " << light.position.z << ")\n";
    cout << "Light color: (" << light.color.red << ", "
        << light.color.green << ", " << light.color.blue << ")\n\n";

    cout << "Emitting " << NUM_PHOTONS << " photons...\n";

    for (int i = 0; i < NUM_PHOTONS; i++)
    {
        if ((i + 1) % 20000 == 0) {
            cout << "  Progress: " << (i + 1) << " / " << NUM_PHOTONS << "\n";
        }

        float z = 1.0f - 2.0f * randf();
        float r = sqrt(max(0.0f, 1.0f - z * z));
        float theta = 2.0f * M_PI * randf();

        float dx = r * cos(theta);
        float dy = z;
        float dz = r * sin(theta);
        normalize(dx, dy, dz);

        tracePhoton(light.position,
            dx, dy, dz,
            light.color.red * POWERSCALE,
            light.color.green * POWERSCALE,
            light.color.blue * POWERSCALE,
            0,
            false, false);
    }

    cout << "\n  === PHOTON EMISSION STATISTICS ===\n";
    cout << "  Photons emitted: " << NUM_PHOTONS << "\n";
    cout << "  Photons that hit objects: " << photonsHit << "\n";
    cout << "  Photons that missed: " << photonsMissed << "\n";
    cout << "  Photons stored at depth 0: " << photonsStored << "\n";
    cout << "  Global photons stored: " << globalMap.size() << "\n";
    cout << "  Caustic photons stored: " << causticMap.size() << "\n";

    if (globalMap.empty() && causticMap.empty()) {
        cout << "\n*** WARNING: NO PHOTONS STORED! ***\n";
        if (photonsHit == 0) {
            cout << "PROBLEM: Photons are not hitting ANY objects!\n";
        }
        else {
            cout << "PROBLEM: Photons hit objects but none had Kd > 0.001!\n";
        }
    }
    else {
        cout << "\n*** SUCCESS: Photons stored! ***\n";
    }

    causticRoot = buildKDTree(causticMap, 0);
    globalRoot = buildKDTree(globalMap, 0);

    cout << "KD-trees built.\n\n";
}