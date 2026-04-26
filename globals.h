// globals.h
#pragma once
#include <SDL3/SDL.h>
#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_sdl3.h"
#include "imgui/backend/imgui_impl_sdlrenderer3.h"

extern SDL_Renderer *renderer;
extern SDL_Window *window;