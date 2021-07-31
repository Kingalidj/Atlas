#include "atlpch.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Atlas {

	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	ATL_CORE_ASSERT(false, "RendererAPI is no't supported!"); return nullptr;
			case RendererAPI::API::OpenGL:	return new OpenGLVertexArray();
		}

		ATL_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

}