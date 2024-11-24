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

	m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

	// globalize aspectratio
	m_AspectRatio = m_Width / static_cast<float>(m_Height);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
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

	// create float array
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, std::numeric_limits<float>::max());

	// clear BackBuffer
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//Renderer_W1();
	Renderer_W2();
	
	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

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

	/*	
	=============
	RENDER LOGIC
	=============
	*/

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

				if (Vector2::Cross(v1 - v0, P - v0) > 0 and Vector2::Cross(v2 - v1, P - v1) > 0 and Vector2::Cross(v0 - v2, P - v2) > 0)
				{
					finalColor = colors::White;
				}

				/*float gradient = px / static_cast<float>(m_Width);
				//gradient += py / static_cast<float>(m_Width);
				gradient /= 2.0f;

				ColorRGB finalColor{ gradient, gradient, gradient };*/

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

	/*
	================
	Projection stage
	================
	*/
	std::vector<Vertex> vertices_screenSpace;
	VertexTransformationFunction(vertices_world, vertices_screenSpace);

	/*
	================
	Rasterization stage
	================
	*/
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

	/*
	================
	Projection stage
	================
	*/
	std::vector<Vertex> vertices_screenSpace;
	VertexTransformationFunction(vertices_world, vertices_screenSpace);

	/*
	================
	Rasterization stage
	================
	*/
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

	/*
	================
	Projection stage
	================
	*/
	std::vector<Vertex> vertices_screenSpace;
	VertexTransformationFunction(vertices_world, vertices_screenSpace);

	/*
	================
	Rasterization stage
	================
	*/
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

	/*
	================
	Projection stage
	================
	*/
	std::vector<Vertex> vertices_screenSpace;
	VertexTransformationFunction(vertices_world, vertices_screenSpace);

	/*
	================
	Rasterization stage
	================
	*/
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
	Renderer_W2_Part2(); //Triangle Strip
	//Renderer_W2_Part3(); //Barycentric coordinates
	//Renderer_W2_Part4(); //Depth Buffer
	//Renderer_W2_Part5(); //BoundingBox Optimization
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

	/*
	================
	Projection stage
	================
	*/
	std::vector<Vertex> vertices_screenSpace;
	VertexTransformationFunction(vertices_world, vertices_screenSpace);

	/*
	================
	Rasterization stage
	================
	*/
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
void Renderer::Renderer_W2_Part2()
{
	// define triangle - Vertices in world space
	std::vector<Vertex> vertices_world
    {
        // quad vertices
        { {-3,  3, -2}, {1, 1, 1} },
        { { 0,  3, -2}, {1, 1, 1} },
        { { 3,  3, -2}, {1, 1, 1} },
        { {-3,  0, -2}, {1, 1, 1} },
        { { 0,  0, -2}, {1, 1, 1} },
        { { 3,  0, -2}, {1, 1, 1} },
        { {-3, -3, -2}, {1, 1, 1} },
        { { 0, -3, -2}, {1, 1, 1} },
        { { 3, -3, -2}, {1, 1, 1} }
    };

	// define triangle strip indices
	std::vector<int> triangleIndices
	{
		3, 0, 4,
		1, 5, 2,
		2, 6,	// degenerate triangle
		6, 3, 7,
		4, 8, 5
	};

	// transform vertices to screen space
	std::vector<Vertex> vertices_screenSpace;
	VertexTransformationFunction(vertices_world, vertices_screenSpace);

	for (size_t idx = 0; idx < triangleIndices.size() - 2; ++idx)
	{
		const int idx0 = triangleIndices[idx];
		const int idx1 = triangleIndices[idx + 1];
		const int idx2 = triangleIndices[idx + 2];

		// check for degenerate triangles
		if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0) continue;

		// precompute alternate winding
		int v1Index = (idx & 1) ? idx2 : idx1;
		int v2Index = (idx & 1) ? idx1 : idx2;

		// fetch vertices
		const Vertex& v0 = vertices_screenSpace[idx0];
		const Vertex& v1 = vertices_screenSpace[v1Index];
		const Vertex& v2 = vertices_screenSpace[v2Index];

		const Vector2 p0 = v0.position.GetXY();
		const Vector2 p1 = v1.position.GetXY();
		const Vector2 p2 = v2.position.GetXY();

		// compute bounding box
		int minX = static_cast<int>(std::min(p0.x, std::min(p1.x, p2.x)));
		int maxX = static_cast<int>(std::max(p0.x, std::max(p1.x, p2.x)));
		int minY = static_cast<int>(std::min(p0.y, std::min(p1.y, p2.y)));
		int maxY = static_cast<int>(std::max(p0.y, std::max(p1.y, p2.y)));

		// clamp bounding box to screen
		minX = std::max(minX, 0);
		maxX = std::min(maxX, m_Width - 1);
		minY = std::max(minY, 0);
		maxY = std::min(maxY, m_Height - 1);	

		// tterate over pixels within bounding box
		for (int px = minX; px <= maxX; ++px)
		{
			for (int py = minY; py <= maxY; ++py)
			{
				Vector2 pixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

				std::array<float, 3> weights;
				if (IsPointInTriangle(pixel, p0, p1, p2, weights))
				{
					// tnterpolate depth
					float depth = 
						weights[0] * v0.position.z +
						weights[1] * v1.position.z +
						weights[2] * v2.position.z;

					int bufferIdx = px + py * m_Width;

					// Z-test
					if (depth < m_pDepthBufferPixels[bufferIdx])
					{
						m_pDepthBufferPixels[bufferIdx] = depth;

						// interpolate color
						ColorRGB finalColor =
							weights[0] * v0.color +
							weights[1] * v1.color +
							weights[2] * v2.color;

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

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	vertices_out.clear();
	vertices_out.reserve(vertices_in.size());

	//Todo > W1 Projection Stage
	for (int idx{}; idx < vertices_in.size(); ++idx)
	{
		const Vector3 v_viewspace{m_Camera.viewMatrix.TransformPoint(vertices_in[idx].position) };
		Vector3 v_projected{};

		v_projected.x = v_viewspace.x / v_viewspace.z;
		v_projected.y = v_viewspace.y / v_viewspace.z;
		v_projected.z = v_viewspace.z;

		v_projected.x = v_projected.x / (m_AspectRatio * m_Camera.fov);
		v_projected.y = v_projected.y / m_Camera.fov;

		v_projected.x = ((v_projected.x + 1) / 2.f) * m_Width;
		v_projected.y = ((1 - v_projected.y) / 2.f) * m_Height;

		vertices_out.emplace_back(v_projected, vertices_in[idx].color);
	}
}
bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
