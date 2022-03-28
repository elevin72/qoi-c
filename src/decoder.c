#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qoi.h"

const uint8_t QOI_BIT_MASK = 0xc0;

uint8_t *Decode(uint8_t *qoiData, QoiHeader *qoiHeader) {
    int isQOIfile = qoiData[0] == 'q' && qoiData[1] == 'o' &&
                    qoiData[2] == 'i' && qoiData[3] == 'f';
    if (!isQOIfile) {
        return NULL;
    }

    uint8_t word[4];
    size_t readPtr = 4;

    uint32_t height = *(uint32_t *)QOI_Read32(word, qoiData, &readPtr);
    uint32_t width = *(uint32_t *)QOI_Read32(word, qoiData, &readPtr);
    uint8_t channels = *QOI_Read8(word, qoiData, &readPtr);
    uint8_t colorspace = *QOI_Read8(word, qoiData, &readPtr);

    uint32_t dataLen = height * width;
    uint8_t *data = malloc(dataLen * channels);
    size_t writePtr = 0;

    Pixel previouslySeen[64];
    memset(previouslySeen, 0, sizeof(previouslySeen));
    Pixel pixel, previousPixel;
    previousPixel.rgba = (RGBA){.r = 0, .g = 0, .b = 0, .a = 0xFF};

    uint8_t alphaMask = channels == 3 ? 0xFF : 0;

    uint8_t runLength;
    uint32_t pixelCount = 0;
    while (pixelCount < dataLen) {
        runLength = 1;
        if (qoiData[readPtr] == QOI_OP_RGB_TAG) {  // rgb
            pixel.rgba = (RGBA){.r = qoiData[readPtr + 1],
                                .g = qoiData[readPtr + 2],
                                .b = qoiData[readPtr + 3],
                                .a = previousPixel.rgba.a};
            readPtr += 4;
        } else if (qoiData[readPtr] == QOI_OP_RGBA_TAG) {  // rgba
            pixel.rgba = (RGBA){.r = qoiData[readPtr + 1],
                                .g = qoiData[readPtr + 2],
                                .b = qoiData[readPtr + 3],
                                .a = qoiData[readPtr + 4]};
            readPtr += 5;
        } else if ((qoiData[readPtr] & QOI_BIT_MASK) ==
                   QOI_OP_LUMA_TAG) {  // luma
            uint8_t dg = (qoiData[readPtr] & 0x3f) - 32;
            uint8_t dr = ((qoiData[readPtr + 1] & 0xf0) >> 4) - 8;
            uint8_t db = (qoiData[readPtr + 1] & 0x0f) - 8;
            pixel.rgba = (RGBA){.r = dr + dg + previousPixel.rgba.r,
                                .g = previousPixel.rgba.g + dg,
                                .b = db + dg + previousPixel.rgba.b,
                                .a = previousPixel.rgba.a};
            readPtr += 2;
        } else if ((qoiData[readPtr] & QOI_BIT_MASK) ==
                   QOI_OP_INDEX_TAG) {  // index
            pixel = previouslySeen[qoiData[readPtr]];
            readPtr += 1;
        } else if ((qoiData[readPtr] & QOI_BIT_MASK) ==
                   QOI_OP_DIFF_TAG) {  // diff
            uint8_t dr = ((qoiData[readPtr] & 0x30) >> 4) - 2;
            uint8_t dg = ((qoiData[readPtr] & 0x0c) >> 2) - 2;
            uint8_t db = (qoiData[readPtr] & 0x03) - 2;
            pixel.rgba = (RGBA){.r = previousPixel.rgba.r + dr,
                                .g = previousPixel.rgba.g + dg,
                                .b = previousPixel.rgba.b + db,
                                .a = previousPixel.rgba.a};
            readPtr += 1;
        } else {  // (qoiData[readPtr] & QOI_BIT_MASK) == QOI_OP_RUN_TAG
            runLength = (qoiData[readPtr] & 0x3f) + 1;
            pixel = previousPixel;
            readPtr += 1;
        }

        for (int j = 0; j < runLength; ++j) {
            QOI_WriteBuf(data, (uint8_t *)&pixel, &writePtr, channels);
        }

        pixelCount += runLength;
        previouslySeen[QOI_HashIndex(pixel)] = previousPixel = pixel;
    }

    if (memcmp(qoiData + readPtr, FOOTER, FOOTER_LENGTH)) {
        return NULL;
    }

    return data;
}
