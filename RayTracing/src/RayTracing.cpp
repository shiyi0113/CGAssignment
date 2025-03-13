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

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))  // 若相机移动则重新积累
			m_Renderer.ResetFrameIndex();   
	}
	virtual void OnUIRender() override
	{
		// 窗口1:Settings
		ImGui::Begin("Settings");
		ImGui::Text("Last render:%.3fms", m_LastRenderTime);
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
			Render();
		}
		ImGui::Separator();
		if (ImGui::Button("ReadFile"))
		{
			ReadFile();
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

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene,m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
	void ReadFile() {
		g_SceneFolderPath = OpenFolderDialog();
		if (g_SceneFolderPath.empty())
			return;
		std::string Flodername = std::filesystem::path(g_SceneFolderPath).filename().string();
		m_Reader.LoadXml(g_SceneFolderPath + "/" + Flodername + ".xml");
		m_Camera.SetPosition(glm::vec3(m_Reader.getCamera().eye.x, m_Reader.getCamera().eye.y, m_Reader.getCamera().eye.z));
		m_Camera.SetDirection(glm::vec3(m_Reader.getCamera().lookat.x, m_Reader.getCamera().lookat.y, m_Reader.getCamera().lookat.z));
		m_Camera.SetUpDirection(glm::vec3(m_Reader.getCamera().up.x, m_Reader.getCamera().up.y, m_Reader.getCamera().up.z));
		m_Reader.LoadModel(m_Scene, g_SceneFolderPath + "/" + Flodername + ".obj");
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