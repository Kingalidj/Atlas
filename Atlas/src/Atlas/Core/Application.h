#pragma once

#include "Core.h"

#include "Window.h"
#include "Atlas/Events/Event.h"
#include "Atlas/Events/ApplicationEvent.h"

#include "Atlas/Scene/Scene.h"

#include "Atlas/Core/LayerStack.h"
#include "Atlas/ImGui/ImGuiLayer.h"

#include "Atlas/Editor/EditorLayer.h"

#include "Atlas/Core/Timestep.h"

namespace Atlas {

	class Application
	{

	public:

		struct AppProps
		{
			std::string m_Title;
			int m_Width, m_Height;
			EditorType type = EditorType::ATLAS2D;

		};


	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		EditorLayer* m_EditorLayer;
		
		EditorType m_EditorType = EditorType::ATLAS2D;

		bool m_Running = true;
		bool m_Minimized = false;

		LayerStack m_LayerStack;

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		static Application* s_Instance;
	
		float m_LastFrameTime;

	public:
		float MouseX, MouseY;

		Application();
		Application(AppProps props);
		Application(std::string title, uint32_t width, uint32_t height, EditorType type);
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		Window& GetWindow() { return *m_Window; }
		static Scope<Window>& GetWindowScope() { return s_Instance->m_Window; }
		static uint32_t GetWindowHeight() { return s_Instance->m_Window->GetHeight(); }
		static uint32_t GetWindowWidth() { return s_Instance->m_Window->GetWidth(); }
		static Application& Get() { return *s_Instance; }

		static glm::vec2& GetViewportSize() { return s_Instance->m_EditorLayer->GetViewportSize(); }
		static Ref<FrameBuffer>& GetViewportFrameBuffer() { return s_Instance->m_EditorLayer->GetFrameBuffer(); }

		static EditorType GetEditorType() { return s_Instance->m_EditorType; }
		static void ToggleViewport(bool toggle) { s_Instance->m_EditorLayer->GetEditor()->ToggleViewport(toggle); }

		static const Window::EventCallbackFn& GetEventCallback() { return s_Instance->m_Window->GetEventCallback(); }
		//TEMP
		static Ref<Scene>& GetActiveScene() { return s_Instance->m_EditorLayer->GetActiveScene(); }

	};

	//To be defined in Client
	Application* CreateApplication();

}

