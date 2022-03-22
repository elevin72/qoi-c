#ifndef QOI_H
#define QOI_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t channels;
    uint8_t colorspace;
} QoiHeader;

extern const uint32_t HEADER_LENGTH;
extern const uint32_t FOOTER_LENGTH;

extern const uint8_t MAGIC[];
extern const uint8_t FOOTER[];

extern const uint8_t QOI_OP_RUN_TAG;
extern const uint8_t QOI_OP_LUMA_TAG;
extern const uint8_t QOI_OP_DIFF_TAG;
extern const uint8_t QOI_OP_INDEX_TAG;
extern const uint8_t QOI_OP_RGB_TAG;
extern const uint8_t QOI_OP_RGBA_TAG;

extern const Pixel R;
extern const Pixel G;
extern const Pixel B;
extern const Pixel A;
extern const Pixel EMPTY;

extern const uint32_t ZERO;

// recieve as a signed int since then it is easier to check in a range like
// -2..1
static inline int8_t QOI_InDiffRange(int8_t num) {
    return num >= -2 && num <= 1;
}
static inline int8_t QOI_InLumaRange(int8_t num) {
    return num >= -32 && num <= 31;
}
static inline int8_t QOI_InLumaRange_BR(int8_t num) {
    return num >= -8 && num <= 7;
}
static inline uint8_t QOI_HashIndex(Pixel pixel) {
    return ((pixel.rgba.r * 3) + (pixel.rgba.g * 5) + (pixel.rgba.b * 7) +
            (pixel.rgba.a * 11)) %
           64;
}

// write n bytes from src to dest, while incrementing the counter of size
static inline uint8_t *QOI_WriteBuf(uint8_t *dest, const uint8_t *src,
                                    uint32_t *writeOffset, uint32_t n) {
    uint8_t *ret = (uint8_t *)memcpy(dest + (*writeOffset), src, n);
    *writeOffset += n;
    return ret;
}

static inline uint8_t *QOI_ReadBuf(uint8_t *dest, const uint8_t *src,
                                   uint32_t *readOffset, uint32_t n) {
    uint8_t *ret = (uint8_t *)memcpy(dest, src + (*readOffset), n);
    *readOffset += n;
    return ret;
}

#define QOI_Write32(dest, src, writeOffset) \
    QOI_WriteBuf(dest, src, writeOffset, 4)
#define QOI_Write16(dest, src, writeOffset) \
    QOI_WriteBuf(dest, src, writeOffset, 2)
#define QOI_Write8(dest, src, writeOffset) \
    QOI_WriteBuf(dest, src, writeOffset, 1)

#define QOI_Read32(dest, src, readOffset) \
    QOI_ReadBuf(dest, src, readOffset, 4)
#define QOI_Read16(dest, src, readOffset) \
    QOI_ReadBuf(dest, src, readOffset, 2)
#define QOI_Read8(dest, src, readOffset) \
    QOI_ReadBuf(dest, src, readOffset, 1)

/* #define QOI_Read32(dest, src, size) QOI_CopyBuf(dest, src, size, 4)
#define QOI_Read8(dest, src, size) QOI_CopyBUf(dest, src, size, 1) */

uint8_t *Encode(QoiHeader *header, uint8_t *bytes, size_t numPixels,
                uint32_t *outputSize);
uint8_t *Decode(uint8_t *bytes, QoiHeader *qoiHeader);

#endif
