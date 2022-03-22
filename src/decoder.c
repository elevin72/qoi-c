#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "qoi.h"

uint8_t *Decode(uint8_t *qoiData, QoiHeader *qoiHeader) {
    int isQOIfile = qoiData[0] == 'q' && qoiData[1] == 'o' && qoiData[2] == 'i' &&
                    qoiData[3] == 'f';
    if (!isQOIfile) {
        return NULL;
    }


    uint8_t word[4];
    uint32_t readPtr = 4;

    uint32_t height = *(uint32_t *)QOI_Read32(word, qoiData, &readPtr);
    uint32_t width = *(uint32_t *)QOI_Read32(word, qoiData, &readPtr);
    uint8_t channels = *QOI_Read8(word, qoiData, &readPtr);
    uint8_t colorspace = *QOI_Read8(word, qoiData, &readPtr);

    uint32_t dataLen = height * width;
    uint8_t *data = malloc(dataLen * channels);
    uint32_t writePtr = 0;

    Pixel previouslySeen[64];
    memset(previouslySeen, 0, sizeof(previouslySeen));
    Pixel pixel, previousPixel;
    previousPixel.rgba = (RGBA){.r = 0, .g = 0, .b = 0, .a = 0xFF};

    uint8_t alphaMask = channels == 3 ? 0xFF : 0;

    uint8_t *chunk;
    uint8_t singleChunk, chunkLength, runLength;
    uint32_t pixelCount = 0;

    while (pixelCount < dataLen) {
        runLength = 1;
        if (qoiData[readPtr] == 0xfe) {  // rgb
            chunkLength = 4;
            chunk = QOI_Read32(word, qoiData, &readPtr);
            pixel.rgba = (RGBA){.r = chunk[1],
                                .g = chunk[2],
                                .b = chunk[3],
                                .a = previousPixel.rgba.a};
        } else if (qoiData[readPtr] == 0xff) {  // rgba
            chunkLength = 5;
            chunk = QOI_Read32(word, qoiData + 1, &readPtr);
            pixel.rgba = (RGBA){.r = chunk[0],
                                .g = chunk[1],
                                .b = chunk[2],
                                .a = chunk[3]};
        } else if ((qoiData[readPtr] | 0x3f) == 0x3f) {  // index
            chunkLength = 1;
            singleChunk = *QOI_Read8(word, qoiData, &readPtr);
            pixel = previouslySeen[singleChunk];
        } else if ((qoiData[readPtr] | 0x7f) == 0x7f) {  // diff
            chunkLength = 1;
            singleChunk = *QOI_Read8(word, qoiData, &readPtr);
            uint8_t dr = ((singleChunk & 0x30) >> 4) - 2;
            uint8_t dg = ((singleChunk & 0x0c) >> 2) - 2;
            uint8_t db = (singleChunk & 0x03) - 2;
            pixel.rgba = (RGBA){.r = previousPixel.rgba.r + dr,
                                .g = previousPixel.rgba.g + dg,
                                .b = previousPixel.rgba.b + db,
                                .a = previousPixel.rgba.a};

        } else if ((qoiData[readPtr] | 0xbf) == 0xbf) {  // luma
            chunkLength = 2;
            chunk = QOI_Read16(word, qoiData, &readPtr);
            uint8_t dr = (chunk[0] & 0x0f) - 32;
            uint8_t dg = ((chunk[1] & 0x0c) >> 4) - 8;
            uint8_t db = (chunk[1] & 0x03) - 8;
            uint8_t dr_dg = dr - dg;
            uint8_t db_dg = dr - dg;
            pixel.rgba = (RGBA){.r = dg + dr_dg + previousPixel.rgba.r,
                                .g = previousPixel.rgba.g + dg,
                                .b = db + db_dg + previousPixel.rgba.b,
                                .a = previousPixel.rgba.a};
        } else {  // (bytes[i] | 0xff) == 0xff // run
            chunkLength = 1;
            singleChunk = *QOI_Write8(word, qoiData, &readPtr);
            runLength = (singleChunk & 0x3f) + 1;
            pixel = previousPixel;
        }


        for (int j = 0; j < runLength; ++j) {
            QOI_WriteBuf(data, (uint8_t *)&pixel, &writePtr, channels);
        }

        pixelCount += runLength;
        previouslySeen[QOI_HashIndex(pixel)] = previousPixel = pixel;
    }

    if (!memcmp(qoiData + HEADER_LENGTH + dataLen, FOOTER, FOOTER_LENGTH)) {
        return NULL;
    }

    return data;
}
