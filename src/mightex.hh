#ifdef SWIG
// Generate libraries for scripting languages with SWIG
// e.g., for Lua, generate with swig -lua -c++ -o mightex_lua.cpp mightex.hpp
// then build with:
// clang++ mightex_lua.cpp ../lib/libusb-1.0.a ../lib/libmightex_static.a -I /usr/local/opt/lua@5.3/include/lua -L /usr/local/opt/lua@5.3/lib/ -llua -shared -framework IOKit -framework CoreFoundation -omightex.so 
%module mightex
%{
#include "mightex.hpp"
%}
%include "mightex1304.h"
%include "std_string.i"
%include "std_vector.i"
namespace std {
  %template(vectori) vector<int>;
};
#endif

#include <string>
#include <vector>

#include "mightex1304.h"

std::string version() { return mightex_sw_version(); }

class Mightex1304 {
private:
  mightex_t *m;
  std::string _serial;
  std::string _version;
  uint16_t *_frame_p, *_raw_frame_p;

public:
  Mightex1304() {
    m = mightex_new();
    _frame_p = mightex_frame_p(m);
    _raw_frame_p = mightex_raw_frame_p(m);
    _serial = mightex_serial_no(m);
    _version = mightex_version(m);
  }
  ~Mightex1304() { mightex_close(m); }

  std::string serial_no() { return _serial; }
  std::string version() { return _version; }
  uint16_t pixel_count() { return mightex_pixel_count(m); }
  int dark_pixel_count() { return (int)mightex_dark_pixel_count(m); }

  mtx_result_t set_exptime(float t) {
    return mightex_set_exptime(m, t);
  }
  mtx_result_t set_mode(mtx_mode_t mode) {
    return mightex_set_mode(m, mode);
  }

  mtx_result_t read_frame() { return mightex_read_frame(m); }

  int dark_mean() { return (int)mightex_dark_mean(m); }

  int frame_timestamp() { return (int)mightex_frame_timestamp(m); }

  std::vector<int> frame() {
    std::vector<int> data(_frame_p, _frame_p + mightex_pixel_count(m));
    return data;
  }

  std::vector<int> raw_frame() {
    std::vector<int> data(_raw_frame_p, _raw_frame_p + mightex_pixel_count(m));
    return data;
  }

  unsigned int timestamp() { return (unsigned int)mightex_frame_timestamp(m); }

  void apply_filter() { mightex_apply_filter(m, NULL); }
  double apply_estimator() { return mightex_apply_estimator(m, NULL); }

  // these are not really usable in scripting languages
#ifndef SWIG
  void apply_filter(void *ud) { mightex_apply_filter(m, ud); }
  void set_filter(mightex_filter_t *f) { mightex_set_filter(m, f); }
  void reset_filter() { mightex_reset_filter(m); }

  double apply_estimator(void *ud) { return mightex_apply_estimator(m, ud); }
  void set_estimator(mightex_estimator_t *e) { mightex_set_estimator(m, e); }
  void reset_estimator() { mightex_reset_estimator(m); }
#endif
};
