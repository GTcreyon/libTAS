/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "hook.h"
#include "logging.h"
#include <dlfcn.h>

#include <SDL2/SDL.h> // SDL_version

namespace libtas {

DECLARE_ORIG_POINTER(SDL_GetVersion);

namespace orig {
    /* SDL 1.2 specific function */
    static SDL_version * (*SDL_Linked_Version)(void);
}

int get_sdlversion(void)
{
    /* We save the version major so that we can return it in a future calls */
    static int SDLver = -1;

    if (SDLver != -1)
        return SDLver;

    /* Determine SDL version */
    SDL_version ver = {0, 0, 0};

    /* First look if symbols are already accessible */
    if (orig::SDL_GetVersion == nullptr)
        NATIVECALL(orig::SDL_GetVersion = (decltype(orig::SDL_GetVersion)) dlsym(RTLD_DEFAULT, "SDL_GetVersion"));
    NATIVECALL(orig::SDL_Linked_Version = (decltype(orig::SDL_Linked_Version)) dlsym(RTLD_DEFAULT, "SDL_Linked_Version"));

    if (orig::SDL_GetVersion) {
        orig::SDL_GetVersion(&ver);
    }

    if (orig::SDL_Linked_Version) {
        SDL_version *verp;
        verp = orig::SDL_Linked_Version();
        ver = *verp;
    }

    if (orig::SDL_GetVersion && orig::SDL_Linked_Version) {
        debuglog(LCF_SDL | LCF_HOOK | LCF_ERROR, "Both SDL versions were detected! Taking SDL1 in priority");
    }

    if (ver.major > 0) {
        SDLver = ver.major;
        debuglog(LCF_SDL | LCF_HOOK, "Detected SDL ", static_cast<int>(ver.major), ".", static_cast<int>(ver.minor), ".", static_cast<int>(ver.patch));
        return SDLver;
    }

    /* If not, determine which library was already dynamically loaded. */
    void *sdl1, *sdl2;
    NATIVECALL(sdl1 = dlopen("libSDL-1.2.so.0", RTLD_NOLOAD));
    NATIVECALL(sdl2 = dlopen("libSDL2-2.0.so.0", RTLD_NOLOAD));
    if (sdl1 && !sdl2) {
        dlclose(sdl1);
        SDLver = 1;
    }
    else if (!sdl1 && sdl2) {
        dlclose(sdl2);
        SDLver = 2;
    }
    else if (sdl1 && sdl2) {
        dlclose(sdl1);
        dlclose(sdl2);
        debuglog(LCF_SDL | LCF_HOOK | LCF_ERROR, "Multiple SDL versions were detected!");
    }
    else {
        debuglog(LCF_SDL | LCF_HOOK | LCF_ERROR, "No SDL versions were detected!");
    }

    return SDLver;
}

}
