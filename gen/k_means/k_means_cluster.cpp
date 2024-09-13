#include <cstdint>
#include <cstring>
#include <cassert>
#include <limits>

uint32_t xorshift32(uint32_t * state)
{
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}

/*
  return: point indicies in `ix`
 */
void random_k_points(uint32_t * random_state,
		     int k,
		     int num_points,
		     int * point_indices)
{
  int indices[num_points];
  for (int i = 0; i < num_points; i++) {
    indices[i] = i;
  }

  for (int i = 0; i < k; i++) {
    int ix = xorshift32(random_state) % num_points;
    num_points -= 1;
    int point_ix = indices[ix];
    *point_indices++ = point_ix;
    memmove(&indices[ix], &indices[ix + 1], (num_points - ix) * (sizeof (indices[0])));
  }
}

template <int dimension>
double distance_squared(double a[dimension], double b[dimension])
{
  double sum = 0;
  for (int i = 0; i < dimension; i++) {
    double c = a[i] - b[i];
    sum += c * c;
  }
  return sum;
}

template <int dimension>
void calculate_centroid(double points[][dimension], int cluster[], int length, double out[dimension])
{
  for (int i = 0; i < dimension; i++) {
    out[i] = 0;
  }

  for (int d = 0; d < dimension; d++) {
    for (int i = 0; i < length; i++) {
      out[d] += points[cluster[i]][d];
    }
    out[d] /= (double)length;
  }
}

template <int dimension>
int minimum_distance_centroid(double point[dimension],
			      double centroids[][dimension],
			      int k)
{
  double min_distance = distance_squared<dimension>(point, centroids[0]);
  int min_ix = 0;
  for (int centroid_ix = 1; centroid_ix < k; centroid_ix++) {
    double distance = distance_squared<dimension>(point, centroids[centroid_ix]);
    if (distance < min_distance) {
      min_distance = distance;
      min_ix = centroid_ix;
    }
  }
  return min_ix;
}

constexpr double epsilon = 0.000001;

template <int dimension>
bool point_equal(double a[dimension], double b[dimension])
{
  for (int i = 0; i < dimension; i++) {
    if (a[i] - b[i] > epsilon)
      return false;
  }
  return true;
}

template <int dimension>
void set_vector(double dst[dimension], double src[dimension])
{
  for (int i = 0; i < dimension; i++) {
    dst[i] = src[i];
  }
}

template <int dimension>
void k_means_cluster(uint32_t * random_state,
		     int k,
		     double points[][dimension],
		     int length,
		     double out[][dimension])
{
  int centroid_indices[k];
  random_k_points(random_state, k, length, centroid_indices);
  double centroids[k][dimension];
  for (int i = 0; i < k; i++) {
    set_vector<dimension>(centroids[i], /* = */ points[centroid_indices[i]]);
  }

  // 268.4 MB stack usage at 1024×1024 px
  int clusters[k][length];
  int cluster_lengths[length];

  while (true) {
    // clear cluster lengths
    for (int i = 0; i < length; i++) { cluster_lengths[i] = 0; }

    // assign each point to the closest centroid
    for (int point_ix = 0; point_ix < length; point_ix++) {
      int min_cluster_ix = minimum_distance_centroid<dimension>(points[point_ix], centroids, k);
      clusters[min_cluster_ix][cluster_lengths[min_cluster_ix]] = point_ix;
      cluster_lengths[min_cluster_ix]++;
    }

    // calculate new centroids
    bool converged = true;
    for (int cluster_ix = 0; cluster_ix < k; cluster_ix++) {
      double new_centroid[dimension];
      calculate_centroid<dimension>(points, clusters[cluster_ix], cluster_lengths[cluster_ix], new_centroid);
      converged &= point_equal<dimension>(new_centroid, centroids[cluster_ix]);
      set_vector<dimension>(centroids[cluster_ix], /* = */ new_centroid);
    }

    if (converged)
      break;
  }

  // return centroids
  for (int centroid_ix = 0; centroid_ix < k; centroid_ix++) {
    set_vector<dimension>(out[centroid_ix], /* = */ centroids[centroid_ix]);
  }
}
