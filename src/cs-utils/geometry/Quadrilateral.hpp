////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTILS_GEOMETRY_QUADRILATERAL_HPP
#define CS_UTILS_GEOMETRY_QUADRILATERAL_HPP

#include "Triangle.hpp"
#include <array>
#include <glm/glm.hpp>

namespace cs::utils::geom {

template <typename T>
struct Quadrilateral {
  Quadrilateral(glm::tvec2<T> const& a, glm::tvec2<T> const& b, glm::tvec2<T> const& c,
      glm::tvec2<T> const& d)
      : a(a)
      , b(b)
      , c(c)
      , d(d) {
  }

  Quadrilateral(std::array<glm::tvec2<T>, 4> const& corners)
      : corners(corners) {
  }

  union {
    std::array<glm::tvec2<T>, 4> corners;

    struct {
      glm::tvec2<T> a;
      glm::tvec2<T> b;
      glm::tvec2<T> c;
      glm::tvec2<T> d;
    };
  };

  TTriangle2<T> abc() const {
    return {a, b, c};
  }

  TTriangle2<T> abd() const {
    return {a, b, d};
  }

  TTriangle2<T> acb() const {
    return {a, c, b};
  }

  TTriangle2<T> acd() const {
    return {a, c, d};
  }

  TTriangle2<T> adb() const {
    return {a, d, b};
  }

  TTriangle2<T> adc() const {
    return {a, d, c};
  }

  TTriangle2<T> bac() const {
    return {b, a, c};
  }

  TTriangle2<T> bad() const {
    return {b, a, d};
  }

  TTriangle2<T> bca() const {
    return {b, c, a};
  }

  TTriangle2<T> bcd() const {
    return {b, c, d};
  }

  TTriangle2<T> bda() const {
    return {b, d, a};
  }

  TTriangle2<T> bdc() const {
    return {b, d, c};
  }

  TTriangle2<T> cab() const {
    return {c, a, b};
  }

  TTriangle2<T> cad() const {
    return {c, a, d};
  }

  TTriangle2<T> cba() const {
    return {c, b, a};
  }

  TTriangle2<T> cbd() const {
    return {c, b, d};
  }

  TTriangle2<T> cda() const {
    return {c, d, a};
  }

  TTriangle2<T> cdb() const {
    return {c, d, c};
  }

  TTriangle2<T> dab() const {
    return {d, a, b};
  }

  TTriangle2<T> dac() const {
    return {d, a, c};
  }

  TTriangle2<T> dba() const {
    return {d, b, a};
  }

  TTriangle2<T> dbc() const {
    return {d, b, c};
  }

  TTriangle2<T> dca() const {
    return {d, c, a};
  }

  TTriangle2<T> dcb() const {
    return {d, c, b};
  }
};

using FQuadrilateral = Quadrilateral<float>;
using DQuadrilateral = Quadrilateral<double>;
} // namespace cs::utils::geom

#endif // CS_UTILS_GEOMETRY_QUADRILATERAL_HPP
