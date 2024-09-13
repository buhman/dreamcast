#include <stdbool.h>
#include <stdio.h>

#include "runner.h"
#include "../k_means_vq.cpp"

static bool k_means_vq_0(const char ** scenario)
{
  *scenario = "random_k_points all indices";

  uint32_t random_state = 0x12345678;
  const int k = 10;
  const int length = 10;
  int indices[length];
  memset(indices, 0xff, (sizeof (indices)));
  random_k_points(&random_state, k, length, indices);
  bool duplicate = false;
  uint32_t seen = 0;
  for (int i = 0; i < k; i++) {
    uint32_t bit = 1 << indices[i];
    if (seen & bit) duplicate = true;
    seen |= bit;
  }
  return
    duplicate == false &&
    seen == 0x3ff
    ;
}

static bool k_means_vq_1(const char ** scenario)
{
  *scenario = "random_k_points vaugely random";

  uint32_t random_state = 0x12345678;
  const int length = 30;
  const int k = 10;
  int indices[length];
  const int num_tests = 6;
  uint32_t seen[num_tests] = {0};

  bool out_of_range = false;
  bool duplicate = false;
  for (int j = 0; j < num_tests; j++) {
    random_k_points(&random_state, k, length, indices);
    for (int i = 0; i < k; i++) {
      int point_ix = indices[i];
      if (point_ix > length) out_of_range = true;
      uint32_t bit = 1 << point_ix;
      if (seen[j] & bit) duplicate = true;
      seen[j] |= bit;
    }
  }

  return
    out_of_range == false &&
    duplicate == false &&
    seen[0] != 0 &&
    seen[1] != 0 &&
    seen[2] != 0 &&
    seen[3] != 0 &&
    seen[4] != 0 &&
    seen[5] != 0 &&
    seen[0] < 0xffffffff &&
    seen[1] < 0xffffffff &&
    seen[2] < 0xffffffff &&
    seen[3] < 0xffffffff &&
    seen[4] < 0xffffffff &&
    seen[5] < 0xffffffff &&
    seen[0] != seen[1] && seen[0] != seen[2] && seen[0] != seen[3] && seen[0] != seen[4] && seen[0] != seen[5] &&
    seen[1] != seen[2] && seen[1] != seen[3] && seen[1] != seen[4] && seen[1] != seen[5] &&
    seen[2] != seen[3] && seen[2] != seen[4] && seen[2] != seen[5] &&
    seen[3] != seen[4] && seen[3] != seen[5] &&
    seen[4] != seen[5]
    ;
}

static bool equal(double a, double b)
{
  return a - b < 0.00001;
}

static bool k_means_vq_2(const char ** scenario)
{
  *scenario = "calculate_centroid";

  double cluster[2][3] = {
    {1, 5, 9},
    {3, 7, 11},
  };

  double result[3];

  calculate_centroid<3>(&cluster[0], 2, result);

  return
    equal(result[0], 2) &&
    equal(result[1], 6);
    equal(result[2], 10);
}

static bool k_means_vq_3(const char ** scenario)
{
  *scenario = "minimum_distance_centroid";

  int min_ix[2];

  constexpr int k = 5;
  double centroids[k][2] = {
    {5, 10},
    {5, 4},
    {2, 1},
    {10, 20},
    {6, 6},
  };

  {
    double point[2] = {4, 3};
    min_ix[0] = minimum_distance_centroid<2>(point, centroids, k);
  }
  {
    double point[2] = {11, 21};
    min_ix[1] = minimum_distance_centroid<2>(point, centroids, k);
  }

  return
    min_ix[0] == 1 &&
    min_ix[1] == 3;
}

test_t k_means_vq_tests[] = {
  k_means_vq_0,
  k_means_vq_1,
  k_means_vq_2,
  k_means_vq_3,
};

RUNNER(k_means_vq_tests);
