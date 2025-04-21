/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "RmlUi_Renderer_SDL.h"
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Types.h>

#include <stb_image.h>

static void SetRenderClipRect(SDL_Renderer* renderer, const SDL_Rect* rect)
{
	SDL_SetRenderClipRect(renderer, rect);
}
static void SetRenderViewport(SDL_Renderer* renderer, const SDL_Rect* rect)
{
	SDL_SetRenderViewport(renderer, rect);
}

RenderInterface_SDL::RenderInterface_SDL()
{
	// RmlUi serves vertex colors and textures with premultiplied alpha, set the blend mode accordingly.
	// Equivalent to glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA).
	blend_mode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD, SDL_BLENDFACTOR_ONE,
		SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
}

void RenderInterface_SDL::BeginFrame(SDL_Renderer* renderer)
{
	this->renderer = renderer;
	
	SetRenderViewport(renderer, nullptr);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawBlendMode(renderer, blend_mode);
}

void RenderInterface_SDL::EndFrame() {}

Rml::CompiledGeometryHandle RenderInterface_SDL::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
	GeometryView* data = new GeometryView{vertices, indices};
	return reinterpret_cast<Rml::CompiledGeometryHandle>(data);
}

void RenderInterface_SDL::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
	delete reinterpret_cast<GeometryView*>(geometry);
}

void RenderInterface_SDL::RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture)
{
	const GeometryView* geometry = reinterpret_cast<GeometryView*>(handle);
	const Rml::Vertex* vertices = geometry->vertices.data();
	const size_t num_vertices = geometry->vertices.size();
	const int* indices = geometry->indices.data();
	const size_t num_indices = geometry->indices.size();

	Rml::UniquePtr<SDL_Vertex[]> sdl_vertices{new SDL_Vertex[num_vertices]};

	for (size_t i = 0; i < num_vertices; i++)
	{
		sdl_vertices[i].position = {vertices[i].position.x + translation.x, vertices[i].position.y + translation.y};
		sdl_vertices[i].tex_coord = {vertices[i].tex_coord.x, vertices[i].tex_coord.y};

		const auto& color = vertices[i].colour;
		sdl_vertices[i].color = {color.red / 255.f, color.green / 255.f, color.blue / 255.f, color.alpha / 255.f};
	}

	SDL_Texture* sdl_texture = (SDL_Texture*)texture;

	SDL_RenderGeometry(renderer, sdl_texture, sdl_vertices.get(), (int)num_vertices, indices, (int)num_indices);
}

void RenderInterface_SDL::EnableScissorRegion(bool enable)
{
	if (enable)
		SetRenderClipRect(renderer, &rect_scissor);
	else
		SetRenderClipRect(renderer, nullptr);

	scissor_region_enabled = enable;
}

void RenderInterface_SDL::SetScissorRegion(Rml::Rectanglei region)
{
	rect_scissor.x = region.Left();
	rect_scissor.y = region.Top();
	rect_scissor.w = region.Width();
	rect_scissor.h = region.Height();

	if (scissor_region_enabled)
		SetRenderClipRect(renderer, &rect_scissor);
}

Rml::TextureHandle RenderInterface_SDL::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(source.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, texWidth, texHeight);
	SDL_UpdateTexture(texture, 0, pixels, texWidth * 4);
	SDL_SetTextureBlendMode(texture, blend_mode);
	stbi_image_free(pixels);

	return (Rml::TextureHandle)texture;
}

Rml::TextureHandle RenderInterface_SDL::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
	RMLUI_ASSERT(source.data() && source.size() == size_t(source_dimensions.x * source_dimensions.y * 4));

	auto CreateSurface = [&]() {
		return SDL_CreateSurfaceFrom(source_dimensions.x, source_dimensions.y, SDL_PIXELFORMAT_RGBA32, (void*)source.data(), source_dimensions.x * 4);
	};
	auto DestroySurface = [](SDL_Surface* surface) { SDL_DestroySurface(surface); };

	SDL_Surface* surface = CreateSurface();

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetTextureBlendMode(texture, blend_mode);

	DestroySurface(surface);
	return (Rml::TextureHandle)texture;
}

void RenderInterface_SDL::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	SDL_DestroyTexture((SDL_Texture*)texture_handle);
}
