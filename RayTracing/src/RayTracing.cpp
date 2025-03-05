#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"

#include <iostream>

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		:m_Camera(45.0f, 0.1f, 100.0f)
	{
		Sphere sphere;
		
		sphere.Position = glm::vec3(5.0f, 0.0f, 0.0f);
		sphere.Radius = 1.0f;
		sphere.Color = glm::vec3(1.0f, 0.0f, 0.0f);
		m_Scene.Spheres.push_back(sphere);

		sphere.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		sphere.Radius = 1.0f;
		sphere.Color = glm::vec3(2.0f, 1.0f, 0.0f);
		m_Scene.Spheres.push_back(sphere);
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}
	virtual void OnUIRender() override
	{
		// 窗口1:Settings
		ImGui::Begin("Settings");
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::Text("Last render:%.3fms", m_LastRenderTime);
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

		Render();
	}
	void Render() {
		Walnut::Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene,m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	float m_LastRenderTime = 0.0f;   // 记录渲染时间

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
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