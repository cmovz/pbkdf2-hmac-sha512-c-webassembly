#ifndef SHA512_H
#define SHA512_H

#include <stdint.h>

#define SHA512_MD_SIZE 64
#define SHA512_BLOCK_SIZE 128

typedef struct {
  uint64_t hs[8];
  uint8_t buffer[SHA512_BLOCK_SIZE];
  uint64_t size;
  uint32_t pos;
} SHA512_CTX;

typedef unsigned int size_t;

void sha512(void const* m, size_t m_size, void* md);

void sha512_init(SHA512_CTX* ctx);
void sha512_update(SHA512_CTX* ctx, void const* data, size_t size);
void sha512_final(SHA512_CTX* ctx, void* md);

#endif