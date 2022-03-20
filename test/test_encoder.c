
#include <criterion/criterion.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../src/encoder.c"

const int HEADER_LENGTH = 14;

uint8_t *MakeHeader(uint32_t height, uint32_t width, uint8_t channels) {
    uint8_t *h = (uint8_t *)&height;
    uint8_t *w = (uint8_t *)&width;
    uint8_t header[14] = {'q',  'o',  'i',  'f',  h[0], h[1],     h[2],
                          h[3], w[0], w[1], w[2], w[3], channels, 0};
    uint8_t *ptr = (uint8_t *)malloc(14);
    memcpy(ptr, header, 14);
    return ptr;
}

int CompareMinusHeader(uint8_t *actual, uint8_t *expect, int n) {
    return !memcmp(actual + HEADER_LENGTH, expect, n);
}

Test(encoder, run) {
    QoiHeader q = {
        .height = 1,
        .width = 2,
        .channels = 3,
        .colorspace = 0,
    };

    uint8_t data[] = {0x12, 0x34, 0x56, 0x12, 0x34, 0x56};
    uint8_t expect[5] = {0xFE, 0x12, 0x34, 0x56, 0xc0};
    uint8_t actual[5 + HEADER_LENGTH];
    Encode(&q, data, 6, "test.qoi");
    FILE *output = fopen("test.qoi", "r");
    fread(actual, 5 + HEADER_LENGTH, 1, output);
    cr_assert(CompareMinusHeader(actual, expect, 5), "run test failed");
}

Test(encoder, index) {
    QoiHeader q = {
        .height = 1,
        .width = 3,
        .channels = 3,
        .colorspace = 0,
    };

    uint8_t data[] = {0xcc, 0xdd, 0xcc, 0xaa, 0xbb, 0xaa, 0xcc, 0xdd, 0xcc};
    uint8_t expect[5] = {0xFE, 0xcc, 0xdd, 0xcc, 0xc0};
    uint8_t actual[5 + HEADER_LENGTH];
    Encode(&q, data, 6, "test.qoi");
    FILE *output = fopen("test.qoi", "r");
    fread(actual, 5 + HEADER_LENGTH, 1, output);
    cr_assert(CompareMinusHeader(actual, expect, 5), "run test failed");
}
