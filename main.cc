#include <uv.h>
#include <memory.h>
#include <filesystem>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <sstream>
#include <functional>
#include <algorithm>
#include <regex>
#include <map>
#include <random>

#include "http-parser/http_parser.h"
#include "drivers/webcam.h"
#include "png/png.h"
#include "markpos.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

const int httpd_port = 80;

using namespace std;
namespace fs = std::experimental::filesystem;

uv_loop_t uv;
uv_tcp_t httpd;
bool halt = false;
Webcam *webcam;
uv_timer_t poller, long_timer;
uv_timer_t per_sec;

struct HttpBody
{
  string header;
  vector<uint8_t> body;
  HttpBody(const char *in) {
    body.assign(in, in+strlen(in));
    header += "Content-Type: text/html\r\n";
  }
  HttpBody(const string &in){
    body.assign(in.begin(), in.end());
    header += "Content-Type: text/html\r\n";
  }
  HttpBody(const vector<uint8_t> &in){
    body.assign(in.begin(), in.end());
  }
  HttpBody(){}
};

struct Request {
  string path;
  smatch match;
};

typedef vector <pair< regex, function<HttpBody(Request)> > > path_map;
path_map get;

template<typename A, typename B> A round(B b){
  return b < 0 ? 0 : (sizeof(A) * 256 <= b) ? sizeof(A) * 256 - 1 : b;
}

struct StreamContext {
  int session_number;
  uv_tcp_t stream;
  http_parser parser;
  uv_shutdown_t shutdown;
  struct Buffer {
    std::vector<char> base;
    bool used = false;
  };
  std::vector<Buffer*> read_buf;
  ~StreamContext() {
    for (auto b : read_buf) {
      delete b;
    }
  }
  void accept(uv_stream_t *listener) {
    static int session_counter = 1;
    this->session_number = session_counter++;
    //cout << this->session_number << "accept: " << endl;
    http_parser_init(&parser, HTTP_REQUEST);
    stream.data = this;
    parser.data = this;
    if (0 == uv_tcp_init(&uv, &stream) &&
      0 == uv_tcp_simultaneous_accepts(&stream, 1) &&
      0 == uv_accept(listener, reinterpret_cast<uv_stream_t*>(&stream)) &&
      0 == uv_tcp_nodelay(&stream, 1))
    {
      uv_read_start(reinterpret_cast<uv_stream_t*>(&stream), StreamContext::alloc_cb, StreamContext::read_cb);
    }
    else
    {
      //setup error
      uv_close(reinterpret_cast<uv_handle_t*>(&stream),close_cb);
    }
  }
  static void close_cb(uv_handle_t* handle) {
    StreamContext *that = reinterpret_cast<StreamContext*>(handle->data);
    //cout << that->session_number << " close: "  << endl;
    delete that;
  }
  static void alloc_cb(uv_handle_t*handle, size_t suggested_size, uv_buf_t* buf) {
    StreamContext *that = reinterpret_cast<StreamContext*>(handle->data);
    for (auto b : that->read_buf) {
      if (!b->used && suggested_size <= b->base.size()) {
        buf->base = b->base.data();
        buf->len = suggested_size;
        b->used = true;
        return;
      }
    }
    auto b = new Buffer;
    b->base.resize(suggested_size);
    b->used = true;
    that->read_buf.push_back(b);
    buf->base = b->base.data();
    buf->len = suggested_size;
  }
  static int on_url(http_parser *parser, const char *at, size_t length) { //url
    StreamContext *that = reinterpret_cast<StreamContext*>(parser->data);
    const string path(at, at + length);
    cout << that->session_number << " on_url: " << path << endl;
    vector<uint8_t> res;
    static const char base_header[] =
      "HTTP/1.1 200 OK\r\n"
      "Connection: close\r\n";
    res.insert(res.begin(), base_header, base_header + strlen(base_header));
    if (none_of(::get.begin(), ::get.end(), [&](decltype(*::get.end()) &i) {
      std::smatch sm;
      if(regex_match(path, sm, i.first)) {
        Request r;
        r.path = path;
        r.match = sm;
        HttpBody b = i.second(r);
        b.header += "Content-Length: " + std::to_string(b.body.size());
        b.header += "\r\n\r\n";
        res.insert(res.end(), b.header.begin(), b.header.end());
        res.insert(res.end(), b.body.begin(), b.body.end());
        return true;
      }
      return false;
    })) {
    }
    uv_write_t *write_req = new uv_write_t;
    uv_buf_t buf;
    buf.len = res.size();
    buf.base = new char[buf.len];
    write_req->data = buf.base;
    memcpy(buf.base, res.data(), buf.len);
    uv_write(write_req, reinterpret_cast<uv_stream_t*>(&that->stream), &buf, 1, [](uv_write_t *write_req, int status) {
      delete[]reinterpret_cast<char*>(write_req->data);
      StreamContext *that = reinterpret_cast<StreamContext*>(write_req->handle->data);
      uv_shutdown(&that->shutdown, write_req->handle, [](uv_shutdown_t *st, int status) {
      });
      delete write_req;
    });
    return length;
  }
  static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    StreamContext *that = reinterpret_cast<StreamContext*>(stream->data);
    //cout << that->session_number << " read: " << nread << endl;
    if (nread < 0) { //error
      uv_close(reinterpret_cast<uv_handle_t*>(stream),close_cb);
    }
    else if (0 == nread) { //remote close
      uv_close(reinterpret_cast<uv_handle_t*>(stream), close_cb);
    }
    else { //good
      static const http_parser_settings settings = {
        NULL, StreamContext::on_url, NULL, NULL, NULL, NULL, NULL, NULL
      };
      http_parser_execute(&that->parser, &settings, buf->base, nread);
    }
    for (auto b : that->read_buf) {
      if (buf->base == b->base.data()) {
        b->used = false;
      }
    }
  }
};

namespace Marker {
#define B 0,0,0
#define W 255,255,255

  const uint8_t mark1[64*3] = {
    B,B,B,B,B,B,B,B,
    B,W,W,W,B,B,B,B,
    B,W,W,W,B,B,B,B,
    B,W,W,W,B,B,B,B,
    B,W,W,W,W,W,W,B,
    B,W,W,W,W,W,W,B,
    B,W,W,W,W,W,W,B,
    B,B,B,B,B,B,B,B,
  };

#undef B
#undef W

  float match(const uint8_t* rgb, const uint8_t* pattern) {
    float dot=0;
    for (int i = 0; i < 64 * 3; i++) {
      dot += (rgb[i] * pattern[i]) / (float)(255 * 255) / 3;
    }
    return dot;
  }
}

namespace Field {
  int camera_width;
  int camera_height;

  struct Node {
    float x, y;
    float dir;
  };

  vector<Node> nodes;
}

vector<uint8_t> mark_rgb;
vector<ColorMatrix> cmat;
int main()
{
  Field::nodes.resize(8);
  webcam = Webcam::Create();
  if (webcam){
    webcam->Start();
  }
  uv_loop_init(&uv);
  uv_timer_init(&uv, &poller);
  //short time loop
  uv_timer_start(&poller, [](uv_timer_t*timer){
    if (halt){
      uv_timer_stop(&poller);
      uv_timer_stop(&long_timer);
      uv_close(reinterpret_cast<uv_handle_t*>(&httpd), [](uv_handle_t*){});
    }
  }, 0, 10);

  //long timer
  uv_timer_init(&uv, &long_timer);
  uv_timer_start(&long_timer, [](uv_timer_t*timer){
    if (webcam)
    {
      webcam->Stop();
      webcam->Start();
    }
  }, 0, 1000 * 60 * 60);

  //1sec timer
  uv_timer_init(&uv, &per_sec);
  uv_timer_start(&per_sec, [](uv_timer_t*timer) {
    if (webcam)
    {
      int w = webcam->Width();
      int h = webcam->Height();
      Field::camera_width = w;
      Field::camera_height = h;
      auto buffer = webcam->Buffer();
      mark_rgb.resize(w*h * 3);
      for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
          mark_rgb[(x + y*w) * 3 + 0] = buffer[(x + y*w) * 3 + 0];
          mark_rgb[(x + y*w) * 3 + 1] = buffer[(x + y*w) * 3 + 1];
          mark_rgb[(x + y*w) * 3 + 2] = buffer[(x + y*w) * 3 + 2];
        }
      }
      
      const auto hits = GetMark(mark_rgb.data(), w, h);
      vector<QuadArea> qa;
      cmat.clear();
      for (auto &p : hits) {
        for (int i = 0; i < 4; i++) {
          QuadArea q;
          for (int j = 0; j < 4; j++) {
            q.vertex.push_back(p.vertex[(i + j) % 4]);
          }
          qa.push_back(q);
        }
      }
      for (auto &p : qa) {
        auto m = GetMatrix(buffer, w, h, p);
        float f = Marker::match(m.raw.data(), Marker::mark1);
        if (20 < f) {
          Field::nodes[0].x = (p.vertex[0].x + p.vertex[1].x + p.vertex[2].x + p.vertex[3].x) / 4;
          Field::nodes[0].y = (p.vertex[0].y + p.vertex[1].y + p.vertex[2].y + p.vertex[3].y) / 4;
          Field::nodes[0].dir = atan2f((p.vertex[3].y - p.vertex[2].y),(p.vertex[3].x - p.vertex[0].y));
          cmat.push_back(m);
          cout << f << std::endl;
        }
      }

      cout << "hits:" << hits.size() << endl;
    }
  }, 1000, 1000);

  //httpd
  sockaddr_in addr;
  uv_ip4_addr("0.0.0.0", httpd_port, &addr);
  if (0 == uv_tcp_init(&uv, &httpd) &&
    0 == uv_tcp_bind(&httpd, (const struct sockaddr*) &addr, 0) &&
    0 == uv_tcp_simultaneous_accepts((uv_tcp_t*)&httpd, 1) &&
    0 == uv_listen((uv_stream_t*)&httpd, SOMAXCONN,
      [](uv_stream_t* listener, int status) {
        auto sc = new StreamContext;
        sc->accept(listener);
      })
    ) {
    cout << "listen: 80" << endl;
  }
  else
  {
    cout << "listen error" << endl;;
    return -1;
  }

  ::get.push_back({ regex("/status"), [](const Request &req) {
    return webcam ? std::to_string(webcam->Width()) : "-1";
  } });

  //webcam
  ::get.push_back({ regex(R"(/set/markpos/threshold_plus)"), [](const Request &req)->HttpBody {
    if(markpos_settings.bin_thres<90) markpos_settings.bin_thres += 10;
    return std::to_string(markpos_settings.bin_thres);
  } });

  ::get.push_back({ regex(R"(/set/markpos/threshold_minus)"), [](const Request &req)->HttpBody {
    if (markpos_settings.bin_thres>10) markpos_settings.bin_thres -= 10;
    return std::to_string(markpos_settings.bin_thres);
  } });

  ::get.push_back({ regex("/width"), [](const Request &req){
    return webcam ? std::to_string(webcam->Width()) : "-1";
  } });

  ::get.push_back({ regex("/height"), [](const Request &req){
    return webcam ? std::to_string(webcam->Height()) : "-1";
  } });

  ::get.push_back({ regex(R"(/capture\.png?.*)"), [](const Request &req)->HttpBody{
    if (nullptr == webcam)
      return "";

    HttpBody b;
    b.header += "Content-Type: image/png\r\n";

    wts::Raw raw;
    wts::WriteFromR8G8B8(&raw, webcam->Width(), webcam->Height(), webcam->Buffer());
    b.body.assign(raw.data, raw.data + raw.size);
    wts::FreeRaw(&raw);

    return b;
  } });

  ::get.push_back({ regex(R"(/mono_step(\d)\.png.*)"), [](const Request &req)->HttpBody {
    if (nullptr == webcam)
      return "";

    HttpBody b;
    b.header += "Content-Type: image/png\r\n";
    int n = atoi(string(req.match[1].first, req.match[1].second).c_str());
    if (webcam&&n<(int)progress.size()) {
      const auto &p = progress[n];
      vector<uint8_t> data;
      data.resize(p.width*p.height * 3);
      for (int i = 0; i < p.width*p.height; i++) {
        data[i * 3 + 0] = p.mono[i];
        data[i * 3 + 1] = p.mono[i];
        data[i * 3 + 2] = p.mono[i];
      }

      wts::Raw raw;
      wts::WriteFromR8G8B8(&raw, p.width, p.height, data.data());
      b.body.assign(raw.data, raw.data + raw.size);
      wts::FreeRaw(&raw);
    }
    else {
      uint8_t dmy;
      wts::Raw raw;
      wts::WriteFromR8G8B8(&raw, 0, 0, &dmy);
      b.body.assign(raw.data, raw.data + raw.size);
      wts::FreeRaw(&raw);
    }

    return b;
  } });

  ::get.push_back({ regex(R"(/mat(\d)\.png.*)"), [](const Request &req)->HttpBody {
    if (nullptr == webcam)
      return "";

    HttpBody b;
    b.header += "Content-Type: image/png\r\n";
    int n = atoi(string(req.match[1].first, req.match[1].second).c_str());
    if (webcam&&n<(int)cmat.size()) {
      const auto &p = cmat[n];
      vector<uint8_t> data;
      int s = p.mat[0].size();
      data.resize(s*s * 3);
      for (int y = 0; y < s; y++) {
        for (int x = 0; x < s; x++) {
          data[(x + y*s) * 3 + 0] = p.mat[y][x].r;
          data[(x + y*s) * 3 + 1] = p.mat[y][x].g;
          data[(x + y*s) * 3 + 2] = p.mat[y][x].b;
        }
      }

      wts::Raw raw;
      wts::WriteFromR8G8B8(&raw, s, s, data.data());
      b.body.assign(raw.data, raw.data + raw.size);
      wts::FreeRaw(&raw);
    }
    else {
      uint8_t dmy;
      wts::Raw raw;
      wts::WriteFromR8G8B8(&raw, 0, 0, &dmy);
      b.body.assign(raw.data, raw.data + raw.size);
      wts::FreeRaw(&raw);
    }

    return b;
  } });

  ::get.push_back({ regex(R"(/color_step(\d)\.png.*)"), [](const Request &req)->HttpBody {
    if (nullptr == webcam)
      return "";

    HttpBody b;
    b.header += "Content-Type: image/png\r\n";
    int n = atoi(string(req.match[1].first, req.match[1].second).c_str());
    if (webcam&&n<(int)progress.size()) {
      const auto &p = progress[n];
      vector<uint8_t> data;
      data.resize(p.width*p.height * 3);
      vector<uint8_t> pal;
      pal.resize(256);
      mt19937 mt(123456);
      for (int i = 0; i < 256; i++) {
        int a = mt() % 256;
        pal[i] = a;
      }
      for (int i = 0; i < p.width*p.height; i++) {
        if (p.mono[i]) {
          data[i * 3 + 0] = pal[p.mono[i] % 256];
          data[i * 3 + 1] = 255 - pal[p.mono[i] % 256];
          data[i * 3 + 2] = p.mono[i] % 256;
        }
        else {
          data[i * 3 + 0] = 0;
          data[i * 3 + 1] = 0;
          data[i * 3 + 2] = 0;
        }
      }

      wts::Raw raw;
      wts::WriteFromR8G8B8(&raw, p.width, p.height, data.data());
      b.body.assign(raw.data, raw.data + raw.size);
      wts::FreeRaw(&raw);
    }
    else {
      uint8_t dmy;
      wts::Raw raw;
      wts::WriteFromR8G8B8(&raw, 0, 0, &dmy);
      b.body.assign(raw.data, raw.data + raw.size);
      wts::FreeRaw(&raw);
    }

    return b;
  } });

  //load static files
  std::map<std::string, std::vector<uint8_t>> files;
  for (auto & p : fs::directory_iterator("www"))
  {
    std::string name = p.path().string().c_str() + 4;
    std::string path = "^/";
    path += name;
    path += "$";
    if (FILE *f = fopen(p.path().string().c_str(), "rb")) {
      fseek(f, 0, SEEK_END);
      int len = (int)ftell(f);
      fseek(f, 0, SEEK_SET);
      files[name].resize(len);
      fread(files[name].data(), 1, len, f);
      fclose(f);
    }

    ::get.push_back({ regex(path), [&,name](const Request &req) {
      return files[name];
    } });
  }
  ::get.push_back({ regex("^/$"), [&](const Request &req){
    return files["index.html"];
  } });

  uv_run(&uv, UV_RUN_DEFAULT);

  if (webcam){
    webcam->Release();
    delete webcam;
  }
  return 0;
}
