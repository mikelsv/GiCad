#define PROJECTNAME "GiCad"
#define PROJECTVER PROJECTNAME ## _versions
#define USEMSV_MSVCORE

#define USEMSV_MLIST
#define USEMSV_MWND
//#define USEMSV_MWND_OPENGL

#define USEMSV_XDATACONT
#define USEMSV_HTTP
#define USEMSV_CONSOLE

#include "../msvcore2/msvcore.cpp"

Versions PROJECTVER[] = {
    // new version to up
	"0.0.1.1", "04.04.2024 22:27",
	"0.0.1.0", "26.03.2024 19:14",
	"0.0.0.9", "25.03.2024 14:58",
	"0.0.0.8", "21.03.2024 11:22",
	"0.0.0.7", "20.03.2024 11:57",
	"0.0.0.6", "09.03.2024 16:07",
	"0.0.0.5", "07.03.2024 09:05",
	"0.0.0.4", "06.03.2024 10:49",
	"0.0.0.3", "05.03.2024 10:30",
	"0.0.0.2", "30.02.2024 08:01",
    "0.0.0.1", "26.02.2024 08:51" // (Moscow time)
};

char _msv_zero_code = '\0';
VString _msv_zero_str(&_msv_zero_code, 1);

// GL3W
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")

// Imgui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// tinyfiledialogs
#include "tinyfiledialogs.h"

// OpenGL
#include "../msvcore2/opengl/mgl.h"

// Stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Base
#include "Base.h"

// Gerber
#include "Gerber.h"

// Windows
#include "Glsl.h"
#include "Gui.h"
//#include "Graphics.h"
#include "GiLayer.h"
#include "GiPath.h"
#include "GiProject.h"
#include "GiWindows.h"

/* Structure:
Windows -> Project ->
	Grbl
	Drill
	Layers
*/

int main(int args, char* arg[], char* env[]){
	// Init
	print(PROJECTNAME, " v.", PROJECTVER[0].ver, " (", PROJECTVER[0].date, ").\r\n");
	//msvcoremain(__argc, __argv, _environ);
	msvcoremain(args, arg, env);

	// Glfw
	glfwInit();

	glfwWindowHint ( GLFW_RESIZABLE, 1 );
 //   glfwWindowHint ( GLFW_DOUBLEBUFFER, 1 );
    //glfwWindowHint ( GLFW_DEPTH_BITS, 0 );
    //glfwWindowHint ( GLFW_CLIENT_API, GLFW_OPENGL_API );
    //glfwWindowHint ( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    //glfwWindowHint ( GLFW_CONTEXT_VERSION_MINOR, 4 );
 //   glfwWindowHint ( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
 //   glfwWindowHint ( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	//glDisable(GL_DEPTH_TEST);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Window
	GLFWwindow* window;	
	window = glfwCreateWindow((GLsizei)GiCadWindows.screen.x, (GLsizei)GiCadWindows.screen.y, VString(LString() + PROJECTNAME + " " + PROJECTVER[0].ver + _msv_zero_str), 0, NULL); // 
	
	// Position
	if(GiCadWindows.pos.x >= 0){
		glfwSetWindowPos(window, GiCadWindows.pos.x, GiCadWindows.pos.y);
	}

	if(GiCadWindows.maximized)
		glfwMaximizeWindow(window);

	glfwMakeContextCurrent(window);

	gladLoadGL();
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// OpenGL state 
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glLineWidth(2.0);

	// Debug
	MsvGlDebug(1, 0);

	// Init windows
	GiCadWindows.Init(window);
	GiWindowsUpdateTitle();
	GiWndResize(window, (GLsizei)GiCadWindows.screen.x, (GLsizei)GiCadWindows.screen.y);

	// Init Tools
	GiTools.Init();

	// Imit Images
	GiImages.Init();

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& imgui = ImGui::GetIO(); (void)imgui;
	ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	// Design
	GiCadWindows.InsertPopUp(LString() + "Wellcome to GiCad " + PROJECTVER[0].ver + " !");
	
	// Extender options
	srand(time());

	// Callbacks
	glfwSetWindowSizeCallback(window, GiWndResize);
	//glfwSetKeyCallback(window, GiWndKeyCallback);
	glfwSetMouseButtonCallback(window, GiWndMouseClickCallback);
	glfwSetCursorPosCallback(window, GiWndMouseMotionCallback);
	glfwSetScrollCallback(window, GiWndMouseScrollCallback);
	glfwSetDropCallback(window, GiWndDrop);
	glfwSetWindowCloseCallback(window, GiWndClose);

	// Variables to be changed in the ImGUI window
	bool drawTriangle = true;
	float size = 1.0f;
	float color[4] = { 0.8f, 0.3f, 0.02f, 1.0f };

	// Process
	while (!glfwWindowShouldClose(window)) {
		//double delta = glfwGetTime();
		GiWndUpdate(window, 0);
//		MaticalsOpenGl.UpdateTime(delta);
		GiWndRenderScene(window, 0);


		// Tell OpenGL a new frame is about to begin
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		GuiRender();

		// Renders the ImGUI elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glsl.Render(draw_text.);

		/*
		proj.Render(delta);
		//dfont.Draw();

		if(MaticalsOpenGl.IsUpdateTitle()){
			glfwSetWindowTitle(window, MaticalsOpenGl.GetWindowTitle());
		}

		// Render
		if(MaticalsOpenGl.IsRender()){
			MaticalsOpenGl.time_now = 1. * MaticalsOpenGl.frame_id / MaticalsOpenGl.opt_fps;
			MaticalsOpenGl.time_delta = 1. / MaticalsOpenGl.opt_fps;

			//MaticalsOpenGlRender.Write(MaticalsOpenGl.frame_id, proj.GetScriptTime() * MaticalsOpenGl.opt_fps);

			//MaticalsOpenGl.frame_id ++;

			if(MaticalsOpenGl.frame_id >= proj.GetScriptTime() * MaticalsOpenGl.opt_fps)
				break;
		}*/

		//glfwSetTime(0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	glfwDestroyWindow(window);

	return 0;
}


