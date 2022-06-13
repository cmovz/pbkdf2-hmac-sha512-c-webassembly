void *memcpy(void *dest, const void *src, unsigned long size)
{
  return __builtin_memcpy(dest, src, size);
}

void *memset(void *mem, int c, unsigned long size)
{
  return __builtin_memset(mem, c, size);
}

