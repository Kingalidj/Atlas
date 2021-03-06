#pragma once

#include <memory>

#ifdef ATL_PLATFORM_WINDOWS
#if ATL_DYNAMIC_LINK
	#ifdef ATL_BUILD_DLL
		#define ATLAS_API __declspec(dllexport)
	#else
		#define ATLAS_API __declspec(dllimport)
	#endif
#else
	#define ATLAS_API
#endif
#else 
	#error Atlas only supports Windows!
#endif

#ifdef ATL_DEBUG
	#define ATL_ENABLE_ASSERTS
#endif

#ifdef ATL_ENABLE_ASSERTS
	#define ATL_ASSERT(x, ...) { if(!(x)) { ATL_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
	#define ATL_CORE_ASSERT(x, ...) { if(!(x)) { ATL_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
#else
	#define ATL_ASSERT(x, ...)
	#define ATL_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define ATL_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace Atlas
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


}
