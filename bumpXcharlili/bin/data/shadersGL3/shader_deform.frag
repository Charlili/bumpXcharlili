#version 150
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

in vec2 texCoordVarying;

// this is the output of the fragment shader
out vec4 outputColor;

void main()
{
    
    outputColor = gl_Color;
}