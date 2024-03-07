#version 450

uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iMouse, iMouseMenu;

uniform sampler2D iFont;

// Out
out highp vec4 fragColor;

// Menu
struct _MenuOptions{
    // Size
    float fontSize, letterWidth, maxWidth;    
    float borderSize, freeSize;

    // Colors
    vec4 textColor, bgColor, borderColor, activeColor;
};

layout(std140, binding = 2) uniform asda{
_MenuOptions MenuOptions;
};

struct Menu{
    int id, pos, size;
};

layout(std430, binding = 3) buffer MenuHead{
	Menu menu[];
};

/*
Menu menu[] = Menu[](
Menu(0, 0, 5), // Hello
Menu(1, 5, 5), // World
Menu(2, 10, 10) // 1234567890
); // Delete last ','
*/

layout(std430, binding = 4) buffer MenuTextBuffer{
	int menu_text[];
//int menu_text[] = int[]( 72,101,108,108,111,87,111,114,108,100,49,50,51,52,53,54,55,56,57,48 );
};

/*
// Init
void MenuInit(){
    MenuOptions.bgColor = vec3(.94, .94, .94);
    MenuOptions.textColor = vec3(.06, .94, .94);
    MenuOptions.activeColor = vec3(1.0,0.83,0.22);
    MenuOptions.borderColor = vec3(.0, .0, .94);    
    
    MenuOptions.borderSize = 2.;
    MenuOptions.freeSize = 10.;    
    
    //MenuOptions.cellHeigth = 40.;
    MenuOptions.letterWidth = 20.;
    MenuOptions.fontSize = 40.;
    
    MenuOptions.maxWidth = 0.;
    for(int i = 0; i < menu.length(); i ++){
        MenuOptions.maxWidth = max(MenuOptions.maxWidth, float(menu[i].size) * MenuOptions.letterWidth);
    }
} */

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

void DrawMenuText(inout vec4 fragColor, vec2 coord, int menuId, float menuHeight){
    int size = menu[menuId].size;
    if(size > 20)
        size = 0;

    for(int i = 0; i < 200; i ++){ // menu[menuId].size not work. It's freeze.
        if(i >= size) // Bug fix
            break;
    
        int c = menu_text[menu[menuId].pos + i];
        fragColor.xyz -= PrintChar(c, (coord - vec2(float(i) * MenuOptions.letterWidth, menuHeight * float(menuId))) / MenuOptions.fontSize);    
    }        
}

void DrawMenu(inout vec4 fragColor, in vec2 fragCoord, vec2 menuCoord){
    // Size
    float menuWidh = MenuOptions.maxWidth;
    vec2 menuSize = vec2(menuWidh, MenuOptions.fontSize * float(menu.length())) + MenuOptions.borderSize * 2.  + MenuOptions.freeSize * 2.;
    
    // Rect
    vec4 rect = vec4(menuCoord.x, menuCoord.y - menuSize.y, menuCoord.x + menuSize.x, menuCoord.y);
    vec2 coord, mouseCoord;
    
    if(InRect(fragCoord, rect, coord)){
        // Set background
        fragColor = MenuOptions.bgColor;
	fragColor.w = 1.;
    
        // In border
        CropRect(rect, MenuOptions.borderSize);
        
        if(!InRect(fragCoord, rect, coord)){
            fragColor = MenuOptions.borderColor;
            return ;
        }
        
        // In free
        CropRect(rect, MenuOptions.freeSize);
                
        if(!InRect(fragCoord, rect, coord)){
            return ;
        }
        
        // Invert Y
        coord -= vec2(MenuOptions.borderSize, MenuOptions.borderSize );
    
        // MenuId
        int menuId = menu.length() - int(coord.y / MenuOptions.fontSize) - 1;
        
        // Mouse
        float cellY = float(menu.length() - menuId - 1) * MenuOptions.fontSize;
        vec4 cellRect = vec4(rect.xy, rect.xy) + vec4(0., cellY, MenuOptions.maxWidth, cellY + MenuOptions.fontSize);
        
        if(InRect(iMouse.xy, cellRect, mouseCoord))
            fragColor = MenuOptions.activeColor;           
        
        // Draw
        coord.y = mod(coord.y, MenuOptions.fontSize) + MenuOptions.fontSize * float(menuId);
        DrawMenuText(fragColor, coord, menuId, MenuOptions.fontSize);              
    }
}

void mainImage(inout vec4 fragColor, in vec2 fragCoord){
    if(menu.length() <= 0)
	return ;

    // Output to screen
    fragColor = vec4(0., 0., 0., 0.0);    
    //fragColor = MenuOptions.activeColor;
    
    // Init Menu
//    MenuInit();
    
    // Draw menu
    vec2 menuCoord = vec2(iMouseMenu.x, iMouseMenu.y);
    DrawMenu(fragColor, fragCoord, menuCoord);
       
}

void main(){
	// Main
	mainImage(fragColor, gl_FragCoord.xy);
}