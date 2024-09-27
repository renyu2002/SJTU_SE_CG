#version 450 core
in vec3 TexCoords;
out vec4 FragColor;
uniform vec3 color;
#define threshold 0.01
void main()
{
    highp vec4 res=vec4(color,1.0);
    if(TexCoords.x<threshold||TexCoords.y<threshold||TexCoords.z<threshold)
    {
        res=vec4(1.0,0.0,0.0,1.0);
    }
    else
    {
        discard;
    }
    FragColor = res;
    
}