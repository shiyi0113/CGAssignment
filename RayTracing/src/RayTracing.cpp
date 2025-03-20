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
		// ����1:Settings
		ImGui::Begin("Settings");
		if (ImGui::Button("ReadFile"))
		{
			ReadFile();
		}
		ImGui::Text("Current Scene: %s", SceneName);
		ImGui::Separator();
		ImGui::Text("Render Setting");
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSetting().Accumulate);
		if (!start) 
		{
			if (ImGui::Button("Render"))
			{
				if (!g_SceneFolderPath.empty())
					start = true;
			}
		}
		else
		{
			if (ImGui::Button("Pause"))
			{
				start = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}
		if (start)
		{
			Render();
		}
		ImGui::Separator();
		ImGui::Text("Last render:%.3fms", m_LastRenderTime);
		ImGui::Text("Rendering Times: %d", m_Renderer.GetFrameIndex() - 1);
		ImGui::End();

		// ����2:Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));// ȥ���߿���
		ImGui::Begin("Viewport");
		auto m_Image = m_Renderer.GetFinalImage();
		if (m_Image)
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(),(float)m_Image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
		ImGui::PopStyleVar();        // �ָ�ԭ���Ĵ����������
	}
	void Render() {
		Walnut::Timer timer;

		m_Renderer.OnResize(m_Width, m_Height);
		m_Camera.OnResize(m_Width, m_Height);
		m_Camera.SetVerticalFOV(m_fov);
		m_Renderer.Render(m_Scene,m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
	void ReadFile() {
		g_SceneFolderPath = OpenFolderDialog();
		if (g_SceneFolderPath.empty())
			return;
		m_Reader.SetSceneFolderPath(g_SceneFolderPath);
		std::string Flodername = std::filesystem::path(g_SceneFolderPath).filename().string();
		// ���ԭ���ĳ���
		m_Scene.Clear();

		SceneName = Flodername;
		m_Reader.LoadXml(g_SceneFolderPath + "/" + Flodername + ".xml");

		glm::vec3 eye = glm::vec3(m_Reader.getCamera().eye.x, m_Reader.getCamera().eye.y, m_Reader.getCamera().eye.z);
		glm::vec3 lookat = glm::vec3(m_Reader.getCamera().lookat.x, m_Reader.getCamera().lookat.y, m_Reader.getCamera().lookat.z);
		glm::vec3 up = glm::vec3(m_Reader.getCamera().up.x, m_Reader.getCamera().up.y, m_Reader.getCamera().up.z);
		m_Width = (uint32_t)m_Reader.getCamera().width;
		m_Height = (uint32_t)m_Reader.getCamera().height;
		m_fov = m_Reader.getCamera().fovy;

		m_Camera.SetPosition(eye);
		m_Camera.SetDirection(glm::normalize(lookat - eye));
		m_Camera.SetUpDirection(up);
		m_Camera.SetVerticalFOV(m_fov);

		// ��ӡ�������
		std::cout << "Camera Position: (" << m_Camera.GetPosition().x << ", " << m_Camera.GetPosition().y << ", " << m_Camera.GetPosition().z << ")\n";
		std::cout << "Camera LookAt: (" << m_Camera.GetDirection().x << ", " << m_Camera.GetDirection().y << ", " << m_Camera.GetDirection().z << ")\n";
		std::cout << "Camera UpDirection: (" << m_Camera.GetUpDirection().x << ", " << m_Camera.GetUpDirection().y << ", " << m_Camera.GetUpDirection().z << ")\n";
		std::cout << "Camera VerticalFOV: " << m_Camera.GetVerticalFOV() << "\n";
		m_Reader.LoadModel(m_Scene, g_SceneFolderPath + "/" + Flodername + ".obj");
		// ��ӡ���������񡢲��ʺ���������
		std::cout << "Scene has " << m_Scene.Meshes.size() << " meshes and " << m_Scene.Materials.size() << " materials\n";
		m_Renderer.ResetFrameIndex();
	}

	std::string OpenFolderDialog() {
		const char* path = tinyfd_selectFolderDialog("ѡ�񳡾��ļ���", nullptr);
		return (path != nullptr) ? std::string(path) : "";
	}

private:
	float m_LastRenderTime = 0.0f;   // ��¼��Ⱦʱ��

	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
	float m_fov = 45.0f;

	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	ReadFiles m_Reader;
	std::string g_SceneFolderPath;
	bool start = false;
	std::string SceneName = "Unspecified";
};


Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracing";

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