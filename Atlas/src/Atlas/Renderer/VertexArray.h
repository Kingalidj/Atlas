#pragma once

#include <memory>
#include "Atlas/Renderer/Buffer.h"

namespace Atlas {

	class VertexArray
	{
		public:
			virtual ~VertexArray() {}

			virtual void Bind() const = 0;
			virtual void BindAll() const = 0;
			virtual void Unbind() const = 0;

			virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
			virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

			virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
			virtual const Ref<VertexBuffer>& GetVertexBuffer(uint32_t index = 0) const = 0;
			virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

			virtual const uint32_t GetRendererID() const = 0;

			static Ref<VertexArray> Create();
	};

}
