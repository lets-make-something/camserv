#include "markpos.h"
#include <algorithm>
#include <map>
#include <array>

#include <fstream>
#include <random>
#include <iostream>

std::vector<Progress> progress;

static void toMono(const uint8_t *rgc, int width, int height, uint8_t *momo) {

}

static void sobel(uint8_t *mono, int w, int h) {
  std::vector<uint8_t> tmp_buf;
  tmp_buf.resize(w*h);
  uint8_t *tmp = tmp_buf.data();

  memset(tmp, 0, w*h);
  for (int y = 1; y < h - 1; y++)
  {
    for (int x = 1; x < w - 1; x++)
    {
      tmp[x + y*w] += std::max({ 0,
        (int)mono[(x - 1) + (y - 1)*w] * -1 +
        (int)mono[(x)+(y - 1)*w] * -2 +
        (int)mono[(x + 1) + (y - 1)*w] * -1 +
        (int)mono[(x - 1) + (y + 1)*w] * 1 +
        (int)mono[(x)+(y + 1)*w] * 2 +
        (int)mono[(x + 1) + (y + 1)*w] * 1 });
      tmp[x + y*w] += std::max({ 0,
        (int)mono[(x - 1) + (y - 1)*w] * 1 +
        (int)mono[(x)+(y - 1)*w] * 2 +
        (int)mono[(x + 1) + (y - 1)*w] * 1 +
        (int)mono[(x - 1) + (y + 1)*w] * -1 +
        (int)mono[(x)+(y + 1)*w] * -2 +
        (int)mono[(x + 1) + (y + 1)*w] * -1 });
      tmp[x + y*w] += std::max({ 0,
        (int)mono[(x - 1) + (y - 1)*w] * -1 +
        (int)mono[(x - 1) + (y)*w] * -2 +
        (int)mono[(x - 1) + (y + 1)*w] * -1 +
        (int)mono[(x + 1) + (y - 1)*w] * 1 +
        (int)mono[(x + 1) + (y)*w] * 2 +
        (int)mono[(x + 1) + (y + 1)*w] * 1 });
      tmp[x + y*w] += std::max({ 0,
        (int)mono[(x - 1) + (y - 1)*w] * 1 +
        (int)mono[(x - 1) + (y)*w] * 2 +
        (int)mono[(x - 1) + (y + 1)*w] * 1 +
        (int)mono[(x + 1) + (y - 1)*w] * -1 +
        (int)mono[(x + 1) + (y)*w] * -2 +
        (int)mono[(x + 1) + (y + 1)*w] * -1 });
    }
  }
  memcpy(mono, tmp, w*h);
}

static void labeling(uint8_t *mono, int cw, int ch) {
  //int count = 2;
  //struct Area {
  //  int size, x_min, x_max, y_min, y_max, x_center, y_center;
  //  bool invalid;
  //};
  //map<int, Area> area;
  //for (int y = 1; y < ch - 1; y++)
  //{
  //  for (int x = 1; x < cw - 1; x++)
  //  {
  //    if (count >= 255)
  //      continue;
  //    vector<Pos> stack;
  //    stack.push_back({ x,y });
  //    bool hit = false;
  //    while (stack.size()) {
  //      auto c = stack.back();
  //      stack.pop_back();
  //      if (1 == mono[c.x + cw*c.y]) {
  //        if (hit) {
  //          area[count] = {
  //            area[count].size + 1,
  //            min({ area[count].x_min,c.x }),
  //            max({ area[count].x_max,c.x }),
  //            min({ area[count].y_min,c.y }),
  //            max({ area[count].y_max,c.y }),
  //            area[count].x_center + c.x,
  //            area[count].y_center + c.y,
  //            false
  //          };
  //        }
  //        else {
  //          hit = true;
  //          area[count] = {
  //            1,  c.x,c.x,c.y,c.y,c.x,c.y
  //          };
  //        }
  //        mono[c.x + cw*c.y] = count;
  //        stack.push_back({ c.x,c.y - 1 });
  //        stack.push_back({ c.x - 1,c.y });
  //        stack.push_back({ c.x + 1,c.y });
  //        stack.push_back({ c.x,c.y + 1 });
  //        //if (64 * 64 < area[count].size) {
  //        //  area[count].invalid = true;
  //        //  stack.clear();
  //        //}
  //      }
  //    }
  //    if (hit) {
  //      area[count].x_center /= area[count].size;
  //      area[count].y_center /= area[count].size;
  //      count++;
  //    }
  //  }
  //}
  //progress.resize(progress.size() + 1);
  //progress.back().mono.assign(mono, mono + cw*ch);
  //progress.back().width = cw;
  //progress.back().height = ch;
  //vector<uint8_t> guide;
  //guide.resize(cw*ch);
  //for (int y = 0; y < ch; y++)
  //{
  //  for (int x = 0; x < cw; x++)
  //  {
  //    auto o = mono[x + cw*y];
  //    for (const auto &n : area) {
  //      if (x == n.second.x_center&&y == n.second.y_center) {
  //        o = 255;
  //      }
  //      if ((x == n.second.x_min || x == n.second.x_max) && y < n.second.y_max&&n.second.y_min < y) {
  //        o = 255;
  //      }
  //      if ((y == n.second.y_min || y == n.second.y_max) && x < n.second.x_max&&n.second.x_min < x) {
  //        o = 255;
  //      }
  //    }
  //    guide[x + cw*y] = o;
  //  }
  //}
  //progress.resize(progress.size() + 1);
  //progress.back().mono.assign(mono, mono + cw*ch);
  //progress.back().width = cw;
  //progress.back().height = ch;

}

std::vector<QuadArea> GetMark(uint8_t * rgb, int width, int height)
{
  std::vector<QuadArea> area;
  const int scale = 2;
  const int cw = width / scale;
  const int ch = height / scale;

  using namespace std;
  unsigned char *mono = new unsigned char[cw*ch];
  unsigned char *mono_tmp = new unsigned char[cw*ch];

  // RGB を白黒にする

  unsigned char *c = mono;
  unsigned char *pixel = rgb;
  for (int y = 0; y < height; y+=scale)
  {
    for (int x = 0; x < width; x+=scale)
    {
      int q = 0;
      for (int yy = y; yy < y + scale; yy++) {
        for (int xx = x; xx < x + scale; xx++) {
          q += max({
            rgb[(xx + yy*width) * 3 + 0],
            rgb[(xx + yy*width) * 3 + 1],
            rgb[(xx + yy*width) * 3 + 2],
          });
        }
      }
      *c = q / scale / scale;
      c++;
    }
  }
  progress.clear();

  progress.resize(progress.size() + 1);
  progress.back().mono.assign(mono, mono + cw*ch);
  progress.back().width = cw;
  progress.back().height = ch;

  // 二値化して、黒っぽいところだけ抽出

  c = mono;
  for (int y = 0; y < ch; y++)
  {
    for (int x = 0; x < cw; x++)
    {
      if (y == 0 || x == 0 || y == ch - 1 || x == cw - 1) {
        *c = 0;
      }
      else {
        *c = *c < 64 ? 1 :0;
      }
      c++;
    }
  }
  progress.resize(progress.size() + 1);
  progress.back().mono.assign(mono, mono + cw*ch);
  progress.back().width = cw;
  progress.back().height = ch;

  // 輪郭チェック

  struct Edge {
    Pos p;
    float a;
  };
  int col = 0;

  memset(mono_tmp, 0, cw*ch);
  std::vector<Edge> edge;
  std::vector<Pos> vertex;
  for (int y = 1; y < ch - 1; y++)
  {
    for (int x = 1; x < cw - 1; x++)
    {
      if (0 == mono[(x - 1) + (y - 1)*cw] &&
        0 == mono[(x)+(y - 1)*cw] &&
        0 == mono[(x + 1) + (y - 1)*cw] &&
        0 == mono[(x - 1) + (y)*cw] &&
        0 == mono_tmp[x + y*cw] &&
        mono[x + y*cw]) {
        int sx = x;
        int sy = y;
        Pos p[8] = {
          { -1,1 },
          { 0,1 },
          { 1,1 },
          { 1,0 },
          { 1,-1 },
          { 0,-1 },
          { -1,-1 },
          { -1,0 }
        };
        // 輪郭追跡開始

        edge.clear();
        vertex.clear();
        int mx = x, my = y;
        int s = 0;
        do {
          mono_tmp[mx + my*cw] = 1 + (col % 255);
          edge.push_back({ mx,my });
          for (int i = 0; i < 8; i++) {
            int n = (s + i) % 8;
            if (mono[(mx + p[n].x) + (my + p[n].y)*cw]) {
              mx += p[n].x;
              my += p[n].y;
              s = (n + 6) % 8;
              break;
            }
          }
        } while (mx != sx || my != sy);

        // 輪郭ピクセルの前後の内積を記録

        col++;
        const float thres = cosf(3.1415f * 2 / 360 * 150);
        for (int i = 0; i < edge.size(); i++) {
          auto va = edge[i].p;
          auto vb = edge[(i + 9) % edge.size()].p;
          auto &vo = edge[(i + 4) % edge.size()];
          va.x = vo.p.x - va.x;
          va.y = vo.p.y - va.y;
          vb.x = vo.p.x - vb.x;
          vb.y = vo.p.y - vb.y;
          float la = sqrtf(va.x*va.x + va.y*va.y);
          float lb = sqrtf(vb.x*vb.x + vb.y*vb.y);
          float ax = va.x / la;
          float ay = va.y / la;
          float bx = vb.x / lb;
          float by = vb.y / lb;
          vo.a = ax*bx + ay*by;
        }

        // 周辺から内積の最大部分だけ残す

        for (int i = 0; i < edge.size(); i++) {
          if (thres < edge[i].a) {
            const float m = max({
              edge[(i - 4) % edge.size()].a,
              edge[(i - 3) % edge.size()].a,
              edge[(i - 2) % edge.size()].a,
              edge[(i - 1) % edge.size()].a,
              edge[i].a,
              edge[(i + 1) % edge.size()].a,
              edge[(i + 2) % edge.size()].a,
              edge[(i + 3) % edge.size()].a,
              edge[(i + 4) % edge.size()].a,
            });
            if (m != edge[i].a) {
              for (int j = -4; j <= 4; j++) {
                if (m != edge[(i + j) % edge.size()].a) {
                  edge[(i + j) % edge.size()].a = -1;
                }
              }
            }
          }
        }

        //残った頂点だけ抽出

        for (int i = 0; i < edge.size(); i++) {
          if (thres < edge[i].a) {
            vertex.push_back(edge[i].p);
            mono_tmp[edge[i].p.x + edge[i].p.y*cw] = 254;
          }
        }

        if (4 == vertex.size()) {
          float cross = (vertex[1].x - vertex[0].x)*(vertex[1].y - vertex[2].y) - (vertex[1].y - vertex[0].y)*(vertex[1].x - vertex[2].x);
          //if (0 > cross)
          {
            area.push_back({
              vertex
            });
            for (auto &a : area.back().vertex) {
              a.x *= scale;
              a.y *= scale;
            }
          }
        }

      }
    }
  }
  memcpy(mono, mono_tmp, cw*ch);
  progress.resize(progress.size() + 1);
  progress.back().mono.assign(mono, mono + cw*ch);
  progress.back().width = cw;
  progress.back().height = ch;


  delete[]mono;
  delete[]mono_tmp;
  return area;
}
//
//ColorMatrix GetMatrix(uint8_t * rgb, int width, int height, QuadArea quad)
//{
//  ColorMatrix cm;
//  float scale = 16;
//  int a = 0;
//  int b = 2;
//  int o = 1;
//  float affine[2][3];
//  affine[0][0] = (quad.vertex[b].x - quad.vertex[o].x)/(scale-1);
//  affine[0][1] = (quad.vertex[b].y - quad.vertex[o].y)/(scale-1);
//  affine[0][2] = quad.vertex[o].x;
//  affine[1][0] = (quad.vertex[a].x - quad.vertex[o].x)/(scale-1);
//  affine[1][1] = (quad.vertex[a].y - quad.vertex[o].y)/(scale-1);
//  affine[1][2] = quad.vertex[o].y;
//  cm.mat.resize(scale);
//  for (int y = 0; y < scale; y++) {
//    cm.mat[y].resize(scale);
//    for (int x = 0; x < scale; x++) {
//      int px = x*affine[0][0] + y*affine[0][1] + affine[0][2];
//      int py = x*affine[1][0] + y*affine[1][1] + affine[1][2];
//      if (px < 0)px = 0;
//      if (py < 0)py = 0;
//      if (px >= width)px = width-1;
//      if (py >height)py = height-1;
//      cm.mat[y][x].r = rgb[(px + py*width) * 3 + 0];
//      cm.mat[y][x].g = rgb[(px + py*width) * 3 + 1];
//      cm.mat[y][x].b = rgb[(px + py*width) * 3 + 2];
//    }
//  }
//  return cm;
//}
//
ColorMatrix GetMatrix(uint8_t * rgb, int width, int height, QuadArea quad)
{
  struct Posf {
    float x, y;
  };
  ColorMatrix cm;
  float scale = 16;
  cm.mat.resize(scale);
  for (int y = 0; y < scale; y++) {
    Posf sy = {
      quad.vertex[0].x + (float)(quad.vertex[1].x - quad.vertex[0].x) / (scale - 1)*y,
      quad.vertex[0].y + (float)(quad.vertex[1].y - quad.vertex[0].y) / (scale - 1)*y
    };
    Posf ey = {
      quad.vertex[3].x + (float)(quad.vertex[2].x - quad.vertex[3].x) / (scale - 1)*y,
      quad.vertex[3].y + (float)(quad.vertex[2].y - quad.vertex[3].y) / (scale - 1)*y
    };
    cm.mat[y].resize(scale);
    for (int x = 0; x < scale; x++) {
      Posf p = {
        sy.x + (ey.x - sy.x) / (scale - 1)*x,
        sy.y + (ey.y - sy.y) / (scale - 1)*x,
      };
      int px = p.x;
      int py = p.y;
      cm.mat[y][x].r = rgb[(px + py*width) * 3 + 0];
      cm.mat[y][x].g = rgb[(px + py*width) * 3 + 1];
      cm.mat[y][x].b = rgb[(px + py*width) * 3 + 2];
    }
  }
  return cm;
}
