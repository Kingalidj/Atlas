#pragma once

#include "Atlas/Renderer/RendererAPI.h"

namespace Atlas {

	class OpenGLRendererAPI : public RendererAPI
	{
	private:

	public:
		virtual void Init() override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;

	};

}
