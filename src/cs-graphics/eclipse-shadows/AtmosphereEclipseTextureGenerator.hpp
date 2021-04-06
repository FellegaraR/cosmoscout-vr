////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_GENERATOR_HPP
#define CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_GENERATOR_HPP

#include "../../cs-utils/SimpleTexture.hpp"
#include "AtmosphereTracer.hpp"
#include "BodyProperties.hpp"
#include "ColorConverter.hpp"
#include "Photon.hpp"
#include "PhotonGenerator.hpp"
#include "TextureTracer.hpp"
#include <memory>
#include <random>

namespace cs::graphics {
class AtmosphereEclipseTextureGenerator {
 public:
  AtmosphereEclipseTextureGenerator();

  cs::utils::Texture4f createShadowMap(BodyWithAtmosphere const& body, size_t photonCount);

 private:
  std::mt19937_64             mRNG;
  std::bernoulli_distribution mDistributionBoolean;

  std::unique_ptr<PhotonGenerator>  mPhotonGenerator;
  std::unique_ptr<AtmosphereTracer> mAtmosphereTracer;
  std::unique_ptr<TextureTracer>    mTextureTracer;
  ColorConverter                    mColorConverter;
};
} // namespace cs::graphics

#endif // CS_GRAPHICS_ATMOSPHERE_ECLIPSE_SHADOW_GENERATOR_HPP
