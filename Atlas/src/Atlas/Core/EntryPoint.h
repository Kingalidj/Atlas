#pragma once

extern Atlas::Application* Atlas::CreateApplication();

int main(int argc, char** argv)
{
	Atlas::Log::Init();
	ATL_CORE_INFO("Welcome to Atlas enigne!");

	ATL_PROFILE_BEGIN_SESSION("Startup", "AtlasProfile-Startup.json");
	auto app = Atlas::CreateApplication();
	ATL_PROFILE_END_SESSION();

	ATL_PROFILE_BEGIN_SESSION("Runtime", "AtlasProfile-Runtime.json");
	app->Run();
	ATL_PROFILE_END_SESSION();

	ATL_PROFILE_BEGIN_SESSION("Runtime", "AtlasProfile-Shutdown.json");
	delete app;
	ATL_PROFILE_END_SESSION();
}
