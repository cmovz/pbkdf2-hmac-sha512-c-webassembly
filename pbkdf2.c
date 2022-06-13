#include "hmac.h"
#include "memory.h"

void pbkdf2(
  const uint8_t *pass, size_t pass_len,
  uint8_t *salt, size_t salt_len,
  size_t c, void *output
)
{
  HMAC_CTX ctx;
  hmac_init(&ctx, pass, pass_len);

  uint64_t buffer[8];
  uint64_t temp_buffer[8];

  salt[salt_len++] = 0x00;
  salt[salt_len++] = 0x00;
  salt[salt_len++] = 0x00;
  salt[salt_len++] = 0x01;

  hmac(&ctx, salt, salt_len, buffer);
  memcpy(temp_buffer, buffer, 64);
  for (size_t i = 1; i < c; ++i) {
    hmac(&ctx, temp_buffer, 64, temp_buffer);
    for (int j = 0; j < 8; ++j)
      buffer[j] ^= temp_buffer[j];
  }
  memcpy(output, buffer, 64);
}