#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "qoi.h"

/* uint8_t *MakeHeader(uint32_t height, uint32_t width, uint8_t channels) {
    uint8_t *h = (uint8_t *)&height;
    uint8_t *w = (uint8_t *)&width;
    uint8_t header[14] = {'q',  'o',  'i',  'f',  h[0], h[1],     h[2],
                          h[3], w[0], w[1], w[2], w[3], channels, 0};
    uint8_t *ptr = (uint8_t *)malloc(14);
    memcpy(ptr, header, 14);
    return ptr;
} */

int CompareMinusHeader(uint8_t *actual, uint8_t *expect, int n) {
    return !memcmp(actual, expect + 14, n);
}

int main(int argc, char* argv[]) {
    QoiHeader q = {
        .height = 1,
        .width = 3,
        .channels = 3,
        .colorspace = 0,
    };

    uint32_t size;

    uint8_t data[9] = {0xcc, 0xdd, 0xcc, 0xca, 0xdb, 0xca, 0xc9, 0xda, 0xc9};
    uint8_t expect[6] = {0xFE, 0xcc, 0xdd, 0xcc, 0x40, 0x55};
    uint8_t actual[20];
    uint8_t *out = Encode(&q, data, sizeof(data), &size);
    QoiHeader qNew;
    uint8_t *orig = Decode(out, &qNew);
    if (!memcmp(data, orig, 9)) {
        printf("pass");
    }

}
