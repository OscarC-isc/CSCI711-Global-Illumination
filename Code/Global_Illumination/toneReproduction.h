#pragma once
#include "globals.h"
#include <vector>

#define LIGHTSCALE 1.0f;

float luminance(const Color& c);

float computeLogAvgLuminance(std::vector<Color>& image);

Color toneMapReinhard(const Color& c, float L_avg, float Ldmax);

float wardScale(float L_avg, float Ldmax);

Color toneMapWard(const Color& c, float scale);