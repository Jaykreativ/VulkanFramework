#version 450
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location=0) in vec3 pos;

layout(location=0) out vec4 fragColor;
layout(location=1) out vec3 fragPos;

layout(binding=0) uniform UBO{
    vec4 color;
    mat4 transform;
    mat4 view;
    mat4 perspective;
    vec2 viewport;
} ubo;

void main() {
    vec3 pos = (ubo.perspective * ubo.view * ubo.transform * vec4(pos, 1.0)).xyz;
    gl_Position = vec4(pos, 1);
    fragColor = vec4(ubo.color.rgb, 1);
    fragPos = pos;
}