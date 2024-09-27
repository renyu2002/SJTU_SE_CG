#version 450 core

out vec4 FragColor;
uniform vec3 color;
#define threshold 0.01
void main()
{
    highp vec4 res=vec4(color,1.0);

    res=vec4(1.0,0.0,0.0,1.0);

    FragColor = res;
    
}