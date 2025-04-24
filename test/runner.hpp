#pragma once

typedef bool (*test_t)(const char ** scenario);

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_RESET   "\x1b[0m"

#define RUNNER(tests)                                                   \
  int main()                                                            \
  {                                                                     \
    int fail_count = 0;                                                 \
    for (int i = 0; i < (sizeof (tests)) / (sizeof (test_t)); i++) {    \
      const char * scenario = NULL;                                     \
      bool result = tests[i](&scenario);                                \
      const char * result_s = result ? "ok" : ANSI_RED "fail" ANSI_RESET; \
      fail_count += !result;                                            \
      fprintf(stderr, "%s: %s\n", scenario, result_s);                  \
    }                                                                   \
    if (fail_count == 0) {                                              \
      fprintf(stderr, ANSI_GREEN "failed tests: %d\n\n" ANSI_RESET, fail_count); \
    } else {                                                            \
      fprintf(stderr, ANSI_RED "failed tests: %d\n\n" ANSI_RESET, fail_count); \
    }                                                                   \
                                                                        \
    return !(fail_count == 0);                                          \
  }
