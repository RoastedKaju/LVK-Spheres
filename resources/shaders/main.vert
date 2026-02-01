//
#version 460

layout (location=0) in vec3 inPos;

layout (location=0) out vec3 vColor;

layout (push_constant) uniform PerFrameData {
	mat4 MVP;
} pc;

void main()
{
	gl_Position = pc.MVP * vec4(inPos, 1.0);
	vColor = inPos.xyz;
}