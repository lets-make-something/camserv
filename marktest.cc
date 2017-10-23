#include "markpos.h"
#include "png/png.h"

#include <fstream>
#include <stdint.h>
#include <vector>
#include <random>
#include <algorithm>
#include <array>
#include <map>

void writestepcolor(const char *name, uint8_t *mono, int width, int height) {
  using namespace std;
  vector<uint8_t> data;
  data.resize(width*height * 3);
  vector<uint8_t> pal;
  pal.resize(256);
  mt19937 mt(123456);
  for (int i = 0; i < 256; i++) {
    int a = mt() % 256;
    pal[i] = a;
  }
  for (int i = 0; i < width*height; i++) {
    if (mono[i]) {
      data[i * 3 + 0] = pal[mono[i]%256];
      data[i * 3 + 1] = 255 - pal[mono[i] % 256];
      data[i * 3 + 2] = mono[i] % 256;
    }
    else {
      data[i * 3 + 0] = 0;
      data[i * 3 + 1] = 0;
      data[i * 3 + 2] = 0;
    }
  }
  wts::Raw raw;
  wts::WriteFromR8G8B8(&raw, width, height, data.data());

  ofstream of(name, ios::binary | ios::out);
  if (of.is_open()) {
    of.write(reinterpret_cast<const char*>(raw.data), raw.size);
  }
}

void writestep(const char *name, uint8_t *mono, int width, int height) {
  using namespace std;
  vector<uint8_t> data;
  data.resize(width*height * 3);
  for (int i = 0; i < width*height; i++) {
    data[i * 3 + 0] = mono[i];
    data[i * 3 + 1] = mono[i];
    data[i * 3 + 2] = mono[i];
  }
  wts::Raw raw;
  wts::WriteFromR8G8B8(&raw, width, height, data.data());

  ofstream of(name, ios::binary | ios::out);
  if (of.is_open()) {
    of.write(reinterpret_cast<const char*>(raw.data), raw.size);
  }
}

void filter(uint8_t *mono, int width, int height,int *pattern) {
  std::vector<uint8_t> out;
  out.resize(width*height);
  for (int y = 1; y < height-1; y++)
  {
    for (int x = 1; x < width-1; x++)
    {
    }
  }
}

int main() {
  using namespace std;
  ifstream file("../mark.png", ios::binary | ios::in);
  if (file.is_open()) {
    vector<uint8_t> data;
    file.seekg(0, file.end);
    data.resize((int)file.tellg());
    file.seekg(0, file.beg);
    file.read(reinterpret_cast<char*>(&*data.begin()), data.size());
    file.close();

    wts::Raw raw = { &*data.begin(),data.size() };
    wts::Png png;
    wts::ReadFromRaw(&png, &raw);

  }
}
