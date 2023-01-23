#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace nugiEngine {
  struct Sphere {
    alignas(16) glm::vec3 center;
    float radius;
    uint32_t materialType;
  };

  struct Lambertian {
    alignas(16) glm::vec3 colorAlbedo;
  };

  struct Metal {
    alignas(16) glm::vec3 colorAlbedo;
    float fuzziness;
  };

  struct Dielectric {
    float indexOfRefraction;
  };

  struct RayTraceUbo {
    alignas(16) glm::vec3 origin;
    alignas(16) glm::vec3 horizontal;
    alignas(16) glm::vec3 vertical;
    alignas(16) glm::vec3 lowerLeftCorner;
  };

  struct RayTraceObject {
    Sphere spheres[500];
    uint32_t numObj;
  };

  struct RayTracePushConstant {
    int randomSeed;
	};
}