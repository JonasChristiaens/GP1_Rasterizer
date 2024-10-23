//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	Renderer_W1_Part1(); //Rasterizer Stage Only
	//Renderer_W1_Part2(); //Projection Stage (Camera)
	//Renderer_W1_Part3(); //Barycentric coordinates
	//Renderer_W1_Part4(); //Depth Buffer
	//Renderer_W1_Part5(); //BoundingBox Optimization

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::Renderer_W1_Part1()
{
	//Define Triangle - Vertices in NDC space
	std::vector<Vector3> vertices_ndc
	{
		{0.f, 0.5f, 1.0f},
		{0.5f, -0.5f, 1.0f},
		{-0.5f, -0.5f, 1.0f}
	};

	/*	
	=============
	RENDER LOGIC
	=============
	*/

	//Loop through all vertices 
	for (int vertexIdx{}; vertexIdx < vertices_ndc.size(); ++vertexIdx)
	{
		//Convert coordinates from  NDC to screen space
		float screenSpaceVertexX{ ((vertices_ndc[vertexIdx].x + 1) / 2) * m_Width };
		float screenSpaceVertexY{ ((1 - vertices_ndc[vertexIdx].y) / 2) * m_Height };
		
		//calculate "next in line" coordinates which we will use for cross product
		float nextScreenSpaceVertexX{ ((vertices_ndc[(vertexIdx + 1) % 3].x + 1) / 2) * m_Width };
		float nextScreenSpaceVertexY{ ((1 - vertices_ndc[(vertexIdx + 1) % 3].y) / 2) * m_Height };

		//create Vector2 of these vertices of the triangle
		Vector3 currentVertex{ screenSpaceVertexX, screenSpaceVertexY, 1.0f };
		Vector3 clockWiseNextVertex{ nextScreenSpaceVertexX, nextScreenSpaceVertexY, 1.0f };

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				ColorRGB finalColor{ };

				//"intersection" Pixel & calculating vector from current vertex to pixel
				Vector3 pixel{ px + 0.5f, py + 0.5f, 1.0f};
				Vector2 vectorFromVertexToPixel{ pixel - currentVertex };
				Vector2 vectorFromVertexToVertex{ clockWiseNextVertex - currentVertex };

				//calculating cross product to check signs of returned areas 
				float signedArea{ Vector3::Cross(vectorFromVertexToVertex, vectorFromVertexToPixel) };
				if (signedArea < 0)
				{
					finalColor = colors::White;
				}
				

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
