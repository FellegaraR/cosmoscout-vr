////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "EclipseShadowReceiver.hpp"
#include "../cs-utils/filesystem.hpp"
#include "../cs-utils/utils.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <utility>

#include "../cs-graphics/eclipse-shadows/EclipseConstants.hpp"
#include "../cs-scene/CelestialObject.hpp"
#include "GraphicsEngine.hpp"
#include "SolarSystem.hpp"

namespace cs::core {

EclipseShadowReceiver::EclipseShadowReceiver(scene::CelestialObject const* const shadowReceiver,
    std::shared_ptr<core::GraphicsEngine>                                        graphicsEngine,
    std::shared_ptr<core::SolarSystem>                                           solarSystem)
    : mShadowReceiver(shadowReceiver)
    , mGraphicsEngine(graphicsEngine)
    , mSolarSystem(solarSystem) {
}

std::string EclipseShadowReceiver::applyExtensionToShader(
    std::string_view const fragmentShaderSource) {
  std::string outputShader = fragmentShaderSource.data();

  utils::replaceString(outputShader, "#include <EclipseShadows>",
      utils::filesystem::loadToString("../share/resources/shaders/EclipseShadowsLib.glsl"));

  return outputShader;
}

void EclipseShadowReceiver::initUniforms(VistaGLSLShader const& shader) {
  mUSunPositionLocation = shader.GetUniformLocation("uSunPosition");
  mUSunRadiusLocation   = shader.GetUniformLocation("uSunRadius");

  mUEclipseCalcTypeLocation = shader.GetUniformLocation("uEclipseCalcType");

  // VistaGLSLShader can't find array uniforms, which cost me 2 days to find out -.-
  mUOccludingBodiesLocation    = glGetUniformLocation(shader.GetProgram(), "uOccludingBodies");
  mUNumOccludingBodiesLocation = shader.GetUniformLocation("uNumOccludingBodies");

  for (int i = 0; i < 16; ++i) {
    mUShadowTextures[i] = glGetUniformLocation(
        shader.GetProgram(), ("uShadowTextures[" + std::to_string(i) + "]").c_str());
    mUShadowLength[i] = glGetUniformLocation(
        shader.GetProgram(), ("uShadowLength[" + std::to_string(i) + "]").c_str());
    mUBodyShadowNormals[i] = glGetUniformLocation(
        shader.GetProgram(), ("uBodyShadowNormals[" + std::to_string(i) + "]").c_str());
  }
}

void EclipseShadowReceiver::setupRender(
    VistaGLSLShader& shader, EclipseCalcType eclipseCalcType, int textureOffset) {
  if (mSun == nullptr)
    mSun = mSolarSystem->getBody("Sun").get();

  // get modelview and projection matrices
  GLfloat glMatMV[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, &glMatMV[0]);
  const glm::dmat4 modelView(glm::make_mat4(glMatMV));

  const glm::dmat4 sunTransform = mSun->getWorldTransform();
  const glm::dvec3 sunPosition(modelView * sunTransform[3]);
  shader.SetUniform(mUSunPositionLocation, static_cast<float>(sunPosition.x),
      static_cast<float>(sunPosition.y), static_cast<float>(sunPosition.z));

  const glm::dmat4 planetTransform = mShadowReceiver->getWorldTransform();
  const glm::dvec3 planetPosition(modelView * planetTransform[3]);

  const double sunScale = glm::length(sunTransform[0].xyz());
  // TODO: Use correct radii
  const double sunRadiusAdjusted = static_cast<float>(sunScale * mSun->getRadii()[0]);

  shader.SetUniform(mUSunRadiusLocation, static_cast<float>(sunRadiusAdjusted));

  const double scale = glm::length(planetTransform[0].xyz());
  // TODO: Use correct radii
  const double radiusAdjusted = scale * mShadowReceiver->getRadii()[0];
  const double distToSun      = glm::length(sunPosition - planetPosition) - radiusAdjusted;

  mNumBodies = 0;
  std::array<glm::vec4, MAX_BODIES> occludingBodies{};

  for (auto&& [casterName, eclipseShadow] : mGraphicsEngine->getEclipseShadowCaster()) {
    if (mNumBodies == MAX_BODIES) {
      break;
    }

    auto const& shadowBody = mSolarSystem->getBody(casterName);

    if (shadowBody == nullptr) {
      continue;
    }

    const glm::dvec3 bodyPosition(modelView * shadowBody->getWorldPosition());
    const double     bodyDistToSun = glm::distance(sunPosition, bodyPosition);

    const double bodyScale          = glm::length(shadowBody->getWorldTransform()[0].xyz());
    const double bodyRadiusAdjusted = bodyScale * (eclipseShadow->mRadius);

    occludingBodies[mNumBodies] = glm::vec4(bodyPosition, bodyRadiusAdjusted);

    const double shadowLength = graphics::TEX_SHADOW_LENGTH_FACTOR *
                                (bodyDistToSun * bodyRadiusAdjusted) /
                                (sunRadiusAdjusted - bodyRadiusAdjusted);

    if (glm::length(bodyPosition - planetPosition) < shadowLength && bodyDistToSun < distToSun) {

      if (eclipseCalcType == EclipseCalcType::CARTESIAN_TEXTURE_LOOKUP) {
        shader.SetUniform(mUShadowTextures[mNumBodies], mNumBodies + textureOffset);
        eclipseShadow->bind(GL_TEXTURE0 + mNumBodies + textureOffset);
        mEclipseShadows[mNumBodies] = eclipseShadow;

        shader.SetUniform(mUShadowLength[mNumBodies], static_cast<float>(shadowLength));

        const glm::dvec3 bodyShadowNormal(
            bodyPosition + (glm::normalize(bodyPosition - sunPosition) * shadowLength));
        shader.SetUniform(mUBodyShadowNormals[mNumBodies], static_cast<float>(bodyShadowNormal.x),
            static_cast<float>(bodyShadowNormal.y), static_cast<float>(bodyShadowNormal.z));
      }

      mNumBodies++;
    }
  }

  shader.SetUniform(mUEclipseCalcTypeLocation, static_cast<int>(eclipseCalcType));

  glUniform4fv(mUOccludingBodiesLocation, MAX_BODIES, glm::value_ptr(occludingBodies[0]));
  shader.SetUniform(mUNumOccludingBodiesLocation, mNumBodies);
}

void EclipseShadowReceiver::cleanUpRender(EclipseCalcType eclipseCalcType, int textureOffset) {
  if (eclipseCalcType == EclipseCalcType::CARTESIAN_TEXTURE_LOOKUP) {
    for (int i = 0; i < mNumBodies; ++i) {
      mEclipseShadows[i]->unbind(GL_TEXTURE0 + i + textureOffset);
    }
  }
}
} // namespace cs::core