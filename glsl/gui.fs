#version 450

uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iMouse;

uniform sampler2D iFont;

// Out
out highp vec4 fragColor;

struct MglGuiOptions{
	float fontSize, letterWidth;
	float borderSize, freeSize;
	//float unk[3];

	vec4 textColor, bgColor, borderColor;
};

layout(std140, binding = 8) uniform _MglGuiOptions{
    MglGuiOptions opt;
};

struct MglGuUpHead{
	int id, text_pos, text_size, _unk;
	vec4 rect;
};

layout(std430, binding = 9) buffer _MglGuUpHead{
	MglGuUpHead gui[];
};

uniform int guiSize;

layout(std430, binding = 10) buffer _MglTextBuffer{
	int mgl_text[];
};

// Rect functions
bool InRect(vec2 coord, vec4 rect, out vec2 coordIn){
    if(coord.x > rect.x && coord.x < rect.z &&
        coord.y > rect.y && coord.y < rect.w
    ){
        coordIn = coord - rect.xy;    
        return true;
    }
    return false;
}

vec4 MoveRect(vec4 rect, vec2 move){
    return vec4(rect.xy + move, rect.zw + move);
}

void CropRect(inout vec4 rect, float size){
    rect.x += size;
    rect.y += size;
    rect.z -= size;
    rect.w -= size;
}

// Draw
float PrintChar(int c, vec2 p){
    p.x += .25;
    
    vec2 dFdx = dFdx(p / 16.), dFdy = dFdy(p / 16.);
    if(p.x < .0 || p.x > 1. || p.y < 0. || p.y > 1.)
        return 0.;//vec4(0,0,0,1e5);
    
	return textureGrad(iFont, p / 16. + fract(vec2(c, 15 - c / 16) / 16.), dFdx, dFdy).x;
}

void DrawGuiText(inout vec4 fragColor, vec2 coord, int id){
    int size = gui[id].text_size;

    for(int i = 0; i < size; i ++){
        int c = mgl_text[gui[id].text_pos + i];
        fragColor.xyz -= PrintChar(c, (coord - vec2(float(i) * opt.letterWidth, 0.)) / opt.fontSize);    
    }
}

void mainImage(inout vec4 fragColor, in vec2 fragCoord){
    if(guiSize <= 0)
        return;

    // Get item in rect
    vec2 coord;

    for(int i = 0; i < guiSize; i ++){
        vec4 rect = gui[i].rect;

    /*
        if(popup[i].stime > iTime || popup[i].etime < iTime)
            continue;

        

        // Move up
        if(iTime - popup[i].stime < opt.showTime)
            rect = MoveRect(rect, - sin(vec2(0., opt.showTime - (iTime - popup[i].stime))) * 600.);


        // Move right
       if(popup[i].etime - iTime < opt.hideTime){
            //rect = MoveRect(rect, vec2(iTime * 5., 0.));
            rect = MoveRect(rect, -sin(vec2((popup[i].etime - iTime) - opt.hideTime, 0.)) * 400.);
        }*/


        //rect = MoveRect(rect, - vec2(0., popup[i].etime));

        // In rect
        if(InRect(fragCoord, rect, coord)){
            // Set background
            fragColor = opt.bgColor;
            CropRect(rect, opt.borderSize);

            // In border
            if(!InRect(fragCoord, rect, coord)){
                fragColor = opt.borderColor;
                return ;
            }

            // In free
            CropRect(rect, opt.freeSize);
                
            if(!InRect(fragCoord, rect, coord))
                return ;
            
            // Draw
            DrawGuiText(fragColor, coord, i);
        }        
    }
}

void main(){
	// Main
	mainImage(fragColor, gl_FragCoord.xy);
}