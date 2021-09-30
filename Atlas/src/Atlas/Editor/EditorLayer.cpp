#include "atlpch.h"
#include "EditorLayer.h"

#include "Atlas/ImGui/AtlasTheme.h"
#include "Atlas/Core/Core.h"
#include "Atlas/Math/Math.h"

#include "imgui.h"

#include "Atlas/Renderer/Renderer3D.h"
#include "Atlas/Renderer/Renderer2D.h"
#include "Atlas/Renderer/RenderCommand.h"

#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <charconv>

//TODO: look at it
#include "Atlas/Core/Application.h"

namespace Atlas {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		ATL_PROFILE_FUNCTION();

		Atlas::SetAtlasTheme();

		m_ViewportFrameBuffer = FrameBuffer::Create({
				(uint32_t)m_ViewportSize.x,
				(uint32_t)m_ViewportSize.y,
				{
					FBTextureFormat::RGBA8,
					FBTextureFormat::RED_INTEGER,
					FBTextureFormat::DEPTH24STENCIL8
				}
			});

		m_PostProcessingFrameBuffer = FrameBuffer::Create({
				(uint32_t)m_ViewportSize.x,
				(uint32_t)m_ViewportSize.y,
				{
					FBTextureFormat::RGBA8
				}
			});

		m_DirLightFrameBuffer = FrameBuffer::Create({
				1024,
				1024,
				{
					FBTextureFormat::DEPTH24STENCIL8
				}
			});

		m_ActiveScene = CreateRef<Scene>();
		m_SceneHierarchy.SetContext(m_ActiveScene);

	}

	void EditorLayer::OnDetach()
	{
	}

	ImGuizmo::OPERATION AtlOpToImGuizmoOp(Utils::Transform transform)
	{
		switch (transform)
		{
		case Utils::Transform::TRANSLATE:
			return ImGuizmo::OPERATION::TRANSLATE;

		case Utils::Transform::ROTATE:
			return ImGuizmo::OPERATION::ROTATE;

		case Utils::Transform::SCALE:
			return ImGuizmo::OPERATION::SCALE;
		}

		ATL_CORE_ASSERT(false, "Unknown Tranform");
		return ImGuizmo::BOUNDS;
	}


	void EditorLayer::OnImGuiRender()
	{
		ATL_PROFILE_FUNCTION();


		ImGui::Begin("Viewport");
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
		m_ViewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		my = m_ViewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;


		//LIGHT
		m_DirLightFrameBuffer->Bind();
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		RenderCommand::Clear();
		for (DirLightComponent& light : m_ActiveScene->GetComponentGroup<DirLightComponent>())
		{
			glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
			glm::mat4 lightView = glm::lookAt(light.Direction, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;
			Renderer3D::DrawLightDepthMap(m_ActiveScene, lightSpaceMatrix);
		}
		m_DirLightFrameBuffer->Unbind();

		m_ViewportFrameBuffer->Bind();
		//TEMP
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();
		m_ViewportFrameBuffer->ClearAttachment(1, -1);

		Renderer3D::DrawScene(m_ActiveScene);
		if (m_SceneHierarchy.GetSelectedEntity() != ECS::null)
		{
			if (m_ActiveScene->HasComponent<MeshComponent>(m_SceneHierarchy.GetSelectedEntity()))
			{
				auto& mesh = m_ActiveScene->GetComponent<MeshComponent>(m_SceneHierarchy.GetSelectedEntity());
				Renderer3D::DrawOutline(mesh, m_ActiveScene->getCamera().GetViewProjectionMatrix(), {ATL_RED_COL.x, ATL_RED_COL.y, ATL_RED_COL.z, 1.0f}, m_OutlineThickness);
			}
		}

		//TODO: find a better fix
		if (m_ViewportSize.x > 100 && m_ViewportSize.y > 100)
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)m_ViewportSize.x && mouseY < (int)m_ViewportSize.y)
		{
			int value = m_ViewportFrameBuffer->ReadPixel(1, mouseX, mouseY);
			if (value >= 0) m_HoveredEntity = value;
			else if (value == -1) m_HoveredEntity = ECS::null;
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && ImGui::IsWindowHovered()) 
		{
			const Window::EventCallbackFn& callbackFn = Application::GetEventCallback();
			callbackFn(EntitySelectedEvent(m_HoveredEntity));
		}

		m_ViewportFrameBuffer->Unbind();

		m_PostProcessingFrameBuffer->Bind();
		Renderer2D::DrawFrameBuffer(m_ViewportFrameBuffer->GetColorAttachmentRendererID(0), m_ViewportSize.x, m_ViewportSize.y);
		m_PostProcessingFrameBuffer->Unbind();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::Image((void*)(size_t)m_PostProcessingFrameBuffer->GetColorAttachmentRendererID(0), { m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		ImGui::PushStyleColor(ImGuiCol_Text, { 0.6f, 0.6f, 0.6f, 1.0f });
		ImGui::SetCursorPos(cursorPos);
		if (m_HoveredEntity != ECS::null)
		{
			if (m_ActiveScene->HasComponent<TagComponent>(m_HoveredEntity))
			{
				std::string text = "Hovered: " + (std::string)m_ActiveScene->GetComponent<TagComponent>(m_HoveredEntity);
				ImGui::Text(text.c_str());
			}
		}
		else ImGui::Text("Hovered: ");

		ImGui::PopStyleColor();

		ImGui::PopStyleVar();

		ImGuiWindow* window = GImGui->CurrentWindow;
		ImGuizmo::SetRect(window->Pos.x, window->Pos.y, window->Size.x, window->Size.y);
		ImGuizmo::SetDrawlist(window->DrawList);

		auto& camera = m_ActiveScene->GetActiveCamera();

		if (m_SelectedEntity != ECS::null)
		{
			if (m_ActiveScene->HasComponent<TransformComponent>(m_SelectedEntity))
			{

				TransformComponent& component = m_ActiveScene->GetComponent<TransformComponent>(m_SelectedEntity);
				glm::mat4 transform = component.GetTransform();
				ImGuizmo::Manipulate(glm::value_ptr(camera.GetView()), glm::value_ptr(camera.GetProjection()), AtlOpToImGuizmoOp(component.TransformOperation), ImGuizmo::LOCAL, glm::value_ptr(transform));

				if (ImGuizmo::IsUsing()) Math::DecomposeTransform(transform, component.Translation, component.Rotation, component.Scale);

				if (ImGui::IsKeyPressed('1'))
				{
					component.TransformOperation = Utils::Transform::TRANSLATE;
				}
				else if (ImGui::IsKeyPressed('2'))
				{
					component.TransformOperation = Utils::Transform::ROTATE;
				}
				else if (ImGui::IsKeyPressed('3'))
				{
					component.TransformOperation = Utils::Transform::SCALE;
				}
			}
			else if (m_ActiveScene->HasComponent<DirLightComponent>(m_SelectedEntity))
			{
				DirLightComponent& component = m_ActiveScene->GetComponent<DirLightComponent>(m_SelectedEntity);
				glm::mat4 transform = glm::toMat4(glm::quat(component.Direction));
				ImGuizmo::Manipulate(glm::value_ptr(camera.GetView()), glm::value_ptr(camera.GetProjection()), ImGuizmo::ROTATE, ImGuizmo::WORLD, glm::value_ptr(transform));

				glm::vec3 tmp;
				if (ImGuizmo::IsUsing()) { Math::DecomposeTransform(transform, tmp, component.Direction, tmp); }
			}
		}

		m_SceneHierarchy.OnImGuiRender();
		camera.OnImGuiUpdate();
		ImGui::End();

		Log::GetAtlasLogger().Draw("Atlas Log");
		m_ActiveScene->OnUpdateEditor();

		ImGui::Begin("Atlas Settings");
		ImGui::DragFloat("Outline thickness", &m_OutlineThickness, 0.01f, 0.0f, 1.0f);
		ImGui::End();
		
		ImGui::Begin("Image Inspector");
		ImGui::Image((void*)(uint32_t)m_DirLightFrameBuffer->GetDepthAttachmentRendererID(), ImVec2(800, 800));
		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_SceneHierarchy.OnEvent(e);

		if (e.GetEventType() == EventType::EntitySelected)
		{
			m_SelectedEntity = ((EntitySelectedEvent&)e).GetEntity();
			m_ActiveScene->SetSelectedEntity(m_SelectedEntity);
		}
	}

	void EditorLayer::Begin()
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = m_GlobalFontScale;

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		m_ActiveScene->GetActiveCamera().OnUpdate(ts);

		FrameBufferSpecs specs = m_ViewportFrameBuffer->GetSpecs();

		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (specs.Width != (uint32_t)m_ViewportSize.x || specs.Height != (uint32_t)m_ViewportSize.y))
		{
			m_ViewportFrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_PostProcessingFrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

	}

	void EditorLayer::End()
	{
	}

	void EditorLayer::BindViewportFrameBuffer()
	{
		m_ViewportFrameBuffer->Bind();
	}

	void EditorLayer::UnbindViewportFrameBuffer()
	{
		m_ViewportFrameBuffer->Unbind();
	}

}