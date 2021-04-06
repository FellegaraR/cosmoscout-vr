////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TextureTracerCPU.hpp"

#include "../../cs-utils/geometry/Algortithms.hpp"
#include "../../cs-utils/geometry/Rectangle.hpp"
#include "../../cs-utils/parallel.hpp"
#include "EclipseConstants.hpp"
#include "Photon.hpp"

#include <GL/glew.h>
#include <array>
#include <atomic>
#include <cmath>
#include <glm/glm.hpp>

namespace cs::graphics {

void operator+=(std::atomic<double>& atomic, double value) {
  double old;
  do {
    old = atomic;
  } while (!atomic.compare_exchange_strong(old, old + value));
}

std::vector<DoublePixel> TextureTracerCPU::traceThroughTexture(
    std::variant<GPUBuffer, CPUBuffer> const& photonBuffer, BodyWithAtmosphere const& body) {

  double const atmosphericRadius = body.meanRadius + body.atmosphere.height;
  double const rectangleHeight =
      atmosphericRadius * TEX_HEIGHT_TO_RADIUS_FACTOR / static_cast<double>(TEX_HEIGHT);

  double const shadowHeight = atmosphericRadius * TEX_HEIGHT_TO_RADIUS_FACTOR;

  double const shadowLength = TEX_SHADOW_LENGTH_FACTOR *
                              (body.orbit.semiMajorAxisSun * atmosphericRadius) /
                              (SUN_RADIUS - atmosphericRadius);

  auto horizontalRectangles = std::array<glm::dvec2, TEX_WIDTH>{};

  double xx0 = 0.0;
  for (uint32_t x = 0; x < TEX_WIDTH; ++x) {
    double const xx1 =
        std::pow(static_cast<double>(x + 1) / TEX_WIDTH, TEX_SHADOW_WIDTH_EXPONENT) * shadowLength;
    horizontalRectangles[x] = glm::dvec2(xx0, xx1 - xx0);
    xx0                     = xx1;
  }

  struct AtomicPixel {
    std::array<std::atomic<double>, NUM_WAVELENGTHS> intensityAtWavelengths{};

    void addAtWavelength(uint32_t wavelength, double value) {
      uint32_t index = wavelength - MIN_WAVELENGTH;
      intensityAtWavelengths[index] += value;
    }
  };

  std::vector<AtomicPixel> pixels(TEX_WIDTH * TEX_HEIGHT);
  cs::utils::executeParallel(pixels.size(), [&](size_t i) {
    for (auto&& a : pixels[i].intensityAtWavelengths)
      a = 0.0;
  });

  detail::GridTracer tracer(rectangleHeight, shadowHeight, shadowLength, horizontalRectangles);

  CPUBuffer photons;
  if (std::holds_alternative<GPUBuffer>(photonBuffer)) {
    GPUBuffer gpuBuffer = std::get<GPUBuffer>(photonBuffer);
    photons             = CPUBuffer(gpuBuffer.size);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpuBuffer.buffer);
    glGetBufferSubData(
        GL_SHADER_STORAGE_BUFFER, 0, sizeof(Photon) * gpuBuffer.size, photons.data());

    glDeleteBuffers(1, &gpuBuffer.buffer);
  } else {
    photons = std::get<CPUBuffer>(photonBuffer);
  }

  double const pixelInAtmosphere = body.atmosphere.height / rectangleHeight;
  double const photonsPerPixel   = NUM_PHOTONS / pixelInAtmosphere;

  cs::utils::executeParallel(photons.size(), [&](size_t gid) {
    Photon localPhoton = photons[gid];

    // TODO temporary hack to convert 3D to 2D
    localPhoton.position.x = glm::length(localPhoton.position.xz());

    if (localPhoton.intensity > 0.0) {
      glm::ivec2 photonTexIndices = tracer.getRectangleIdxAt(localPhoton.position);

      while (photonTexIndices.x < static_cast<int32_t>(TEX_WIDTH) &&
             photonTexIndices.y < static_cast<int32_t>(TEX_HEIGHT) && photonTexIndices.x >= 0 &&
             photonTexIndices.y >= 0) {
        utils::geom::DRectangle rect = tracer.getRectangleAt(photonTexIndices);

        double const distance = utils::geom::rayDistanceInRectangle(
            {localPhoton.position, localPhoton.direction}, rect);

        double const contrib = (distance / rect.width);

        pixels[photonTexIndices.y * TEX_WIDTH + photonTexIndices.x].addAtWavelength(
            localPhoton.wavelength, localPhoton.intensity * contrib);

        glm::ivec2 dir = tracer.getRayRectangleExitEdge(localPhoton, rect);
        photonTexIndices += dir;

        // When the ray goes out of bounds on the bottom then mirror it to simulate rays
        // coming from the other side of the planet. This works because of the rotational
        // symmetry of the system.
        if (photonTexIndices.y < 0) {
          photonTexIndices.y = 0;
          localPhoton.position.y *= -1.0;
          localPhoton.direction.y *= -1.0;
        }
      }
    }
  });

  std::vector<DoublePixel> resultBuffer(TEX_WIDTH * TEX_HEIGHT);

  cs::utils::executeParallel(resultBuffer.size(), [&](size_t i) {
    DoublePixel  fp{};
    AtomicPixel& ap = pixels[i];

    for (size_t p = 0; p < fp.intensitiesAtWavelengths.size(); ++p) {
      fp.intensitiesAtWavelengths[p] = ap.intensityAtWavelengths[p] / photonsPerPixel;
    }

    resultBuffer[i] = fp;
  });

  return resultBuffer;
}

namespace detail {

////////////////////////////////////////////////////////////////////////////////////////////////////

utils::geom::DRectangle GridTracer::getRectangleAt(glm::ivec2 const& indices) {
  glm::dvec2 horRect = horizontalRectangles[indices.x];
  return utils::geom::DRectangle(
      horRect.x, rectangleHeight * indices.y, horRect.y, rectangleHeight);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int GridTracer::binarySearchForHorizontalRectangle(double x) {
  int left  = 0;
  int right = static_cast<int>(TEX_WIDTH) - 1;

  while (left <= right) {
    int        middle = (left + right) / 2;
    glm::dvec2 rectM  = horizontalRectangles[middle];
    if (rectM.x + rectM.y < x) {
      left = middle + 1;
    } else if (rectM.x > x) {
      right = middle - 1;
    } else
      return middle;
  }

  // outside of grid (should never happen in any reasonable scenario...)
  return static_cast<int>(TEX_WIDTH);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::ivec2 GridTracer::getRectangleIdxAt(glm::dvec2 const& location) {
  if (location.x >= 0 && location.x < shadowLength && location.y >= 0 &&
      location.y < shadowHeight) {
    int x = binarySearchForHorizontalRectangle(location.x);
    int y = int(location.y / rectangleHeight);
    return glm::ivec2(x, y);
  } else
    return glm::ivec2(TEX_WIDTH, TEX_HEIGHT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double GridTracer::getRayIntersectAtX(Photon const& ray, double x) {
  double slope = ray.direction.y / ray.direction.x;
  return slope * (x - ray.position.x) + ray.position.y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::ivec2 GridTracer::getRayRectangleExitEdge(
    Photon const& ray, utils::geom::DRectangle const& rect) {
  double intersectHeight = getRayIntersectAtX(ray, rect.x + rect.width);
  if (intersectHeight < rect.y) {
    return glm::ivec2(0, -1);
  } else if (intersectHeight > rect.y + rect.height) {
    return glm::ivec2(0, 1);
  } else {
    return glm::ivec2(1, 0);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace detail

} // namespace cs::graphics
