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

vec3 hsl2rgb( vec3 c ) {
    vec3 rgb = clamp(abs(mod(c.x*6.0 + vec3(0.0, 4.0, 2.0), 6.0)-3.0)-1.0, 0.0, 1.0);
    return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

void main() {
    gl_Position = vec4(pos, 1.0);
    fragColor = vec4(hsl2rgb(ubo.color.rgb), 1);
}

