
#version 450 core
layout (vertices = 3) out;
uniform int inner;
uniform int outer;
void main(){
    //inner
    gl_TessLevelInner[0] = inner;

    // outer
    gl_TessLevelOuter[0] = outer;
    gl_TessLevelOuter[1] = outer;
    gl_TessLevelOuter[2] = outer;
    
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    //TextureCoord[gl_InvocationID] = TexCoord[gl_InvocationID];
 
}
