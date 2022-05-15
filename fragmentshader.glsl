#version 330 core
out vec3 color;
in vec2 UV;
in vec3 Normal;
in vec3 l;
in float cosAlpha;
in float distance;
uniform sampler2D myTextureSampler;
float lightPower = 1.0;
vec3 lightColor = vec3(1.0, 1.0, 1.0);
void main(){
	//distance = abs()
	vec3 AmbientColor = vec3(0.5,0.5,0.5) * texture(myTextureSampler, UV).rgb;
	float cosTheta = clamp(dot(Normal, l), 0,1);
	color = AmbientColor + texture(myTextureSampler, UV).rgb * lightColor * lightPower * cosTheta /(distance * distance);
	texture(myTextureSampler, UV).rgb * lightColor * lightPower * pow(cosAlpha,5) / (distance*distance);
}