function(add_main_dependencies)

	# CPPLocate (they have extreme cmake goofyness)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
	set(OPTION_BUILD_TESTS OFF) # these are cpplocate's tests
	add_subdirectory("${EXTERNAL_DIR}/cpplocate" EXCLUDE_FROM_ALL)
	list(APPEND EXTERNAL_LINKS cpplocate::cpplocate)

	# wasmtime
	if (APPLE AND CONNECTION_MACHINE_DISTRIBUTE_APP)
		add_library(wasmtime STATIC IMPORTED GLOBAL)
		set_target_properties(wasmtime PROPERTIES
			IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib-binary/libwasmtime.a"
			INTERFACE_INCLUDE_DIRECTORIES "${EXTERNAL_DIR}/wasmtime/crates/c-api"  # where .h file is
		)
	else()
		add_subdirectory("${EXTERNAL_DIR}/wasmtime/crates/c-api")
	endif()
	list(APPEND EXTERNAL_LINKS wasmtime)

	# JSON
	add_library(json INTERFACE)
	target_include_directories(json INTERFACE "${EXTERNAL_DIR}/json/")
	list(APPEND EXTERNAL_LINKS json)

	# parallel hashmap
	add_library(parallel_hashmap INTERFACE)
	target_include_directories(parallel_hashmap INTERFACE "${EXTERNAL_DIR}/parallel_hashmap/")
	list(APPEND EXTERNAL_LINKS parallel_hashmap)

	# tracy
	if (RUN_TRACY_PROFILER)
		message("tracy is ON")
		option(TRACY_ENABLE "" ON)
		option(TRACY_ON_DEMAND "" ON)
		add_subdirectory("${EXTERNAL_DIR}/tracy")
		list(APPEND EXTERNAL_LINKS Tracy::TracyClient)
	endif()

	set(EXTERNAL_LINKS "${EXTERNAL_LINKS}" PARENT_SCOPE)

endfunction()

function(add_app_dependencies)

	# Find vulkan (we don't actually use the headers, we just want shaderc compiler (won't be needed when we switch to slang hopefully))
	find_package(Vulkan REQUIRED COMPONENTS glslc)

	# Volk vulkan meta loader
	add_subdirectory("${EXTERNAL_DIR}/volk" SYSTEM EXCLUDE_FROM_ALL)
	list(APPEND EXTERNAL_LINKS volk)

	# SDL
	if (CONNECTION_MACHINE_BUILD_TESTS) # hack to allow SDL to build without window system on linux
		set(SDL_UNIX_CONSOLE_BUILD ON)
	endif()
	set(SDL_STATIC ON)
	add_subdirectory("${EXTERNAL_DIR}/SDL" SYSTEM EXCLUDE_FROM_ALL)
	list(APPEND EXTERNAL_LINKS SDL3::SDL3)

	# STB Image
	set(CONNECTION_MACHINE_STB_IMAGE_LIB_NAME "stb_image")
	add_library(${CONNECTION_MACHINE_STB_IMAGE_LIB_NAME} INTERFACE)
	target_include_directories(${CONNECTION_MACHINE_STB_IMAGE_LIB_NAME} INTERFACE "${EXTERNAL_DIR}/stb/")
	list(APPEND EXTERNAL_LINKS stb_image)
	
	# Freetype
	add_subdirectory("${EXTERNAL_DIR}/freetype" SYSTEM EXCLUDE_FROM_ALL)
	add_library(Freetype::Freetype ALIAS freetype)
	list(APPEND EXTERNAL_LINKS freetype)

	# RmlUi
	set(RMLUI_BACKEND native)
	add_subdirectory("${EXTERNAL_DIR}/RmlUi" SYSTEM EXCLUDE_FROM_ALL)
	list(APPEND EXTERNAL_LINKS RmlUi::RmlUi)
	list(APPEND EXTERNAL_LINKS RmlUi::Debugger)

	# VMA
	add_subdirectory("${EXTERNAL_DIR}/VulkanMemoryAllocator" SYSTEM EXCLUDE_FROM_ALL)
	list(APPEND EXTERNAL_LINKS VulkanMemoryAllocator)

	# Vk Bootstrap
	add_subdirectory("${EXTERNAL_DIR}/vk-bootstrap" SYSTEM EXCLUDE_FROM_ALL)
	list(APPEND EXTERNAL_LINKS vk-bootstrap::vk-bootstrap)

	# GLM
	add_subdirectory("${EXTERNAL_DIR}/glm" SYSTEM EXCLUDE_FROM_ALL)
	list(APPEND EXTERNAL_LINKS glm)

	set(EXTERNAL_LINKS "${EXTERNAL_LINKS}" PARENT_SCOPE)

endfunction()
