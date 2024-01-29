#include "ColorFunctions.h"

constexpr float div255 = 0.0039216F;

constexpr int aR[9] = { 255, 255, 255, 255, 255, 0, 0, 75, 128 };
constexpr int aG[9] = { 255, 192, 0, 165, 255, 128, 0, 0, 0 };
constexpr int aB[9] = { 255, 203, 0, 0, 0, 0, 255, 130, 128 };

colorRGB hslToRGB(colorHSL colorOne) {

    float C = (1 - abs(2 * colorOne.l - 1)) * colorOne.s;
    float m = colorOne.l - (C * 0.5);
    float H = colorOne.h * 6;
    float X = C * (1 - abs(fmod(H, 2) - 1));
    
    C += m;
    X += m;

    C *= 255;
    X *= 255;
    m *= 255;

    if (H > 5) {
        return colorRGB(C, m, X);
    } else if (H > 4) {
        return colorRGB(X, m, C);
    } else if (H > 3) {
        return colorRGB(m, X, C);
    } else if (H > 2) {
        return colorRGB(m, C, X);
    } else if (H > 1) {
        return colorRGB(X, C, m);
    } else {
        return colorRGB(C, X, m);
    }
    
};

colorRGB hslToRGB(float h, float s, float l) {
	return hslToRGB(colorHSL(h, s, l));
};

colorHSL rgbToHSL(colorRGB source) {
	float R = source.r * div255;
	float G = source.g * div255;
	float B = source.b * div255;

	float M = max(max(R, G), B);
	float m = min(min(R, G), B);
	float C = M - m;

    float hue;
    float sat;
    float lum = (M + m) * 0.5;

    ((lum == 1) || (lum == 0)) ? sat = 0 : sat = C / (1 - abs(2 * lum - 1));

    if (C == 0) {
        hue = 0;
    }
    else {
        float segment;
        float shift;
        if (M == R) {
            segment = (G - B) / C;
            segment < 0 ? shift = 6 : shift = 0;
        }
        else if (M == G) {
            segment = (B - R) / C;
            shift = 2;
        }
        else {
            segment = (R - G) / C;
            shift = 4;
        }
        hue = (segment + shift) * 0.1666666F;
    }

    return colorHSL(hue, sat, lum);

};

colorHSL rgbToHSL(int r, int g, int b) {
	return rgbToHSL(colorRGB(r, g, b));
};

colorRGB getColorFade(float t, colorRGB c1, colorRGB c2) {

    colorHSL colorOne = rgbToHSL(c1);
    colorHSL colorTwo = rgbToHSL(c2);

    float hue;
    float hueDiff = colorTwo.h - colorOne.h;

    if (colorOne.h > colorTwo.h) {
        if ((colorOne.l != 1) && (colorTwo.l != 1)) {
            std::swap(colorOne.h, colorTwo.h);
            hueDiff *= -1;
            t = 1 - t; 
        }
    }

    if (hueDiff > 0.5) {
        colorOne.h += 1;
        hue = fmod((colorOne.h + t * hueDiff), 1);
    } else {
        hue = colorOne.h + t * hueDiff;
    }

    float sat = colorOne.s + t * (colorTwo.s - colorOne.s);
    float lum = colorOne.l + t * (colorTwo.l - colorOne.l);

    return hslToRGB(hue, sat, lum);

};

colorRGB getColorFade(float t, int c1, int c2) {
    return getColorFade(t, colorRGB(aR[c1], aG[c1], aB[c1]), colorRGB(aR[c2], aG[c2], aB[c2]));
};

USHORT rgb888to565(byte r, byte g, byte b) {
    return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
};
