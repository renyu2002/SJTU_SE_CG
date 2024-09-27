#version 330 core
uniform sampler2D ourTexture;
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;
void main()
{
	 //FragColor = vec4(ourColor,1.0);
	FragColor = texture(ourTexture,vec2(TexCoord.x,1.0-TexCoord.y));
}