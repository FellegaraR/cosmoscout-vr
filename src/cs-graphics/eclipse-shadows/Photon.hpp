////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_PHOTON_HPP
#define CS_GRAPHICS_PHOTON_HPP

#include "../../cs-utils/utils.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <ostream>

namespace cs::graphics {

struct Photon {
  Photon() = default;

  Photon(const glm::dvec3& position, const glm::dvec3& direction, double intensity,
      uint32_t wavelength)
      : position(position)
      , intensity(intensity)
      , direction(direction)
      , wavelength(wavelength) {
  }

  // ###############################################################################################
  // DO NOT CHANGE THIS LAYOUT! IT IS REQUIRED TO BE LIKE THIS FOR GPU PADDING REASONS!

  glm::dvec3 position; ///< m
  double     intensity;
  glm::dvec3 direction;  ///< normalized
  uint64_t   wavelength; ///< nm

  // DO NOT CHANGE THIS LAYOUT! IT IS REQUIRED TO BE LIKE THIS FOR GPU PADDING REASONS!
  // ###############################################################################################

  friend std::ostream& operator<<(std::ostream& os, const Photon& photon) {
    os << "position: " << photon.position << " direction: " << photon.direction
       << " intensity: " << photon.intensity << " wavelength: " << photon.wavelength;
    return os;
  }
};

} // namespace cs::graphics

#endif // CS_GRAPHICS_PHOTON_HPP
