#pragma once

#include <cstdint>
#include <vector>

#include "DataTypes.h"
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

		/*void Renderer_W1();
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
		void Renderer_W3_Part2();*/

		void Renderer_W4_Part1(const std::vector<Vertex>& vertex_in, std::vector<Vertex_Out>& vertex_out, const std::vector<uint32_t>& indices);

		void VertexTransformationFunction(const std::vector<Vertex>& vertex_in, std::vector<Vertex_Out>& vertex_out) const;
		void CycleLightingMode();
		void RotateModel();
		void ShowDepthBuffer();
		void ShowNormalMap();

		bool IsPointInTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const Vector2& pixel, Vertex_Out& interpolatedVertex );
		static float Remap(float value, float inputMin, float inputMax);
		ColorRGB ObvAreaPixelShading(Vector3 normal);
		ColorRGB DiffusePixelShading(Vector3 normal, ColorRGB diffuseColor);
		ColorRGB CombinedPixelShading(Vector3 normal, ColorRGB diffuseColor, ColorRGB specularColor, ColorRGB glossColor, Vector3 viewDirection);

		bool SaveBufferToImage() const;

	private:
		enum class LightMode
		{
			ObservedArea,
			Diffuse, 
			Specular,
			Combined
		};

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Texture* m_pDiffuseTexture;
		Texture* m_pNormalTexture;
		Texture* m_pSpecularTexture;
		Texture* m_pGlossTexture;

		Camera m_Camera{};
		LightMode m_CurrentLightMode{ LightMode::Combined };

		std::vector<Mesh> meshes_world{};

		int m_Width{};
		int m_Height{};

		float weights[3]{};
		float m_Aspectratio{};

		bool m_ShowDepthBuffer{ false };
		bool m_ShowNormalMap{ true };
	};
}
