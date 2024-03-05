#version 450
// In
in vec4 pointColor;

// Out
out highp vec4 fragColor;


void main(){
	//fragColor = vec4(1., 0., 0., 1.);
    fragColor = pointColor;
}