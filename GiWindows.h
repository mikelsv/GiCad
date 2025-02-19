#define MGLYSCALE -1
#define GISCREENZOOM	1

#define GICAD_ZOOM_MAX	25000
#define GICAD_ZOOM_MIN	.05


class GiWndMouse {
public:
	KiVec2 cur, hold;
	double click;
	bool down;

	GiWndMouse(){
		cur = hold = KiInt2();
		down = 0;
		click = 0;
	}

	void Move(KiVec2 pos){
		cur = pos;
		click = 0;
	}

	void Click(bool state){
		if (state) {
			hold = cur;
			click = glfwGetTime();
		}
		else
			hold = KiInt2();

		down = state;
	}

	bool IsClick() const{
		return (glfwGetTime() - click) < 1;
	}
};

class GiWndToolBar {
	// Mouse pos: home, zero
	KiVec2 hpos, zpos;

	// Zoom
	float zoom;

	// Fps
	float fps;

public:
	ImGuiCharIdExt<31> c_home, c_zero, c_zoom, c_fps;

	void UpdateMouse(KiVec2 pos, KiVec2 move, KiVec2 zero, float zoom) {
		KiVec2 p = (pos + move) / zoom;

		// Home
		SetPos(c_home, 0, (pos + move) / zoom);

		// Zero
		SetPos(c_zero, 1, (pos + move) / zoom - zero);

		c_zoom.SetStr("Zoom: ");
		c_zoom.AddFloat2(zoom);
	}

	void SetPos(ImGuiCharIdExt<31>& chr, int type, KiVec2 pos) {
		chr.Clean();
		chr.AddStr(type == 0 ? "Home: " : "Zero: ");
		chr.AddStr("(");
		chr.AddFloat2(pos.x);
		chr.AddStr(", ");
		chr.AddFloat2(pos.y);
		chr.AddStr(") ");
	}

	void UpdateFps(int fps){
		c_fps.SetStr("Fps: ");
		c_fps.AddInt(fps);
	}

} GiWndToolBar;

class GiCadWindows{
public:
	// Screen
	KiVec2 screen, pos, move;//, move, mouse, mouse_move;
	float zoom;
	bool maximized, close_window;
	GLFWwindow* window;
	
	// Mouse
	GiWndMouse mouse[3];
	//KiInt2 mouse_cur, mouse_hold;
	//bool mouse_down[3];

	// GLSL
	GlslMain glsl_main;
	//MglMenu glsl_menu;	
	GlslObjects glsl_objects;

	// PopUp
	//GiPopUp glsl_popup;

	// Timer
	MTimer ttime;
	double stime;

	// FPS
	double fps_time;
	int fps, fps_count;
	
	// New
	GiCadWindows(){
		screen = KiInt2(1024, 768);
		pos = KiInt2(-1, -1);
		move = KiInt2(0, 0);
		//mouse = KiInt2(0, 0);

		zoom = 1;
		fps_time = 0;
		close_window = 0;

		// Load config
		LoadConfig();
	}

	void Init(GLFWwindow *wnd){
		window = wnd;

		glsl_main.Init(screen);
		//glsl_popup.Init();
		glsl_objects.Init();

		GlslFontTexture.Init();
		glsl_main.SetFont(GlslFontTexture.GetTexture());
		//glsl_popup.SetFont(GlslFontTexture.GetTexture());

		glsl_main.UpdateZoom(zoom);
		glsl_objects.UpdateZoom(zoom);

		MaticalsOpenGl.width = screen.x;
		MaticalsOpenGl.height = screen.y;
		
		//CreateFrameBuffer(tex, fbo);
		//glsl.SetLayers(tex);
		//glsl.SetFont(tex);

		// Timer
		ttime.Init();
		stime = ttime.dtime();

		//glsl_popup.UpdateTime((ttime.dtime() - stime) / 1000);
	}

	void UpdateFps(){
		fps_count ++;

		if(glfwGetTime() - fps_time >= 1.0){		
			fps_time += 1.0;

			fps = fps_count;
			fps_count = 0;

			glsl_main.UpdateFps(fps);
			GiWndToolBar.UpdateFps(fps);
		}
	}

	// Set
	void SetScreen(int x, int y, bool m){
		screen = KiInt2(x, y);
		maximized = m;

		glsl_main.UpdateRes(screen);
		//glsl_popup.UpdateRes(screen);		
		glsl_objects.UpdateRes(screen);
	}

	void SetZoom(float z){
		zoom = z;

		glsl_main.UpdateZoom(zoom);
		

		//
		glsl_objects.UpdateZoom(zoom);
		
	}

	void SetMove(KiVec2 m){
		move = m;

		glsl_main.UpdateMove(move);
		glsl_objects.UpdateMove(move);
	}

	// Gui
	void CallGui(int id){
		// File
		if(id == GIGUI_MENU_PROJECT_NEW)
			GiProject.NewProject();

		if(id == GIGUI_MENU_PROJECT_OPEN)
			GiProject.OpenProject();

		if(id == GIGUI_MENU_PROJECT_EXIT)
			close_window = 1;
			
		// View
		if(id == GIGUI_HMENU_VIEW_RESET){
			SetZoom(1);
			SetMove(0);
		}

		if(id == GIGUI_HMENU_VIEW_100P){
			KiVec4 rect = GiProject.GetLayersRect();

			if(rect.IsNull())
				return ;

			float width =  rect.z - rect.x, height = rect.w - rect.y;
			float scale = min(screen.x / width, screen.y / height);

			SetZoom(scale);
			SetMove(KiVec2(rect.x * scale - (screen.x - width * scale) / 2, rect.y * scale - (screen.y - height * scale) / 2));
		}

	}

	void ResetView() {
		SetZoom(1);
		SetMove(0);
	}

	void ResetViewP(int p) {
		KiVec4 rect = GiProject.GetLayersRect();

		if (rect.IsNull())
			return;

		float width = rect.z - rect.x, height = rect.w - rect.y;
		float scale = min(screen.x / width, screen.y / height);

		scale = scale * p / 100;

		SetZoom(scale);
		SetMove(KiVec2(rect.x * scale - (screen.x - width * scale) / 2, rect.y * scale - (screen.y - height * scale) / 2));
	}

	void UpdateZeroPoint(GiCadZeroPoint type) {
		KiVec4 rect = GiProject.GetLayersRect();
		KiVec2 zero;

		switch (type) {
		default:
		case GiCadZeroPointNull:
			zero = KiVec2(0, 0);
			break;

		case GiCadZeroPointLeftUp:
			zero = KiVec2(rect.x, rect.w);
			break;

		case GiCadZeroPointLeftDown:
			zero = KiVec2(rect.x, rect.y);
			break;

		case GiCadZeroPointRightUp:
			zero = KiVec2(rect.z, rect.w);
			break;

		case GiCadZeroPointRightDown:
			//zero = KiVec2(rect.z, rect.y);
			break;
		}

		glsl_main.UpdateZero(zero);
		GiProject.SetZeroPoint(zero);
	}

	// Popup
	void InsertPopUp(VString text, int lifetime = 30){
		GiWndPopUp.Insert(text, lifetime);
	}

	// Render
	void Render(){
		if(GiProject.IsPaintLayers()){
			GiProject.PaintLayers(glsl_main);
			//glsl.SetCirData(GiProject.GetPaintData());
		}

		//
		//GiProject.Render(move, KiVec2(screen.x, screen.y), zoom);
		float wtime = (ttime.dtime() - stime) / 1000;

		glsl_main.Render((ttime.dtime() - stime) / 1000);

		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glsl_objects.Render(wtime);
		//glsl_popup.Draw((ttime.dtime() - stime) / 1000);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);		
	}

	float GetTime() {
		return (ttime.dtime() - stime) / 1000;
	}

	void LoadConfig(){
		MString data = LoadFile("GiCad.cfg");
		XDataCont json(data);

		pos.x = json["screen.px"].toi();
		pos.y = json["screen.py"].toi();
		screen.x = json["screen.x"].toi();
		screen.y = json["screen.y"].toi();
		maximized = json["screen.max"].toi();

		if(screen.IsNull()){
			screen = KiInt2(1024, 768);
			pos.x = -1;
		}
	}

	void SaveConfig(){
		KiInt2 pos;
		glfwGetWindowPos(window, &pos.x, &pos.y);
		int maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

		JsonEncode json;

		json.Up("screen");
		json("x", itos(screen.x));
		json("y", itos(screen.y));
		json("px", itos(pos.x));
		json("py", itos(pos.y));	
		json("max", itos(maximized));

		json.Save("GiCad.cfg");
	}

	~GiCadWindows(){
	}

} GiCadWindows;

static void GiWndUpdate(GLFWwindow* window, float delta){
	GiCadWindows.UpdateFps();

	if(GiCadWindows.close_window)
		glfwSetWindowShouldClose(window, 1);

	//std::cout << "delta:"<<delta<< std::endl;
//	if (keyArr[GLFW_KEY_ESCAPE])
//		glfwSetWindowShouldClose(window, 1);
//	rotatex += keyArr[GLFW_KEY_LEFT] - keyArr[GLFW_KEY_RIGHT];
//	rotatey += keyArr[GLFW_KEY_UP] - keyArr[GLFW_KEY_DOWN];
}

static void GiWndRenderScene(GLFWwindow* window, float delta){
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

	GiCadWindows.Render();
}

static void GiWndResize(GLFWwindow* window, int w, int h){
    if (w < 1 || h < 1){
		return ;
	}	

	if(MaticalsOpenGl.IsDisableResize())
		return ;

	int maximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED);

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	GiCadWindows.SetScreen(w, h, maximized);

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
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

	if (key == GLFW_KEY_A && mods == GLFW_MOD_CONTROL && action == GLFW_PRESS) {
		GiProject.SelectAll(1);
	}
}

static void GiWndMouseClickCallback(GLFWwindow* window, int button, int action, int mods){
	// ImGui
	ImGui::GetIO().MouseDown[button] = action;

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_ChildWindows))
		return;

	KiVec2 click = KiVec2(GiCadWindows.mouse[2].cur.x, GiCadWindows.screen.y - GiCadWindows.mouse[2].cur.y);
	int clicked = 0;

	int ctrl_on = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

	switch (button) {
		// Left button
		case GLFW_MOUSE_BUTTON_1:
			if (!action && GiCadWindows.mouse[0].down)
				GiProject.OnMouseAreaClick(1, ctrl_on);

			GiCadWindows.mouse[0].Click(action);
			GiCadWindows.glsl_main.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);
			//GiCadWindows.mouse_hold = GiCadWindows.mouse_cur;
			//GiCadWindows.mouse_down[1] = action;
			//GiCadWindows.glsl.UpdateMouseSelect(GiCadWindows.mouse[0].cur, );

			//GiCadWindows.glsl_popup.OnClick(KiVec2(GiCadWindows.mouse[2].cur.x, GiCadWindows.screen.y - GiCadWindows.mouse[2].cur.y));

			if (GiCadWindows.mouse[0].IsClick() && !action)
				GiProject.OnMouseClick(ctrl_on);

			GlslObjectsBuffer.SetMouseSelection(0, 0, action);			

		break;

		// Right button
		case GLFW_MOUSE_BUTTON_2:
			GiCadWindows.mouse[GLFW_MOUSE_BUTTON_2].Click(action);
			GiProject.OnMouseRightClick();
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
	ImGui::GetIO().MousePos = ImVec2(x, y);

	// Вы можете использовать функции ImGui, такие как ImGui::IsMouseHoveringWindow() или ImGui::IsMouseHoveringAnyWindow(), чтобы определить, находится ли курсор мыши над окном ImGui или любым другим элементом интерфейса.
	
	//MaticalsOpenGl.SetMouse(KiInt2(x, y));

	// Left button
	//GiCadWindows.mouse[0].cur = KiVec2(x, GiCadWindows.screen.y - y) / GISCREENZOOM;
	GiCadWindows.mouse[0].Move(KiVec2(x, GiCadWindows.screen.y - y) / GISCREENZOOM);
	GiCadWindows.glsl_main.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);

	//if (GiCadWindows.mouse[0].down) {
		GlslObjectsBuffer.SetMouseSelection(
			(GiCadWindows.mouse[0].cur + GiCadWindows.move) / GiCadWindows.zoom,
			(GiCadWindows.mouse[0].hold + GiCadWindows.move) / GiCadWindows.zoom,
			GiCadWindows.mouse[0].down
		);
	//}

	// Center button
	if(GiCadWindows.mouse[2].down){
		KiVec2 delta = GiCadWindows.mouse[2].hold - KiVec2(x, y);
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

		//GiCadWindows.mouse[2].hold += GiCadWindows.mouse[2].hold - KiInt2(x, y);
		GiCadWindows.mouse[2].hold = KiInt2(x, y);

		GiCadWindows.glsl_main.UpdateMove(GiCadWindows.move);
		GiCadWindows.glsl_objects.UpdateMove(GiCadWindows.move);
	}

	GiCadWindows.mouse[2].cur = KiInt2(x, y);
	GiCadWindows.mouse[2].hold = KiInt2(x, y);

	//print("Mouse ", itos(GiCadWindows.mouse[2].cur.x), "\r\n");

	//GiCadWindows.mouse_move = KiInt2(x, y);

	//if (dragging) {
	//	mousex += x;
	//	mousey += y;
	//}

	int ctrl_on = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

	KiVec2 mouse = (GiCadWindows.mouse[0].cur + GiCadWindows.move) / GiCadWindows.zoom;
	KiVec2 mouse2 = (GiCadWindows.mouse[0].hold + GiCadWindows.move) / GiCadWindows.zoom;

	GiProject.OnMouseMove(mouse, mouse2, GiCadWindows.mouse[0].down, GiCadWindows.zoom, ctrl_on);
	GiWndToolBar.UpdateMouse(GiCadWindows.mouse[0].cur, GiCadWindows.move, GiProject.GetZeroPoint(), GiCadWindows.zoom);
}

void GiWndMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
	ImGui::GetIO().MouseWheelH = xoffset;
	ImGui::GetIO().MouseWheel = yoffset;

	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		return;
	
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
	//KiVec2 move = (GiCadWindows.screen / GiCadWindows.zoom - GiCadWindows.screen / zoom) / 2.;
	//GiCadWindows.move = GiCadWindows.move + (GiCadWindows.size / GiCadWindows.zoom - GiCadWindows.size / zoom) / 2.;

	// New move
	KiVec2 move = (GiCadWindows.mouse[0].cur + GiCadWindows.move) / GiCadWindows.zoom;
	move = move * zoom - GiCadWindows.mouse[0].cur;

	GiCadWindows.SetMove(move);	

	float center_x = GiCadWindows.screen.x / 2;
	float center_y = GiCadWindows.screen.y / 2;
	//GiCadWindows.move = KiInt2(GiCadWindows.move.x * GiCadWindows.zoom / zoom, GiCadWindows.move.y * GiCadWindows.zoom / zoom);
	
	
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);
	//GiCadWindows.move -= KiInt2(xpos * yoffset, 0);
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);

	//GiCadWindows.zoom = zoom;

	GiCadWindows.SetZoom(zoom);

	GiWndToolBar.UpdateMouse(GiCadWindows.mouse[0].cur, GiCadWindows.move, GiProject.GetZeroPoint(), GiCadWindows.zoom);


	//glfwSetCursorPos(window, GiCadWindows.size.x / 2, GiCadWindows.size.y / 2);
//	GiCadWindows.mouse_move = KiInt2(GiCadWindows.size.x / 2, GiCadWindows.size.y / 2);
}

void GiWndDropMessage(VString path, int ok){
	ILink link(path);

	if(path.size() > 50)
		path = path.str(-50);

	if(ok)
		GiCadWindows.InsertPopUp(LString() + "Loaded success: " + path);
	else
		GiCadWindows.InsertPopUp(LString() + "[WARNING!] Loading fail: " + path);
}

void GiWndDrop(GLFWwindow* window, int count, const char** paths){
    for (int i = 0; i < count; i++){
		ILink link(paths[i]);
		VString file(paths[i]);

		GiProject.AddFileCall(file);
	}

	GiProject.OnDrop();
}

void GiWndClose(GLFWwindow* window){
	GiCadWindows.SaveConfig();
}

// Windows call
void GiWindowsUpdateTitle(){
	if(GiCadWindows.window)
		glfwSetWindowTitle(GiCadWindows.window, VString(LString() + GiProject.GetProjectName() + " - " + PROJECTNAME + " " + PROJECTVER[0].ver + _msv_zero_str));
}

void GiWindowsLayerInsertItem(int id, int active, MRGB color, VString Text){
		
	//GiCadWindows.glsl_gui.InsertList
}

void GiWindowsInsertPopUp(VString text) {
	GiCadWindows.InsertPopUp(text);
}

void GiWindowsClose() {
	GiCadWindows.close_window = 1;
}

void GuiMenuRender() {
	if (ImGui::BeginMainMenuBar()) {
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		ImVec2 isize(32, 32);

		// File
		if (ImGui::BeginMenu("File")) {
			// New Project
			if (ImGui::MenuItem("New Projects", "Ctrl+N"))
				GiProject.NewProject();

			ImGui::SameLine();
			ImGui::Image(GiImages.new_project, isize);
			
			// Open Project
			if (ImGui::MenuItem("Open Project", "Ctrl+O"))
				GiProject.OpenProject();

			ImGui::SameLine();
			ImGui::Image(GiImages.open_project, isize);

			// Save project
			if (ImGui::MenuItem("Save project", "Ctrl+S")){}
			ImGui::SameLine();
			ImGui::Image(GiImages.save_project, isize);

			// Save project as
			if (ImGui::MenuItem("Save project as", "Ctrl+Shift+S")) {}
			ImGui::SameLine();
			ImGui::Image(GiImages.save_project2, isize);

			// Add file
			if (ImGui::MenuItem("Add file", "Ctrl+Shift+F"))
				GiProject.MenuAddFile();
			ImGui::SameLine();
			ImGui::Image(GiImages.open_project, isize);

			// Add script
			if (ImGui::MenuItem("Add script", ""))
				GiProject.MenuAddScript();
			ImGui::SameLine();
			ImGui::Image(GiImages.open_project, isize);


			// Exit
			if (ImGui::MenuItem("Exit", ""))
				GiWindowsClose();

			ImGui::EndMenu();
		}
		ImGui::Separator();
		
		// Edit
		if (ImGui::BeginMenu("Edit")){
			if (ImGui::MenuItem("Reset view", "Ctrl+Z"))
			{
				// Обработка действия отмены
			}
			if (ImGui::MenuItem("100%", "Ctrl+Y"))
			{
				// Обработка действия повтора
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();

		// View
		if (ImGui::BeginMenu("View")) {
			static int zoom = 100;
			int nzoom = 0;

			if (ImGui::MenuItem("Reset view", "Ctrl+0")){
				GiCadWindows.ResetView();				
			}

			if (ImGui::MenuItem("100%", "Ctrl+1"))
				nzoom = 100;
			
			if (ImGui::MenuItem("75%", "Ctrl+1"))
				nzoom = 75;
			
			if (ImGui::MenuItem("50%", "Ctrl+1"))
				nzoom = 50;

			if (ImGui::SliderInt("%", &zoom, 0, 150))
				nzoom = zoom;

			if (nzoom) {
				zoom = nzoom;
				GiCadWindows.ResetViewP(zoom);
			}

			ImGui::EndMenu();
		}
		ImGui::Separator();

		if (ImGui::BeginMenu("Points")) {
			if (ImGui::MenuItem("Reset zero point"))
				GiCadWindows.UpdateZeroPoint(GiCadZeroPointNull);

			if (ImGui::MenuItem("Zero point to Left Up"))
				GiCadWindows.UpdateZeroPoint(GiCadZeroPointLeftUp);

			if (ImGui::MenuItem("Zero point to Left Down"))
				GiCadWindows.UpdateZeroPoint(GiCadZeroPointLeftDown);

			if (ImGui::MenuItem("Zero point to Right Up"))
				GiCadWindows.UpdateZeroPoint(GiCadZeroPointRightUp);

			if (ImGui::MenuItem("Zero point to Right Down"))
				GiCadWindows.UpdateZeroPoint(GiCadZeroPointRightDown);

			ImGui::EndMenu();
		}
		ImGui::Separator();

		// Tools
		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem("Tools base", "")) {
				GiTools.ShowTools(1);
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();

		// Path
		if (ImGui::BeginMenu("Path")) {
			if (ImGui::MenuItem("Drill", "")) {
				GiProject.SetProg(GiProjectProgDrill);
			}
			if (ImGui::MenuItem("Laser", "")) {
				GiProject.SetProg(GiProjectProgLaser);
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();

		// About
		if (ImGui::BeginMenu("About")) {
			if (ImGui::MenuItem("About GiCad", "Ctrl+?")) {
				//GiCadWindows.ResetView();
			}
			if (ImGui::MenuItem("Source", "Ctrl+Shift+?")) {
				//GiCadWindows.ResetView100p();
			}
			ImGui::EndMenu();
		}


		ImGui::EndMainMenuBar();
	}
}

void GuiLayersRender() {
	GiProject.GuiRender();
	GiTools.Render();

	// Status Bar
	ImGui::SetNextWindowPos(ImVec2(0, GiCadWindows.screen.y - 50));
	ImGui::SetNextWindowSize(ImVec2(GiCadWindows.screen.x, 50));

	ImGui::Begin("StatusBar", nullptr, ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

	ImGui::Text(GiWndToolBar.c_home);
	ImGui::SameLine();
	ImGui::Text(GiWndToolBar.c_zero);
	ImGui::SameLine();
	ImGui::Text(GiWndToolBar.c_zoom);
	ImGui::SameLine();
	ImGui::Text(GiWndToolBar.c_fps);

	ImGui::End();
}

void GuiRender() {
	GuiMenuRender();
	GuiLayersRender();

	GiWndPopUp.Draw(GiCadWindows.GetTime());
}

void GiWindowsResetView100p() {
	GiCadWindows.ResetViewP(100);
}