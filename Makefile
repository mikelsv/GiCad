PROJECTNAME=GiCad
ADDLIB= -lpthread -lglfw -lGL -ldl -lGL -lGLU
ADDFLAGS= -I ../msvcore2/include -I imgui
FILES= glad tinyfiledialogs ../imgui/imgui ../imgui/imgui_draw ../imgui/backends/imgui_impl_glfw ../imgui/backends/imgui_impl_opengl3 ../imgui/imgui_tables ../imgui/imgui_widgets

include ../msvcore2/make/msvmake-2c