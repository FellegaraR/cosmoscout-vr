////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTILS_EXPORT_HPP
#define CS_UTILS_EXPORT_HPP

#include "filesystem.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <vector>

namespace cs::utils {

/// Saves a gray scale image to .pgm file format.
template <typename T, uint32_t Size = 8>
void savePGM(
    std::vector<T> const& data, uint64_t width, uint64_t height, std::string_view fileName) {
  const uint32_t MAX_VALUE = (std::pow(2, Size) - 1);

  std::ostringstream oss;

  oss << "P2 " << width << " " << height << " " << MAX_VALUE << "\n";

  int linebreakCounter = 0;
  for (const auto& datum : data) {
    oss << static_cast<uint16_t>(datum * MAX_VALUE) << " ";

    // pgm doesn't allow more then 70 characters per line. Every value uses up to 4 characters.
    if (linebreakCounter++ == 16) {
      oss << "\n";
      linebreakCounter = 0;
    }
  }

  filesystem::saveToFile(oss.str(), fileName);
}

/// Saves a gray scale image to .pgm file format with 16 bits of precision.
template <typename T>
const auto savePGM16 = savePGM<T, 16>;

/// Saves RGM images to .ppm file format.
template <typename T, uint32_t Size = 8>
void savePPM(
    std::vector<T> const& data, uint64_t width, uint64_t height, std::string_view fileName) {
  const uint32_t MAX_VALUE = (std::pow(2, Size) - 1);

  std::ostringstream oss;
  oss << "P3 " << width << " " << height << " " << MAX_VALUE << "\n";

  size_t counter = 0;
  for (const auto& pixel : data) {
    oss << static_cast<uint16_t>(lround(std::clamp(pixel.r * 150.0, 0.0, 1.0) * MAX_VALUE)) << " "
        << static_cast<uint16_t>(lround(std::clamp(pixel.g * 150.0, 0.0, 1.0) * MAX_VALUE)) << " "
        << static_cast<uint16_t>(lround(std::clamp(pixel.b * 150.0, 0.0, 1.0) * MAX_VALUE));

    if (counter++ == 4) {
      oss << "\n";
      counter = 0;
    } else
      oss << " ";
  }

  filesystem::saveToFile(oss.str(), fileName);
}

/// Saves RGM images to .ppm file format with 16 bits of precision.
template <typename T>
const auto savePPM16 = savePPM<T, 16>;

template <int Size, typename T>
void exportVerticesAsOBJ(
    std::vector<glm::vec<Size, T>> const& vertices, std::string_view fileName) {
  std::ostringstream oss;

  oss << std::fixed;

  for (const auto& vertex : vertices) {
    oss << "v";
    for (int i = 0; i < Size; ++i) {
      oss << " " << vertex[i];
    }
    oss << "\n";
  }

  filesystem::saveToFile(oss.str(), fileName);
}

} // namespace cs::utils

#endif // CS_UTILS_EXPORT_HPP
