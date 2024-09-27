#version 330 compatibility
varying vec4 vertex_light_position;
varying vec4 v;

uniform mat4 cameraTransform;

void main() {
    vertex_light_position = vec4(normalize(gl_LightSource[0].position.xyz), 0);  // directional light


    v = vec4(gl_Vertex);

    gl_Position = gl_Vertex;  // Set the position of the current vertex
}
