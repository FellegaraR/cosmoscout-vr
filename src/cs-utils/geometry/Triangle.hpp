////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTILS_GEOMETRY_TRIANGLE_HPP
#define CS_UTILS_GEOMETRY_TRIANGLE_HPP

#include <array>

namespace cs::utils::geom {

template <int Size, typename T>
struct Triangle {
  Triangle(glm::vec<Size, T> const& a, glm::vec<Size, T> const& b, glm::vec<Size, T> const& c)
      : a(a)
      , b(b)
      , c(c) {
  }

  Triangle(std::array<glm::vec<Size, T>, 3> const& corners)
      : corners(corners) {
  }

  union {
    std::array<glm::vec<Size, T>, 3> corners;
    struct {
      glm::vec<Size, T> a;
      glm::vec<Size, T> b;
      glm::vec<Size, T> c;
    };
  };
};

template <typename T>
using TTriangle2 = Triangle<2, T>;

template <typename T>
using TTriangle3 = Triangle<3, T>;

using FTriangle2 = TTriangle2<float>;
using FTriangle3 = TTriangle3<float>;

using DTriangle2 = TTriangle2<double>;
using DTriangle3 = TTriangle3<double>;

} // namespace cs::utils::geom

#endif // CS_UTILS_GEOMETRY_TRIANGLE_HPP
