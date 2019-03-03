#ifndef QPP_QPPCAD
#define QPP_QPPCAD
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <chrono>
#include <ctime>
#include <set>
#include <algorithm>
#include <functional>
#include <tuple>
#include <unordered_set>
#include <data/compiler_fallback.hpp>
#include <io/strfun.hpp>

namespace qpp {

  namespace cad {

    typedef std::uint8_t  uint8_t;
    typedef std::uint16_t uint16_t;
    typedef std::uint32_t uint32_t;
    typedef std::uint64_t uint64_t;

    typedef std::int8_t  int8_t;
    typedef std::int16_t int16_t;
    typedef std::int32_t int32_t;
    typedef std::int64_t int64_t;

    enum spatial_units : int {
      spatial_units_ang,
      spatial_units_bohr
    };

    enum ws_edit_t {
      edit_item = 0,
      edit_content = 1
    };

    const size_t max_recent_files = 10;

    template<typename T>
    using opt = std::optional<T>;

  }

}



#endif
