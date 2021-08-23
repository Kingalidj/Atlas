#include "atlpch.h"

#include "Renderer3D.h"
#include "RenderCommand.h"

#include "VertexArray.h"
#include "Shader.h"
#include "Mesh.h"

#include <glm/gtc/matrix_transform.hpp>

//TEMP
#include "PerspectiveCamera.h"

namespace Atlas {

	struct Renderer3DData
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> VertexArray;
		Ref<VertexBuffer> VertexBuffer;
		Ref<IndexBuffer> IndexBuffer;
		Ref<Shader> Shader;

		uint32_t IndexCount = 0;
		uint32_t VertexCount = 0;

		//TEMP
		//Mesh Object;
		glm::vec3 BasicLightDir = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));
		float Angle = 0.0f;

		Ref<Mesh> m_Mesh;
		PerspectiveCamera Camera = PerspectiveCamera(0.1f, 1000.0f, glm::radians(90.0f), 1.0f);

	};

	static Renderer3DData s_Data;

	void Renderer3D::Init()
	{
		ATL_PROFILE_FUNCTION();
		s_Data.m_Mesh = Mesh::Create("assets/Models/Teapot.obj");

		s_Data.m_Mesh->SetTranslation(glm::vec3(-0.0f, -0.0f, -7.0f));

		s_Data.Shader = Atlas::Shader::Create("assets/Shaders/Material.glsl");
		s_Data.Shader->Bind();
	}

	void Renderer3D::Shutdown()
	{
		ATL_PROFILE_FUNCTION();
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera)
	{
		ATL_PROFILE_FUNCTION();
		s_Data.Shader->Bind();

		s_Data.m_Mesh->SetRotation(glm::vec3(1.0f), s_Data.Angle);
		s_Data.Angle += 0.5f;

		s_Data.Shader->SetMat4("u_ViewProjection", camera.GetProjectionMatrix());
		s_Data.Shader->SetMat4("u_RotationMatrix", s_Data.m_Mesh->GetRotationMatrix());
		s_Data.Shader->SetMat4("u_TranslationMatrix", s_Data.m_Mesh->GetTranslationMatrix());
		s_Data.Shader->SetFloat3("u_LightDir", s_Data.BasicLightDir);

	}

	void Renderer3D::EndScene()
	{
		ATL_PROFILE_FUNCTION();
		Flush();
	}

	void Renderer3D::Flush()
	{
		ATL_PROFILE_FUNCTION();
		s_Data.m_Mesh->GetVertexArray()->BindAll();

		RenderCommand::DrawIndexed(s_Data.m_Mesh->GetVertexArray(), s_Data.m_Mesh->GetTriangleCount() * 3);
	}
}
