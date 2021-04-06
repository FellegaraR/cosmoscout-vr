////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "LUTPrecalculator.hpp"
#include "../../cs-utils/filesystem.hpp"
#include "EclipseConstants.hpp"
#include <GL/glew.h>
#include <array>
#include <iostream>
#include <vector>

namespace cs::graphics {

LUTPrecalculator::LUTPrecalculator() {
  mProgram                        = glCreateProgram();
  uint32_t    precalculatorShader = glCreateShader(GL_COMPUTE_SHADER);
  std::string shaderCode          = cs::utils::filesystem::loadToString(
      "../share/resources/shaders/EclipseLUTPrecalculator.glsl");
  auto shaderCodeData = shaderCode.data();
  glShaderSource(precalculatorShader, 1, &shaderCodeData, nullptr);
  glCompileShader(precalculatorShader);

  int rvalue;
  glGetShaderiv(precalculatorShader, GL_COMPILE_STATUS, &rvalue);
  if (!rvalue) {
    std::cerr << "Error in compiling the compute shader\n";
    GLchar  log[10240];
    GLsizei length;
    glGetShaderInfoLog(precalculatorShader, 10239, &length, log);
    std::cerr << "Compiler log:\n" << log << std::endl;
    exit(40);
  }

  glAttachShader(mProgram, precalculatorShader);
  glLinkProgram(mProgram);

  glGetProgramiv(mProgram, GL_LINK_STATUS, &rvalue);
  if (!rvalue) {
    std::cerr << "Error in linking compute shader program\n";
    GLchar  log[10240];
    GLsizei length;
    glGetProgramInfoLog(mProgram, 10239, &length, log);
    std::cerr << "Linker log:\n" << log << std::endl;
    exit(41);
  }

  mUniforms.planet.uAtmosphericHeight = glGetUniformLocation(mProgram, "planet.atmosphericHeight");
  mUniforms.planet.uGravity   = glGetUniformLocation(mProgram, "planet.gravitationAcceleration");
  mUniforms.planet.uMolarMass = glGetUniformLocation(mProgram, "planet.molarMass");

  mUniforms.sellmeierCoefficients.uA = glGetUniformLocation(mProgram, "sellmeierCoefficients.a");
  mUniforms.sellmeierCoefficients.uNumTerms =
      glGetUniformLocation(mProgram, "sellmeierCoefficients.numTerms");

  for (int i = 0; i < 8; ++i)
    mUniforms.sellmeierCoefficients.uTerms[i] = glGetUniformLocation(
        mProgram, ("sellmeierCoefficients.terms[" + std::to_string(i) + "]").c_str());

  glDetachShader(mProgram, precalculatorShader);
  glDeleteShader(precalculatorShader);
}

std::pair<uint32_t, uint32_t> LUTPrecalculator::createLUT(BodyWithAtmosphere const& body) {
  const double DX        = 1.0;
  auto         heightDim = static_cast<uint32_t>(body.atmosphere.height / DX);

  size_t bufferSize = heightDim * NUM_WAVELENGTHS;
  auto   data       = std::vector<double>(bufferSize);
  auto   densities  = std::vector<double>(heightDim);

  glUseProgram(mProgram);

  uint32_t ssboRefractiveIndices;
  glGenBuffers(1, &ssboRefractiveIndices);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboRefractiveIndices);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(double) * bufferSize, nullptr, GL_STATIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboRefractiveIndices);

  uint32_t ssboDensities;
  glGenBuffers(1, &ssboDensities);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDensities);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(double) * heightDim, nullptr, GL_STATIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboDensities);

  glUniform1d(mUniforms.planet.uAtmosphericHeight, body.atmosphere.height);
  glUniform1d(mUniforms.planet.uGravity, static_cast<double>(body.gravity));
  glUniform1d(mUniforms.planet.uMolarMass, body.atmosphere.molarMass);

  auto& coefficients = body.atmosphere.sellmeierCoefficients;
  // glUniform1f(mUniforms.sellmeierCoefficients.uA, 8.06051 * 1e-5);
  glUniform1d(mUniforms.sellmeierCoefficients.uA, coefficients.a);

  // std::array<glm::vec2, 2> coefficients = {
  // glm::vec2{2.480990e-2f, 132.274f}, glm::vec2{1.74557e-4f, 39.32957f}};

  // glUniform1ui(mUniforms.sellmeierCoefficients.uNumTerms, coefficients.size());
  glUniform1ui(mUniforms.sellmeierCoefficients.uNumTerms, coefficients.terms.size());

  for (size_t i = 0; i < coefficients.terms.size(); ++i) {
    glUniform2d(mUniforms.sellmeierCoefficients.uTerms[i], coefficients.terms[i].first,
        coefficients.terms[i].second);
  }

  uint32_t layerSSBO;
  glGenBuffers(1, &layerSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, layerSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, body.atmosphere.layers.size() * sizeof(AtmosphereLayer),
      body.atmosphere.layers.data(), GL_STATIC_READ);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, layerSSBO);

  const uint32_t numThreadsX = 32;
  const uint32_t numThreadsY = 32;
  glDispatchComputeGroupSizeARB(heightDim / numThreadsX + 1, NUM_WAVELENGTHS / numThreadsY + 1, 1,
      numThreadsX, numThreadsY, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboRefractiveIndices);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(double) * bufferSize, data.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboDensities);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(double) * heightDim, densities.data());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  glUseProgram(0);

  glDeleteBuffers(1, &layerSSBO);
  /*
    std::cout << body << "\n" << std::endl;

    for (int h = 0; h < body.atmosphere.height; h += 10000) {
      std::cout << "Height: " << h * DX << std::endl;
      std::cout << "Density: " << densities[h] << std::endl;
      for (int w = 0; w < NUM_WAVELENGTHS; w += 10) {
        int idx = h * NUM_WAVELENGTHS + w;
        printf("%.8f, ", data[idx]);
      }
      std::cout << std::endl;
    }*/

  // std::exit(0);

  return {ssboRefractiveIndices, ssboDensities};
}

LUTPrecalculator::~LUTPrecalculator() {
  glDeleteProgram(mProgram);
}

} // namespace cs::graphics