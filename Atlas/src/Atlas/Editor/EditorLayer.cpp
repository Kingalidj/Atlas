#include "atlpch.h"
#include "EditorLayer.h"

#include "Atlas/ImGui/AtlasTheme.h"
#include "Atlas/Core/Core.h"
#include "Atlas/Math/Math.h"

#include "imgui.h"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Atlas/Renderer/Renderer3D.h"
#include "Atlas/Renderer/Renderer2D.h"
#include "Atlas/Renderer/RenderCommand.h"

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

		m_ViewportFrameBuffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();
		m_ViewportFrameBuffer->ClearAttachment(1, -1);

		Renderer3D::DrawScene(m_ActiveScene);

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)m_ViewportSize.x && mouseY < (int)m_ViewportSize.y)
		{
			int value = m_ViewportFrameBuffer->ReadPixel(1, mouseX, mouseY);
			if (value >= 0) m_HoveredEntity = value;
			else if (value == -1) m_HoveredEntity = ECS::null;
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && ImGui::IsWindowHovered()) m_SceneHierarchy.SetSelectedEntity(m_HoveredEntity);

		m_ViewportFrameBuffer->Unbind();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImVec2 cursorPos = ImGui::GetCursorPos();
		ImGui::Image((void*)(size_t)m_ViewportFrameBuffer->GetColorAttachmentRendererID(0), { m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		ImGui::PushStyleColor(ImGuiCol_Text, { 0.6f, 0.6f, 0.6f, 1.0f });
		if (m_HoveredEntity != ECS::null)
		{
			if (m_ActiveScene->HasComponent<TagComponent>(m_HoveredEntity))
			{
				ImGui::SetCursorPos(cursorPos);
				std::string text = "Hovered: " + (std::string)m_ActiveScene->GetComponent<TagComponent>(m_HoveredEntity);
				ImGui::Text(text.c_str());
			}
		}
		else
		{
			ImGui::SetCursorPos(cursorPos);
			ImGui::Text("Hovered: ");
		}
		ImGui::PopStyleColor();

		ImGui::PopStyleVar();

		ImGuiWindow* window = GImGui->CurrentWindow;
		ImGuizmo::SetRect(window->Pos.x, window->Pos.y, window->Size.x, window->Size.y);
		ImGuizmo::SetDrawlist(window->DrawList);

		auto& camera = m_ActiveScene->GetActiveCamera();

		ECS::Entity selectedEntity = m_SceneHierarchy.GetSelectedEntity();
		if (selectedEntity != ECS::null)
		{
			if (m_ActiveScene->HasComponent<TransformComponent>(selectedEntity))
			{

				TransformComponent& component = m_ActiveScene->GetComponent<TransformComponent>(selectedEntity);
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
			else if (m_ActiveScene->HasComponent<DirLightComponent>(selectedEntity))
			{
				DirLightComponent& component = m_ActiveScene->GetComponent<DirLightComponent>(selectedEntity);
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
	}

	void EditorLayer::OnEvent(Event& event)
	{
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