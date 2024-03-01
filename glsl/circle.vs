#version 450
in vec4 fragCoord;
uniform vec2 iMove;
uniform vec2 iResolution;
uniform float iZoom;

out highp vec4 fragColor;

void main(){
	gl_Position = vec4((fragCoord.xy - iMove.xy) / iResolution / iZoom, 0.0, 1.0);
}