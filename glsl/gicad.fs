#version 450
uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iMove;
uniform float iZoom;
uniform vec4 iMouse;
uniform sampler2D iChannel0;

// Circle
struct GiCircle{
    vec2 pos;
    float dia;
    //vec4  color;

    float none;
};

layout(std430, binding = 3) buffer GiCircles{
        GiCircle cls[];
};
uniform uint GiCirclesCount;


// Path [head: pos in data, size], [data: pos]
struct GiPathHead{
    uint pos, size;
};

struct GiPathData{
    vec2 pos;
};

layout(std430, binding = 4) buffer _GiPathHead{
        GiPathHead giPathHeads[];
};

layout(std430, binding = 5) buffer _GiPathData{
        GiPathHead giPathDatas[];
};

uniform uint GiPathsCount;

// Out
out highp vec4 fragColor;


bool InMouseRange(inout vec4 fragColor, in vec2 fragCoord){
    // Cursor //
    vec2 cDist = fragCoord.xy - iMouse.xy - iMove * iZoom;    
    vec2 sDist = fragCoord.xy - iMouse.zw - iMove * iZoom;
    float mDist = length(cDist);

    // Vertical
    if(abs(cDist.x) < 1. * iZoom && abs(cDist.y) < 10. * iZoom){
        fragColor.xyz = vec3(0., 0., 1.);
        return true;
    }
       
    // Horizontal
    if(abs(cDist.x) < 10. * iZoom && abs(cDist.y) < 1. * iZoom){
        fragColor.xyz = vec3(0., 0., 1.);
        return true;
    }

    // Circle    
    if(mDist < 10){
        fragColor.xyz = vec3(1., 0., 0.);
        return true;
    }

    // Select
    if(iMouse.z != 0. && iMouse.w != 0.){
        if(max(abs(cDist.x), sDist.x) < (cDist.x - sDist.x)){
           //fragColor.xyz = vec3(0., 1., 0.);
           //return true;
        }

        if(max(length(cDist), length(sDist)) < length(cDist - sDist)){
            fragColor.xyz = vec3(0., 1., 0.);
            return true;
        }

     if(length(sDist) < 10){
        fragColor.xyz = vec3(1., 1., 0.);
        return true;
    }
    }    
    
    return false;
}

bool DrawCircles(inout vec4 fragColor, in vec2 fragCoord){
    for(int i = 0; i < GiCirclesCount; i ++){
    //GiCircle ci = GiCircle(vec2(500, 0.));
    float distance = length(fragCoord.xy - cls[i].pos);

    if(distance < cls[i].dia / 2)
        fragColor.y = 1.;
    }

    return true;
}

void DrawPaths(inout vec4 fragColor, in vec2 fragCoord){
    //vec2 path[2] = 


}

void mainImage(out vec4 fragColor, in vec2 fragCoord){
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));
    //vec3 col = vec3(1., 0., sin(iTime));

    // Output to screen
    fragColor = vec4(col,1.0);

    fragColor.x = max(sin(fragCoord.x * .1), sin(fragCoord.y * .1));
    //fragColor = vec4(1.0, 1.0, 1.0, texture(iChannel0, uv).r);
    fragColor = texture(iChannel0, uv);

    // Circles
    DrawCircles(fragColor, fragCoord);

    // Mouse
    if(InMouseRange(fragColor, fragCoord))
        return ;
}

float PrintChars(vec2 p, int text[16]){
    float val = 0.;
    p.x += .25;
    
    int to = text[0] + 1;

    for(int x = 1; x < to; x ++){
        int c = text[x];
    
        vec2 dFdx = dFdx(p/16.), dFdy = dFdy(p/16.);
        if(p.x < .0|| p.x > 1. || p.y < 0. || p.y > 1.){
          //  return val;
        } else{
        
            float r = textureGrad( iChannel0, p/16. + fract( vec2(c, 15-c/16) / 16. ), dFdx, dFdy ).x;
            if(r > 0.5)
                val += r;
        }
        
        p.x -= .5;
    }
    
    return val;
}

float PrintChar(int c, vec2 p){
    p.x += .25;

    vec2 dFdx = dFdx(p/16.), dFdy = dFdy(p/16.);
    if (p.x<.0|| p.x>1. || p.y<0.|| p.y>1.) return 0.;//vec4(0,0,0,1e5);
    //if (p.x<.25|| p.x>.75 || p.y<0.|| p.y>1.) return vec4(0,0,0,1e5); // strange bug with an old driver
	return textureGrad( iChannel0, p/16. + fract( vec2(c, 15-c/16) / 16. ), dFdx, dFdy ).x;
}

float PrintFloatP2(vec2 p, float val){
    int vali = int(val), count = 0;
    int arr[16];

    // Count
    while(vali > 0){
        vali /= 10;
        count ++;
    }

    if(count < 12){
        arr[0] = count + 3;
        arr[count + 1] = 46;
        arr[count + 2] = 48 + int(fract(val) * 10.);
        arr[count + 3] = 48 + int(fract(val) * 100.) % 10;
    } else
        arr[0] = count;

    // Write
    vali = int(val);
    while(vali > 0){
        arr[count --] = 48 + vali % 10;
        vali /= 10;
    }

    return PrintChars(p, arr);
}

// Tool bar
// Js: var t = 'X coodrinate', ts = t.length; while(t.length < 15) t+= ' '; ts + ', ' + t.split('').map(x=>x.charCodeAt(0)).reduce((a,b)=>a + ', ' + b);
int text_zoom[] = int[](1, 90, 32, 99, 111, 111, 114, 100, 105, 110, 97, 116, 101, 32, 32, 32);

void toolBar(inout vec4 fragColor, in vec2 fragCoord){
    vec2 uv = fragCoord/iResolution.xy;

    if(fragCoord.y < 70.){
        fragColor.y = 0.;

        //  Zoom
        fragColor.x += PrintChars((uv - vec2(.0, 0.)) * 10., text_zoom);
        fragColor.x += PrintFloatP2((uv - vec2(.1, 0.)) * 10., iZoom);
    }
}

void main(){
	// Main
	mainImage(fragColor, (gl_FragCoord.xy + iMove.xy) * iZoom);

    // Tools
    toolBar(fragColor, gl_FragCoord.xy);
}