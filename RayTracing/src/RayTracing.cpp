#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		:m_Camera(39.3077f, 0.1f, 100.0f)
	{
		Sphere sphere;
		
		sphere.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		sphere.Radius = 1.0f;
		sphere.MaterialIndex = 0;
		m_Scene.Spheres.push_back(sphere);

		sphere.Position = glm::vec3(0.0f, -101.0f, 0.0f);
		sphere.Radius = 100.0f;
		sphere.MaterialIndex = 1;
		m_Scene.Spheres.push_back(sphere);

		sphere.Position = glm::vec3(0.0f, 3.0f, 0.0f);
		sphere.Radius = 1.0f;
		sphere.MaterialIndex = 2;
		m_Scene.Spheres.push_back(sphere);

		Material material;

		material.Albedo = glm::vec3(0.8f, 0.8f, 0.8f);
		material.Roughness = 1.0f;
		material.Metallic = 0.0f;
		m_Scene.Materials.push_back(material);

		material.Albedo = glm::vec3(1.0f, 0.0f, 0.8f);
		material.Roughness = 1.0f;
		material.Metallic = 0.0f;
		m_Scene.Materials.push_back(material);

		material.Albedo = glm::vec3(0.6f, 0.7f, 0.2f);
		material.Roughness = 1.0f;
		material.Metallic = 0.0f;
		material.EmmisionPower = 0.5f;
		material.EmmisionColor = material.Albedo;
		m_Scene.Materials.push_back(material);
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
		ImGui::Separator();
		for (size_t i = 0;i < m_Scene.Spheres.size();i++)
		{
			ImGui::PushID(i);
			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::Text("Sphere %d", i);
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f, 0.0f, 60.0f);
			ImGui::DragInt("MaterialIndex", &sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);
			ImGui::Separator();
			ImGui::PopID();
		}
		for (size_t i = 0;i < m_Scene.Materials.size();i++)
		{
			ImGui::PushID(i);
			Material& material = m_Scene.Materials[i];
			ImGui::Text("Material %d", i);

			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &material.Metallic, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("EmmisionPower", &material.EmmisionPower, 0.1f, 0.0f, 100.0f);
			ImGui::ColorEdit3("EmmisionColor", glm::value_ptr(material.EmmisionColor));
			ImGui::Separator();
			ImGui::PopID();
		}
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