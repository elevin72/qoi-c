#include "qoi.h"

// recieve as a signed int since then it is easier to check in a range like
// -2..1
/* inline int8_t QOI_InDiffRange(int8_t num) { return num >= -2 && num <= 1; }
inline int8_t QOI_InLumaRange(int8_t num) { return num >= -32 && num <= 31; }
inline int8_t QOI_InLumaRange_BR(int8_t num) { return num >= -8 && num <= 7; }
inline uint8_t QOI_HashIndex(Pixel pixel) {
    return ((pixel.rgba.r * 3) + (pixel.rgba.g * 5) + (pixel.rgba.b * 7) +
            (pixel.rgba.a * 11)) %
           64;
}

// write n bytes from src to dest, while incrementing the counter of size
inline void QOI_WriteBuf(uint8_t *dest, const uint8_t *src, uint32_t *size,
                         uint32_t n) {
    for (int i = 0; i < n; ++i) {
        dest[(*size)++] = src[i];
    }
} */
