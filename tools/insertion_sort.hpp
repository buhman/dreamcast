#include <cstdint>
#include <algorithm>
#include <iostream>

template <typename T>
void insertion_sort(T * arr, int len)
{
  int i = 1;
  while (i < len) {
    int j = i;
    while (j > 0 && arr[j - 1] < arr[j]) {
      std::swap(arr[j - 1], arr[j]);
      j -= 1;
    }
    i += 1;
  }
}
