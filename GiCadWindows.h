#define MGLYSCALE -1
#define GISCREENZOOM	4

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
	KiInt2 size, move;//, move, mouse, mouse_move;
	float zoom;
	
	// Mouse
	GiCadWindowsMouse mouse[3];
	//KiInt2 mouse_cur, mouse_hold;
	//bool mouse_down[3];

	// GLSL
	GlslMain glsl;
	GlslObjects glslt;

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
		glsl.Init(size);
		glslt.Init();

		GlslFontTexture.Init();
		glsl.SetFont(GlslFontTexture.GetTexture());

		glsl.UpdateZoom(zoom);
		glslt.UpdateZoom(zoom);	

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

			glsl.UpdateFps(fps);
		}
	}

	void Resize(int x, int y){
		size = KiInt2(x, y);
		glsl.UpdateRes(size);
		glslt.UpdateRes(size);
	}

	void Render(){
		if(GiProject.IsPaintLayers()){
			GiProject.DoPaintLayers(glsl);
			//glsl.SetCirData(GiProject.GetPaintData());
		}

		//
		GiProject.Render(move, KiVec2(size.x, size.y), zoom);

		glsl.Render((ttime.dtime() - stime) / 1000);

		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glslt.Render(0);
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
			GiCadWindows.glsl.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);
			//GiCadWindows.mouse_hold = GiCadWindows.mouse_cur;
			//GiCadWindows.mouse_down[1] = action;
			//GiCadWindows.glsl.UpdateMouseSelect(GiCadWindows.mouse[0].cur, );
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
	GiCadWindows.glsl.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);

	// Center button
	if(GiCadWindows.mouse[2].down){
		KiInt2 delta = GiCadWindows.mouse[2].hold - KiInt2(x, y);
		GiCadWindows.move += KiInt2(delta.x, delta.y * MGLYSCALE);

		//GiCadWindows.mouse[2].Move(KiInt2(x, y));

		GiCadWindows.mouse[2].hold += GiCadWindows.mouse[2].hold - KiInt2(x, y);
		GiCadWindows.glsl.UpdateMove(GiCadWindows.move);
		GiCadWindows.glslt.UpdateMove(GiCadWindows.move);
	}

	GiCadWindows.mouse[2].cur = KiInt2(x, y);
	GiCadWindows.mouse[2].hold = KiInt2(x, y);

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

	if(zoom < 0.05)
		zoom = 0.05;
	if(zoom > 25000)
		zoom = 25000;

	float center_x = GiCadWindows.size.x / 2;
	float center_y = GiCadWindows.size.y / 2;

	GiCadWindows.move = KiInt2(GiCadWindows.move.x * GiCadWindows.zoom / zoom, GiCadWindows.move.y * GiCadWindows.zoom / zoom);
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);
	//GiCadWindows.move -= KiInt2(xpos * yoffset, 0);
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);

	GiCadWindows.zoom = zoom;

	GiCadWindows.glsl.UpdateZoom(GiCadWindows.zoom);
	GiCadWindows.glsl.UpdateMove(GiCadWindows.move);

	//
	GiCadWindows.glslt.UpdateZoom(GiCadWindows.zoom);
	GiCadWindows.glslt.UpdateMove(GiCadWindows.move);

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