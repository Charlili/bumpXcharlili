#version 120

uniform sampler2DRect tex0;
uniform sampler2DRect maskTex;
//uniform vec2 pos;

varying vec2 texCoordVarying;

void main()
{
    
    vec3 src = texture2DRect(tex0, texCoordVarying).rgb;
    float mask = texture2DRect(maskTex, texCoordVarying).r;
    if(mask == 255)mask=100;
    gl_FragColor = vec4(src , mask);
}