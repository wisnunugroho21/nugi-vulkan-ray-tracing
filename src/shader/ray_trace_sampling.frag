#version 460

// ------------- layout ------------- 

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0, rgba8) uniform image2D accumulateImage;
layout(set = 0, binding = 1, rgba8) uniform readonly image2D inputImage[3];

layout(push_constant) uniform Push {
  uint randomSeed;
} push;

void main() {
  vec4 accColor = imageLoad(accumulateImage, ivec2(gl_FragCoord.xy));
  vec4 totalColor = vec4(0.0, 0.0, 0.0, 0.0);

  for (uint i = 0; i < 3; i++) {
    vec4 imgColor = imageLoad(inputImage[i], ivec2(gl_FragCoord.xy));
    totalColor += imgColor;
  }

  totalColor = totalColor / 3;
  totalColor = sqrt(totalColor);
  totalColor = (totalColor + accColor * push.randomSeed) / (push.randomSeed + 1.0);

  imageStore(accumulateImage, ivec2(gl_FragCoord.xy), totalColor);
  outColor = totalColor;
}