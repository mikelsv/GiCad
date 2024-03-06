#version 450
in vec4 fragCoord;
out highp vec4 fragColor;

void main(){
	gl_Position = vec4(fragCoord.xy, 0.0, 1.0);
}