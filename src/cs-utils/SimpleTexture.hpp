////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_UTILS_TEXTURE_2D_HPP
#define CS_UTILS_TEXTURE_2D_HPP

#include <cs_utils_export.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace cs::utils {

template <typename T>
class SimpleTexture {
 public:
  size_t const mWidth;
  size_t const mHeight;

  SimpleTexture(size_t width, size_t height)
      : mWidth(width)
      , mHeight(height)
      , mData(width * height) {
  }

  [[nodiscard]] T get(size_t x, size_t y) const {
    return mData[y * mWidth + x];
  }

  void set(size_t x, size_t y, T value) {
    mData[y * mWidth + x] = value;
  }

  [[nodiscard]] T* dataPtr() {
    return mData.data();
  }

 private:
  std::vector<T> mData;
};

typedef SimpleTexture<int32_t>  Texture1i;
typedef SimpleTexture<uint32_t> Texture1ui;
typedef SimpleTexture<int64_t>  Texture1l;
typedef SimpleTexture<uint64_t> Texture1ul;

typedef SimpleTexture<float>  Texture1f;
typedef SimpleTexture<double> Texture1d;

typedef SimpleTexture<glm::vec2>  Texture2f;
typedef SimpleTexture<glm::dvec2> Texture2d;

typedef SimpleTexture<glm::vec3>  Texture3f;
typedef SimpleTexture<glm::dvec3> Texture3d;

typedef SimpleTexture<glm::vec4>  Texture4f;
typedef SimpleTexture<glm::dvec4> Texture4d;

} // namespace cs::utils

#endif // CS_UTILS_TEXTURE_2D_HPP
