#include "glew\glew.h"
#include "imgui\imgui.h"

#include "imgui\imgui_impl_glfw_gl3.h"
#include "Environment.h"
#include "glfw\glfw3.h"

#include <iostream>


using namespace std;

GLFWwindow* window;

Environment* Base;

int InitializeWindow() {

	if (!glfwInit()) return -1;
	window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
	if (!window)	 return -2;
	glfwMakeContextCurrent(window);
	glewInit();

	const char* fontPath = "DroidSans.ttf";
	ImGui::GetIO().Fonts->AddFontFromFileTTF(fontPath, 15.f);

	if (ImGui_ImplGlfwGL3_Init(window, false) == false) {
	
		return 254;
			
	}

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = style.GrabRounding = style.ScrollbarRounding = 2.0f;
	style.FramePadding = ImVec2(4, 2);
	style.DisplayWindowPadding = ImVec2(0, 0);
	style.DisplaySafeAreaPadding = ImVec2(0, 0);

	g_debugDraw.Create();

	uint32 flags = 0;
	flags += true * b2Draw::e_shapeBit;
	flags += false * b2Draw::e_jointBit;
	flags += false * b2Draw::e_aabbBit;
	flags += false * b2Draw::e_centerOfMassBit;
	g_debugDraw.SetFlags(flags);

	return 0;
}

static void sMouseButton(GLFWwindow* window1, int32 button, int32 action, int32 mods) {


	Base->Doo();

}

int main(void) {

	InitializeWindow();

	glfwSetMouseButtonCallback(window, sMouseButton);

	Base = new Environment();
	srand(time(NULL));

	glfwSwapInterval(1);

	double time1 = glfwGetTime();
	double frameTime = 0.0;
	
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	
	while (!glfwWindowShouldClose(window)) {

		glfwGetWindowSize(window, &g_camera.m_width, &g_camera.m_height);

		int bufferWidth, bufferHeight;
		glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
		glViewport(0, 0, bufferWidth, bufferHeight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplGlfwGL3_NewFrame();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2((float)g_camera.m_width, (float)g_camera.m_height));
		ImGui::Begin("Overlay", NULL, ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
		ImGui::SetCursorPos(ImVec2(5, (float)g_camera.m_height - 20));
		ImGui::Text("%.1f ms", 1000.0 * frameTime);
		
		ImGui::End();
		

		glEnable(GL_DEPTH_TEST);
		Base->Step();
		glDisable(GL_DEPTH_TEST);

		double time2 = glfwGetTime();
		double alpha = 0.9f;
		frameTime = alpha * frameTime + (1.0 - alpha) * (time2 - time1);
		time1 = time2;

		ImGui::Render();

		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	
	g_debugDraw.Destroy();

	glfwTerminate();
	return 0;
}