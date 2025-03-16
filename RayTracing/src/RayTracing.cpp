#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"
#include "tinyfiledialogs.h"
#include "ReadFiles.h"

#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		:m_Camera(39.3077f, 0.1f, 100.0f)
	{
	}
	virtual void OnUIRender() override
	{
		// 窗口1:Settings
		ImGui::Begin("Settings");
		ImGui::Text("Last render:%.3fms", m_LastRenderTime);
		ImGui::Separator();
		if (ImGui::Button("ReadFile"))
		{
			ReadFile();
		}
		ImGui::Text("Current Folder: %s", g_SceneFolderPath.empty() ? "Unspecified" : g_SceneFolderPath.c_str());
		ImGui::Separator();
		ImGui::Text("Render Setting");
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSetting().Accumulate);
		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}
		ImGui::Separator();
		if (ImGui::Button("Render"))
		{
			// Render();
			if (!g_SceneFolderPath.empty())
				start = true;
		}
		if (start)
		{
			Render();
		}
		ImGui::End();

		// 窗口2:Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));// 去掉边框间距
		ImGui::Begin("Viewport");
		m_ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;
		auto m_Image = m_Renderer.GetFinalImage();
		if (m_Image)
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(),(float)m_Image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
		ImGui::PopStyleVar();        // 恢复原来的窗口填充设置
	}
	void Render() {
		Walnut::Timer timer;

		m_Renderer.OnResize(1024, 1024);
		m_Camera.OnResize(1024, 1024);
		m_Renderer.Render(m_Scene,m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
	void ReadFile() {
		g_SceneFolderPath = OpenFolderDialog();
		if (g_SceneFolderPath.empty())
			return;
		std::string Flodername = std::filesystem::path(g_SceneFolderPath).filename().string();
		m_Reader.LoadXml(g_SceneFolderPath + "/" + Flodername + ".xml");
		glm::vec3 eye = glm::vec3(m_Reader.getCamera().eye.x, m_Reader.getCamera().eye.y, m_Reader.getCamera().eye.z);
		glm::vec3 lookat = glm::vec3(m_Reader.getCamera().lookat.x, m_Reader.getCamera().lookat.y, m_Reader.getCamera().lookat.z);
		m_Camera.SetPosition(eye);
		m_Camera.SetDirection(glm::normalize(lookat - eye));
		m_Camera.SetUpDirection(glm::vec3(m_Reader.getCamera().up.x, m_Reader.getCamera().up.y, m_Reader.getCamera().up.z));
		// 打印相机参数
		std::cout << "Camera Position: (" << m_Camera.GetPosition().x << ", " << m_Camera.GetPosition().y << ", " << m_Camera.GetPosition().z << ")\n";
		std::cout << "Camera LookAt: (" << m_Camera.GetDirection().x << ", " << m_Camera.GetDirection().y << ", " << m_Camera.GetDirection().z << ")\n";
		std::cout << "Camera UpDirection: (" << m_Camera.GetUpDirection().x << ", " << m_Camera.GetUpDirection().y << ", " << m_Camera.GetUpDirection().z << ")\n";

		m_Reader.LoadModel(m_Scene, g_SceneFolderPath + "/" + Flodername + ".obj");
		// 打印场景的网格、材质和三角形数
		std::cout << "Scene has " << m_Scene.Meshes.size() << " meshes and " << m_Scene.Materials.size() << " materials.\n";
		for (const auto& mesh : m_Scene.Meshes) {
			std::cout << "Mesh has " << mesh.Triangles.size() << " triangles.\n";
			std::cout << "Material Albedo:" << m_Scene.Materials[mesh.MaterialIndex].Albedo.x << "," << m_Scene.Materials[mesh.MaterialIndex].Albedo.y << "," << m_Scene.Materials[mesh.MaterialIndex].Albedo.z <<std::endl;
			std::cout << "Material Emission:" << m_Scene.Materials[mesh.MaterialIndex].GetEmission().x << "," << m_Scene.Materials[mesh.MaterialIndex].GetEmission().y << "," << m_Scene.Materials[mesh.MaterialIndex].GetEmission().z << std::endl;
			std::cout << "Material Metallic:" << m_Scene.Materials[mesh.MaterialIndex].Metallic << std::endl;
			std::cout << "Material Roughness:" << m_Scene.Materials[mesh.MaterialIndex].Roughness << std::endl;
		}
		m_Renderer.ResetFrameIndex();
	}

	std::string OpenFolderDialog() {
		const char* path = tinyfd_selectFolderDialog("选择场景文件夹", nullptr);
		return (path != nullptr) ? std::string(path) : "";
	}

private:
	float m_LastRenderTime = 0.0f;   // 记录渲染时间

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	ReadFiles m_Reader;
	std::string g_SceneFolderPath;
	bool start = false;
};


Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}