#pragma once

#include "Atlas/Renderer/RendererAPI.h"

namespace Atlas {

	class OpenGLRendererAPI : public RendererAPI
	{
	private:

	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;

	};

}
