//
#version 460

layout (location=0) in vec3 vColor;

layout (location=0) out vec4 out_FragColor;

layout (push_constant) uniform PerFrameData {
	mat4 MVP;
} pc;

void main() {
	out_FragColor = vec4(vColor, 1.0);
};