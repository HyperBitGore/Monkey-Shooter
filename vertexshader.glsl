#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform vec3 LIGHT;
out vec2 UV;
out vec3 Normal;
out vec3 l;
out float cosAlpha;
out float distance;
void main(){
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
	
	//M is model and V is view matrice
	vec3 Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	vec3 EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
	vec3 LightPosition_cameraspace = ( V * vec4(LIGHT,1)).xyz;
	vec3 LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
	vec3 Normal_cameraspace = ( V * M * vec4(vertexNormal,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	
	
	UV = vertexUV;
	Normal = normalize(Normal_cameraspace);
	l = normalize(LightDirection_cameraspace);
	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,Normal);
	cosAlpha = clamp( dot( E,R ), 0,1 );
	vec3 temp = abs(Position_worldspace - LIGHT);
	float avgdis = (temp.x + temp.y + temp.z)/3;
	distance = avgdis;
}