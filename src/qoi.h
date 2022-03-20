#ifndef QOI_H
#define QOI_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RGBA;

typedef union {
    RGBA rgba;
    uint32_t raw;
} Pixel;

static inline uint8_t PixelEquals(Pixel one, Pixel two);

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t channels;
    uint8_t colorspace;
} QoiHeader;

int Encode(QoiHeader* header, uint8_t* bytes, size_t numPixels, char* file);

#endif
