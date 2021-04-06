////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AtmosphereEclipseTextureGenerator.hpp"
#include "../../cs-utils/export.hpp"
#include "../../cs-utils/geometry/Algortithms.hpp"
#include "../../cs-utils/parallel.hpp"
#include "../../cs-utils/utils.hpp"
#include "AtmosphereTracerCPU.hpp"
#include "EclipseConstants.hpp"
#include "SimpleEclipseShadowCaster.hpp"
#include "TextureTracerCPU.hpp"
#include <GL/glew.h>
#include <glm/detail/type_quat.hpp>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

template <size_t SIZE>
std::array<std::array<double, SIZE>, SIZE> generateGaussianKernel() {
  // intialising standard deviation to 1.0
  double sigma = (SIZE / 2) / 2.0;
  double s     = 2.0 * sigma * sigma;

  // sum is for normalization
  double sum = 0.0;

  std::array<std::array<double, SIZE>, SIZE> kernel{};
  int                                        halfSize = SIZE / 2;

  // generating 5x5 kernel
  for (int x = -halfSize; x <= halfSize; x++) {
    for (int y = -halfSize; y <= halfSize; y++) {
      double r                           = glm::sqrt(x * x + y * y);
      kernel[x + halfSize][y + halfSize] = (glm::exp(-(r * r) / s)) / (glm::pi<double>() * s);
      sum += kernel[x + halfSize][y + halfSize];
    }
  }

  // normalising the Kernel
  for (std::size_t i = 0; i < SIZE; ++i)
    for (std::size_t j = 0; j < SIZE; ++j)
      kernel[i][j] /= sum;

  return kernel;
}

template <size_t RADIUS>
std::vector<glm::dvec4> gaussianBlur(
    const std::vector<glm::dvec4>& image, int64_t width, int64_t height) {
  const int64_t filterSize   = RADIUS * 2 + 1;
  const int64_t filterRadius = RADIUS;

  const auto filter = generateGaussianKernel<filterSize>();

  auto filterWeight = [&filter, filterRadius](size_t x, size_t y) -> double {
    return filter[x + filterRadius][y + filterRadius];
  };

  std::vector<glm::dvec4> output(image.size());

  cs::utils::executeParallel(height, [&](size_t y) {
    for (int64_t x = 0; x < width; ++x) {
      glm::dvec4 sum(0.0);

      for (int64_t i = -filterRadius; i <= filterRadius; ++i) {
        for (int64_t j = -filterRadius; j <= filterRadius; ++j) {
          double weight = filterWeight(i, j);

          int64_t dx = glm::abs(x + i);
          if (dx >= width) {
            dx = width - 1;
          }

          int64_t dy = glm::abs(y + j);
          if (dy >= height) {
            dy = height - 1;
          }

          glm::dvec4 value = image[dy * width + dx];
          sum.r += weight * value.r;
          sum.g += weight * value.g;
          sum.b += weight * value.b;
        }
      }

      output[y * width + x] = sum;
    }
  });

  return output;
}
} // namespace

namespace cs::graphics {
AtmosphereEclipseTextureGenerator::AtmosphereEclipseTextureGenerator()
    : mRNG(133713371337) // std::random_device()())
    , mDistributionBoolean(std::bernoulli_distribution(0.5))
    , mPhotonGenerator(std::make_unique<PhotonGenerator>())
    , mAtmosphereTracer(std::make_unique<AtmosphereTracerCPU>())
    , mTextureTracer(std::make_unique<TextureTracerCPU>())
    , mColorConverter() {

  // TODO remove for prod
  mAtmosphereTracer->init();
  mTextureTracer->init();
  mColorConverter.init();
}

utils::Texture4f AtmosphereEclipseTextureGenerator::createShadowMap(
    BodyWithAtmosphere const& body, size_t photonCount) {
  std::vector<Photon> photons = mPhotonGenerator->generatePhotons(photonCount, body);

  std::vector<glm::dvec3> positions(photons.size());
  for (std::size_t i = 0; i < photons.size(); ++i) {
    positions[i] = (photons[i].position / 6371000.0) * 20.0;
  }

  double rOcc = body.meanRadius + body.atmosphere.height;
  double xOcc = (rOcc * (SUN_RADIUS + rOcc)) / body.orbit.semiMajorAxisSun;
  std::variant<GPUBuffer, CPUBuffer> photonBuffer;

  photonBuffer = mAtmosphereTracer->traceThroughAtmosphere(photons, body, xOcc);

  auto result = mTextureTracer->traceThroughTexture(photonBuffer, body);

  std::vector<glm::dvec4> texture = mColorConverter.convert(result);

  utils::savePPM16<glm::dvec4>(texture, TEX_WIDTH, TEX_HEIGHT,
      "eclipse_shadow_" + std::to_string(body.gravity) + ".raw.ppm");

  std::vector<glm::dvec4> outputTexture =
      gaussianBlur<static_cast<size_t>(TEX_WIDTH * 0.01)>(texture, TEX_WIDTH, TEX_HEIGHT);

  auto shadowTexture = generateShadowTexture({body.meanRadius, body.orbit});
  auto data          = shadowTexture.dataPtr();

  utils::Texture4f        resultTexture(TEX_WIDTH, TEX_HEIGHT);
  std::vector<glm::dvec4> resultTextureVec(TEX_WIDTH * TEX_HEIGHT);

  for (size_t i = 0; i < outputTexture.size(); ++i) {
    resultTextureVec[i] =
        glm::dvec4(glm::dvec3(outputTexture[i].rgb()) + glm::dvec3(data[i].rgb()), 1.0);
    resultTexture.dataPtr()[i] =
        glm::vec4(outputTexture[i].rgb() + glm::dvec3(data[i].rgb()), 1.0f);
  }

  utils::savePPM16<glm::dvec4>(resultTextureVec, TEX_WIDTH, TEX_HEIGHT,
      "eclipse_shadow_" + std::to_string(body.gravity) + ".ppm");

  std::cout << 6 << std::endl;

  return resultTexture;
}

} // namespace cs::graphics