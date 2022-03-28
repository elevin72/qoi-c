#include "qoi.h"

// const uint32_t HEADER_LENGTH = 14;
// const uint32_t FOOTER_LENGTH = 8;

const uint8_t MAGIC[] = {'q', 'o', 'i', 'f'};
const uint8_t FOOTER[] = {0, 0, 0, 0, 0, 0, 0, 1};

const uint8_t QOI_OP_RUN_TAG = 0xC0;  /*11xxxxxx*/
const uint8_t QOI_OP_DIFF_TAG = 0x40; /*01xxxxxx*/
const uint8_t QOI_OP_LUMA_TAG = 0x80; /*10xxxxxx*/
const uint8_t QOI_OP_INDEX_TAG = 0;   /*00xxxxxx*/
const uint8_t QOI_OP_RGB_TAG = 0xFE;  /*11111110*/
const uint8_t QOI_OP_RGBA_TAG = 0xFF; /*11111111*/

const Pixel R = {0xFF, 0x00, 0x00, 0x00};
const Pixel G = {0x00, 0xFF, 0x00, 0x00};
const Pixel B = {0x00, 0x00, 0xFF, 0x00};
const Pixel A = {0x00, 0x00, 0x00, 0xFF};
const Pixel EMPTY = {0x00, 0x00, 0x00, 0x00};

const uint32_t ZERO = 0;
