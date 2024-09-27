#version 450 core
layout(triangles , equal_spacing,ccw) in;

void main(){
 
    vec4 upos= gl_TessCoord.x * gl_in[0].gl_Position;
    vec4 vpos= gl_TessCoord.y * gl_in[1].gl_Position;
    vec4 wpos= gl_TessCoord.z * gl_in[2].gl_Position;
    gl_Position = upos + vpos + wpos;
}
