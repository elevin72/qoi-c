
#include <fcntl.h>
#include <png.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qoi.h"

const char MAGIC[4] = {'q', 'o', 'i', 'f'};

// const Pixel R, G, B, A, EMPTY;
const Pixel R = {0xFF, 0x00, 0x00, 0x00};
const Pixel G = {0x00, 0xFF, 0x00, 0x00};
const Pixel B = {0x00, 0x00, 0xFF, 0x00};
const Pixel A = {0x00, 0x00, 0x00, 0xFF};
const Pixel EMPTY = {0x00, 0x00, 0x00, 0x00};

const uint8_t QOI_OP_RUN_TAG = 0xC0;  /*11xxxxxx*/
const uint8_t QOI_OP_LUMA_TAG = 0x80; /*10xxxxxx*/
const uint8_t QOI_OP_DIFF_TAG = 0x40; /*01xxxxxx*/
const uint8_t QOI_OP_INDEX_TAG = 0;   /*00xxxxxx*/
const uint8_t QOI_OP_RGB_TAG = 0xFE;  /*11111110*/
const uint8_t QOI_OP_RGBA_TAG = 0xFF; /*11111111*/

// recieve as a signed int since then it is easier to check in a range like
// -2..1
static inline int8_t InDiffRange(int8_t num) { return num >= -2 && num <= 1; }
static inline int8_t InLumaRange_G(int8_t num) {
    return num >= -32 && num <= 31;
}
static inline int8_t InLumaRange_BR(int8_t num) {
    return num >= -8 && num <= 7;
}
static inline uint8_t HashIndex(Pixel pixel) {
    return ((pixel.rgba.r * 3) + (pixel.rgba.g * 5) + (pixel.rgba.b * 7) +
            (pixel.rgba.a * 11)) %
           64;
}

// BIAS means bump forward/backward by that amount

int Encode(QoiHeader *qoiHeader, uint8_t *data, size_t numBytes, char *file) {
    Pixel a;
    FILE *output = fopen(file, "w+");
    if (output == NULL) {
        fprintf(stderr, "fopen() failed\n");
    }

    // write header
    fwrite(MAGIC, 1, 4, output);
    fwrite(&qoiHeader->height, 4, 1, output);
    fwrite(&qoiHeader->width, 4, 1, output);
    fwrite(&qoiHeader->channels, 1, 1, output);
    fwrite(&qoiHeader->colorspace, 1, 1, output);

    Pixel previouslySeen[64];
    memset(previouslySeen, 0, sizeof(previouslySeen));
    Pixel pixel, previousPixel;
    previousPixel.rgba = (RGBA){.r = 0, .g = 0, .b = 0, .a = 0xFF};

    uint8_t alphaMask = qoiHeader->channels == 3 ? 0xFF : 0;

    for (int i = 0; i < numBytes; i += qoiHeader->channels) {
        // cast address of byte to address of pixel and apply alpha mask
        pixel.rgba.r = data[i];
        pixel.rgba.g = data[i + 1];
        pixel.rgba.b = data[i + 2];
        pixel.rgba.a = data[i + 3] | alphaMask;
        uint8_t index = HashIndex(pixel);

        if (previousPixel.raw == pixel.raw) {
            uint8_t runLength = 0;
            while (previousPixel.raw == pixel.raw && runLength <= 62 &&
                   i < numBytes) {
                ++runLength;
                i += qoiHeader->channels;
            }
            // write QOI_OP_RUN with run length
            // apply bias
            runLength -= 1;
            uint8_t qoi_op_run = QOI_OP_RUN_TAG | runLength;
            fwrite(&qoi_op_run, 1, 1, output);

        } else if (previouslySeen[index].raw == pixel.raw) {
            // write QOI_OP_INDEX to stream
            uint8_t qoi_op_index = QOI_OP_INDEX_TAG | index;
            fwrite(&qoi_op_index, 1, 1, output);

        } else if (pixel.rgba.a == previousPixel.rgba.a) {
            uint8_t dr = pixel.rgba.r - previousPixel.rgba.r;
            uint8_t dg = pixel.rgba.g - previousPixel.rgba.g;
            uint8_t db = pixel.rgba.b - previousPixel.rgba.b;
            uint8_t dr_dg = dr - dg;
            uint8_t db_dg = dr - dg;

            if (InDiffRange(dr) && InDiffRange(dg) && InDiffRange(db)) {
                // write QOI_OP_DIFF to stream
                // apply bias and shift
                dr = (dr + 2) << 4;
                dg = (dg + 2) << 2;
                db += 2;
                uint8_t qoi_op_diff = QOI_OP_DIFF_TAG | dr | dg | db;
                fwrite(&qoi_op_diff, 1, 1, output);

            } else if (InLumaRange_G(dg) && InLumaRange_BR(dr_dg) &&
                       InLumaRange_BR(db_dg)) {
                // write QOI_OP_LUMA

                // apply bias
                dg += 32;
                dr_dg = (dr_dg + 8) << 4;
                db_dg += 8;
                uint8_t byte0 = QOI_OP_LUMA_TAG | dg;
                uint8_t byte1 = dr_dg | db_dg;
                uint8_t qoi_op_luma[2] = {byte0, byte1};
                fwrite(&qoi_op_luma, 2, 1, output);

            } else {
                // write QOI_OP_RGB to stream
                uint8_t qoi_op_rgb[4] = {QOI_OP_RGB_TAG, pixel.rgba.r, pixel.rgba.g,
                                         pixel.rgba.b};
                fwrite(&qoi_op_rgb, 4, 1, output);
            }
        } else {
            // write QOI_OP to stream
            uint8_t qoi_op_rgb[5] = {QOI_OP_RGB_TAG, pixel.rgba.r, pixel.rgba.g, pixel.rgba.b,
                                     pixel.rgba.a};
            fwrite(&qoi_op_rgb, 5, 1, output);
        }
        previousPixel = previouslySeen[index] = pixel;
    }
    fclose(output);
    return 0;
}
