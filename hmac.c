#include "hmac.h"
#include "memory.h"

void hmac_init(HMAC_CTX *ctx, const uint8_t *key, size_t key_len)
{
  sha512_init(&ctx->ikctx);
  sha512_init(&ctx->okctx);

  uint8_t ikeypad[128];
  uint8_t okeypad[128];
  uint8_t key1024[128];

  if (key_len > 128) {
    sha512(key, key_len, key1024);
    memset(key1024+64, 0x00, 64);
  }
  else {
    memcpy(key1024, key, key_len);
    memset(key1024+key_len, 0x00, 128-key_len);
  }

  for (int i = 0; i < 128; ++i) {
    ikeypad[i] = 0x36 ^ key1024[i];
  }
  for (int i = 0; i < 128; ++i) {
    okeypad[i] = 0x5c ^ key1024[i];
  }

  sha512_update(&ctx->ikctx, ikeypad, 128);
  sha512_update(&ctx->okctx, okeypad, 128);
}

void hmac(HMAC_CTX *ctx, const void *data, size_t data_len, void *output)
{
  SHA512_CTX ikctx, okctx;
  memcpy(ikctx.hs, ctx->ikctx.hs, 64);
  ikctx.size = 128;
  ikctx.pos = 0;
  memcpy(okctx.hs, ctx->okctx.hs, 64);
  okctx.size = 128;
  okctx.pos = 0;

  uint8_t ikhash[64];
  sha512_update(&ikctx, data, data_len);
  sha512_final(&ikctx, ikhash);
  
  sha512_update(&okctx, ikhash, 64);
  sha512_final(&okctx, output);
}