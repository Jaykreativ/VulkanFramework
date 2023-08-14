#version 450
#extension GL_KHR_vulkan_glsl : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location=0) in vec3 pos;

layout(location=0) out vec4 fragColor;

layout(binding=0) uniform UBO{
    vec4 color;
} ubo;

void main() {
    gl_Position = vec4(pos, 1.0);
    fragColor = vec4(ubo.color.rgb, 1);
}

