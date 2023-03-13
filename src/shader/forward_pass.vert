#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in uint materialIndex;

layout(location = 0) out vec4 positionFrag;
layout(location = 1) out vec4 albedoFrag;

struct Material {
  vec3 color;
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
	vec4 position = ubo.projection * ubo.view * push.modelMatrix * vec4(position, 1.0);
	gl_Position = position;

	positionFrag = position;
	albedoFrag = vec4(materials[materialIndex].color, 1.0);
}