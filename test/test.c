#include <criterion/criterion.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "qoi.h"

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

size_t size;

Test(encoder, run) {
    QoiHeader q = {
        .height = 1,
        .width = 2,
        .channels = 3,
        .colorspace = 0,
    };

    uint8_t data[] = {0x12, 0x34, 0x56, 0x12, 0x34, 0x56,
                      0x12, 0x34, 0x56, 0x12, 0x34, 0x56};
    uint8_t expect[5] = {0xFE, 0x12, 0x34, 0x56, 0xc2};
    uint8_t actual[5 + HEADER_LENGTH];
    uint8_t *out = Encode(&q, data, sizeof(data), &size);
    memcpy(actual, out, 19);
    cr_assert(CompareMinusHeader(actual, expect, 5), "run test failed");
}

Test(encoder, index) {
    QoiHeader q = {
        .height = 1,
        .width = 3,
        .channels = 3,
        .colorspace = 0,
    };

    uint8_t data[12] = {0xcc, 0xdd, 0xcc, 0xaa, 0xbb, 0xaa,
                        0xcc, 0xdd, 0xcc, 0xaa, 0xbb, 0xaa};
    uint8_t expect[10] = {0xFE, 0xcc, 0xdd, 0xcc, 0xfe,
                          0xaa, 0xbb, 0xaa, 0x3e, 0x00};
    uint8_t actual[10 + HEADER_LENGTH];
    uint8_t *out = Encode(&q, data, sizeof(data), &size);
    memcpy(actual, out, 24);
    cr_assert(CompareMinusHeader(actual, expect, 10), "index test failed");
}

Test(encoder, diff) {
    QoiHeader q = {
        .height = 1,
        .width = 3,
        .channels = 3,
        .colorspace = 0,
    };

    uint8_t data[12] = {0xcc, 0xdd, 0xcc, 0xca, 0xdb, 0xca, 0xc9, 0xda, 0xc9};
    uint8_t expect[6] = {0xFE, 0xcc, 0xdd, 0xcc, 0x40, 0x55};
    uint8_t actual[20];
    uint8_t *out = Encode(&q, data, sizeof(data), &size);
    memcpy(actual, out, 20);
    cr_assert(CompareMinusHeader(actual, expect, 6), "diff test failed");
}
