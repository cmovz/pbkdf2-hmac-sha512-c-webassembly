#include "sha512.h"
#include "memory.h"

#define ROR(x,n) ((x >> n)|(x << (64-n)))
#define CH(e,f,g) ((e & f) ^ ((~e) & g))
#define MAJ(a,b,c) ((a & b) ^ (a & c) ^ (b & c))
#define S0(a) (ROR(a,28) ^ ROR(a,34) ^ ROR(a,39))
#define S1(e) (ROR(e,14) ^ ROR(e,18) ^ ROR(e,41))
#define s0(w,i) (ROR(w[i-15],1) ^ ROR(w[i-15],8) ^ (w[i-15] >> 7))
#define s1(w,i) (ROR(w[i-2],19) ^ ROR(w[i-2],61) ^ (w[i-2] >> 6))

typedef uint64_t u64;

static const u64 k[80] = {
  0x428a2f98d728ae22,0x7137449123ef65cd,0xb5c0fbcfec4d3b2f,0xe9b5dba58189dbbc,
  0x3956c25bf348b538,0x59f111f1b605d019,0x923f82a4af194f9b,0xab1c5ed5da6d8118,
  0xd807aa98a3030242,0x12835b0145706fbe,0x243185be4ee4b28c,0x550c7dc3d5ffb4e2,
  0x72be5d74f27b896f,0x80deb1fe3b1696b1,0x9bdc06a725c71235,0xc19bf174cf692694,
  0xe49b69c19ef14ad2,0xefbe4786384f25e3,0x0fc19dc68b8cd5b5,0x240ca1cc77ac9c65,
  0x2de92c6f592b0275,0x4a7484aa6ea6e483,0x5cb0a9dcbd41fbd4,0x76f988da831153b5,
  0x983e5152ee66dfab,0xa831c66d2db43210,0xb00327c898fb213f,0xbf597fc7beef0ee4,
  0xc6e00bf33da88fc2,0xd5a79147930aa725,0x06ca6351e003826f,0x142929670a0e6e70,
  0x27b70a8546d22ffc,0x2e1b21385c26c926,0x4d2c6dfc5ac42aed,0x53380d139d95b3df,
  0x650a73548baf63de,0x766a0abb3c77b2a8,0x81c2c92e47edaee6,0x92722c851482353b,
  0xa2bfe8a14cf10364,0xa81a664bbc423001,0xc24b8b70d0f89791,0xc76c51a30654be30,
  0xd192e819d6ef5218,0xd69906245565a910,0xf40e35855771202a,0x106aa07032bbd1b8,
  0x19a4c116b8d2d0c8,0x1e376c085141ab53,0x2748774cdf8eeb99,0x34b0bcb5e19b48a8,
  0x391c0cb3c5c95a63,0x4ed8aa4ae3418acb,0x5b9cca4f7763e373,0x682e6ff3d6b2b8a3,
  0x748f82ee5defb2fc,0x78a5636f43172f60,0x84c87814a1f0ab72,0x8cc702081a6439ec,
  0x90befffa23631e28,0xa4506cebde82bde9,0xbef9a3f7b2c67915,0xc67178f2e372532b,
  0xca273eceea26619c,0xd186b8c721c0c207,0xeada7dd6cde0eb1e,0xf57d4f7fee6ed178,
  0x06f067aa72176fba,0x0a637dc5a2c898a6,0x113f9804bef90dae,0x1b710b35131c471b,
  0x28db77f523047d84,0x32caab7b40c72493,0x3c9ebe0a15c9bebc,0x431d67c49c100d4c,
  0x4cc5d4becb3e42b6,0x597f299cfc657e2a,0x5fcb6fab3ad6faec,0x6c44198c4a475817
};

static u64 convert_endianness(u64 x)
{
  return (x << 56)
    | ((x & 0xff00) << 40)
    | ((x & 0xff0000) << 24)
    | ((x & 0xff000000) << 8)
    | ((x & 0xff00000000) >> 8)
    | ((x & 0xff0000000000) >> 24)
    | ((x & 0xff000000000000) >> 40)
    | (x >> 56);
}

static void le_to_be(void* dest_ptr, void const* be, int size)
{
  unsigned char const* src = be;
  unsigned char* dest = dest_ptr;
  int i;

  for(i = size; i > 0; --i)
    dest[size - i] = src[i - 1];
}

static void sha512_compress(SHA512_CTX *ctx, void const *block)
{
  u64 w[80];
  u64 a, b, c, d, e, f, g, h, temp1, temp2;
  const u64 *src = block;
  int i;

  for (i = 0; i < 16; ++i)
    w[i] = convert_endianness(src[i]);

  for (; i < 80; ++i)
    w[i] = w[i-16] + s0(w,i) + w[i-7] + s1(w,i);

  a = ctx->hs[0];
  b = ctx->hs[1];
  c = ctx->hs[2];
  d = ctx->hs[3];
  e = ctx->hs[4];
  f = ctx->hs[5];
  g = ctx->hs[6];
  h = ctx->hs[7];

  for(i = 0; i < 80;){
    temp1 = h + S1(e) + CH(e, f, g) + k[i] + w[i];
    temp2 = S0(a) + MAJ(a, b, c);
    d += temp1;
    h = temp1 + temp2;
    ++i;
  
    temp1 = g + S1(d) + CH(d, e, f) + k[i] + w[i];
    temp2 = S0(h) + MAJ(h, a, b);
    c += temp1;
    g = temp1 + temp2;
    ++i;
  
    temp1 = f + S1(c) + CH(c, d, e) + k[i] + w[i];
    temp2 = S0(g) + MAJ(g, h, a);
    b += temp1;
    f = temp1 + temp2;
    ++i;
  
    temp1 = e + S1(b) + CH(b, c, d) + k[i] + w[i];
    temp2 = S0(f) + MAJ(f, g, h);
    a += temp1;
    e = temp1 + temp2;
    ++i;
  
    temp1 = d + S1(a) + CH(a, b, c) + k[i] + w[i];
    temp2 = S0(e) + MAJ(e, f, g);
    h += temp1;
    d = temp1 + temp2;
    ++i;
  
    temp1 = c + S1(h) + CH(h, a, b) + k[i] + w[i];
    temp2 = S0(d) + MAJ(d, e, f);
    g += temp1;
    c = temp1 + temp2;
    ++i;
  
    temp1 = b + S1(g) + CH(g, h, a) + k[i] + w[i];
    temp2 = S0(c) + MAJ(c, d, e);
    f += temp1;
    b = temp1 + temp2;
    ++i;
  
    temp1 = a + S1(f) + CH(f, g, h) + k[i] + w[i];
    temp2 = S0(b) + MAJ(b, c, d);
    e += temp1;
    a = temp1 + temp2;
    ++i;
  }

  ctx->hs[0] += a;
  ctx->hs[1] += b;
  ctx->hs[2] += c;
  ctx->hs[3] += d;
  ctx->hs[4] += e;
  ctx->hs[5] += f;
  ctx->hs[6] += g;
  ctx->hs[7] += h;
}

void sha512_init(SHA512_CTX *ctx)
{
  ctx->hs[0] = 0x6a09e667f3bcc908;
  ctx->hs[1] = 0xbb67ae8584caa73b;
  ctx->hs[2] = 0x3c6ef372fe94f82b;
  ctx->hs[3] = 0xa54ff53a5f1d36f1;
  ctx->hs[4] = 0x510e527fade682d1;
  ctx->hs[5] = 0x9b05688c2b3e6c1f;
  ctx->hs[6] = 0x1f83d9abfb41bd6b;
  ctx->hs[7] = 0x5be0cd19137e2179;

  ctx->pos = 0;
  ctx->size = 0;
}

void sha512_update(SHA512_CTX *ctx, void const *data, size_t size)
{
  ctx->size += size;

  if (ctx->pos == 0 && size >= SHA512_BLOCK_SIZE) {
    while (size >= SHA512_BLOCK_SIZE) {
      sha512_compress(ctx, data);
      size -= SHA512_BLOCK_SIZE;
      data = (const char*)data + SHA512_BLOCK_SIZE;
    }
  }

  size_t rem = sizeof ctx->buffer - ctx->pos;
  size_t n = (size < rem) ? size : rem;
  size_t source_blocks;

  memcpy(ctx->buffer + ctx->pos, data, n);
  ctx->pos += n;

  /* nothing else to do here */
  if(SHA512_BLOCK_SIZE != ctx->pos)
    return;

  /* consume buffer */
  sha512_compress(ctx, ctx->buffer);

  /* consume any other data directly from source */
  size -= n;
  data = (unsigned char const*)data + n;
  source_blocks = size / SHA512_BLOCK_SIZE;
  ctx->pos = size % SHA512_BLOCK_SIZE;
  while(source_blocks--){
    sha512_compress(ctx, data);
    data = (unsigned char const*)data + SHA512_BLOCK_SIZE;
  }

  /* keep any data that doesn't fill a block */
  memcpy(ctx->buffer, data, ctx->pos);
}

void sha512_final(SHA512_CTX *ctx, void *md)
{
  __uint128_t L = ctx->size * 8;
  int i;

  ctx->buffer[ctx->pos++] = 0x80;
  memset(ctx->buffer + ctx->pos, 0x00, SHA512_BLOCK_SIZE - ctx->pos);

  /* fits? */
  if (ctx->pos + sizeof L <= SHA512_BLOCK_SIZE) {
    le_to_be(ctx->buffer + SHA512_BLOCK_SIZE - sizeof L, &L, sizeof L);
    sha512_compress(ctx, ctx->buffer);
  }
  else {
    sha512_compress(ctx, ctx->buffer);
    memset(ctx->buffer, 0x00, SHA512_BLOCK_SIZE);
    le_to_be(ctx->buffer + SHA512_BLOCK_SIZE - sizeof L, &L, sizeof L);
    sha512_compress(ctx, ctx->buffer);
  }

  u64 *md_ptr = md;
  for (i = 0; i < 8; ++i)
    md_ptr[i] = convert_endianness(ctx->hs[i]);
}

void sha512(void const *m_ptr, size_t m_size, void *md_ptr)
{
  SHA512_CTX ctx;

  sha512_init(&ctx);
  sha512_update(&ctx, m_ptr, m_size);
  sha512_final(&ctx, md_ptr);
}