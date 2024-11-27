#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		void Renderer_W1();
		void Renderer_W1_Part1();
		void Renderer_W1_Part2();
		void Renderer_W1_Part3();
		void Renderer_W1_Part4();
		void Renderer_W1_Part5();

		void Renderer_W2();
		void Renderer_W2_Part1();
		void Renderer_W2_Part2_TriangleStrip();
		void Renderer_W2_Part2_TriangleList();

		void Renderer_W3();
		void Renderer_W3_Part1();

		bool IsPointInTriangle(const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& pixel);
		bool SaveBufferToImage() const;

		void VertexTransformationFunction(Mesh& mesh) const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Texture* m_pTexture;
		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		float weights[3]{};
		float m_Aspectratio{};
	};
}
