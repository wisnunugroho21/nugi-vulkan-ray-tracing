#version 450

layout(location = 0) in vec4 positionFrag;
layout(location = 1) in vec4 albedoFrag;

layout(location = 0) out vec4 positionResource;
layout(location = 1) out vec4 albedoResource;

struct Material {
  vec3 albedo;
};

layout(set = 0, binding = 0) uniform readonly GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 inverseView;
} ubo;

layout(set = 0, binding = 1) buffer readonly materialSsbo {
  Material materials[100];
};

layout(push_constant) uniform Push {
  mat4 modelMatrix;
} push;

void main() {
  positionResource = positionFrag;
  albedoResource = albedoFrag;
}