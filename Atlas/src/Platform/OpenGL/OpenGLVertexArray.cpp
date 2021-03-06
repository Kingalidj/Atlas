#include "atlpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Atlas {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case Atlas::ShaderDataType::Bool:		return GL_BOOL;
			case Atlas::ShaderDataType::Int:		return GL_INT;
			case Atlas::ShaderDataType::Int2:		return GL_INT;
			case Atlas::ShaderDataType::Int3:		return GL_INT;
			case Atlas::ShaderDataType::Int4:		return GL_INT;
			case Atlas::ShaderDataType::Float:		return GL_FLOAT;
			case Atlas::ShaderDataType::Float2:		return GL_FLOAT;
			case Atlas::ShaderDataType::Float3:		return GL_FLOAT;
			case Atlas::ShaderDataType::Float4:		return GL_FLOAT;
			case Atlas::ShaderDataType::Mat3:		return GL_FLOAT;
			case Atlas::ShaderDataType::Mat4:		return GL_FLOAT;
		}

		ATL_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		ATL_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		ATL_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		ATL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::BindAll() const
	{
		Bind();
		for (Ref<VertexBuffer> vertexBuffer : m_VertexBuffers) vertexBuffer->Bind();
		m_IndexBuffer->Bind();
	}

	void OpenGLVertexArray::Unbind() const
	{
		ATL_PROFILE_FUNCTION();
		
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		ATL_PROFILE_FUNCTION();

		ATL_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(m_VertexBufferOffset);
			glVertexAttribPointer(m_VertexBufferOffset, element.GetComponentCount(), ShaderDataTypeToOpenGLBaseType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE, vertexBuffer->GetLayout().GetStride(), (const void*)(intptr_t)element.Offset);
			m_VertexBufferOffset++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
		m_VertexBufferOffset += (uint32_t) layout.GetElements().size();
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		ATL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
}