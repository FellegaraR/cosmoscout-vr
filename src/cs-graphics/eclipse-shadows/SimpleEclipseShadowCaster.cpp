////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SimpleEclipseShadowCaster.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <string>

#include "../../cs-utils/SimpleTexture.hpp"
#include "../../cs-utils/export.hpp"
#include "../../cs-utils/geometry/Algortithms.hpp"
#include "../../cs-utils/parallel.hpp"
#include "EclipseConstants.hpp"

namespace cs::graphics {
cs::utils::Texture4f generateShadowTexture(Body const& body) {
  const double shadowLength = TEX_SHADOW_LENGTH_FACTOR *
                              (body.orbit.semiMajorAxisSun * body.meanRadius) /
                              (SUN_RADIUS - body.meanRadius);

  const double pixSize = body.meanRadius / (TEX_HEIGHT / TEX_HEIGHT_TO_RADIUS_FACTOR);

  cs::utils::Texture4f texture(TEX_WIDTH, TEX_HEIGHT);

  cs::utils::executeParallel(TEX_HEIGHT, [&](size_t y) {
    for (size_t x = 0; x < TEX_WIDTH; ++x) {
      const double xx =
          std::pow(static_cast<double>(x) / TEX_WIDTH, TEX_SHADOW_WIDTH_EXPONENT) * shadowLength;
      const glm::dvec2 pixelPositionRelPlanet(xx, y * pixSize);
      const glm::dvec2 pixelPositionRelPlanetNorm = glm::normalize(pixelPositionRelPlanet);
      const double     pixelDistanceToPlanet      = glm::length(pixelPositionRelPlanet);
      const double     planetAngularRadius =
          utils::geom::angularRadOfSphere(pixelDistanceToPlanet, body.meanRadius);

      const glm::dvec2 pixelPositionRelSun(body.orbit.semiMajorAxisSun + xx, y * pixSize);
      const glm::dvec2 pixelPositionRelSunNorm = glm::normalize(pixelPositionRelSun);
      const double     pixelDistanceToSun      = glm::length(pixelPositionRelSun);
      const double     sunAngularRadius =
          utils::geom::angularRadOfSphere(pixelDistanceToSun, SUN_RADIUS);
      const double sunSolidAngle = utils::geom::areaOfCircle(sunAngularRadius);

      const double angularDistanceToSun =
          glm::angle(pixelPositionRelSunNorm, pixelPositionRelPlanetNorm);

      const double sunHiddenPart = utils::geom::areaOfCircleIntersection(
          sunAngularRadius, planetAngularRadius, angularDistanceToSun);

      const double visibleArea         = sunSolidAngle - sunHiddenPart;
      double       visibleAreaRelative = std::clamp(visibleArea / sunSolidAngle, 0.0, 1.0);

      if (std::isnan(visibleAreaRelative))
        visibleAreaRelative = 0.0;

      texture.set(x, y, glm::vec4(static_cast<float>(visibleAreaRelative)));
    }
  });

  std::vector<float> data(TEX_WIDTH * TEX_HEIGHT);
  for (std::size_t i = 0; i < TEX_WIDTH * TEX_HEIGHT; ++i) {
    data[i] = texture.dataPtr()[i].r;
  }
  utils::savePGM16<float>(
      data, TEX_WIDTH, TEX_HEIGHT, "eclipse_shadow_" + std::to_string(body.meanRadius));

  return texture;
}

SimpleEclipseShadowCaster::SimpleEclipseShadowCaster(Body const& body) {
  mRadius        = body.meanRadius;
  auto shadowTex = generateShadowTexture(body);

  glGenTextures(1, &mShadowTexture);
  glBindTexture(GL_TEXTURE_2D, mShadowTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, shadowTex.mWidth, shadowTex.mHeight, 0, GL_RGBA,
      GL_FLOAT, shadowTex.dataPtr());
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

SimpleEclipseShadowCaster::~SimpleEclipseShadowCaster() {
  glDeleteTextures(1, &mShadowTexture);
}

void SimpleEclipseShadowCaster::bind(GLenum textureUnit) {
  int nActiveTexUnit = 0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &nActiveTexUnit);

  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, mShadowTexture);
  glActiveTexture(nActiveTexUnit);
}

void SimpleEclipseShadowCaster::unbind(GLenum textureUnit) {
  int nActiveTexUnit = 0;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &nActiveTexUnit);

  glActiveTexture(textureUnit);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(nActiveTexUnit);
}

} // namespace cs::graphics