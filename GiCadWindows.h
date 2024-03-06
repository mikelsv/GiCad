#define MGLYSCALE -1
#define GISCREENZOOM	4

#define GICAD_ZOOM_MAX	25000
#define GICAD_ZOOM_MIN	.05


class GiCadWindowsMouse{
public:
	KiInt2 cur, hold;
	bool down;

	GiCadWindowsMouse(){
		cur = hold = KiInt2();
		down = 0;
	}

	void Click(bool state){
		if(state)
			hold = cur;
		else
			hold = KiInt2();

		down = state;
	}

};

class GiCadWindows{
public:
	// Screen
	KiVec2 size, move;//, move, mouse, mouse_move;
	float zoom;
	
	// Mouse
	GiCadWindowsMouse mouse[3];
	//KiInt2 mouse_cur, mouse_hold;
	//bool mouse_down[3];

	// GLSL
	GlslMain glsl_main;
	MglMenu glsl_menu;
	GlslObjects glsl_objects;

	// Timer
	MTimer ttime;
	double stime;

	// FPS
	double fps_time;
	int fps, fps_count;

	GLuint tex, fbo;
	
	// Nww
	GiCadWindows(){
		size = KiInt2(1024, 768);
		move = KiInt2(0, 0);
		//mouse = KiInt2(0, 0);
		zoom = 1;
		fps_time = 0;
	}

	void Init(){
		glsl_main.Init(size);
		glsl_menu.Init();
		glsl_objects.Init();

		GlslFontTexture.Init();
		glsl_main.SetFont(GlslFontTexture.GetTexture());
		glsl_menu.SetFont(GlslFontTexture.GetTexture());

		glsl_main.UpdateZoom(zoom);
		glsl_objects.UpdateZoom(zoom);

		MaticalsOpenGl.width = size.x;
		MaticalsOpenGl.height = size.y;
		
		//CreateFrameBuffer(tex, fbo);
		//glsl.SetLayers(tex);
		//glsl.SetFont(tex);

		ttime.Init();
		stime = ttime.dtime();
	}

	void UpdateFps(){
		fps_count ++;

		if(glfwGetTime() - fps_time >= 1.0){		
			fps_time += 1.0;

			fps = fps_count;
			fps_count = 0;

			glsl_main.UpdateFps(fps);
		}
	}

	void Resize(int x, int y){
		size = KiInt2(x, y);
		glsl_main.UpdateRes(size);
		glsl_menu.UpdateRes(size);
		glsl_objects.UpdateRes(size);
	}

	// Menu
	void ShowMenu(){
		glsl_menu.UpdateMouseMenu(KiInt2(mouse[2].cur.x, size.y - mouse[2].cur.y));

		glsl_menu.InsertItem(0, "Hello World!");
		glsl_menu.InsertItem(1, "1234567890");
		glsl_menu.InsertItem(2, "");
		glsl_menu.InsertItem(1, "London is the...");

		glsl_menu.DrawMenu();
	}

	void HideMenu(){
		glsl_menu.CleanMenu();
		glsl_menu.DrawMenu();
	}

	// Render
	void Render(){
		if(GiProject.IsPaintLayers()){
			GiProject.DoPaintLayers(glsl_main);
			//glsl.SetCirData(GiProject.GetPaintData());
		}

		//
		GiProject.Render(move, KiVec2(size.x, size.y), zoom);

		glsl_main.Render((ttime.dtime() - stime) / 1000);

		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glsl_objects.Render(0);
		glsl_menu.Render(0);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	}

	void LoadCongig(){


	}

	void SaveConfig(){



	}

} GiCadWindows;

static void GiWndUpdate(GLFWwindow* window, float delta){
	GiCadWindows.UpdateFps();	

	//std::cout << "delta:"<<delta<< std::endl;
//	if (keyArr[GLFW_KEY_ESCAPE])
//		glfwSetWindowShouldClose(window, 1);
//	rotatex += keyArr[GLFW_KEY_LEFT] - keyArr[GLFW_KEY_RIGHT];
//	rotatey += keyArr[GLFW_KEY_UP] - keyArr[GLFW_KEY_DOWN];
}

static void GiWndRenderScene(GLFWwindow* window, float delta){
	//glClearColor(0.90, 0.50, 0.30, 0.0);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

	/*
	glColor3f(1, 1, 1);

	glBegin(GL_LINE_LOOP);
	glVertex2f(0.25, 0.25);
	glVertex2f(0.75, 0.25);
	glVertex2f(0.75, 0.75);
	glVertex2f(0.25, 0.75);
	glEnd();
	glFlush();

	glColor3f(0.90, 0.50, 0.30);

	glBegin(GL_POLYGON);
	glVertex2f(0.05, 0.05);
	glVertex2f(0.90, 0.25);
	glVertex2f(0.95, 0.30);
	glVertex2f(0.99, 0.35);
	glVertex2f(0.75, 0.75);
	glVertex2f(0.25, 0.75);
	glEnd();
	glFlush();
	*/

	GiCadWindows.Render();

	//render();
}

static void GiWndResize(GLFWwindow* window, int w, int h){
    if (h < 1){
		//exit(0);
        //h = 1;
		return ;
	}/*

	

	if(MaticalsOpenGl.IsDisableResize())
		return ;
		*/

	glViewport ( 0, 0, (GLsizei)w, (GLsizei)h );
	GiCadWindows.Resize(w, h);

	/*
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 1000, -1000); // , 1000, -1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	GiCadWindows.Resize(w, h);
	*/

	/*
	MaticalsOpenGl.UpdateSize(w, h);
	//DrawTextScale = KiVec2(0.0002 * h / w, 0.0002);
	DrawTextScale = KiVec2(0.002 * h / w, 0.002);
	*/
}

static void GiWndKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
	int on_title = 0;

}

static void GiWndMouseClickCallback(GLFWwindow* window, int button, int action, int mods){
	switch (button) {
		case GLFW_MOUSE_BUTTON_1:
			GiCadWindows.mouse[0].Click(action);
			GiCadWindows.glsl_main.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);
			//GiCadWindows.mouse_hold = GiCadWindows.mouse_cur;
			//GiCadWindows.mouse_down[1] = action;
			//GiCadWindows.glsl.UpdateMouseSelect(GiCadWindows.mouse[0].cur, );

			if(GiCadWindows.glsl_menu.IsActive())
				GiCadWindows.HideMenu();
		break;

		case GLFW_MOUSE_BUTTON_2:
			if(action == 0 && GiCadWindows.mouse[GLFW_MOUSE_BUTTON_2].down)
				GiCadWindows.ShowMenu();

			if(action == 1)
				GiCadWindows.HideMenu();

			GiCadWindows.mouse[GLFW_MOUSE_BUTTON_2].Click(action);
		break;

		case GLFW_MOUSE_BUTTON_3:
		//MaticalsOpenGl.mouse_down = 1;
		//dragging = action;
		//GiCadWindows.mouse_down[3] = action;
		GiCadWindows.mouse[2].Click(action);
		//
		break;
	}
}

static void GiWndMouseMotionCallback(GLFWwindow* window, double x, double y){
	//MaticalsOpenGl.SetMouse(KiInt2(x, y));

	// Left button
	GiCadWindows.mouse[0].cur = KiInt2(x, GiCadWindows.size.y - y) / GiCadWindows.zoom / GISCREENZOOM;
	GiCadWindows.glsl_main.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);

	// Center button
	if(GiCadWindows.mouse[2].down){
		KiInt2 delta = GiCadWindows.mouse[2].hold - KiInt2(x, y);
		GiCadWindows.move += KiVec2(delta.x, delta.y * MGLYSCALE);

		// Limits
		if(GiCadWindows.move.x < -5000)
			GiCadWindows.move.x = -5000;
		if(GiCadWindows.move.x > 5000)
			GiCadWindows.move.x = 5000;
		if(GiCadWindows.move.y < -5000)
			GiCadWindows.move.y = -5000;
		if(GiCadWindows.move.y > 5000)
			GiCadWindows.move.y = 5000;

		GiCadWindows.mouse[2].hold += GiCadWindows.mouse[2].hold - KiInt2(x, y);
		GiCadWindows.glsl_main.UpdateMove(GiCadWindows.move);
		GiCadWindows.glsl_objects.UpdateMove(GiCadWindows.move);
	}

	GiCadWindows.mouse[2].cur = KiInt2(x, y);
	GiCadWindows.mouse[2].hold = KiInt2(x, y);

	if(GiCadWindows.glsl_menu.IsActive())
		GiCadWindows.glsl_menu.UpdateMouse(KiInt2(x, GiCadWindows.size.y - y));

	//print("Mouse ", itos(GiCadWindows.mouse[2].cur.x), "\r\n");

	//GiCadWindows.mouse_move = KiInt2(x, y);

	//if (dragging) {
	//	mousex += x;
	//	mousey += y;
	//}
}

void GiWndMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
	double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

	float zoom = GiCadWindows.zoom + yoffset * 0.2f;
	//KiVec2 move = KiVec2(GiCadWindows.size) / GiCadWindows.zoom;
	

	if(yoffset > 0){
		zoom = min(GICAD_ZOOM_MAX, (1 + 1 / 3.0) * GiCadWindows.zoom);
		//GiCadWindows.move.x += (GiCadWindows.size.x / GiCadWindows.zoom - GiCadWindows.size.x / zoom) / 2.;
		//GiCadWindows.move.y += (GiCadWindows.size.y / GiCadWindows.zoom - GiCadWindows.size.y / zoom) / 2.;
	}
	else
		zoom = max(GICAD_ZOOM_MIN, (1 - 1 / 3.0) * GiCadWindows.zoom);

	//KiVec2 size = KiVec2(GiCadWindows.size);
	KiVec2 move = (GiCadWindows.size / GiCadWindows.zoom - GiCadWindows.size / zoom) / 2.;
	//GiCadWindows.move = GiCadWindows.move + (GiCadWindows.size / GiCadWindows.zoom - GiCadWindows.size / zoom) / 2.;
	

	float center_x = GiCadWindows.size.x / 2;
	float center_y = GiCadWindows.size.y / 2;

	GiCadWindows.move = KiInt2(GiCadWindows.move.x * GiCadWindows.zoom / zoom, GiCadWindows.move.y * GiCadWindows.zoom / zoom);
	
	
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);
	//GiCadWindows.move -= KiInt2(xpos * yoffset, 0);
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);

	GiCadWindows.zoom = zoom;

	GiCadWindows.glsl_main.UpdateZoom(GiCadWindows.zoom);
	GiCadWindows.glsl_main.UpdateMove(GiCadWindows.move);

	//
	GiCadWindows.glsl_objects.UpdateZoom(GiCadWindows.zoom);
	GiCadWindows.glsl_objects.UpdateMove(GiCadWindows.move);

	//glfwSetCursorPos(window, GiCadWindows.size.x / 2, GiCadWindows.size.y / 2);
//	GiCadWindows.mouse_move = KiInt2(GiCadWindows.size.x / 2, GiCadWindows.size.y / 2);
}

void GiWndDrop(GLFWwindow* window, int count, const char** paths){
    for (int i = 0; i < count; i++){
		ILink link(paths[i]);
		VString file(paths[i]);

		//if(link.ext() == "gbr"){
		if(file.str(-3) == "gbr"){
			GiProject.AddGbrFile(paths[i]);
		}

		if(file.str(-3) == "drl"){
			GiProject.AddDrlFile(paths[i]);
			//GiDrill
		}		
	}
}