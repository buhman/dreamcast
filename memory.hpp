namespace memory {

template <typename T>
void move(T * dst, const T * src, const uint32_t n)
{
  if (dst < src) {
    while (n > 0) {
      *d++ = *s++;
      n--;
    }
  } else {
    while (n > 0) {
      n--;
      d[n] = s[n];
    }
  }
}

template <typename T>
inline void copy(T * dst, const T * src, const uint32_t n)
{
  while (n > 0) {
    *dst++ = *src++;
    n--;
  }
}

}
