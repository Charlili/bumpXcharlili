#version 150
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

// these come from the programmable pipeline
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec2 texcoord;

// texture coordinates are sent to fragment shader
out vec2 texCoordVarying;

uniform float phase = 0.0; //Phase for "sin" function
uniform float distortAmount = 0.25; //Amount of distortion

void main()
{
    texCoordVarying = texcoord;
    gl_Position = modelViewProjectionMatrix * position;
    
    vec3 v = gl_Vertex.xyz;
    //Compute value of distortion for current vertex
    float distort = distortAmount * sin( phase + 0.015 * v.y );
    //Move the position
    v.x /= 1.0 + distort;
    v.y /= 1.0 + distort;
    v.z /= 1.0 + distort;
    //Set output vertex position
    vec4 posHomog = vec4( v, 1.0 );
    gl_Position = gl_ModelViewProjectionMatrix * posHomog;
    //Set output texture coordinate and color in a standard way
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
}