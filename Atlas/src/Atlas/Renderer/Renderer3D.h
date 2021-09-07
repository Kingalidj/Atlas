#pragma once

#include "PerspectiveCamera.h"
#include "Scene.h"
#include "Mesh.h"

namespace Atlas {

	class Renderer3D
	{

	private:
		static void DrawMesh(const Ref<Mesh>& mesh);

	public:
		static void Init();
		static void Shutdown();

		static void DrawScene(const Ref<Scene> scene);
		static void EndScene();
		static void Flush(const Ref<Mesh>& scene);

	};

}
