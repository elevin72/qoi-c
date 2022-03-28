
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spng.h"
#include "qoi.h"

static uint8_t *ptr;


int main(int argc, char *argv[]) {
    size_t size;
    spng_ctx *ctx = spng_ctx_new(0);

    FILE *file = fopen(argv[1], "rb");
    // FILE *file = fopen("libspng-0.7.2/tests/images/basi4a08.png", "rb");
    spng_set_png_file(ctx, file);
    // spng_set_png_buffer(ctx, buf, 1024);
    struct spng_ihdr ihdr;
    spng_get_ihdr(ctx, &ihdr);
    spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &size);
    uint8_t *image = malloc(size);
    spng_decode_image(ctx, image, size, SPNG_FMT_RGBA8, SPNG_DECODE_TRNS);
    size_t width = ihdr.width;
    size_t height = ihdr.height;
    ptr = image;

    QoiHeader q = {
        .height = height,
        .width = width,
        .channels = 4,
        .colorspace = 0,
    };

    size_t c_size;
    uint8_t *out = Encode(&q, image, size, &c_size);
    QoiHeader q_new;
    uint8_t *orig = Decode(out, &q_new);
    int m = memcmp(image, orig, size);
    //printf("%s", m);
    if (m == 0) {
        printf("raw size: %zu    encoded size: %zu\n",size, c_size);
        printf("pass\n");
    } else {
        printf("fail\n");
    }
}

/* int FullCircle(void *data, size_t width, size_t height, size_t channels) {
    QoiHeader q = {
        .height = height,
        .width = width,
        .channels = channels,
        .colorspace = 0,
    };

    size_t c_size;
    size_t size = width * height * channels;
    uint8_t *out = Encode(&q, data, size, &c_size);
    QoiHeader q_new;
    uint8_t *orig = Decode(out, &q_new);
    if (!memcmp(data, orig, size)) {
        printf("pass");
    } else {
        printf("fail");
    }

} */

/* int main(int argc, char* argv[]) {
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

} */
