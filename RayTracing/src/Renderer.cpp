#include "Renderer.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage) {
		// 长宽一样时无需改变
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
}
