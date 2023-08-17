#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location=0) in vec4 fragColor;
layout(location=1) in vec3 fragPos;

layout(location=0) out vec4 outColor;

vec3 hsl2rgb( vec3 c ) {
    vec3 rgb = clamp(abs(mod(c.x*6.0 + vec3(0.0, 4.0, 2.0), 6.0)-3.0)-1.0, 0.0, 1.0);
    return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

void main() {
    //float dist = sqrt(pow(gl_FragCoord.x-viewport.x/2, 2) + pow(gl_FragCoord.y-viewport.y/2, 2)); 
    /*float dist = length(fragPos);
    vec3 color = fragColor.rgb;
    color.r += dist;
    color.b = sin(dist/7.5+fragColor.r*6.1);
    outColor = vec4(hsl2rgb(color), 1);*/
    outColor = vec4(fragPos, 1);
}