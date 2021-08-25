#pragma once

#include "Atlas/Core/Core.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Atlas/Core/Core.h"
#include "Atlas/Renderer/VertexArray.h"

namespace Atlas {

	struct Vertex
	{
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f);
		glm::vec2 TextureCoord = glm::vec2(0.0f);
	};

	struct VertexTriangle
	{
		Vertex Vertices[3];
	};

	struct Triangle
	{
		uint32_t indices[3];
	};

	class Mesh
	{
	private:
		bool m_HasTexCoords = false;
		bool m_SmoothShading = true;

		uint32_t m_VertexCount = 0, m_TriangleCount = 0, m_TexterCoordCount = 0;
		VertexTriangle* m_VertexTriangles = nullptr;
		uint32_t* m_Indices = nullptr;

		glm::vec4 m_Color = { 0.6f, 0.0f, 0.8f, 1.0f };

		glm::mat4 m_TranslationMatrix = glm::mat4(1.0f), m_RotationMatrix = glm::mat4(1.0f);

		Ref<VertexArray> m_VertexArray;

	public:
		Mesh(const char* path);
		~Mesh();

		static Ref<Mesh> Create(const char* path);

		bool Load(const char* path);
		void Invalidate();
		void CalculateNormals();
		void SetShading(bool smooth);

		inline uint32_t GetTriangleCount() const { return m_TriangleCount; }
		inline uint32_t GetIndexCount() const { return m_TriangleCount * 3; }
		inline uint32_t GetVertexCount() const { return m_TriangleCount * 3; }
		inline VertexTriangle* GetTriangles() const { return m_VertexTriangles; }
		inline Ref<VertexArray> GetVertexArray() const { return m_VertexArray; }

		inline glm::mat4& GetTranslationMatrix() { return m_TranslationMatrix; }
		inline glm::mat4& GetRotationMatrix() { return m_RotationMatrix; }

		inline const glm::vec4& GetColor() const { return m_Color; }
		inline const void SetColor(const glm::vec4& color) { m_Color = color; }

		inline void SetTranslation(const glm::vec3 position) { m_TranslationMatrix = glm::translate(glm::mat4(1.0f), position); }
		inline void SetRotation(const glm::vec3 axis, const float angle) { m_RotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis); }
	};

}
