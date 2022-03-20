#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

int main(int argc, char* argv[]) {
    QoiHeader q = {
        .height = 1,
        .width = 3,
        .channels = 3,
        .colorspace = 0,
    };

    uint8_t data[] = {0xcc, 0xdd, 0xcc, 0xcb, 0xdc, 0xcb, 0xcc, 0xdd, 0xcc};
    Encode(&q, data, 9, "txt");

}
