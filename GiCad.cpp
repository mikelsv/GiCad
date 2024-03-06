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
	"0.0.0.4", "06.03.2024 10:49",
	"0.0.0.3", "05.03.2024 10:30",
	"0.0.0.2", "30.02.2024 08:01",
    "0.0.0.1", "26.02.2024 08:51" // (Moscow time)
};

// GL3W
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")

// OpenGL
#include "../msvcore2/opengl/mgl.h"

// Gerber
#include "Gerber.h"

// Windows
#include "Glsl.h"
#include "GiLayer.h"
#include "GiProject.h"
#include "GiCadWindows.h"

/* Structure:
Windows -> Project ->
	Grbl
	Drill
	Layers
*/

int main(int args, char* arg[], char* env[]){
	// Init
	print(PROJECTNAME, " v.", PROJECTVER[0].ver, " (", PROJECTVER[0].date, ").\r\n");
	msvcoremain(__argc, __argv, _environ);

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

	// Window
	GLFWwindow* window;
	window = glfwCreateWindow(GiCadWindows.size.x, GiCadWindows.size.y, MString(LString() + "GiCad " + PROJECTVER[0].ver), 0, NULL); // 

	glfwMakeContextCurrent(window);

	gladLoadGL();
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// OpenGL state 
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Debug
	MsvGlDebug(1, 0);

	// Init windows
	GiCadWindows.Init();
	GiWndResize(window, GiCadWindows.size.x, GiCadWindows.size.y);
	
	// Extender options
	srand(time());

	// Callbacks
	glfwSetWindowSizeCallback(window, GiWndResize);
	glfwSetKeyCallback(window, GiWndKeyCallback);
	glfwSetMouseButtonCallback(window, GiWndMouseClickCallback);
	glfwSetCursorPosCallback(window, GiWndMouseMotionCallback);
	glfwSetScrollCallback(window, GiWndMouseScrollCallback);
	glfwSetDropCallback(window, GiWndDrop);

	// Process
	while (!glfwWindowShouldClose(window)) {
		//double delta = glfwGetTime();
		GiWndUpdate(window, 0);
//		MaticalsOpenGl.UpdateTime(delta);
		GiWndRenderScene(window, 0);

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
	
	glfwDestroyWindow(window);

	GiCadWindows.SaveConfig();

	return 0;
}


