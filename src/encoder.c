
#include <fcntl.h>
#include <math.h>
#include <png.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qoi.h"

uint8_t *Encode(QoiHeader *qoiHeader, uint8_t *rawImage, size_t length,
                size_t *outputSize) {
    size_t qoiLen = HEADER_LENGTH + length + FOOTER_LENGTH;
    uint8_t *qoiBuf = malloc(qoiLen);  // rough estimate
    size_t writePtr = 0;

    // write header
    QOI_Write32(qoiBuf, MAGIC, &writePtr);
    QOI_Write32(qoiBuf, (uint8_t *)&qoiHeader->height, &writePtr);
    QOI_Write32(qoiBuf, (uint8_t *)&qoiHeader->width, &writePtr);
    QOI_Write8(qoiBuf, &qoiHeader->channels, &writePtr);
    QOI_Write8(qoiBuf, &qoiHeader->colorspace, &writePtr);

    Pixel previouslySeen[64];
    memset(previouslySeen, 0, sizeof(previouslySeen));
    Pixel pixel, previousPixel;
    previousPixel.rgba = (RGBA){.r = 0, .g = 0, .b = 0, .a = 0xFF};

    uint8_t alphaMask = qoiHeader->channels == 3 ? 0xFF : 0;

    for (int readPtr = 0; readPtr <= length - qoiHeader->channels;
         readPtr += qoiHeader->channels) {
        pixel.rgba.r = rawImage[readPtr];
        pixel.rgba.g = rawImage[readPtr + 1];
        pixel.rgba.b = rawImage[readPtr + 2];
        pixel.rgba.a = rawImage[readPtr + 3] | alphaMask;
        uint32_t index = QOI_HashIndex(pixel);

        uint8_t chunk[5] = {0};
        size_t chunkLength = 0;
        if (previousPixel.raw == pixel.raw) {  // run
            uint8_t runLength;
            for (runLength = 0;
                 runLength < 62 && readPtr <= length - qoiHeader->channels;
                 ++runLength) {
                readPtr += qoiHeader->channels;
                pixel.rgba.r = rawImage[readPtr];
                pixel.rgba.g = rawImage[readPtr + 1];
                pixel.rgba.b = rawImage[readPtr + 2];
                pixel.rgba.a = rawImage[readPtr + 3] | alphaMask;
                if (previousPixel.raw != pixel.raw) {
                    pixel = previousPixel;
                    ++runLength;
                    break;
                }
            }

            runLength -= 1;
            readPtr -= qoiHeader->channels;
            chunk[0] = QOI_OP_RUN_TAG | runLength;
            chunkLength = 1;

        } else if (previouslySeen[index].raw == pixel.raw) {
            chunk[0] = QOI_OP_INDEX_TAG | index;
            chunkLength = 1;
        } else if (pixel.rgba.a == previousPixel.rgba.a) {
            uint8_t dr = pixel.rgba.r - previousPixel.rgba.r;
            uint8_t dg = pixel.rgba.g - previousPixel.rgba.g;
            uint8_t db = pixel.rgba.b - previousPixel.rgba.b;
            uint8_t dr_dg = dr - dg;
            uint8_t db_dg = db - dg;

            if (QOI_InDiffRange(dr) && QOI_InDiffRange(dg) &&
                QOI_InDiffRange(db)) {
                dr = (dr + 2) << 4;
                dg = (dg + 2) << 2;
                db += 2;
                chunk[0] = QOI_OP_DIFF_TAG | dg | dr | db;
                chunkLength = 1;

            } else if (QOI_InLumaRange(dg) && QOI_InLumaRange_BR(dr_dg) &&
                       QOI_InLumaRange_BR(db_dg)) {
                dg += 32;
                dr_dg = (dr_dg + 8) << 4;
                db_dg += 8;
                chunk[0] = QOI_OP_LUMA_TAG | dg;
                chunk[1] = dr_dg | db_dg;
                chunkLength = 2;

            } else {
                chunk[0] = QOI_OP_RGB_TAG;
                chunk[1] = pixel.rgba.r;
                chunk[2] = pixel.rgba.g;
                chunk[3] = pixel.rgba.b;
                chunkLength = 4;
            }
        } else {
            chunk[0] = QOI_OP_RGBA_TAG;
            chunk[1] = pixel.rgba.r;
            chunk[2] = pixel.rgba.g;
            chunk[3] = pixel.rgba.b;
            chunk[4] = pixel.rgba.a;
            chunkLength = 5;
        }

        if (writePtr + chunkLength >= qoiLen) {
            uint8_t *newQoiBuf;
            qoiLen = (size_t)fabs(qoiLen * 1.2);
            if ((newQoiBuf = realloc(qoiBuf, qoiLen)) == 0) {
                // alloc error
                return NULL;
            } else {
                qoiBuf = newQoiBuf;
            }
        }

        QOI_WriteBuf(qoiBuf, chunk, &writePtr, chunkLength);
        previousPixel = previouslySeen[index] = pixel;
    }

    // write footer
    QOI_Write32(qoiBuf, FOOTER, &writePtr);
    QOI_Write32(qoiBuf, FOOTER + 4, &writePtr);
    *outputSize = writePtr;
    return qoiBuf;
}
