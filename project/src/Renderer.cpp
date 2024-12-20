//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"
#include "Texture.h"
#include "Utils.h"

#include <iostream>

using namespace dae;

// used for old triangles, without mesh
/*std::vector<Mesh> meshes_world
{
	Mesh
	{
		{
			Vertex{{-3.f, 3.f, -2.f}, {}, {0.f, 0.f}},
			Vertex{{ 0.f, 3.f, -2.f}, {}, {.5f, 0.f}},
			Vertex{{ 3.f, 3.f, -2.f}, {}, {1.f, 0.f}},
			Vertex{{-3.f, 0.f, -2.f}, {}, {0.f, .5f}},
			Vertex{{ 0.f, 0.f, -2.f}, {}, {.5f, .5f}},
			Vertex{{ 3.f, 0.f, -2.f}, {}, {1.f, .5f}},
			Vertex{{-3.f, -3.f, -2.f}, {}, {0.f, 1.f}},
			Vertex{{ 0.f, -3.f, -2.f}, {}, {.5f, 1.f}},
			Vertex{{ 3.f, -3.f, -2.f}, {}, {1.f, 1.f}}
		},
		{
			3, 0, 4, 1, 5, 2,
			2, 6,    // degenerate triangle
			6, 3, 7, 4, 8, 5
		},
		PrimitiveTopology::TriangleStrip
	}
};
std::vector<Mesh> meshes_world
{
    Mesh
    {
        {
            Vertex{{-3,  3, -2}},
            Vertex{{ 0,  3, -2}},
            Vertex{{ 3,  3, -2}},
            Vertex{{-3,  0, -2}},
            Vertex{{ 0,  0, -2}},
            Vertex{{ 3,  0, -2}},
            Vertex{{-3, -3, -2}},
            Vertex{{ 0, -3, -2}},
            Vertex{{ 3, -3, -2}},
        },
        {
            3, 0, 1,    1, 4, 3,    4, 1, 2,
            2, 5, 4,    6, 3, 4,    4, 7, 6,
            7, 4, 5,    5, 8, 7

        },
        PrimitiveTopology::TriangleList
    }
};*/

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_Aspectratio = (m_Width / static_cast<float>(m_Height));

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	// initialize Camera
	m_Camera.Initialize(45.f, { .0f, 5.0f,-64.f });

	// load textures
	m_pDiffuseTexture = Texture::LoadFromFile("./resources/vehicle_diffuse.png");
	m_pNormalTexture = Texture::LoadFromFile("./resources/vehicle_normal.png");
	m_pSpecularTexture = Texture::LoadFromFile("./resources/vehicle_specular.png");
	m_pGlossTexture = Texture::LoadFromFile("./resources/vehicle_gloss.png");


	// load model
	meshes_world.push_back(Mesh{ {}, {}, PrimitiveTopology::TriangleList });
	Utils::ParseOBJ("./resources/vehicle.obj", meshes_world.at(0).vertices, meshes_world.at(0).indices);
}
Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pDiffuseTexture;
	delete m_pNormalTexture;
	delete m_pSpecularTexture;
	delete m_pGlossTexture;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer, m_Aspectratio);
}
void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	// clear BackBuffer
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//Renderer_W1();
	//Renderer_W2();
	//Renderer_W3();
	Renderer_W4_Part1(meshes_world.at(0).vertices, meshes_world.at(0).vertices_out, meshes_world.at(0).indices);
	
	// create float array
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, std::numeric_limits<float>::max());

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

/*
void Renderer::Renderer_W1()
{
	//Renderer_W1_Part1(); //Rasterizer Stage Only
	//Renderer_W1_Part2(); //Projection Stage (Camera)
	//Renderer_W1_Part3(); //Barycentric coordinates
	//Renderer_W1_Part4(); //Depth Buffer
	Renderer_W1_Part5(); //BoundingBox Optimization
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

		
	//=============
	//RENDER LOGIC
	//=============
	

	//Loop through all vertices 
	for (int vertexIdx{}; vertexIdx < vertices_ndc.size(); vertexIdx += 3)
	{
		Vector2 v0{ ((vertices_ndc[vertexIdx].x + 1) / 2) * m_Width, ((1 - vertices_ndc[vertexIdx].y) / 2) * m_Height };
		Vector2 v1{ ((vertices_ndc[vertexIdx + 1].x + 1) / 2) * m_Width, ((1 - vertices_ndc[vertexIdx + 1].y) / 2) * m_Height };
		Vector2 v2{ ((vertices_ndc[vertexIdx + 2].x + 1) / 2) * m_Width, ((1 - vertices_ndc[vertexIdx + 2].y) / 2) * m_Height };

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				Vector2 P{ (float)px + .5f, (float)py + .5f };

				ColorRGB finalColor{};

				if (Vector2::Cross(v1 - v0, P - v0) > 0 && Vector2::Cross(v2 - v1, P - v1) > 0 && Vector2::Cross(v0 - v2, P - v2) > 0)
				{
					finalColor = colors::White;
				}

				//float gradient = px / static_cast<float>(m_Width);
				//gradient += py / static_cast<float>(m_Width);
				//gradient /= 2.0f;

				//ColorRGB finalColor{ gradient, gradient, gradient };

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
void Renderer::Renderer_W1_Part2()
{
	//Define Triangle - Vertices in NDC space
	std::vector<Vertex> vertices_world
	{
		{{ 0.0f, 2.0f, 0.0f }},
		{{ 1.0f, 0.0f, 0.0f }},
		{{ -1.0f, 0.0f, 0.0f }}
	};

	
	//================
	//Projection stage
	//================
	
	std::vector<Vertex> vertices_screenSpace;
	//VertexTransformationFunction(vertices_world, vertices_screenSpace);

	
	//================
	//Rasterization stage
	//================
	
	//Loop through all vertices 
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pixel_ScreenSpace{ float(px), float(py) };

			bool pixelInTriangle{ true };

			// Check edges
			for (int edgeIndex = 0; edgeIndex < 3; ++edgeIndex)
			{
				int nextIndex = (edgeIndex + 1) % 3;
				auto edge = vertices_screenSpace[nextIndex].position.GetXY() - vertices_screenSpace[edgeIndex].position.GetXY();
				auto pointToPixel = pixel_ScreenSpace - vertices_screenSpace[edgeIndex].position.GetXY();

				if (Vector2::Cross(edge, pointToPixel) < 0)
				{
					pixelInTriangle = false;
					break;
				}
			}

			ColorRGB finalColor{};
			if (pixelInTriangle) finalColor = ColorRGB{ 1.0f, 1.0f, 1.0f };

			// Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}
void Renderer::Renderer_W1_Part3()
{
	//Define Triangle - Vertices in NDC space
	std::vector<Vertex> vertices_world
	{
		{{ 0.0f, 4.0f, 2.0f }, {1, 0, 0}},
		{{ 3.0f, -2.0f, 2.0f }, {0, 1, 0}},
		{{ -3.0f, -2.0f, 2.0f }, {0, 0, 1}}
	};

	//================
	//Projection stage
	//================
	
	std::vector<Vertex> vertices_screenSpace;
	//VertexTransformationFunction(vertices_world, vertices_screenSpace);

	//================
	//Rasterization stage
	//================

	//Loop through all vertices 
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pixel_ScreenSpace{ float(px), float(py) };

			bool pixelInTriangle{ true };

			float weights[3]{};
			float totalArea{};

			// Check edges
			for (int edgeIdx{}; edgeIdx < 3; ++edgeIdx)
			{
				int nextIndex{ (edgeIdx + 1) % 3 };
				int weightIndex{ (edgeIdx + 2) % 3 };

				auto edge{ vertices_screenSpace[nextIndex].position.GetXY() - vertices_screenSpace[edgeIdx].position.GetXY() };
				auto pointToPixel{ pixel_ScreenSpace - vertices_screenSpace[edgeIdx].position.GetXY() };
				auto weight{ Vector2::Cross(edge, pointToPixel) };

				if (weight < 0)
				{
					pixelInTriangle = false;
					break;
				}

				totalArea += weight;
				weights[weightIndex] = weight;
			}

			ColorRGB finalColor{};
			if (pixelInTriangle) 
			{
				finalColor = (weights[0] / totalArea) * vertices_screenSpace[0].color + (weights[1] / totalArea) * vertices_screenSpace[1].color + (weights[2] / totalArea) * vertices_screenSpace[2].color;
			}

			// Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}
void Renderer::Renderer_W1_Part4()
{
	//Define Triangle - Vertices in world space
	std::vector<Vertex> vertices_world
	{
		// RGB triangle
		{ {0.0f, 4.0f, 2.0f}, {1, 0, 0} },
		{ {3.0f, -2.0f, 2.0f}, {0, 1, 0} },
		{ {-3.0f, -2.0f, 2.0f}, {0, 0, 1} },

		//RED triangle
		{ {0.0f, 2.0f, 0.0f}, {1, 0, 0} },
		{ {1.5f, -1.0f, 0.0f}, {1, 0, 0} },
		{ {-1.5f, -1.0f, 0.0f}, {1, 0, 0} }
	};

	//================
	//Projection stage
	//================

	std::vector<Vertex> vertices_screenSpace;
	//VertexTransformationFunction(vertices_world, vertices_screenSpace);

	//================
	//Rasterization stage
	//================
	
	//Loop through all vertices 
	for (int triangleIdx{}; triangleIdx < vertices_screenSpace.size(); triangleIdx += 3)
	{
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				Vector2 pixel_ScreenSpace{ float(px), float(py) };

				bool pixelInTriangle{ true };

				float weights[3]{};
				float totalArea{};

				// Check edges
				for (int edgeIdx{}; edgeIdx < 3; ++edgeIdx)
				{
					int nextIndex{ (edgeIdx + 1) % 3 };
					int weightIndex{ (edgeIdx + 2) % 3 };

					auto edge{ vertices_screenSpace[triangleIdx + nextIndex].position.GetXY() - vertices_screenSpace[triangleIdx + edgeIdx].position.GetXY() };
					auto pointToPixel{ pixel_ScreenSpace - vertices_screenSpace[triangleIdx + edgeIdx].position.GetXY() };
					auto weight{ Vector2::Cross(edge, pointToPixel) };

					if (weight < 0)
					{
						pixelInTriangle = false;
						break;
					}

					totalArea += weight;
					weights[weightIndex] = weight;
				}

				ColorRGB finalColor{};
				if (pixelInTriangle)
				{
					float depth{ (weights[0] / totalArea) * vertices_screenSpace[triangleIdx].position.z + (weights[1] / totalArea) * vertices_screenSpace[triangleIdx + 1].position.z + (weights[2] / totalArea) * vertices_screenSpace[triangleIdx + 2].position.z };
					if (depth < m_pDepthBufferPixels[px + (py * m_Width)])
					{
						m_pDepthBufferPixels[px + (py * m_Width)] = depth;
						finalColor = (weights[0] / totalArea) * vertices_screenSpace[triangleIdx].color + (weights[1] / totalArea) * vertices_screenSpace[triangleIdx + 1].color + (weights[2] / totalArea) * vertices_screenSpace[triangleIdx + 2].color;

						// Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}


			}
		}
	}
}
void Renderer::Renderer_W1_Part5()
{
	//Define Triangle - Vertices in world space
	std::vector<Vertex> vertices_world
	{
		// RGB triangle
		{ {0.0f, 4.0f, 2.0f}, {1, 0, 0} },
		{ {3.0f, -2.0f, 2.0f}, {0, 1, 0} },
		{ {-3.0f, -2.0f, 2.0f}, {0, 0, 1} },

		//RED triangle
		{ {0.0f, 2.0f, 0.0f}, {1, 0, 0} },
		{ {1.5f, -1.0f, 0.0f}, {1, 0, 0} },
		{ {-1.5f, -1.0f, 0.0f}, {1, 0, 0} }
	};

	//================
	//Projection stage
	//================
	
	std::vector<Vertex> vertices_screenSpace;
	//VertexTransformationFunction(vertices_world, vertices_screenSpace);

	//================
	//Rasterization stage
	//================
	
	//Loop through all vertices 
	for (int triangleIdx{}; triangleIdx < vertices_screenSpace.size(); triangleIdx += 3)
	{
		// create const variables for reusability
		const Vector2 v0{ vertices_screenSpace[triangleIdx].position.GetXY() };
		const Vector2 v1{ vertices_screenSpace[triangleIdx + 1].position.GetXY() };
		const Vector2 v2{ vertices_screenSpace[triangleIdx + 2].position.GetXY() };


		// create bounding box
		int minX{ static_cast<int>(std::min(v0.x, std::min(v1.x, v2.x))) };
		int maxX{ static_cast<int>(std::max(v0.x, std::max(v1.x, v2.x))) };
		int minY{ static_cast<int>(std::min(v0.y, std::min(v1.y, v2.y))) };
		int maxY{ static_cast<int>(std::max(v0.y, std::max(v1.y, v2.y))) };

		// clamp box to screen
		minX = std::max(minX, 0);
		maxX = std::min(maxX, m_Width - 1);
		minY = std::max(minY, 0);
		maxY = std::min(maxY, m_Height - 1);

		for (int px{ minX }; px < maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				Vector2 pixel_ScreenSpace{ float(px), float(py) };

				bool pixelInTriangle{ true };

				float weights[3]{};
				float totalArea{};

				// Check edges
				for (int edgeIdx{}; edgeIdx < 3; ++edgeIdx)
				{
					int nextIndex{ (edgeIdx + 1) % 3 };
					int weightIndex{ (edgeIdx + 2) % 3 };

					auto edge{ vertices_screenSpace[triangleIdx + nextIndex].position.GetXY() - vertices_screenSpace[triangleIdx + edgeIdx].position.GetXY() };
					auto pointToPixel{ pixel_ScreenSpace - vertices_screenSpace[triangleIdx + edgeIdx].position.GetXY() };
					auto weight{ Vector2::Cross(edge, pointToPixel) };

					if (weight < 0)
					{
						pixelInTriangle = false;
						break;
					}

					totalArea += weight;
					weights[weightIndex] = weight;
				}

				ColorRGB finalColor{};
				if (pixelInTriangle)
				{
					float depth{ (weights[0] / totalArea) * vertices_screenSpace[triangleIdx].position.z + (weights[1] / totalArea) * vertices_screenSpace[triangleIdx + 1].position.z + (weights[2] / totalArea) * vertices_screenSpace[triangleIdx + 2].position.z };
					if (depth < m_pDepthBufferPixels[px + (py * m_Width)])
					{
						m_pDepthBufferPixels[px + (py * m_Width)] = depth;
						finalColor = (weights[0] / totalArea) * vertices_screenSpace[triangleIdx].color + (weights[1] / totalArea) * vertices_screenSpace[triangleIdx + 1].color + (weights[2] / totalArea) * vertices_screenSpace[triangleIdx + 2].color;

						// Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}
	}
}

void Renderer::Renderer_W2()
{
	//Renderer_W2_Part1(); //Implementing Quads
	switch (meshes_world.at(0).primitiveTopology)
	{
	case PrimitiveTopology::TriangleList:
		Renderer_W2_Part2_TriangleList(); //triangle list
		break;
	case PrimitiveTopology::TriangleStrip:
		Renderer_W2_Part2_TriangleStrip(); //triangle strip
		break;
	default:
		break;
	}
}
void Renderer::Renderer_W2_Part1()
{
	//Define Triangle - Vertices in world space
	std::vector<Vertex> vertices_world
	{
		// Quad
		{ {-3, 3, -2}, {1, 1, 1} },
		{ {0, 3, -2}, {1, 1, 1} },
		{ {3, 3, -2}, {1, 1, 1}},
		{ {-3, 0, -2}, {1, 1, 1}},
		{ {0, 0, -2}, {1, 1, 1}},
		{ {3, 0, -2}, {1, 1, 1}},
		{ {-3, -3, -2}, {1, 1, 1}},
		{ {0, -3, -2}, {1, 1, 1}},
		{ {3, -3, -2}, {1, 1, 1}}
	};

	std::vector<int> indices_triangle
	{
		3, 0, 4,
		0, 1, 4,
		4, 1, 5,
		1, 2, 5,
		6, 3, 7,
		3, 4, 7,
		7, 4, 8,
		4, 5, 8
	};

	//================
	//Projection stage
	//================
	
	std::vector<Vertex> vertices_screenSpace;
	//VertexTransformationFunction(vertices_world, vertices_screenSpace);

	//================
	//Rasterization stage
	//================
	
	//Loop through all vertices 
	for (int triangleIdx{}; triangleIdx < indices_triangle.size(); triangleIdx += 3)
	{
		// create const variables for reusability
		const Vector2 v0{ vertices_screenSpace[indices_triangle[triangleIdx]].position.GetXY() };
		const Vector2 v1{ vertices_screenSpace[indices_triangle[triangleIdx + 1]].position.GetXY() };
		const Vector2 v2{ vertices_screenSpace[indices_triangle[triangleIdx + 2]].position.GetXY() };


		// create bounding box
		int minX{ static_cast<int>(std::min(v0.x, std::min(v1.x, v2.x))) };
		int maxX{ static_cast<int>(std::max(v0.x, std::max(v1.x, v2.x))) };
		int minY{ static_cast<int>(std::min(v0.y, std::min(v1.y, v2.y))) };
		int maxY{ static_cast<int>(std::max(v0.y, std::max(v1.y, v2.y))) };

		// clamp box to screen
		minX = std::max(minX, 0);
		maxX = std::min(maxX, m_Width - 1);
		minY = std::max(minY, 0);
		maxY = std::min(maxY, m_Height - 1);

		for (int px{ minX }; px < maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				Vector2 pixel_ScreenSpace{ float(px), float(py) };

				bool pixelInTriangle{ true };

				float weights[3]{};
				float totalArea{};

				// Check edges
				for (int edgeIdx{}; edgeIdx < 3; ++edgeIdx)
				{
					int nextIndex{ (edgeIdx + 1) % 3 };
					int weightIndex{ (edgeIdx + 2) % 3 };

					auto edge{ vertices_screenSpace[indices_triangle[triangleIdx + nextIndex]].position.GetXY() - vertices_screenSpace[indices_triangle[triangleIdx + edgeIdx]].position.GetXY() };
					auto pointToPixel{ pixel_ScreenSpace - vertices_screenSpace[indices_triangle[triangleIdx + edgeIdx]].position.GetXY() };
					auto weight{ Vector2::Cross(edge, pointToPixel) };

					if (weight < 0)
					{
						pixelInTriangle = false;
						break;
					}

					totalArea += weight;
					weights[weightIndex] = weight;
				}

				ColorRGB finalColor{};
				if (pixelInTriangle)
				{
					float depth{ (weights[0] / totalArea) * vertices_screenSpace[indices_triangle[triangleIdx]].position.z + (weights[1] / totalArea) * vertices_screenSpace[indices_triangle[triangleIdx + 1]].position.z + (weights[2] / totalArea) * vertices_screenSpace[indices_triangle[triangleIdx + 2]].position.z };
					if (depth < m_pDepthBufferPixels[px + (py * m_Width)])
					{
						m_pDepthBufferPixels[px + (py * m_Width)] = depth;
						finalColor = (weights[0] / totalArea) * vertices_screenSpace[indices_triangle[triangleIdx]].color + (weights[1] / totalArea) * vertices_screenSpace[indices_triangle[triangleIdx + 1]].color + (weights[2] / totalArea) * vertices_screenSpace[indices_triangle[triangleIdx + 2]].color;

						// Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}
	}
}
void dae::Renderer::Renderer_W2_Part2_TriangleStrip()
{
	// transform vertices to screen space
	VertexTransformationFunction(meshes_world.at(0));

	for (size_t idx = 0; idx < meshes_world.at(0).indices.size() - 2; ++idx)
	{
		const int idx0 = meshes_world.at(0).indices[idx];
		const int idx1 = meshes_world.at(0).indices[idx + 1];
		const int idx2 = meshes_world.at(0).indices[idx + 2];

		// check for degenerate triangles
		if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0) continue;

		// precompute alternate winding
		int v1Index = (idx & 1) ? idx2 : idx1;
		int v2Index = (idx & 1) ? idx1 : idx2;

		// fetch vertices
		const Vector2 p0 = meshes_world.at(0).vertices_out[idx0].position.GetXY();
		const Vector2 p1 = meshes_world.at(0).vertices_out[v1Index].position.GetXY();
		const Vector2 p2 = meshes_world.at(0).vertices_out[v2Index].position.GetXY();

		// compute bounding box
		int minX = static_cast<int>(std::ceil(std::min(p0.x, std::min(p1.x, p2.x))));
		int maxX = static_cast<int>(std::ceil(std::max(p0.x, std::max(p1.x, p2.x))));
		int minY = static_cast<int>(std::ceil(std::min(p0.y, std::min(p1.y, p2.y))));
		int maxY = static_cast<int>(std::ceil(std::max(p0.y, std::max(p1.y, p2.y))));

		// clamp bounding box to screen
		minX = std::max(minX, 0);
		maxX = std::min(maxX, m_Width - 1);
		minY = std::max(minY, 0);
		maxY = std::min(maxY, m_Height - 1);

		// iterate over pixels within bounding box
		for (int px = minX; px <= maxX; ++px)
		{
			for (int py = minY; py <= maxY; ++py)
			{
				Vector2 pixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

				if (IsPointInTriangle(p0, p1, p2, pixel))
				{
					// interpolate depth 
					float depth = 1 / (
						weights[0] * (1 / (meshes_world.at(0).vertices_out[idx0].position.z)) +
						weights[1] * (1 / (meshes_world.at(0).vertices_out[v1Index].position.z)) +
						weights[2] * (1 / (meshes_world.at(0).vertices_out[v2Index].position.z)));	

					// interpolate uv
					const Vector2 uv
					{
						(((meshes_world.at(0).vertices_out[idx0].uv / meshes_world.at(0).vertices_out[idx0].position.z) * weights[0]) +
						((meshes_world.at(0).vertices_out[v1Index].uv / meshes_world.at(0).vertices_out[v1Index].position.z) * weights[1]) +
						((meshes_world.at(0).vertices_out[v2Index].uv / meshes_world.at(0).vertices_out[v2Index].position.z) * weights[2])) * depth
					};

					int bufferIdx = px + (py * m_Width);

					ColorRGB finalColor{};
					// Z-test
					if (depth < m_pDepthBufferPixels[bufferIdx])
					{
						m_pDepthBufferPixels[bufferIdx] = depth;

						// interpolate color
						finalColor = m_pDiffuseTexture->Sample(uv);

						// update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[bufferIdx] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}
	}
}
void dae::Renderer::Renderer_W2_Part2_TriangleList()
{
	// not fully working, fixed in Renderer_W3_Part2()

	// transform vertices to screen space
	VertexTransformationFunction(meshes_world.at(0));

	for (size_t idx = 0; idx < meshes_world.at(0).indices.size(); idx += 3)
	{
		const Vector2 v0{ meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx]].position.GetXY() };
		const Vector2 v1{ meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 1]].position.GetXY() };
		const Vector2 v2{ meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 2]].position.GetXY() };

		int minX{ static_cast<int>(std::ceil(std::min(v0.x, std::min(v1.x, v2.x)))) };
		int minY{ static_cast<int>(std::ceil(std::min(v0.y, std::min(v1.y, v2.y)))) };
		int maxX{ static_cast<int>(std::ceil(std::max(v0.x, std::max(v1.x, v2.x)))) };
		int maxY{ static_cast<int>(std::ceil(std::max(v0.y, std::max(v1.y, v2.y)))) };

		if (minX <= 0.f or minX >= m_Width) continue;
		if (maxX <= 0.f or maxX >= m_Width) continue;

		if (minY <= 0.f or minY >= m_Width) continue;
		if (maxY <= 0.f or maxY >= m_Width) continue;

		minX = minX <= 0 ? 0.f : minX;
		maxX = maxX >= (m_Width - 1) ? (m_Width - 1) : maxX;

		minY = minY <= 0 ? 0.f : minY;
		maxY = maxY >= (m_Height - 1) ? (m_Height - 1) : maxY;

		for (int px{ minX }; px < maxX; ++px)
		{
			for (int py{ minY }; py < maxY; ++py)
			{
				Vector2 pixel_viewSpace{ float(px), float(py) };

				float totalArea{};

				ColorRGB finalColor{};
				if (IsPointInTriangle(v0, v1, v2, pixel_viewSpace))
				{

					float zDepth{ 1 / (
						(weights[0] * (1 / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx]].position.z)) +	
						(weights[1] * (1 / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 1]].position.z)) + 
						(weights[2] * (1 / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 2]].position.z))) 
					};

					if (m_pDepthBufferPixels[px + (py * m_Width)] < zDepth or zDepth < 0.f or zDepth > 1.f) continue;
					m_pDepthBufferPixels[px + (py * m_Width)] = zDepth;

					float wDepth{ 1 / (
						(weights[0] * (1 / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx]].position.w)) +
						(weights[1] * (1 / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 1]].position.w)) +
						(weights[2] * (1 / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 2]].position.w))) 
					};

					const Vector2 uv{
						(weights[0] * (meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx]].uv / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx]].position.w) +
						weights[1] * (meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 1]].uv / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 1]].position.w) +
						weights[2] * (meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 2]].uv / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 2]].position.w)) * wDepth 
					};

					const Vector3 normal{ 
						(weights[0] * (meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx]].normal / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx]].position.w) +
						weights[1] * (meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 1]].normal / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 1]].position.w) +
						weights[2] * (meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 2]].normal / meshes_world.at(0).vertices_out[meshes_world.at(0).indices[idx + 2]].position.w)) * wDepth
					};

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
}

void dae::Renderer::Renderer_W3()
{
	//Renderer_W3_Part1(); // ProjectionMatrix and DepthBuffer
	Renderer_W3_Part2(); // Meshes
}
void dae::Renderer::Renderer_W3_Part1() //working with triangleStrip
{
	// transform vertices to screen space
	VertexTransformationFunction(meshes_world.at(0));

	for (size_t idx = 0; idx < meshes_world.at(0).indices.size() - 2; ++idx)
	{
		const int idx0 = meshes_world.at(0).indices[idx];
		const int idx1 = meshes_world.at(0).indices[idx + 1];
		const int idx2 = meshes_world.at(0).indices[idx + 2];

		// check for degenerate triangles
		if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0) continue;

		// precompute alternate winding
		int v1Index = (idx & 1) ? idx2 : idx1;
		int v2Index = (idx & 1) ? idx1 : idx2;

		// fetch vertices
		const Vector2 p0 = meshes_world.at(0).vertices_out[idx0].position.GetXY();
		const Vector2 p1 = meshes_world.at(0).vertices_out[v1Index].position.GetXY();
		const Vector2 p2 = meshes_world.at(0).vertices_out[v2Index].position.GetXY();

		// compute bounding box
		int minX = static_cast<int>(std::ceil(std::min(p0.x, std::min(p1.x, p2.x))));
		int maxX = static_cast<int>(std::ceil(std::max(p0.x, std::max(p1.x, p2.x))));
		int minY = static_cast<int>(std::ceil(std::min(p0.y, std::min(p1.y, p2.y))));
		int maxY = static_cast<int>(std::ceil(std::max(p0.y, std::max(p1.y, p2.y))));

		// clamp bounding box to screen
		minX = std::max(minX, 0);
		maxX = std::min(maxX, m_Width - 1);
		minY = std::max(minY, 0);
		maxY = std::min(maxY, m_Height - 1);

		// frustum culling
		if (minX == 0 || maxX == m_Width - 1) continue;
		if (minY == 0 || maxY == m_Height - 1) continue;

		// iterate over pixels within bounding box
		for (int px = minX; px <= maxX; ++px)
		{
			for (int py = minY; py <= maxY; ++py)
			{
				Vector2 pixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };
				ColorRGB finalColor{};

				if (IsPointInTriangle(p0, p1, p2, pixel))
				{
					int bufferIdx = px + (py * m_Width);

					// non-linear interpolate depth 
					float depth = 1 / (
						weights[0] * (1 / (meshes_world.at(0).vertices_out[idx0].position.z)) +
						weights[1] * (1 / (meshes_world.at(0).vertices_out[v1Index].position.z)) +
						weights[2] * (1 / (meshes_world.at(0).vertices_out[v2Index].position.z)));

					// Z-test
					if (depth > m_pDepthBufferPixels[bufferIdx] || depth < 0 || depth > 1) continue;
					m_pDepthBufferPixels[bufferIdx] = depth;

					// linear interpolated depth
					float wDepth{ (1 / (
						weights[0] * (1 / (meshes_world.at(0).vertices_out[idx0].position.w)) +
						weights[1] * (1 / (meshes_world.at(0).vertices_out[v1Index].position.w)) +
						weights[2] * (1 / (meshes_world.at(0).vertices_out[v2Index].position.w))))
					};

					// interpolate uv and sample texture
					const Vector2 uv
					{
						(((meshes_world.at(0).vertices_out[idx0].uv / meshes_world.at(0).vertices_out[idx0].position.w) * weights[0]) +
						((meshes_world.at(0).vertices_out[v1Index].uv / meshes_world.at(0).vertices_out[v1Index].position.w) * weights[1]) +
						((meshes_world.at(0).vertices_out[v2Index].uv / meshes_world.at(0).vertices_out[v2Index].position.w) * weights[2])) * wDepth
					};


					// toggling between finalColor and Depthbuffer
					switch (m_CurrentLightMode)
					{
					case dae::Renderer::LightMode::Diffuse:
						finalColor = m_pDiffuseTexture->Sample(uv);
						break;
					case dae::Renderer::LightMode::ObservedArea:
						// will add later
						break;
					default:
						break;
					}

					finalColor.MaxToOne();

					// update color in backbuffer
					m_pBackBufferPixels[bufferIdx] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}
void dae::Renderer::Renderer_W3_Part2() // working with triangleList
{
	// transform vertices to screen space
	VertexTransformationFunction(meshes_world.at(0));

	for (size_t idx = 0; idx < meshes_world.at(0).indices.size(); idx += 3)
	{
		const int idx0 = meshes_world.at(0).indices[idx];
		const int idx1 = meshes_world.at(0).indices[idx + 1];
		const int idx2 = meshes_world.at(0).indices[idx + 2];

		// fetch vertices
		const Vector2 p0 = meshes_world.at(0).vertices_out[idx0].position.GetXY();
		const Vector2 p1 = meshes_world.at(0).vertices_out[idx1].position.GetXY();
		const Vector2 p2 = meshes_world.at(0).vertices_out[idx2].position.GetXY();

		// compute bounding box
		int minX = static_cast<int>(std::min(p0.x, std::min(p1.x, p2.x)));
		int maxX = static_cast<int>(std::ceil(std::max(p0.x, std::max(p1.x, p2.x))));
		int minY = static_cast<int>(std::min(p0.y, std::min(p1.y, p2.y)));
		int maxY = static_cast<int>(std::ceil(std::max(p0.y, std::max(p1.y, p2.y))));

		// frustum culling
		if (minX <= 0 || maxX >= m_Width - 1) continue;
		if (minY <= 0 || maxY >= m_Height - 1) continue;

		// clamp bounding box to screen
		minX = std::max(minX, 0);
		maxX = std::min(maxX, m_Width - 1);
		minY = std::max(minY, 0);
		maxY = std::min(maxY, m_Height - 1);

		// iterate over pixels within bounding box
		for (int px = minX; px <= maxX; ++px)
		{
			for (int py = minY; py <= maxY; ++py)
			{
				Vector2 pixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };
				ColorRGB finalColor{};

				if (IsPointInTriangle(p0, p1, p2, pixel))
				{
					// caching vertices and precalculating values
					const auto& vertices = meshes_world.at(0).vertices_out;
					const auto& indices = meshes_world.at(0).indices;

					const auto& v0 = vertices[indices[idx]];
					const auto& v1 = vertices[indices[idx + 1]];
					const auto& v2 = vertices[indices[idx + 2]];

					const float invZ0 = 1.0f / v0.position.z;
					const float invZ1 = 1.0f / v1.position.z;
					const float invZ2 = 1.0f / v2.position.z;

					int bufferIdx = px + (py * m_Width);

					// calculate Z depth
					float zDepth { 1.0f / ( weights[0] * invZ0 + weights[1] * invZ1 + weights[2] * invZ2 ) };

					// depth test
					if (m_pDepthBufferPixels[bufferIdx] < zDepth or zDepth < 0.f or zDepth > 1.f) continue;
					m_pDepthBufferPixels[bufferIdx] = zDepth;

					// precalculate wdepth values
					const float invW0 = 1.0f / v0.position.w;
					const float invW1 = 1.0f / v1.position.w;
					const float invW2 = 1.0f / v2.position.w;

					// calculate W depth uv and normal
					float wDepth{ 1.0f / ( weights[0] * invW0 + weights[1] * invW1 + weights[2] * invW2 ) };
					const Vector2 uv{ (weights[0] * (v0.uv * invW0) + weights[1] * (v1.uv * invW1) + weights[2] * (v2.uv * invW2)) * wDepth };
					const Vector3 normal{ (weights[0] * (v0.normal * invW0) + weights[1] * (v1.normal * invW1) + weights[2] * (v2.normal * invW2)) * wDepth };

					// toggling between finalColor and Depthbuffer
					switch (m_CurrentLightMode)
					{
					case dae::Renderer::LightMode::Diffuse:
						finalColor = m_pDiffuseTexture->Sample(uv);
						break;
					case dae::Renderer::LightMode::ObservedArea:
					{
						const float remappedWbuffer{ Remap(zDepth, 0.985f, 1.0f) };
						finalColor = ColorRGB(remappedWbuffer, remappedWbuffer, remappedWbuffer);
					}
						break;
					default:
						break;
					}

					finalColor.MaxToOne();

					// update color in backbuffer
					m_pBackBufferPixels[bufferIdx] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}
*/

void dae::Renderer::Renderer_W4_Part1(const std::vector<Vertex>& vertex_in, std::vector<Vertex_Out>& vertex_out, const std::vector<uint32_t>& indices)
{
	// transform vertices to screen space
	VertexTransformationFunction(vertex_in, vertex_out);

	const auto& mesh = meshes_world.at(0);

	for (size_t idx = 0; idx < indices.size(); idx += 3)
	{
		const int idx0 = mesh.indices[idx];
		const int idx1 = mesh.indices[idx + 1];
		const int idx2 = mesh.indices[idx + 2];

		const Vertex_Out& v0 = mesh.vertices_out[idx0];
		const Vertex_Out& v1 = mesh.vertices_out[idx1];
		const Vertex_Out& v2 = mesh.vertices_out[idx2];

		// compute bounding box
		int minX = static_cast<int>(std::min(v0.position.x, std::min(v1.position.x, v2.position.x)));
		int maxX = static_cast<int>(std::ceil(std::max(v0.position.x, std::max(v1.position.x, v2.position.x))));
		int minY = static_cast<int>(std::min(v0.position.y, std::min(v1.position.y, v2.position.y)));
		int maxY = static_cast<int>(std::ceil(std::max(v0.position.y, std::max(v1.position.y, v2.position.y))));

		// frustum culling
		if (maxX <= 0 || minX >= m_Width - 1 || maxY <= 0 || minY >= m_Height - 1) continue;

		// bounding box clamping
		minX = std::max(minX, 0);
		maxX = std::min(maxX, m_Width - 1);
		minY = std::max(minY, 0);
		maxY = std::min(maxY, m_Height - 1);

		// iterate over pixels within bounding box
		for (int px = minX; px <= maxX; ++px)
		{
			for (int py = minY; py <= maxY; ++py)
			{
				Vector2 pixel = { static_cast<float>(px), static_cast<float>(py) };
				Vertex_Out interpolatedVertex{};

				if (IsPointInTriangle(v0, v1, v2, pixel, interpolatedVertex))
				{
					ColorRGB finalColor{};

					// depth test
					int pixelIndex = px + (py * m_Width);
					if (m_pDepthBufferPixels[pixelIndex] < interpolatedVertex.position.z || interpolatedVertex.position.z < 0.f || interpolatedVertex.position.z > 1.f) continue;
					m_pDepthBufferPixels[pixelIndex] = interpolatedVertex.position.z;

					if (m_ShowDepthBuffer)
					{
						const float remappedZbuffer{ Remap(interpolatedVertex.position.z, 0.995f, 1.0f) };
						finalColor = ColorRGB(remappedZbuffer, remappedZbuffer, remappedZbuffer);
					}
					else
					{
						Vector3 normal{ interpolatedVertex.normal };

						if (m_ShowNormalMap)
						{
							ColorRGB sampledNormal{ m_pNormalTexture->Sample(interpolatedVertex.uv) };
							Vector3 binormal{ Vector3::Cross(interpolatedVertex.normal, interpolatedVertex.tangent) };
							Matrix tangentSpaceAxis{ interpolatedVertex.tangent, binormal, interpolatedVertex.normal, Vector3::Zero };

							sampledNormal = 2.f * sampledNormal - ColorRGB{ 1.f, 1.f, 1.f };
							normal = tangentSpaceAxis.TransformVector({ sampledNormal.r, sampledNormal.g, sampledNormal.b });
						}

						// toggling between different states
						switch (m_CurrentLightMode)
						{
						case dae::Renderer::LightMode::ObservedArea:
							finalColor = ObvAreaPixelShading(normal);
							break;

						case dae::Renderer::LightMode::Diffuse:
						{
							ColorRGB diffuseColor{ m_pDiffuseTexture->Sample(interpolatedVertex.uv) };

							finalColor = DiffusePixelShading(normal, diffuseColor);
						}
							break;

						case dae::Renderer::LightMode::Specular:
						{
							ColorRGB specularColor{ m_pSpecularTexture->Sample(interpolatedVertex.uv) };
							ColorRGB glossColor{ m_pGlossTexture->Sample(interpolatedVertex.uv) };
							glossColor.MaxToOne();

							finalColor = CombinedPixelShading(normal, ColorRGB{ 0.f, 0.f, 0.f }, specularColor, glossColor, interpolatedVertex.viewDirection);
						}
							break;

						case dae::Renderer::LightMode::Combined:
						{
							ColorRGB diffuseColor{ m_pDiffuseTexture->Sample(interpolatedVertex.uv) };
							ColorRGB specularColor{ m_pSpecularTexture->Sample(interpolatedVertex.uv) };
							ColorRGB glossColor{ m_pGlossTexture->Sample(interpolatedVertex.uv) };
							glossColor.MaxToOne();

							finalColor = CombinedPixelShading(normal, diffuseColor, specularColor, glossColor, interpolatedVertex.viewDirection);
						}
							break;

						default:
							break;
						}
					}

					// update color in backbuffer
					finalColor.MaxToOne();
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertex_in, std::vector<Vertex_Out>& vertex_out) const
{
	const auto& worldMatrix = m_Camera.worldMatrix;
	const auto& worldViewProjMatrix = m_Camera.worldViewProjectionMatrix;
	const float halfWidth{ 0.5f * m_Width };
	const float halfHeight{ 0.5f * m_Height };

	vertex_out.clear();
	vertex_out.reserve(vertex_in.size());

	Vector4 v_viewspace, v_projected;
	Vector3 normal, tangent, viewDirection;

	for (const auto& vertex : vertex_in)
	{
		// transform position, normal, tangent and calculate view direction
		v_viewspace = worldViewProjMatrix.TransformPoint(Vector4{ vertex.position, 1 });
		normal = worldMatrix.TransformVector(vertex.normal);
		normal.Normalize();
		tangent = worldMatrix.TransformVector(vertex.tangent);
		tangent.Normalize();
		viewDirection = { v_viewspace.x, v_viewspace.y, v_viewspace.w };

		// project to NDC
		const float inv_w = 1.0f / v_viewspace.w;
		v_projected.x = (v_viewspace.x * inv_w + 1.0f) * halfWidth;
		v_projected.y = (1.0f - v_viewspace.y * inv_w) * halfHeight;
		v_projected.z = v_viewspace.z * inv_w;
		v_projected.w = v_viewspace.w;

		vertex_out.emplace_back(v_projected, vertex.color, vertex.uv, normal, tangent, viewDirection);
	}
}
void dae::Renderer::CycleLightingMode()
{
	int lightState{ int(m_CurrentLightMode) };
	m_CurrentLightMode = LightMode((lightState + 1) % 4);
}
void dae::Renderer::RotateModel()
{
	m_Camera.RotateModel();
}
void dae::Renderer::ShowDepthBuffer()
{
	m_ShowDepthBuffer = !m_ShowDepthBuffer;
}
void dae::Renderer::ShowNormalMap()
{
	m_ShowNormalMap = !m_ShowNormalMap;
}

bool dae::Renderer::IsPointInTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2, const Vector2& pixel, Vertex_Out& interpolatedVertex)
{
	const Vector2 vertex0Pos{ v0.position.GetXY() };
	const Vector2 vertex1Pos{ v1.position.GetXY() };
	const Vector2 vertex2Pos{ v2.position.GetXY() };

	if (pixel == vertex0Pos || pixel == vertex1Pos || pixel == vertex2Pos) return true;

	weights[0] = Vector2::Cross(vertex2Pos - vertex1Pos, pixel - vertex1Pos);
	weights[1] = Vector2::Cross(vertex0Pos - vertex2Pos, pixel - vertex2Pos);
	weights[2] = Vector2::Cross(vertex1Pos - vertex0Pos, pixel - vertex0Pos);

	if (weights[0] < 0 || weights[1] < 0 || weights[2] < 0) return false;

	// total triangle area
	float totalArea{ weights[0] + weights[1] + weights[2] };

	float invTotalArea = 1.0f / totalArea;
	weights[0] *= invTotalArea;
	weights[1] *= invTotalArea;
	weights[2] *= invTotalArea;

	// interpolated z and w using barycentric coordinates
	interpolatedVertex.position.z = 1 / ((weights[0] / v0.position.z) + (weights[1] / v1.position.z) + (weights[2] / v2.position.z));
	interpolatedVertex.position.w = 1 / ((weights[0] / v0.position.w) + (weights[1] / v1.position.w) + (weights[2] / v2.position.w));

	// interpolated attributes
	interpolatedVertex.uv = ((v0.uv / v0.position.w) * weights[0] + (v1.uv / v1.position.w) * weights[1] + (v2.uv / v2.position.w) * weights[2]) * interpolatedVertex.position.w;
	interpolatedVertex.normal = ((v0.normal / v0.position.w) * weights[0] + (v1.normal / v1.position.w) * weights[1] + (v2.normal / v2.position.w) * weights[2]) * interpolatedVertex.position.w;
	interpolatedVertex.tangent = ((v0.tangent / v0.position.w) * weights[0] + (v1.tangent / v1.position.w) * weights[1] + (v2.tangent / v2.position.w) * weights[2]) * interpolatedVertex.position.w;
	interpolatedVertex.viewDirection = ((v0.viewDirection / v0.position.w) * weights[0] + (v1.viewDirection / v1.position.w) * weights[1] + (v2.viewDirection / v2.position.w) * weights[2]) * interpolatedVertex.position.w;

	return true;
}
float dae::Renderer::Remap(float value, float inputMin, float inputMax)
{
	value = dae::Clamp(value, inputMin, inputMax);
	if (inputMax == inputMin) return 0.0f;

	return (value - inputMin) / (inputMax - inputMin);
}
ColorRGB dae::Renderer::ObvAreaPixelShading(Vector3 normal)
{
	Vector3 directionOfLight{ 0.577f, -0.577f, 0.577f };
	float observedAreaMeasure{ Vector3::Dot(normal, -directionOfLight) };

	if (observedAreaMeasure < 0.f) return ColorRGB{};

	ColorRGB lambert = ColorRGB{ 0.49f, 0.57f, 0.57f } / PI;
	ColorRGB color = lambert + (observedAreaMeasure * ColorRGB(1.0f, 1.0f, 1.0f));

	return color;
}
ColorRGB dae::Renderer::DiffusePixelShading(Vector3 normal, ColorRGB diffuseColor)
{
	Vector3 directionOfLight{ 0.577f, -0.577f, 0.577f };
	float observedAreaMeasure{ Vector3::Dot(normal, -directionOfLight) };

	if (observedAreaMeasure < 0.f) return ColorRGB{};

	float kd{ 7.0f };
	ColorRGB lambert = (diffuseColor * kd) / PI;
	ColorRGB color = lambert * observedAreaMeasure;

	return color;
}
ColorRGB dae::Renderer::CombinedPixelShading(Vector3 normal, ColorRGB diffuseColor, ColorRGB specularColor, ColorRGB glossColor, Vector3 viewDirection)
{
	Vector3 directionOfLight{ 0.577f, -0.577f, 0.577f };
	float observedAreaMeasure{ Vector3::Dot(normal, -directionOfLight) };

	if (observedAreaMeasure < 0.0f) return ColorRGB{};

	float kd{ 7.f };
	ColorRGB lambert = (diffuseColor * kd) / PI;

	// reflection term
	float shininessFactor{ 25.0f };
	Vector3 reflection{ -directionOfLight - (2 * Vector3::Dot(-directionOfLight, normal) * normal) };
	reflection.Normalize();

	// phong specular shading
	float cos{ std::max(0.0f, Vector3::Dot(reflection, viewDirection.Normalized())) };
	ColorRGB phong{ specularColor * std::powf(cos, shininessFactor * glossColor.r) };

	ColorRGB color = observedAreaMeasure * (lambert + phong);

	return color;
}
bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
