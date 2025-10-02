#version 330 core
layout (location = 0) in vec3 aPos;

out vec2 TexCoords;

uniform float yMin;
uniform float yMax;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 pos = vec3(model * vec4(aPos, 1.0));

    // --- 圆柱投影 ---
    float u = atan(pos.z, pos.x) / (2.0 * 3.1415926);  // [-0.5, 0.5]
    if (u < 0.0) u += 1.0;  // 映射到 [0,1] 避免接缝
    u = fract(u);  // 避免浮点数超过 1.0
    float v = (pos.y - yMin) / (yMax - yMin);

    TexCoords = vec2(u, v);

    gl_Position = projection * view * vec4(pos, 1.0);
}
