#pragma once

#include <Atlas.h>

class Sandbox2D : public Atlas::Layer
{
private:
	Atlas::PerspectiveCameraController m_CameraController;
	float m_Fov = 90;
	bool m_Shading = true;

	Atlas::Ref<Atlas::Texture2D> m_Texture1;
	Atlas::Ref<Atlas::Texture2D> m_Texture2;
	Atlas::Ref<Atlas::FrameBuffer> m_FrameBuffer;
	uint32_t m_ColorAttachment = 0;
	uint32_t m_DepthAttachment = 0;
	glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };

	Atlas::Ref<Atlas::Scene> m_Scene;

public:
	Sandbox2D();
	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Atlas::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Atlas::Event& e) override;

};
