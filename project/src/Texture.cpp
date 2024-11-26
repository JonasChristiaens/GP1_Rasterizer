#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
    Texture::Texture(SDL_Surface* pSurface) :
        m_pSurface{ pSurface },
        m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
    {
    }

    Texture::~Texture()
    {
        if (m_pSurface)
        {
            SDL_FreeSurface(m_pSurface);
            m_pSurface = nullptr;
        }
    }

    Texture* Texture::LoadFromFile(const std::string& path)
    {
        //TODO
        //Load SDL_Surface using IMG_LOAD
        //Create & Return a new Texture Object (using SDL_Surface)
        SDL_Surface* pSurface = IMG_Load(path.c_str());
        if (!pSurface) return nullptr;

        return new Texture(pSurface);
    }

    ColorRGB Texture::Sample(const Vector2& uv) const
    {
        //TODO
        //Sample the correct texel for the given uv
        const float u{ std::ranges::clamp(uv.x, 0.f, 1.f) * float(m_pSurface->w) };
        const float v{ std::ranges::clamp(uv.y, 0.f, 1.f) * float(m_pSurface->h) };

        const int index{ static_cast<int>(u) + (static_cast<int>(v) * m_pSurface->w) };

        Uint8 r{};
        Uint8 g{};
        Uint8 b{};

        SDL_GetRGB(m_pSurfacePixels[index], m_pSurface->format, &r, &g, &b);

        return ColorRGB{ static_cast<float>(r) / 255.f, static_cast<float>(g) / 255.f, static_cast<float>(b) / 255.f };
    }
}