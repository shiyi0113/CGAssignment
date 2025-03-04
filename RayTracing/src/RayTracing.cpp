#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "thread_pool.h"
#include <iostream>

class SimpleTask :public Task {
public:
	void run()override {
		std::cout << "Hello World!" << std::endl;
	}
};

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		// 窗口1:Settings
		ImGui::Begin("Settings");
		if (ImGui::Button("Task_test"))
		{
			ThreadPool thread_pool;
			thread_pool.addTask(new SimpleTask());
		}
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::Text("Last render:%.3fms", m_LastRenderTime);
		ImGui::End();
		// 窗口2:Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));// 去掉边框间距
		ImGui::Begin("Viewport");
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;
		if (m_Image)
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(),(float)m_Image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
		ImGui::PopStyleVar();        // 恢复原来的窗口填充设置
	}
	void Render() {
		Walnut::Timer timer;
		// Resize
		if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight()) 
		{
			m_Image = std::make_shared<Walnut::Image>(m_ViewportWidth, m_ViewportHeight, Walnut::ImageFormat::RGBA);
			delete[] m_ImageData;
			m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
		}
		// 绘制颜色
		for (uint32_t i = 0;i < m_ViewportWidth * m_ViewportHeight;i++)
		{
			m_ImageData[i] = 0xffff00ff;
		}
		m_Image->SetData(m_ImageData);
		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	float m_LastRenderTime = 0.0f;   // 记录渲染时间
	std::shared_ptr<Walnut::Image> m_Image;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth, m_ViewportHeight;
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