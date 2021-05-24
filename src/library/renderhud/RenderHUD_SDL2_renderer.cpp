/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RenderHUD_SDL2_renderer.h"
#ifdef LIBTAS_ENABLE_HUD

#include "../logging.h"
#include "../hook.h"

namespace libtas {

DECLARE_ORIG_POINTER(SDL_CreateTexture)
DECLARE_ORIG_POINTER(SDL_DestroyTexture)
DECLARE_ORIG_POINTER(SDL_LockTexture)
DECLARE_ORIG_POINTER(SDL_UnlockTexture)
DECLARE_ORIG_POINTER(SDL_RenderCopy)

RenderHUD_SDL2_renderer::~RenderHUD_SDL2_renderer()
{
    if (texture != nullptr)
        orig::SDL_DestroyTexture(texture);
}

void RenderHUD_SDL2_renderer::setRenderer(SDL_Renderer* r)
{
    renderer = r;
}

void RenderHUD_SDL2_renderer::renderSurface(std::unique_ptr<SurfaceARGB> surf, int x, int y)
{
    LINK_NAMESPACE_SDL2(SDL_CreateTexture);
    LINK_NAMESPACE_SDL2(SDL_DestroyTexture);
    LINK_NAMESPACE_SDL2(SDL_LockTexture);
    LINK_NAMESPACE_SDL2(SDL_UnlockTexture);
    LINK_NAMESPACE_SDL2(SDL_RenderCopy);

    GlobalNative gn;

    /* Create a new texture if smaller than the text to be rendered */
    if ((texture == nullptr) || (surf->w > tex_w) || (surf->h > tex_h)) {
        if (texture != nullptr)
            orig::SDL_DestroyTexture(texture);

        texture = orig::SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, surf->w, surf->h);
        tex_w = surf->w;
        tex_h = surf->h;        
    }

    /* Copy pixels into the texture */
    SDL_Rect tex_rect = {0, 0, surf->w, surf->h};
    uint8_t* tex_pixels;
    int pitch;

    orig::SDL_LockTexture(texture, &tex_rect, reinterpret_cast<void**>(&tex_pixels), &pitch);
    for (int row = 0; row < surf->h; row++) {
        memcpy(tex_pixels, &surf->pixels[row*surf->w], 4*surf->w);
        tex_pixels += pitch;
    }
    orig::SDL_UnlockTexture(texture);
    
    SDL_Rect rect = {x, y, surf->w, surf->h};
    orig::SDL_RenderCopy(renderer, texture, &tex_rect, &rect);
    
    /* Erase the content of the texture */
    orig::SDL_LockTexture(texture, &tex_rect, reinterpret_cast<void**>(&tex_pixels), &pitch);
    for (int row = 0; row < surf->h; row++) {
        memset(tex_pixels, 0, 4*surf->w);
        tex_pixels += pitch;
    }
    orig::SDL_UnlockTexture(texture);
}

}

#endif
