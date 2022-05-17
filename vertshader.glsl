#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;

void main(){
// Output position of the vertex, in clip space
	// map [0..800][0..600] to [-1..1][-1..1]
	vec2 vertexPosition_homoneneousspace = vertexPosition_modelspace.xy - vec2(512,384); // [0..800][0..600] -> [-400..400][-300..300]
	vertexPosition_homoneneousspace /= vec2(512,384);
	gl_Position =  vec4(vertexPosition_homoneneousspace,0,1);

	//gl_Position = vec4(vertexPosition_modelspace.x, vertexPosition_modelspace.y, 0, 1);
}