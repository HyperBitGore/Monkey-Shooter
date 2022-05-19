#version 330 core
out vec3 color;
uniform sampler2D myTextureSampler;
in vec2 UV;
void main(){
	color = texture(myTextureSampler, UV).rgb;
	//color = vec3(1.0, 0.2, 0.5);
}