// rayTracer.cpp : Main ray tracing pipeline implementation
//
// This file implements:
// - Phong shading (direct illumination)
// - Recursive ray tracing (reflection + refraction)
// - Photon mapping integration (indirect illumination)
// - Camera projection and image generation
//

#include "rayTracer.h"
#include "globals.h"
#include "math.h"
#include "objects.h"
#include "toneReproduction.h"
#include "photonMap.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>

using namespace std;

/*
    Phong Illumination Model

    Computes direct lighting at a surface point using:
    - Ambient term (constant global light)
    - Diffuse term (Lambertian reflection)
    - Specular term (mirror-like highlights)

    Also performs shadow testing by casting a shadow ray toward the light.
*/
Color computePhongShading(Point hitPoint, float normalX, float normalY, float normalZ,
    Point viewPoint, Color ambient, Color diffuse, Color specular,
    float Ka, float Kd, float Ks, float Ke) {
    
    Color result = { 0, 0, 0 };

    // Ambient
    // Simulates indirect constant light in the scene
    result.red = Ka * ambient.red * light.ambientColor.red;
    result.green = Ka * ambient.green * light.ambientColor.green;
    result.blue = Ka * ambient.blue * light.ambientColor.blue;

    float lightDx = light.position.x - hitPoint.x;
    float lightDy = light.position.y - hitPoint.y;
    float lightDz = light.position.z - hitPoint.z;
    normalize(lightDx, lightDy, lightDz);

    // Shadow ray: Cast a ray toward the light to check if point is occluded
    bool inShadow = false;
    float distToLight = vectorLength(light.position.x - hitPoint.x,
        light.position.y - hitPoint.y,
        light.position.z - hitPoint.z);

    for (int i = 0; i < size(object); i++) {
        float tShadow;
        bool hit = false;

        if (object[i].letter == 'S') {
            hit = intersectSphere(hitPoint, lightDx, lightDy, lightDz, i, tShadow);
        }
        else if (object[i].letter == 'F') {
            hit = intersectFloor(hitPoint, lightDx, lightDy, lightDz, i, tShadow);
        }
        else if (object[i].letter == 'W') {
            hit = intersectWall(hitPoint, lightDx, lightDy, lightDz, i, tShadow);
        }
        float transmittance = 1.0f;
        if (hit && tShadow < distToLight - 0.001f) {

            // Skip transparent objects
            if (object[i].Kt > 0.0f) {
                continue;
            }

            transmittance *= object[i].Kt; // or (1 - opacity)

            if (transmittance < 0.01f) {
                inShadow = true;
                break;
            }
        }
    }

    // DIFFUSE
    if (!inShadow) {
        float nDotL = dotProduct(normalX, normalY, normalZ, lightDx, lightDy, lightDz);
        if (nDotL > 0) {
            result.red += Kd * diffuse.red * light.color.red * nDotL;
            result.green += Kd * diffuse.green * light.color.green * nDotL;
            result.blue += Kd * diffuse.blue * light.color.blue * nDotL;
        }

        // SPECULAR 
        float viewDx = viewPoint.x - hitPoint.x;
        float viewDy = viewPoint.y - hitPoint.y;
        float viewDz = viewPoint.z - hitPoint.z;
        normalize(viewDx, viewDy, viewDz);

        // Reflection vector
        float nDotL2 = 2.0f * dotProduct(normalX, normalY, normalZ, lightDx, lightDy, lightDz);
        float reflectDx = nDotL2 * normalX - lightDx;
        float reflectDy = nDotL2 * normalY - lightDy;
        float reflectDz = nDotL2 * normalZ - lightDz;
        normalize(reflectDx, reflectDy, reflectDz);

        float rDotV = dotProduct(reflectDx, reflectDy, reflectDz, viewDx, viewDy, viewDz);
        if (rDotV > 0) {
            float specularFactor = pow(rDotV, Ke);
            result.red += Ks * specular.red * light.color.red * specularFactor;
            result.green += Ks * specular.green * light.color.green * specularFactor;
            result.blue += Ks * specular.blue * light.color.blue * specularFactor;
        }
    }

    return result;
}

/*
    Perfect specular reflection.

    Computes reflected direction using:
        R = I - 2(N · I)N
*/
void reflect(float inDx, float inDy, float inDz,
    float nx, float ny, float nz,
    float& rx, float& ry, float& rz)
{
    float dot = dotProduct(inDx, inDy, inDz, nx, ny, nz);

    rx = inDx - 2.0f * dot * nx;
    ry = inDy - 2.0f * dot * ny;
    rz = inDz - 2.0f * dot * nz;

    normalize(rx, ry, rz);
}

/*
    Refraction using Snell’s Law.

    Handles:
    - Direction bending across media boundary
    - Total Internal Reflection (returns false if occurs)
*/
bool refract(float inDx, float inDy, float inDz,
    float nx, float ny, float nz,
    float eta,
    float& tx, float& ty, float& tz)
{
    // Normal is already guaranteed to face against the ray by the caller
    float cosTheta_i = -dotProduct(inDx, inDy, inDz, nx, ny, nz);

    // sin^2(theta_t) via Snell's law
    float sin2Theta_t = eta * eta * (1.0f - cosTheta_i * cosTheta_i);

    // Total internal reflection
    if (sin2Theta_t > 1.0f)
        return false;

    float cosTheta_t = sqrt(1.0f - sin2Theta_t);

    tx = eta * inDx + (eta * cosTheta_i - cosTheta_t) * nx;
    ty = eta * inDy + (eta * cosTheta_i - cosTheta_t) * ny;
    tz = eta * inDz + (eta * cosTheta_i - cosTheta_t) * nz;
    normalize(tx, ty, tz);
    return true;
}


/*
    Recursive ray tracing function.

    Combines:
    - Direct illumination (Phong)
    - Indirect illumination (Photon Mapping)
    - Reflection and refraction recursion

    This is the core of the rendering algorithm.
*/
Color traceRay(Point rayOrigin, float rayDx, float rayDy, float rayDz, int depth) {
    // Stop recursion
    if (depth > numberOfLevels) {
        return background;
    }

	// Find closest intersection
    float closestT = numeric_limits<float>::max();
    int hitObject = -1;

    for (int i = 0; i < size(object); i++) {
        float t;
        bool hit = false;

        if (object[i].letter == 'S') {
            hit = intersectSphere(rayOrigin, rayDx, rayDy, rayDz, i, t);
        }
        else if (object[i].letter == 'F') {
            hit = intersectFloor(rayOrigin, rayDx, rayDy, rayDz, i, t);
        }
        else if (object[i].letter == 'W') {
            hit = intersectWall(rayOrigin, rayDx, rayDy, rayDz, i, t);
        }

        if (hit && t < closestT) {
            closestT = t;
            hitObject = i;
        }
    }

	// Ray misses scene
    if (hitObject == -1) {
        return background;
    }

    // Compute intersection point
    Point hitPoint;
    hitPoint.x = rayOrigin.x + closestT * rayDx;
    hitPoint.y = rayOrigin.y + closestT * rayDy;
    hitPoint.z = rayOrigin.z + closestT * rayDz;

	// Compute surface normal and color index for floor
    float normalX, normalY, normalZ;
    int colorIndex = 0;

    if (object[hitObject].letter == 'S') {
        normalX = hitPoint.x - object[hitObject].center.x;
        normalY = hitPoint.y - object[hitObject].center.y;
        normalZ = hitPoint.z - object[hitObject].center.z;
        normalize(normalX, normalY, normalZ);
    }
    else if (object[hitObject].letter == 'F') {
        normalX = object[hitObject].normal.dx;
        normalY = object[hitObject].normal.dy;
        normalZ = object[hitObject].normal.dz;

        colorIndex = getFloorColorIndex(hitPoint.x, hitPoint.z, hitObject);
    }
    else if (object[hitObject].letter == 'W') {
        normalX = object[hitObject].normal.dx;
        normalY = object[hitObject].normal.dy;
        normalZ = object[hitObject].normal.dz;
    }

    // DIRECT LIGHTING 
    Color direct = computePhongShading(
        hitPoint, normalX, normalY, normalZ, rayOrigin,
        object[hitObject].ambient[colorIndex],
        object[hitObject].diffuse[colorIndex],
        object[hitObject].specular[colorIndex],
        object[hitObject].Ka,
        object[hitObject].Kd,
        object[hitObject].Ks,
        object[hitObject].Ke
    );

    Color finalColor = direct;

    // PHOTON MAPPING (INDIRECT LIGHT)
    if (renderMode == PHOTON)
    {
        Color indirect = estimateRadiance(globalRoot, hitPoint, normalX, normalY, normalZ);
        Color caustics = estimateRadiance(causticRoot, hitPoint, normalX, normalY, normalZ);

        // Diffuse surfaces receive indirect light
        finalColor.red += object[hitObject].Kd * (indirect.red + caustics.red);
        finalColor.green += object[hitObject].Kd * (indirect.green + caustics.green);
        finalColor.blue += object[hitObject].Kd * (indirect.blue + caustics.blue);
    }

    if (depth < numberOfLevels)
    {
        float Kr = object[hitObject].Kr;
        float Kt = object[hitObject].Kt;

        // REFLECTION
        if (Kr > 0.0f)
        {
            float reflectDx, reflectDy, reflectDz;
            reflect(rayDx, rayDy, rayDz,
                normalX, normalY, normalZ,
                reflectDx, reflectDy, reflectDz);

            // Offset to avoid self-intersection
            Point reflectOrigin;
            reflectOrigin.x = hitPoint.x + normalX * 0.001f;
            reflectOrigin.y = hitPoint.y + normalY * 0.001f;
            reflectOrigin.z = hitPoint.z + normalZ * 0.001f;

            Color reflectColor = traceRay(
                reflectOrigin,
                reflectDx, reflectDy, reflectDz,
                depth + 1);

            finalColor.red += Kr * reflectColor.red;
            finalColor.green += Kr * reflectColor.green;
            finalColor.blue += Kr * reflectColor.blue;
        }

        // REFRACTION
        if (Kt > 0.0f)
        {
            float nx = normalX;
            float ny = normalY;
            float nz = normalZ;

            float cosi = dotProduct(rayDx, rayDy, rayDz, nx, ny, nz);

            float eta_i = 1.0f;
            float eta_t = object[hitObject].refractionIndex;

            // If ray is inside the object, flip normal and swap indices
            if (cosi > 0.0f)
            {
                nx = -nx;
                ny = -ny;
                nz = -nz;
                swap(eta_i, eta_t);
            }

            float eta = eta_i / eta_t;

            float tx, ty, tz;
            bool transmitted = refract(rayDx, rayDy, rayDz,
                nx, ny, nz,
                eta,
                tx, ty, tz);

            // Total internal reflection fallback
            if (!transmitted)
            {
                reflect(rayDx, rayDy, rayDz,
                    nx, ny, nz,
                    tx, ty, tz);
            }

            // offset along transmitted direction
            Point transOrigin;
            transOrigin.x = hitPoint.x + tx * 0.001f;
            transOrigin.y = hitPoint.y + ty * 0.001f;
            transOrigin.z = hitPoint.z + tz * 0.001f;

            Color transColor = traceRay(transOrigin, tx, ty, tz, depth + 1);

            finalColor.red += Kt * transColor.red;
            finalColor.green += Kt * transColor.green;
            finalColor.blue += Kt * transColor.blue;
        }
    }

    return finalColor;
}

/*
    Image generation pipeline.

    Steps:
    1. Build camera coordinate system (view, right, up)
    2. For each pixel:
        - Generate primary ray
        - Trace ray into scene
    3. Store HDR results in framebuffer
    4. Apply tone mapping
    5. Output to PPM image
*/
void output_ppm() {
    float viewDx = camera.lookAt.x - camera.position.x;
    float viewDy = camera.lookAt.y - camera.position.y;
    float viewDz = camera.lookAt.z - camera.position.z;
    normalize(viewDx, viewDy, viewDz);

    float upDx = camera.up.dx;
    float upDy = camera.up.dy;
    float upDz = camera.up.dz;
    normalize(upDx, upDy, upDz);

    float rightDx, rightDy, rightDz;
    crossProduct(viewDx, viewDy, viewDz, upDx, upDy, upDz, rightDx, rightDy, rightDz);
    normalize(rightDx, rightDy, rightDz);

    crossProduct(rightDx, rightDy, rightDz, viewDx, viewDy, viewDz, upDx, upDy, upDz);
    normalize(upDx, upDy, upDz);

    Point imagePlaneCenter;
    imagePlaneCenter.x = camera.position.x + camera.focalLength * viewDx;
    imagePlaneCenter.y = camera.position.y + camera.focalLength * viewDy;
    imagePlaneCenter.z = camera.position.z + camera.focalLength * viewDz;

    float pixelWidth = worldWidth / screenWidth;
    float pixelHeight = worldHeight / screenHeight;

    ofstream scene("raytraced_scene.ppm");
    scene << "P3\n" << screenWidth << " " << screenHeight << "\n255\n";

    // FRAMEBUFFER
    vector<Color> framebuffer(screenWidth * screenHeight);

    // PASS 1: RAY TRACING 
    for (int j = 0; j < screenHeight; j++) {
        for (int i = 0; i < screenWidth; i++) {

            float u = (i + 0.5f - screenWidth / 2.0f) * pixelWidth;
            float v = (screenHeight / 2.0f - j - 0.5f) * pixelHeight;

            Point pixelPoint;
            pixelPoint.x = imagePlaneCenter.x + u * rightDx + v * upDx;
            pixelPoint.y = imagePlaneCenter.y + u * rightDy + v * upDy;
            pixelPoint.z = imagePlaneCenter.z + u * rightDz + v * upDz;

            float rayDx = pixelPoint.x - camera.position.x;
            float rayDy = pixelPoint.y - camera.position.y;
            float rayDz = pixelPoint.z - camera.position.z;
            normalize(rayDx, rayDy, rayDz);

            Color pixelColor = traceRay(camera.position, rayDx, rayDy, rayDz, 0);

            framebuffer[j * screenWidth + i] = pixelColor;
        }
    }

    // PASS 2: TONE MAPPING 
    float L_avg = computeLogAvgLuminance(framebuffer);
    float Ldmax = 1.0f;

    float ward_s = wardScale(L_avg, Ldmax);

    for (int j = 0; j < screenHeight; j++) {
        for (int i = 0; i < screenWidth; i++) {

            Color c = framebuffer[j * screenWidth + i];

            // ---- SWITCH HERE ----
            //Color mapped = toneMapReinhard(c, L_avg, Ldmax);
            Color mapped = toneMapWard(c, ward_s);

            // clamp AFTER tone mapping
            mapped.red = min(1.0f, max(0.0f, mapped.red));
            mapped.green = min(1.0f, max(0.0f, mapped.green));
            mapped.blue = min(1.0f, max(0.0f, mapped.blue));

            int r = (int)(mapped.red * 255.0f);
            int g = (int)(mapped.green * 255.0f);
            int b = (int)(mapped.blue * 255.0f);

            scene << r << " " << g << " " << b;
            if (i < screenWidth - 1) scene << "  ";
        }
        scene << "\n";
    }

    scene.close();
    cout << "\nRay tracing complete! Output written to raytraced_scene.ppm\n";
}
