
#include <fcntl.h>
#include <png.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qoi.h"


uint8_t *Encode(QoiHeader *qoiHeader, uint8_t *data, size_t numBytes, uint32_t *outputSize) {
    uint8_t *buf = malloc(HEADER_LENGTH + numBytes + FOOTER_LENGTH);
    uint32_t size = 0;

    // write header
    QOI_Write32(buf, MAGIC, &size);
    QOI_Write32(buf, (uint8_t *)&qoiHeader->height, &size);
    QOI_Write32(buf, (uint8_t *)&qoiHeader->width, &size);
    QOI_Write8(buf, &qoiHeader->channels, &size);
    QOI_Write8(buf, &qoiHeader->colorspace, &size);

    Pixel previouslySeen[64];
    memset(previouslySeen, 0, sizeof(previouslySeen));
    Pixel pixel, previousPixel;
    previousPixel.rgba = (RGBA){.r = 0, .g = 0, .b = 0, .a = 0xFF};

    uint8_t alphaMask = qoiHeader->channels == 3 ? 0xFF : 0;

    for (int i = 0; i < numBytes; i += qoiHeader->channels) {
        pixel.rgba.r = data[i];
        pixel.rgba.g = data[i + 1];
        pixel.rgba.b = data[i + 2];
        pixel.rgba.a = data[i + 3] | alphaMask;
        uint32_t index = QOI_HashIndex(pixel);

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
            QOI_Write8(buf, &qoi_op_run, &size);

        } else if (previouslySeen[index].raw == pixel.raw) {
            // write QOI_OP_INDEX to stream
            uint8_t qoi_op_index = QOI_OP_INDEX_TAG | index;
            QOI_Write8(buf, &qoi_op_index, &size);

        } else if (pixel.rgba.a == previousPixel.rgba.a) {
            uint8_t dr = pixel.rgba.r - previousPixel.rgba.r;
            uint8_t dg = pixel.rgba.g - previousPixel.rgba.g;
            uint8_t db = pixel.rgba.b - previousPixel.rgba.b;
            uint8_t dr_dg = dr - dg;
            uint8_t db_dg = db - dg;
            // dr_dg + dg + prevpixel.r = pixel.r
            // pixel.r = prevpixel.r + pixel.g - prevpixel.g + dr_dg

            if (QOI_InDiffRange(dr) && QOI_InDiffRange(dg) &&
                QOI_InDiffRange(db)) {
                // write QOI_OP_DIFF to stream
                // apply bias and shift
                dr = (dr + 2) << 4;
                dg = (dg + 2) << 2;
                db += 2;
                uint8_t qoi_op_diff = QOI_OP_DIFF_TAG | dr | dg | db;
                QOI_Write8(buf, &qoi_op_diff, &size);

            } else if (QOI_InLumaRange(dg) && QOI_InLumaRange_BR(dr_dg) &&
                       QOI_InLumaRange_BR(db_dg)) {
                // write QOI_OP_LUMA
                // apply bias
                dg += 32;
                dr_dg = (dr_dg + 8) << 4;
                db_dg += 8;
                uint8_t byte0 = QOI_OP_LUMA_TAG | dg;
                uint8_t byte1 = dr_dg | db_dg;
                uint8_t qoi_op_luma[2] = {byte0, byte1};
                QOI_Write16(buf, qoi_op_luma, &size);

            } else {
                // write QOI_OP_RGB to stream
                uint8_t qoi_op_rgb[4] = {QOI_OP_RGB_TAG, pixel.rgba.r,
                                         pixel.rgba.g, pixel.rgba.b};
                QOI_Write32(buf, qoi_op_rgb, &size);
            }
        } else {
            // write QOI_OP to stream
            uint8_t qoi_op_rgba[5] = {QOI_OP_RGB_TAG, pixel.rgba.r,
                                      pixel.rgba.g, pixel.rgba.b, pixel.rgba.a};
            QOI_Write8(buf, qoi_op_rgba, &size);
            QOI_Write32(buf, qoi_op_rgba, &size);
        }
        previousPixel = previouslySeen[index] = pixel;
    }

    // write footer
    QOI_Write32(buf, FOOTER, &size);
    QOI_Write32(buf, FOOTER + 4, &size);
    *outputSize = size;
    return buf;
}
