#pragma once

enum colors {
    White,
    Pink,
    Red,
    Orange,
    Yellow,
    Green,
    Blue,
    Indigo,
    Violet
};

struct colorRGB {
    int r;
    int g;
    int b;
    colorRGB() = default;
    colorRGB(int R, int G, int B) : r(R) , g(G) , b(B) {}
};

struct colorHSL {
    float h;
    float s;
    float l;
    colorHSL() = default;
    colorHSL(float H, float S, float L) : h(H), s(S), l(L) {}
};

colorRGB hslToRGB(colorHSL);
colorRGB hslToRGB(float, float, float);
colorHSL rgbToHSL(colorRGB);
colorHSL rgbToHSL(int, int, int);
colorRGB getColorFade(float, colorRGB, colorRGB);
colorRGB getColorFade(float, int, int);
