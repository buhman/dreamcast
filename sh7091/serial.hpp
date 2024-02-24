namespace serial {

void init(uint8_t bit_rate);

void character(const char c);

void string(const char * s);

template <typename T>
void integer(const T n, const char end);

template <typename T>
void integer(const T n);

}
