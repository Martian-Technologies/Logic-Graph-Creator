include(cmake/CPM.cmake)

function(add_main_dependencies)
	# JSON
	CPMAddPackage(
		NAME nlohmann_json
		GITHUB_REPOSITORY nlohmann/json
		GIT_TAG v3.12.0
		OPTIONS "OPTION_BUILD_TESTS OFF"
		SOURCE_DIR "${EXTERNAL_DIR}/json"
	)
	list(APPEND EXTERNAL_LINKS nlohmann_json)

	# CPPLocate (they have extreme cmake goofyness)
	CPMAddPackage(
		NAME cpplocate
		GITHUB_REPOSITORY cginternals/cpplocate
		GIT_TAG v2.3.0
		SOURCE_DIR "${EXTERNAL_DIR}/cpplocate"
	)
	list(APPEND EXTERNAL_LINKS cpplocate::cpplocate)

	# wasmtime
	if (APPLE AND CONNECTION_MACHINE_DISTRIBUTE_APP)
		add_library(wasmtime STATIC IMPORTED GLOBAL)
		CPMAddPackage(
			NAME wasmtime
			GITHUB_REPOSITORY bytecodealliance/wasmtime
			GIT_TAG v35.0.0
			DOWNLOAD_ONLY YES
			SOURCE_DIR "${EXTERNAL_DIR}/wasmtime"
		)
		set_target_properties(wasmtime PROPERTIES
			IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib-binary/libwasmtime.a"
			INTERFACE_INCLUDE_DIRECTORIES "${EXTERNAL_DIR}/wasmtime/crates/c-api"  # where .h file is
		)
	else()
		CPMAddPackage(
			NAME wasmtime
			GITHUB_REPOSITORY bytecodealliance/wasmtime
			GIT_TAG v35.0.0
			SOURCE_DIR "${EXTERNAL_DIR}/wasmtime"
		)
		add_subdirectory("${EXTERNAL_DIR}/wasmtime/crates/c-api")
	endif()
	list(APPEND EXTERNAL_LINKS wasmtime)

	# parallel hashmap
	CPMAddPackage(
		NAME parallel-hashmap
		GITHUB_REPOSITORY greg7mdp/parallel-hashmap
		GIT_TAG v2.0.0
		SOURCE_DIR "${EXTERNAL_DIR}/parallel-hashmap"
	)
	if(parallel-hashmap_ADDED)
		add_library(parallel-hashmap INTERFACE)
		target_include_directories(parallel-hashmap INTERFACE ${parallel-hashmap_SOURCE_DIR})
	endif()
	list(APPEND EXTERNAL_LINKS parallel-hashmap)

	# tracy
	if (RUN_TRACY_PROFILER)
		message("tracy is ON")
		CPMAddPackage(
			NAME tracy
			GITHUB_REPOSITORY wolfpld/tracy
			GIT_TAG v2.0.0
			OPTIONS "TRACY_ENABLE ON" "TRACY_ON_DEMAND ON"
			SOURCE_DIR "${EXTERNAL_DIR}/tracy"
		)
		list(APPEND EXTERNAL_LINKS tracy)
	endif()

	set(EXTERNAL_LINKS "${EXTERNAL_LINKS}" PARENT_SCOPE)
endfunction()

function(add_app_dependencies)
	# Find vulkan (we don't actually use the headers, we just want shaderc compiler (won't be needed when we switch to slang hopefully))
	find_package(Vulkan REQUIRED COMPONENTS glslc)

	# Volk vulkan meta loader
	CPMAddPackage(
		NAME volk
		GITHUB_REPOSITORY zeux/volk
		GIT_TAG 1.4.304
		SOURCE_DIR "${EXTERNAL_DIR}/volk"
	)
	list(APPEND EXTERNAL_LINKS volk)

	# SDL
	# if (CONNECTION_MACHINE_BUILD_TESTS) # hack to allow SDL to build without window system on linux
		# set(SDL_UNIX_CONSOLE_BUILD ON)
	# endif()
	CPMAddPackage(
		NAME SDL3
		GITHUB_REPOSITORY libsdl-org/SDL
		GIT_TAG release-3.2.20
		OPTIONS "SDL_STATIC ON"
		SOURCE_DIR "${EXTERNAL_DIR}/SDL"
	)
	add_library(SDL3::SDL3 ALIAS SDL3-static)
	list(APPEND EXTERNAL_LINKS SDL3::SDL3)

	# STB Image
	CPMAddPackage(
		NAME stb_image
		GITHUB_REPOSITORY nothings/stb
		GIT_TAG f58f558c120e9b32c217290b80bad1a0729fbb2c
		SOURCE_DIR "${EXTERNAL_DIR}/stb"
	)
	if(stb_image_ADDED)
		add_library(stb_image INTERFACE)
		target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})
	endif()
	list(APPEND EXTERNAL_LINKS stb_image)
	
	# Freetype
	CPMAddPackage(
        NAME freetype
        GITHUB_REPOSITORY libsdl-org/freetype
        GIT_TAG VER-2-13-3
        # OPTIONS
            # "FT_DISABLE_HARFBUZZ ON"
            # "FT_WITH_HARFBUZZ OFF"
		SOURCE_DIR "${EXTERNAL_DIR}/freetype"
    )
	add_library(Freetype::Freetype ALIAS freetype)
	list(APPEND EXTERNAL_LINKS freetype)

	# RmlUi
	CPMAddPackage(
        NAME RmlUi
        GITHUB_REPOSITORY mikke89/RmlUi
        GIT_TAG 6.1
        OPTIONS
			"RMLUI_BACKEND native"
        EXCLUDE_FROM_ALL FALSE
		SOURCE_DIR "${EXTERNAL_DIR}/RmlUi"
    )
	list(APPEND EXTERNAL_LINKS RmlUi::RmlUi)
	list(APPEND EXTERNAL_LINKS RmlUi::Debugger)

	# VMA
	CPMAddPackage(
		NAME VulkanMemoryAllocator
		GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
		GIT_TAG v3.3.0
		SOURCE_DIR "${EXTERNAL_DIR}/VulkanMemoryAllocator"
	)
	list(APPEND EXTERNAL_LINKS VulkanMemoryAllocator)

	# Vk Bootstrap
	CPMAddPackage(
		NAME vk-bootstrap
		GITHUB_REPOSITORY charles-lunarg/vk-bootstrap
		GIT_TAG 4ac01fd98e05bf8a277ca099a63449f6e5c9cedf#v1.4.323
		SOURCE_DIR "${EXTERNAL_DIR}/vk-bootstrap"
	)
	list(APPEND EXTERNAL_LINKS vk-bootstrap::vk-bootstrap)

	# GLM
	CPMAddPackage(
		NAME glm
		GITHUB_REPOSITORY g-truc/glm
		GIT_TAG 1.0.1
		SOURCE_DIR "${EXTERNAL_DIR}/glm"
	)
	list(APPEND EXTERNAL_LINKS glm)

	set(EXTERNAL_LINKS "${EXTERNAL_LINKS}" PARENT_SCOPE)
endfunction()
