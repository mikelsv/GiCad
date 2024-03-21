#define MGLYSCALE -1
#define GISCREENZOOM	1

#define GICAD_ZOOM_MAX	25000
#define GICAD_ZOOM_MIN	.05


class GiCadWindowsMouse{
public:
	KiVec2 cur, hold;
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
	KiVec2 screen, pos, move;//, move, mouse, mouse_move;
	float zoom;
	bool maximized, close_window;
	GLFWwindow* window;
	
	// Mouse
	GiCadWindowsMouse mouse[3];
	//KiInt2 mouse_cur, mouse_hold;
	//bool mouse_down[3];

	// GLSL
	GlslMain glsl_main;
	MglMenu glsl_menu;
	MglPopUp glsl_popup;
	GlslObjects glsl_objects;

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
		LoadCongig();
	}

	void Init(GLFWwindow *wnd){
		window = wnd;

		glsl_main.Init(screen);
		glsl_menu.Init();
		glsl_popup.Init();
		glsl_objects.Init();

		GlslFontTexture.Init();
		glsl_main.SetFont(GlslFontTexture.GetTexture());
		glsl_menu.SetFont(GlslFontTexture.GetTexture());
		glsl_popup.SetFont(GlslFontTexture.GetTexture());

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

		glsl_popup.UpdateTime((ttime.dtime() - stime) / 1000);
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

	// Set
	void SetScreen(int x, int y, bool m){
		screen = KiInt2(x, y);
		maximized = m;

		glsl_main.UpdateRes(screen);
		glsl_menu.UpdateRes(screen);
		glsl_popup.UpdateRes(screen);		
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
		// Menu
		if(id == GIGUI_HMENU_FILE)
			ShowMenuFile();
		if(id == GIGUI_HMENU_VIEW)
			ShowMenuView();
		if(id == GIGUI_HMENU_HELP)
			ShowMenuHelp();

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

	void ResetView100p() {
		KiVec4 rect = GiProject.GetLayersRect();

		if (rect.IsNull())
			return;

		float width = rect.z - rect.x, height = rect.w - rect.y;
		float scale = min(screen.x / width, screen.y / height);

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

	void ShowMenuFile(){
		glsl_menu.CleanMenu();
		
		glsl_menu.InsertItem(GIGUI_MENU_PROJECT_NEW, "New project");
		glsl_menu.InsertItem(GIGUI_MENU_PROJECT_OPEN, "Open project");
		glsl_menu.InsertItem(GIGUI_MENU_PROJECT_SAVE, "Save project");
		glsl_menu.InsertItem(GIGUI_MENU_PROJECT_SAVEAS, "Save project as");
		glsl_menu.InsertItem(GIGUI_MENU_PROJECT_EXIT, "Exit");

		glsl_menu.UpdateMouseMenu(KiVec2(mouse[2].cur.x, screen.y - mouse[2].cur.y));
		glsl_menu.DrawMenu();
	}

	void ShowMenuView(){
		glsl_menu.CleanMenu();
		
		glsl_menu.InsertItem(GIGUI_HMENU_VIEW_RESET, "Reset view");
		glsl_menu.InsertItem(GIGUI_HMENU_VIEW_100P, "100%");

		glsl_menu.UpdateMouseMenu(KiVec2(mouse[2].cur.x, screen.y - mouse[2].cur.y));
		glsl_menu.DrawMenu();
	}

	void ShowMenuHelp(){
		glsl_menu.CleanMenu();
		
		glsl_menu.InsertItem(GIGUI_HMENU_HELP_ABOUT, "About GiCad");
		glsl_menu.InsertItem(GIGUI_HMENU_HELP_SOURCE, "Source");

		glsl_menu.UpdateMouseMenu(KiVec2(mouse[2].cur.x, screen.y - mouse[2].cur.y));
		glsl_menu.DrawMenu();
	}

	// Menu
	void ShowMenu(){
		glsl_menu.UpdateMouseMenu(KiVec2(mouse[2].cur.x, screen.y - mouse[2].cur.y));

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

	// Popup
	void InsertPopUp(VString text, int lifetime = 30){
		glsl_popup.Insert(text, lifetime);
	}

	// Render
	void Render(){
		if(GiProject.IsPaintLayers()){
			GiProject.DoPaintLayers(glsl_main);
			//glsl.SetCirData(GiProject.GetPaintData());
		}

		//
		//GiProject.Render(move, KiVec2(screen.x, screen.y), zoom);

		glsl_main.Render((ttime.dtime() - stime) / 1000);

		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glsl_objects.Render(0);
		glsl_popup.Render((ttime.dtime() - stime) / 1000);
		glsl_menu.Render(0);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);		
	}

	void LoadCongig(){
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
	int on_title = 0;
}

static void GiWndMouseClickCallback(GLFWwindow* window, int button, int action, int mods){
	// ImGui
	ImGui::GetIO().MouseDown[button] = action;
	
	
	KiVec2 click = KiVec2(GiCadWindows.mouse[2].cur.x, GiCadWindows.screen.y - GiCadWindows.mouse[2].cur.y);
	int clicked = 0;

	switch (button) {
		case GLFW_MOUSE_BUTTON_1:
			GiCadWindows.mouse[0].Click(action);
			GiCadWindows.glsl_main.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);
			//GiCadWindows.mouse_hold = GiCadWindows.mouse_cur;
			//GiCadWindows.mouse_down[1] = action;
			//GiCadWindows.glsl.UpdateMouseSelect(GiCadWindows.mouse[0].cur, );

			if(GiCadWindows.glsl_menu.IsActive() && !action){
				GiCadWindows.CallGui(clicked = GiCadWindows.glsl_menu.OnClick(click));

				GiCadWindows.HideMenu();
			}

			GiCadWindows.glsl_popup.OnClick(KiVec2(GiCadWindows.mouse[2].cur.x, GiCadWindows.screen.y - GiCadWindows.mouse[2].cur.y));

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
	ImGui::GetIO().MousePos = ImVec2(x, y);

	// Вы можете использовать функции ImGui, такие как ImGui::IsMouseHoveringWindow() или ImGui::IsMouseHoveringAnyWindow(), чтобы определить, находится ли курсор мыши над окном ImGui или любым другим элементом интерфейса.
	
	//MaticalsOpenGl.SetMouse(KiInt2(x, y));

	// Left button
	GiCadWindows.mouse[0].cur = KiVec2(x, GiCadWindows.screen.y - y) / GISCREENZOOM;
	GiCadWindows.glsl_main.UpdateMouseSelect(GiCadWindows.mouse[0].cur, GiCadWindows.mouse[0].hold, GiCadWindows.mouse[0].down);

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

	if(GiCadWindows.glsl_menu.IsActive())
		GiCadWindows.glsl_menu.UpdateMouse(KiInt2(x, GiCadWindows.screen.y - y));

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
	KiVec2 move = (GiCadWindows.screen / GiCadWindows.zoom - GiCadWindows.screen / zoom) / 2.;
	//GiCadWindows.move = GiCadWindows.move + (GiCadWindows.size / GiCadWindows.zoom - GiCadWindows.size / zoom) / 2.;
	

	float center_x = GiCadWindows.screen.x / 2;
	float center_y = GiCadWindows.screen.y / 2;
	//GiCadWindows.move = KiInt2(GiCadWindows.move.x * GiCadWindows.zoom / zoom, GiCadWindows.move.y * GiCadWindows.zoom / zoom);
	
	
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);
	//GiCadWindows.move -= KiInt2(xpos * yoffset, 0);
	//GiCadWindows.move += KiInt2((xpos - center_x) * GiCadWindows.zoom / zoom, (ypos - center_y) * GiCadWindows.zoom / zoom);

	//GiCadWindows.zoom = zoom;

	GiCadWindows.SetZoom(zoom);


	//glfwSetCursorPos(window, GiCadWindows.size.x / 2, GiCadWindows.size.y / 2);
//	GiCadWindows.mouse_move = KiInt2(GiCadWindows.size.x / 2, GiCadWindows.size.y / 2);
}

void GiWndDropMessage(VString path, int ok){
	ILink link(path);

	if(path.size() > 20){
		//path = link.file;
		path = path.str(-20);
	}

	if(ok)
		GiCadWindows.InsertPopUp(LString() + "Loaded success: " + path);
	else
		GiCadWindows.InsertPopUp(LString() + "[WARNING!] Loading fail: " + path);
}

void GiWndDrop(GLFWwindow* window, int count, const char** paths){
    for (int i = 0; i < count; i++){
		ILink link(paths[i]);
		VString file(paths[i]);

		if(file.str(-3) == "gbr")
			GiWndDropMessage(paths[i], GiProject.AddGbrFile(paths[i]));

		else if(file.str(-3) == "drl")
			GiWndDropMessage(paths[i], GiProject.AddDrlFile(paths[i]));

		else
			GiWndDropMessage(paths[i], 0);
	}

	GiProject.OnDrop();
}

void GiWndClose(GLFWwindow* window){
	GiCadWindows.SaveConfig();
}

// Windows call
void GiWindowsUpdateTitle(){
	glfwSetWindowTitle(GiCadWindows.window, VString(LString() + GiProject.GetProjectName() + " - " + PROJECTNAME + " " + PROJECTVER[0].ver + _msv_zero_str));
}

void GiWindowsLayerInsertItem(int id, int active, MRGB color, VString Text){
		
	//GiCadWindows.glsl_gui.InsertList
}

void GiWindowsClose() {
	GiCadWindows.close_window = 1;
}

void GuiMenuRender() {
	if (ImGui::BeginMainMenuBar()) {
		ImGui::SetWindowFontScale(GIGUI_GLOBAL_SCALE);

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Projects", "Ctrl+N"))
				GiProject.NewProject();

			if (ImGui::MenuItem("Open Project", "Ctrl+O"))
				GiProject.OpenProject();

			if (ImGui::MenuItem("Save project", "Ctrl+S")){}
			if (ImGui::MenuItem("Save project as", "Ctrl+Shift+S")) {}

			if (ImGui::MenuItem("Exit", ""))
				GiWindowsClose();

			ImGui::EndMenu();
		}
		
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

		// View
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Reset view", "Ctrl+0")){
				GiCadWindows.ResetView();				
			}
			if (ImGui::MenuItem("100%", "Ctrl+1")){
				GiCadWindows.ResetView100p();
			}
			ImGui::EndMenu();
		}

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

		// Program
		if (ImGui::BeginMenu("Prog")) {
			if (ImGui::MenuItem("Drill", "")) {
				GiProject.SetProg(GiProjectProgDrill);
			}
			ImGui::EndMenu();
		}

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
	GiProject.GuiTreeRender();
	GiProject.GuiProg();
}

void GuiRender() {
	GuiMenuRender();
	GuiLayersRender();

	// Menu
	//ImGui::OpenPopup("mypicker");
	//if (ImGui::BeginPopup("mypicker"))
	//{
	//	ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
	//	ImGui::Separator();

	//	ImGui::EndPopup();
	//}
}


void GiWindowsResetView100p() {
	GiCadWindows.ResetView100p();
}