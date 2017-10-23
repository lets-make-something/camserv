
#include <vector>

struct Pos {
  int x, y;
};
struct QuadArea {
  std::vector<Pos> vertex;
};

struct ColorMatrix {
  struct RGB {
    uint8_t r, g, b;
  };
  std::vector<std::vector<RGB> > mat;
};

std::vector<QuadArea> GetMark(uint8_t *rgb, int width, int height);
ColorMatrix GetMatrix(uint8_t *rgb, int width, int height, QuadArea quad);

struct Progress {
  std::vector<uint8_t> mono;
  int width,height;
};
extern std::vector<Progress> progress;