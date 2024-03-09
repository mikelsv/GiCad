#version 450
// In
layout (location = 0) in vec4 pointCoord;
layout (location = 1) in vec4 colorCoord;

uniform vec2 iMove;
uniform vec2 iResolution;
uniform float iZoom;

#define xZoom 1

// Out
out highp vec4 fragColor;
out vec4 pointColor;

void main(){
	gl_Position = vec4(((pointCoord.xy * iZoom * xZoom - iMove.xy ) / iResolution * 2. - 1), 0.0, 1.0);
	pointColor = colorCoord;
}