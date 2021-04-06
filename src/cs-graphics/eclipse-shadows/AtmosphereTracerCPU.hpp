////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_GRAPHICS_ATMOSPHERE_TRACER_CPU_HPP
#define CS_GRAPHICS_ATMOSPHERE_TRACER_CPU_HPP

#include "AtmosphereTracer.hpp"
#include "BodyProperties.hpp"
#include "LUTPrecalculator.hpp"
#include <cstdint>
#include <memory>

namespace cs::graphics {
class AtmosphereTracerCPU : public AtmosphereTracer {
 public:
  AtmosphereTracerCPU() = default;

  void init() override;

  std::variant<GPUBuffer, CPUBuffer> traceThroughAtmosphere(
      CPUBuffer& photonBuffer, const BodyWithAtmosphere& body, double xPosition) override;

  ~AtmosphereTracerCPU() override = default;

 private:
  std::unique_ptr<LUTPrecalculator> mLutPrecalculator;
};

namespace detail {

using RefractiveIndexLUT = std::vector<std::array<double, NUM_WAVELENGTHS>>;

class PhotonTracer {
 public:
  PhotonTracer(BodyWithAtmosphere const& body, glm::dvec3 const& bodyPosition,
      RefractiveIndexLUT const& refractiveIndexes, std::vector<double> const& densities);

  double calcAltitude(glm::dvec3 const& position) noexcept;

  double densityAtAltitude(double altitude) noexcept;

  double refractiveIndexAtSeaLevel(uint64_t wavelength) noexcept;

  double refractiveIndexAtAltitude(double altitude, uint64_t wavelength) noexcept;

  double partialRefractiveIndex(
      double altitude, double altitudeDelta, uint64_t wavelength) noexcept;

  void traceRay(Photon& photon) noexcept;

  double molecularNumberDensityAtAltitude(double altitude) noexcept;

  double rayleighScatteringCrossSection(uint64_t wavelength) noexcept;

  double rayleighVolumeScatteringCoefficient(double altitude, uint64_t wavelength) noexcept;

  void attenuateLight(Photon& photon, glm::dvec3 const& oldPosition) noexcept;

 private:
  BodyWithAtmosphere const&  mBody;
  glm::dvec3 const&          mBodyPosition;
  RefractiveIndexLUT const&  mRefractiveIndexes;
  std::vector<double> const& mDensities;
};

} // namespace detail

} // namespace cs::graphics
#endif // CS_GRAPHICS_ATMOSPHERE_TRACER_CPU_HPP
