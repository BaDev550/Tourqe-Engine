#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"

#include <stdio.h>
#include <filesystem>
#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Logger/Logger.h"

#include "Model/Model.h"
#include "Frame Buffer/FBO.h"

void createIMGUI(GLFWwindow* window);

bool showFileDialog = false;
bool isTextureLoaded = false;
std::string filePath;
std::string fileExtension = "png";
std::string fileName;
std::string fileFullName = "map";

int main() {
	if (!glfwInit()) { return -1; std::cout << "[ERROR]Error while trying to Initialize the libary\n"; }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (!monitor) {
		std::cout << "[ERROR]Failed to get primary monitor" << std::endl;
		glfwTerminate();
		return -1;
	}

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (!mode) {
		std::cout << "[ERROR]Failed to get video mode" << std::endl;
		glfwTerminate();
		return -1;
	}

	GLsizei WIN_WIDTH  = mode->width;
	GLsizei WIN_HEIGHT = mode->height;

	int windowWidth = WIN_WIDTH;
	int windowHeight = static_cast<int>(WIN_WIDTH * 9.0 / 16.0);
	std::string parentDir = (std::filesystem::current_path()).string();
	std::string modelPath = "[ENTER THE MODEL PATH]";

	if (windowHeight > WIN_HEIGHT) {
		windowHeight = WIN_HEIGHT;
		windowWidth = static_cast<int>(WIN_HEIGHT * 16.0 / 9.0);
	}

	glfwSwapInterval(1);
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Vort Grapichs Engine", monitor, NULL);
	glfwMakeContextCurrent(window);

	gladLoadGL();

	Shader shaderProgram("Shaders/default.vert", "Shaders/default.frag");

	FBO fbo(WIN_WIDTH, WIN_HEIGHT);

	Shader lightShader("Shaders/light.vert", "Shaders/light.frag");

	float lightColorValue[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float lightTransform[3]  = { 0.0f, 10.0f, 0.0f };
	float lightRotation[3]   = { 0.0f, 0.0f, 0.0f };

	glm::vec4 lightColor = glm::make_vec4(lightColorValue);
	glm::vec3 lightRot = glm::make_vec3(lightRotation);
	glm::vec3 lightPos = glm::make_vec3(lightTransform);
	glm::mat4 lightModel = glm::mat4(1.0f);

	lightModel = glm::translate(lightModel, lightPos);
	Model light((parentDir + "/Resource/models/light/light.gltf").c_str());

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	bool applyLight = true;
	bool lightRendered = true;
	bool isModelLoaded = false;
	bool canSelectModel = true;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	Camera camera(WIN_WIDTH, WIN_HEIGHT, glm::vec3(0.0f, 0.5f, 2.0f));

	std::vector<std::pair<std::string, LogLevel>> log;
	CoutRedirector coutRedirector(log);
	std::streambuf* originalCoutBuffer = std::cout.rdbuf(&coutRedirector);

	float ModelTransform[3] = { 5.0f, 0.0f, 0.0f };
	float ModelRotation[3] = { 0.0f, 0.0f, 0.0f };

	glm::vec3 ModelPos = glm::make_vec3(ModelTransform);
	glm::vec3 ModelRot = glm::make_vec3(ModelRotation);
	std::cout << parentDir + modelPath << std::endl;
	Model model((parentDir + modelPath).c_str());

	createIMGUI(window);
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "[ERROR]OpenGL error: " << err << std::endl;
	}

	std::cout << "Application started." << std::endl;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		fbo.Bind();

		glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.Inputs(window);
		camera.updateMatrix(45.0f, 0.1f, 100.0f);


		glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui::SetNextWindowSize(ImVec2(WIN_WIDTH, WIN_HEIGHT));
		ImGui::Begin("BaDev Grapichs Engine | Open Source", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse);
		{
			if (ImGui::Button("Close Application", ImVec2(150, 20)))
			{
				ImGui_ImplOpenGL3_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
				shaderProgram.Delete();

				glfwDestroyWindow(window);
				glfwTerminate();
				std::cout << "Application is succsessfully closed with return of 0";
				return 0;
			}

			ImGui::BeginChild("Debugger", ImVec2(WIN_WIDTH / 3, WIN_HEIGHT), true);
				ImGui::Text("Debugger");
				if (ImGui::IsWindowFocused()) {
					camera.inputEnabled = false;
				}
				if (ImGui::CollapsingHeader("Object")) {
					ImGui::BeginDisabled(!canSelectModel);
					if (ImGui::Button("Select Model")) {
						showFileDialog = true;
						ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".gltf");
					}

					if (showFileDialog) {
						ImGui::OpenPopup("ChooseFileDlgKey");

						if (ImGui::BeginPopupModal("ChooseFileDlgKey", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
							ImGuiStyle& style = ImGui::GetStyle();
							ImVec4 originalColor = style.Colors[ImGuiCol_WindowBg];
							style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);

							ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey");

							if (ImGuiFileDialog::Instance()->IsOk() && !isModelLoaded) {
								filePath = ImGuiFileDialog::Instance()->GetFilePathName();

								size_t lastSlashIndex = filePath.find_last_of("/\\");
								if (lastSlashIndex != std::string::npos) {
									std::string fileName = filePath.substr(lastSlashIndex + 1);

									size_t dotIndex = fileName.find_last_of('.');
									if (dotIndex != std::string::npos) {
										fileFullName = fileName.substr(0, dotIndex);
									}
									else {
										fileFullName = fileName;
									}
								}
								else {
									fileFullName = filePath;
								}

								modelPath = (parentDir + "Resource/models/" + fileFullName + "/" + fileFullName + ".gltf");
								model = ((modelPath).c_str());
								model.Draw(shaderProgram, camera, ModelPos, ModelRot);
								std::cout << model.loadedTexName[0] << std::endl;

								canSelectModel = false;
								isModelLoaded = true;

								ImGuiFileDialog::Instance()->Close();
								showFileDialog = false;
							}
							else if (ImGuiFileDialog::Instance()->IsOpened("ChooseFileDlgKey") && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
								ImGuiFileDialog::Instance()->Close();
								showFileDialog = false;
							}

							ImGui::EndPopup();
							style.Colors[ImGuiCol_WindowBg] = originalColor;
						}
					}
					ImGui::EndDisabled();

					ImGui::SliderFloat3("Object Position", ModelTransform, -10, 10);
					ModelPos = glm::make_vec3(ModelTransform);
					ImGui::SliderFloat3("Object Rotation", ModelRotation, -360, 360);
					ModelRot = glm::make_vec3(ModelRotation);

					if (showFileDialog) { camera.inputEnabled = false; }
					else { camera.inputEnabled = true; }

					if (ImGui::Button("Reset Model")) {
						isModelLoaded = false;
						std::cout << "Model Reseted\n";
						canSelectModel = true;
					}
				}
				if (ImGui::CollapsingHeader("Lighting")) {
					ImGui::Checkbox("Render Light", &applyLight);
					ImGui::ColorPicker4("Light Color", lightColorValue);
					ImGui::SliderFloat3("Light Position", lightTransform, -10, 10);
					ImGui::SliderFloat3("Light Rotation", lightRotation, -10, 10);

					applyLight == true ? lightColor = glm::vec4(lightColorValue[0], lightColorValue[1], lightColorValue[2], lightColorValue[3]) : lightColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
					lightRot = glm::make_vec3(lightRotation);
					lightPos = glm::make_vec3(lightTransform);

					lightShader.Activate();
					glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
					glUniform3f(glGetUniformLocation(lightShader.ID, "aPosM"), lightPos.x, lightPos.y, lightPos.z);
					shaderProgram.Activate();
					glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
					glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
				}
				
				ImGui::SliderFloat("Camera Speed", &camera.speed, 0.05f, 0.5f);
				ImGui::SliderFloat("Camera Sens", &camera.sensitivity, 1.f, 100.f);
							
			ImGui::EndChild();
			ImGui::SameLine();
			
			ImGui::BeginChild("Viewport", ImVec2(820, 680), true);
				if (ImGui::IsWindowFocused()) {
					ImGui::Text("Viewport - Focused");
					camera.inputEnabled = true;
				}
				else {
					ImGui::Text("Viewport - Not Focused");
					camera.inputEnabled = false;
				}
				ImGui::Image((void*)(intptr_t)fbo.GetTexture(), ImVec2(800, 640), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::BeginChild("Logs", ImVec2(WIN_WIDTH / 2, WIN_HEIGHT), true);
			{
				ImGui::Text("Logs");
				if(ImGui::Button("Clear"))
				{
					log.clear();
				}
				RenderLogWindow(log);
			}
			ImGui::EndChild();
		}
		ImGui::End();

		model.Draw(shaderProgram, camera, ModelPos, ModelRot);
		light.Draw(shaderProgram, camera, lightPos, lightRot);

		fbo.Unbind();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	shaderProgram.Delete();
	lightShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void createIMGUI(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}
