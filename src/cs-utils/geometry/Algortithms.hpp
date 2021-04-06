////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTIL_GEOMETRY_ALGORITHMS_HPP
#define CS_UTIL_GEOMETRY_ALGORITHMS_HPP

#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <optional>

#include "Line.hpp"
#include "LineSegment.hpp"
#include "Quadrilateral.hpp"
#include "Ray.hpp"
#include "Rectangle.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"

namespace cs::utils::geom {

/// Returns the intersection point of two line segments.
/// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line
template <typename T>
std::optional<glm::tvec2<T>> intersection(TLineSegment2<T> const& l1, TLineSegment2<T> const& l2) {
  T x1 = l1.start.x;
  T x2 = l1.end.x;
  T x3 = l2.start.x;
  T x4 = l2.end.x;

  T y1 = l1.start.y;
  T y2 = l1.end.y;
  T y3 = l2.start.y;
  T y4 = l2.end.y;

  T denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  // parallel or identical
  if (denominator == 0.0)
    return std::nullopt;

  T t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denominator;

  // intersection not on l1
  if (t < 0.0 || t > 1.0)
    return std::nullopt;

  T u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denominator;

  // intersection not on l2
  if (u < 0.0 || u > 1.0)
    return std::nullopt;

  T x = x1 + t * (x2 - x1);
  T y = y1 + t * (y2 - y1);
  return glm::tvec2<T>{x, y};
}

/// Returns the intersection point of two line segments.
/// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line
template <typename T>
bool doIntersect(TLineSegment2<T> const& l1, TLineSegment2<T> const& l2) {
  T x1 = l1.start.x;
  T x2 = l1.end.x;
  T x3 = l2.start.x;
  T x4 = l2.end.x;

  T y1 = l1.start.y;
  T y2 = l1.end.y;
  T y3 = l2.start.y;
  T y4 = l2.end.y;

  T denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  // parallel or identical
  if (denominator == 0.0)
    return false;

  T t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denominator;

  // intersection not on l1
  if (t < 0.0 || t > 1.0)
    return false;

  T u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denominator;

  // intersection not on l2
  if (u < 0.0 || u > 1.0)
    return false;

  return true;
}

/// Returns the intersection point of two lines.
/// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line
template <typename T>
std::optional<glm::tvec2<T>> intersection(TLine2<T> const& l1, TLine2<T> const& l2) {
  T x1 = l1.start.x;
  T x2 = l1.end.x;
  T x3 = l2.start.x;
  T x4 = l2.end.x;

  T y1 = l1.start.y;
  T y2 = l1.end.y;
  T y3 = l2.start.y;
  T y4 = l2.end.y;

  T denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  // parallel or identical
  if (denominator == 0.0)
    return std::nullopt;

  T a = x1 * y2 - y1 * x2;
  T b = x3 * y4 - y3 * x4;

  T x = (a * (x3 - x4) - (x1 - x2) * b) / denominator;
  T y = (a * (y3 - y4) - (y1 - y2) * b) / denominator;
  return glm::tvec2<T>{x, y};
}

/// True if lines intersect exactly in one point.
/// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line
template <typename T>
bool doIntersect(TLine2<T> const& l1, TLine2<T> const& l2) {
  T x1 = l1.start.x;
  T x2 = l1.end.x;
  T x3 = l2.start.x;
  T x4 = l2.end.x;

  T y1 = l1.start.y;
  T y2 = l1.end.y;
  T y3 = l2.start.y;
  T y4 = l2.end.y;

  T denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

  return denominator != 0.0;
}

template <typename T>
bool rayHitSphere(Ray<3, T> const& ray, Sphere<T> const& sphere) {
  glm::tvec3<T> oc           = ray.origin - sphere.center;
  T             a            = glm::dot(ray.direction, ray.direction);
  T             b            = static_cast<T>(2.0L) * glm::dot(oc, ray.direction);
  T             c            = glm::dot(oc, oc) - sphere.radius * sphere.radius;
  T             discriminant = b * b - static_cast<T>(4.0L) * a * c;
  return (discriminant > static_cast<T>(0.0L));
}

template <typename T>
T raySphereDistance(Ray<3, T> const& ray, Sphere<T> const& sphere) {
  glm::tvec3<T> oc           = ray.origin - sphere.center;
  T             a            = glm::dot(ray.direction, ray.direction);
  T             b            = static_cast<T>(2.0L) * glm::dot(oc, ray.direction);
  T             c            = glm::dot(oc, oc) - sphere.radius * sphere.radius;
  T             discriminant = b * b - static_cast<T>(4.0L) * a * c;
  if (discriminant < static_cast<T>(0.0L)) {
    return static_cast<T>(-1.0L);
  } else {
    return (-b - std::sqrt(discriminant)) / (static_cast<T>(2.0L) * a);
  }
}

template <typename T>
T angularRadOfSphere(T distance, T radius) {
  return std::asin(radius / distance);
}

template <typename T>
T areaOfCircle(T radius) {
  return glm::pi<T>() * radius * radius;
}

template <typename T>
T areaOfCircleIntersection(T radiusSun, T radiusPlanet, T centerDistance) {
  // No intersection
  if (centerDistance >= radiusSun + radiusPlanet)
    return 0.0;

  // One circle fully in the other (total eclipse)
  if (std::min(radiusSun, radiusPlanet) <= std::max(radiusSun, radiusPlanet) - centerDistance)
    return areaOfCircle(radiusPlanet);

  const T d = centerDistance;

  T rrS = radiusSun * radiusSun;
  T rrP = radiusPlanet * radiusPlanet;
  T dd  = d * d;

  T d1 = std::fma(radiusSun, radiusSun, std::fma(-radiusPlanet, radiusPlanet, dd)) / (2.0 * d);
  T d2 = d - d1;

  // This should be faster o.0
  T fourth = -d2 * std::sqrt(std::fma(-d2, d2, rrP));
  T third  = std::fma(rrP, std::acos(d2 / radiusPlanet), fourth);
  T second = std::fma(-d1, std::sqrt(std::fma(-d1, d1, rrS)), third);
  return std::fma(rrS, std::acos(d1 / radiusSun), second);
}

template <int Size, typename T>
glm::vec<Size, T> center(Triangle<Size, T> const& triangle) {
  return static_cast<T>(1.0 / 3.0) * (triangle.a + triangle.b + triangle.c);
}

template <typename T>
glm::tvec2<T> center(Quadrilateral<T> const& quad) {
  glm::tvec2<T> c1 = center(quad.abd());
  glm::tvec2<T> c2 = center(quad.bcd());
  glm::tvec2<T> c3 = center(quad.abc());
  glm::tvec2<T> c4 = center(quad.cda());

  TLine2<T> l1{c1, c2};
  TLine2<T> l2{c3, c4};

  return *intersection(l1, l2);
}

template <typename T>
glm::tvec2<T> center(Rectangle<T> const& rect) {
  return rect.position + static_cast<T>(0.5) * rect.dimensions;
}

template <typename T>
std::optional<glm::tvec2<T>> intersection(
    Ray<2, T> const& ray, utils::geom::TLineSegment2<T> const& line) {
  glm::tvec2<T> v1 = glm::tvec2<T>(ray.origin) - line.start;
  glm::tvec2<T> v2 = line.end - line.start;
  glm::tvec2<T> v3 = glm::tvec2<T>(-ray.direction.y, ray.direction.x);

  double dot = glm::dot(v2, v3);
  if (glm::abs(dot) < 0.000001)
    return std::nullopt;

  double t1 = (v1.x * v2.y - v1.y * v2.x) / dot;
  double t2 = glm::dot(v1, v3) / dot;

  if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0))
    return std::make_optional<glm::tvec2<T>>(
        glm::tvec2<T>(ray.origin) + (glm::tvec2<T>(ray.direction) * t1));
  else
    return std::nullopt;
}

template <typename T>
std::pair<glm::tvec2<T>, glm::tvec2<T>> intersection(
    Ray<2, T> const& ray, utils::geom::Rectangle<T> const& rect) {
  glm::tvec2<T> topLeft(rect.position), topRight(rect.x + rect.width, rect.y);
  glm::tvec2<T> botLeft(rect.x, rect.y + rect.height),
      botRight(rect.x + rect.width, rect.y + rect.height);

  std::array<utils::geom::TLineSegment2<T>, 4> rectEdges{
      utils::geom::TLineSegment2<T>{botLeft, topLeft},
      utils::geom::TLineSegment2<T>{topLeft, topRight},
      utils::geom::TLineSegment2<T>{botRight, botLeft},
      utils::geom::TLineSegment2<T>{topRight, botRight}};

  glm::tvec2<T> entry;
  glm::tvec2<T> exit;

  bool entryFound = false;
  for (const auto& edge : rectEdges) {
    if (auto point = intersection(ray, edge)) {
      if (!entryFound) {
        entry      = point.value();
        entryFound = true;
      } else {
        exit = point.value();
        break;
      }
    }
  }

  return std::pair<glm::tvec2<T>, glm::tvec2<T>>(entry, exit);
}

template <typename T>
double rayDistanceInRectangle(Ray<2, T> const& ray, utils::geom::Rectangle<T> const& rect) {
  auto [entry, exit] = intersection(ray, rect);
  return glm::distance(exit, entry);
}

} // namespace cs::utils::geom

#endif // CS_UTIL_GEOMETRY_ALGORITHMS_HPP
