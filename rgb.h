struct rgb
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct hsv
{
  unsigned char h;
  unsigned char s;
  unsigned char v;
};

struct rgb hsv_to_rgb(struct hsv hsv);
