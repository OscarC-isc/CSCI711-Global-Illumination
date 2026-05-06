#include "globals.h"
#include <vector>

using namespace std;

float luminance(const Color& c)
{
    return 0.27f * c.red + 0.67f * c.green + 0.06f * c.blue;
}

float computeLogAvgLuminance(vector<Color>& image)
{
    float delta = 1e-4f;
    float sum = 0.0f;

    for (auto& c : image)
    {
        float L = luminance(c);
        sum += log(delta + L);
    }

    return exp(sum / image.size());
}

Color toneMapReinhard(const Color& c, float L_avg, float Ldmax)
{
    float L = luminance(c);
    if (L <= 0.0f) return { 0,0,0 };

    float a = 0.18f;

    float Ls = (a / L_avg) * L;
    float Ld = Ls / (1.0f + Ls);

    float scale = Ld / L;

    Color out;
    out.red = c.red * scale * Ldmax;
    out.green = c.green * scale * Ldmax;
    out.blue = c.blue * scale * Ldmax;

    return out;
}

float wardScale(float L_avg, float Ldmax)
{
    return pow((1.219f + pow(Ldmax / 2.0f, 0.4f)) /
        (1.219f + pow(L_avg, 0.4f)), 2.5f);
}

Color toneMapWard(const Color& c, float scale)
{
    Color out;
    out.red = c.red * scale;
    out.green = c.green * scale;
    out.blue = c.blue * scale;
    return out;
}
