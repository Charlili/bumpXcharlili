#version 120
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DRect tex0;
uniform sampler2DRect tex1;
uniform vec2 pos;

varying vec2 texCoordVarying;

void main()
{
    
    //Get position fragment
    vec2 pos = gl_TexCoord[0].xy;
    //get color at position blob
    vec3 imgBlob = texture2DRect(tex1, texCoordVarying).rgb;
    //get original color at position
    vec3 origColor = texture2DRect(tex0, pos).rgb;
    
    gl_FragColor = vec4(origColor , 255);
}