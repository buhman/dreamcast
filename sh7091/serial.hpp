namespace serial {

void init();

void character(const char c);

void string(const char * s);

template <typename T>
void integer(const T n, const char end);

template <typename T>
void integer(const T n);

}
