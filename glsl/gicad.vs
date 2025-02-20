#version 430
in vec4 fragCoord;
//in vec2 iResolution;
//in float iTime;

out highp vec4 fragColor;

void main(){
	gl_Position = vec4(fragCoord.xy, 0.0, 1.0);
}