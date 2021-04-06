////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ColorConverter.hpp"
#include "../../cs-utils/filesystem.hpp"
#include "EclipseConstants.hpp"

#include <GL/glew.h>
#include <glm/vec4.hpp>
#include <iostream>
#include <string>

namespace cs::graphics {

void ColorConverter::init() {
  mProgram               = glCreateProgram();
  uint32_t computeShader = glCreateShader(GL_COMPUTE_SHADER);

  std::string code =
      cs::utils::filesystem::loadToString("../share/resources/shaders/EclipseColorTransform.glsl");
  const char* shader = code.c_str();
  glShaderSource(computeShader, 1, &shader, nullptr);
  glCompileShader(computeShader);

  GLint isCompiled = 0;
  glGetShaderiv(computeShader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(computeShader, maxLength, &maxLength, &errorLog[0]);
    for (const auto& item : errorLog) {
      std::cerr << item;
    }
    std::cerr << std::endl;
  }

  glAttachShader(mProgram, computeShader);
  glLinkProgram(mProgram);

  GLint isLinked = 0;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &isLinked);
  if (isLinked == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetProgramInfoLog(mProgram, maxLength, &maxLength, &errorLog[0]);
    for (const auto& item : errorLog) {
      std::cerr << item;
    }
    std::cerr << std::endl;
  }

  glValidateProgram(mProgram);

  glDetachShader(mProgram, computeShader);
  glDeleteShader(computeShader);
}

std::vector<glm::dvec4> ColorConverter::convert(std::vector<DoublePixel> const& pixel) {
  glUseProgram(mProgram);

  uint32_t ssboPixelBuffer;
  glGenBuffers(1, &ssboPixelBuffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPixelBuffer);
  glBufferData(
      GL_SHADER_STORAGE_BUFFER, pixel.size() * sizeof(DoublePixel), pixel.data(), GL_STATIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPixelBuffer);

  uint32_t ssboOutput;
  glGenBuffers(1, &ssboOutput);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutput);
  glBufferData(
      GL_SHADER_STORAGE_BUFFER, pixel.size() * sizeof(glm::dvec4), nullptr, GL_STATIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboOutput);

  uint32_t numThreads = 32u;
  uint32_t numBlocksX = TEX_WIDTH / numThreads;
  uint32_t numBlocksY = TEX_HEIGHT / numThreads;

  glDispatchComputeGroupSizeARB(numBlocksX, numBlocksY, 1, numThreads, numThreads, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glFlush();
  glFinish();

  std::vector<glm::dvec4> output(TEX_WIDTH * TEX_HEIGHT);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOutput);
  glGetBufferSubData(
      GL_SHADER_STORAGE_BUFFER, 0, output.size() * sizeof(glm::dvec4), output.data());
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  glDeleteBuffers(1, &ssboPixelBuffer);
  glDeleteBuffers(1, &ssboOutput);

  return output;
}

ColorConverter::~ColorConverter() {
  glDeleteProgram(mProgram);
}
} // namespace cs::graphics