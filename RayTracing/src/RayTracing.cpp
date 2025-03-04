#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"

#include <iostream>

class ExampleLayer : public Walnut::Layer
{
public:

	virtual void OnUIRender() override
	{
		// ����1:Settings
		ImGui::Begin("Settings");
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::Text("Last render:%.3fms", m_LastRenderTime);
		ImGui::End();

		// ����2:Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));// ȥ���߿���
		ImGui::Begin("Viewport");
		m_ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;
		auto m_Image = m_Renderer.GetFinalImage();
		if (m_Image)
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(),(float)m_Image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
		ImGui::PopStyleVar();        // �ָ�ԭ���Ĵ����������
	}
	void Render() {
		Walnut::Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	float m_LastRenderTime = 0.0f;   // ��¼��Ⱦʱ��

	uint32_t m_ViewportWidth, m_ViewportHeight;
	Renderer m_Renderer;
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