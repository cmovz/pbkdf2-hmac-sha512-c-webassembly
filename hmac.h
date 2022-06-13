#ifndef HMAC_H
#define HMAC_H

#include "sha512.h"
#include <stdint.h>

typedef struct {
  SHA512_CTX ikctx;
  SHA512_CTX okctx;
} HMAC_CTX;

void hmac_init(HMAC_CTX *ctx, const uint8_t *key, size_t key_len);
void hmac(HMAC_CTX *ctx, const void *data, size_t data_len, void *output);

#endif