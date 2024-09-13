#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <ctime>
#include <cmath>
#include <cinttypes>
#include <cerrno>
#include <bit>
#include <sys/resource.h>

#include "k_means_cluster.cpp"
#include "ppm.h"

#include "twiddle.hpp"
#include "color_format.hpp"

void rgb_to_vectors(const uint8_t * rgb, int width, int height, double vectors[][12])
{
  for (int ty = 0; ty < height / 2; ty++) {
    for (int tx = 0; tx < width / 2; tx++) {
      int ai = ((ty * 2) + 0) * width + ((tx * 2) + 0);
      int bi = ((ty * 2) + 1) * width + ((tx * 2) + 0);
      int ci = ((ty * 2) + 0) * width + ((tx * 2) + 1);
      int di = ((ty * 2) + 1) * width + ((tx * 2) + 1);

      vectors[ty * width / 2 + tx][0] = static_cast<double>(rgb[ai * 3 + 0]);
      vectors[ty * width / 2 + tx][1] = static_cast<double>(rgb[ai * 3 + 1]);
      vectors[ty * width / 2 + tx][2] = static_cast<double>(rgb[ai * 3 + 2]);

      vectors[ty * width / 2 + tx][3] = static_cast<double>(rgb[bi * 3 + 0]);
      vectors[ty * width / 2 + tx][4] = static_cast<double>(rgb[bi * 3 + 1]);
      vectors[ty * width / 2 + tx][5] = static_cast<double>(rgb[bi * 3 + 2]);

      vectors[ty * width / 2 + tx][6] = static_cast<double>(rgb[ci * 3 + 0]);
      vectors[ty * width / 2 + tx][7] = static_cast<double>(rgb[ci * 3 + 1]);
      vectors[ty * width / 2 + tx][8] = static_cast<double>(rgb[ci * 3 + 2]);

      vectors[ty * width / 2 + tx][9]  = static_cast<double>(rgb[di * 3 + 0]);
      vectors[ty * width / 2 + tx][10] = static_cast<double>(rgb[di * 3 + 1]);
      vectors[ty * width / 2 + tx][11] = static_cast<double>(rgb[di * 3 + 2]);
    }
  }
}

void codepixels_to_rgb(double codebook[][12], uint8_t codepixels[], int width, int height, uint8_t * rgb)
{
  for (int ty = 0; ty < height / 2; ty++) {
    for (int tx = 0; tx < width / 2; tx++) {
      int codepixel = codepixels[ty * width / 2 + tx];
      double (&vector)[12] = codebook[codepixel];
      int ai = ((ty * 2) + 0) * width + ((tx * 2) + 0);
      int bi = ((ty * 2) + 1) * width + ((tx * 2) + 0);
      int ci = ((ty * 2) + 0) * width + ((tx * 2) + 1);
      int di = ((ty * 2) + 1) * width + ((tx * 2) + 1);
      rgb[ai * 3 + 0] = static_cast<uint8_t>(round(vector[0]));
      rgb[ai * 3 + 1] = static_cast<uint8_t>(round(vector[1]));
      rgb[ai * 3 + 2] = static_cast<uint8_t>(round(vector[2]));

      rgb[bi * 3 + 0] = static_cast<uint8_t>(round(vector[3]));
      rgb[bi * 3 + 1] = static_cast<uint8_t>(round(vector[4]));
      rgb[bi * 3 + 2] = static_cast<uint8_t>(round(vector[5]));

      rgb[ci * 3 + 0] = static_cast<uint8_t>(round(vector[6]));
      rgb[ci * 3 + 1] = static_cast<uint8_t>(round(vector[7]));
      rgb[ci * 3 + 2] = static_cast<uint8_t>(round(vector[8]));

      rgb[di * 3 + 0] = static_cast<uint8_t>(round(vector[9]));
      rgb[di * 3 + 1] = static_cast<uint8_t>(round(vector[10]));
      rgb[di * 3 + 2] = static_cast<uint8_t>(round(vector[11]));
    }
  }
}

void palettize_vectors_to_codebook(double codebook[256][12], double vectors[][12], int vectors_length, uint8_t codepixels[])
{
  for (int vector_ix = 0; vector_ix < vectors_length; vector_ix++) {
    int min_cluster_ix = minimum_distance_centroid(vectors[vector_ix], codebook, 256);
    assert(min_cluster_ix <= 255 && min_cluster_ix >= 0);
    codepixels[vector_ix] = min_cluster_ix;
  }
}

double total_rgb_error(uint8_t const * const a, uint8_t const * const b, int length)
{
  double error = 0;
  for (int i = 0; i < length; i++) {
    double d = ((double)a[i]) - ((double)b[i]);
    d = d * d;
    error += d;
  }
  return error;
}

bool endswith(const char * s, const char * tail)
{
  int s_len = strlen(s);
  int tail_len = strlen(tail);

  if (s_len < tail_len) {
    return false;
  }

  int start = s_len - tail_len;
  for (int i = start; i < s_len; i++) {
    printf("%c %c\n", s[i], tail[i - start]);
    if (s[i] != tail[i - start])
      return false;
  }
  return true;
}

template<class T, std::endian target_endian = std::endian::little>
constexpr T byteswap(const T n)
{
  if (std::endian::native != target_endian) {
    return std::byteswap<T>(n);
  } else {
    return n;
  }
}

uint64_t color_convert(double vector[12])
{
  uint64_t texel[4];
  for (int i = 0; i < 4; i++) {
    double r = round(vector[i * 3 + 0]);
    double g = round(vector[i * 3 + 1]);
    double b = round(vector[i * 3 + 2]);
    double a = 255;
    if (r > 255 || g > 255 || b > 255)
      fprintf(stderr, "%.0f %.0f %.0f\n", r, g ,b);
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;
    //assert(r <= 255 && g <= 255 && b <= 255);
    //assert(r >=   0 && g >=   0 && b >=   0);
    texel[i] = color_format::rgb565(a, r, g, b);
  }
  return (texel[3] << 48) | (texel[2] << 32) | (texel[1] << 16) | (texel[0] << 0);
}

int main(int argc, char * argv[])
{
  // set 300MB stack size limit
  const rlim_t stack_size = 300 * 1024 * 1024;
  struct rlimit rl;
  int res;
  res = getrlimit(RLIMIT_STACK, &rl);
  if (res < 0) {
    perror("getrlimit RLIMIT_STACK");
    return -1;
  }
  rl.rlim_cur = stack_size;
  res = setrlimit(RLIMIT_STACK, &rl);
  if (res < 0) {
    perror("setrlimit RLIMIT_STACK");
    return -1;
  }

  if (argc < 3) {
    printf("%s [in_file.ppm] [out_file.vq | out_file.ppm]\n", argv[0]);
    return -1;
  }

  FILE *f = fopen(argv[1], "rb");
  if (f == nullptr) {
    printf("%s: %s\n", argv[1], strerror(errno));
    return -1;
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  uint8_t buf[size + 1];
  ssize_t read_len = fread(buf, size, 1, f);
  assert(read_len == 1);
  fclose(f);
  buf[size] = 0;

  struct ppm_header ppm;
  int success = ppm_parse(buf, size, &ppm);
  if (success < 0) {
    fprintf(stderr, "ppm parse failed\n");
    return -1;
  }
  assert(ppm.length == ppm.width * ppm.height * 3);

  uint32_t random_state = time(NULL);

  int vectors_length = ppm.width * ppm.height / 4;
  double vectors[vectors_length][12];
  rgb_to_vectors(ppm.data, ppm.width, ppm.height, vectors);

  constexpr int codebook_length = 256;
  double codebook[codebook_length][12];

  int rgb_size = ppm.width * ppm.height * 3;
  double min_error = std::numeric_limits<double>::infinity();

  for (int i = 0; i < 10; i++) {
    double new_codebook[codebook_length][12];
    // find locally-optimal codebook
    k_means_cluster<12>(&random_state,
			codebook_length,
			vectors,
			vectors_length,
			new_codebook);

    uint8_t codepixels[vectors_length];
    palettize_vectors_to_codebook(new_codebook, vectors, vectors_length, codepixels);
    uint8_t rgb[rgb_size];
    codepixels_to_rgb(new_codebook, codepixels, ppm.width, ppm.height, rgb);

    double error = total_rgb_error(rgb, ppm.data, rgb_size);
    if (i % 100 == 0)
      printf("%d %.0f\n", i, min_error);
    if (error < min_error) {
      for (int i = 0; i < codebook_length; i++) {
	set_vector<12>(codebook[i], new_codebook[i]);
      }
      min_error = error;
      printf("%d new min_error %.0f\n", i, min_error);
    }
  }

  assert(min_error != std::numeric_limits<double>::infinity());

  uint8_t codepixels[vectors_length];
  palettize_vectors_to_codebook(codebook, vectors, vectors_length, codepixels);

  printf("w %d h %d\n", ppm.width, ppm.height);
  FILE *of = fopen(argv[2], "wb");
  if (endswith(argv[2], ".ppm")) {
    uint8_t rgb_out[rgb_size];
    codepixels_to_rgb(codebook, codepixels, ppm.width, ppm.height, rgb_out);

    fprintf(stderr, "writing ppm\n");
    fprintf(of, "P6\n%d %d\n%d\n", ppm.width, ppm.height, 255);
    ssize_t write_len = fwrite(rgb_out, rgb_size, 1, of);
    assert(write_len == 1);
  } else if (endswith(argv[2], ".vq")) {
    fprintf(stderr, "writing vq codebook\n");
    for (int i = 0; i < codebook_length; i++) {
      uint64_t out = byteswap(color_convert(codebook[i]));
      ssize_t write_len = fwrite(&out, (sizeof (uint64_t)), 1, of);
      assert(write_len == 1);
    }
    fprintf(stderr, "writing vq codepixels\n");
    int codepixel_width = ppm.width / 2;
    int codepixel_height = ppm.height / 2;
    int max_curve_ix = twiddle::from_xy(codepixel_width - 1, codepixel_height - 1, ppm.width, ppm.height);
    uint8_t twiddled_codepixels[max_curve_ix];
    twiddle::texture(twiddled_codepixels, codepixels, codepixel_width, codepixel_height);
    ssize_t write_len = fwrite(twiddled_codepixels, max_curve_ix + 1, 1, of);
    assert(write_len == 1);
  }
  fclose(of);
}
