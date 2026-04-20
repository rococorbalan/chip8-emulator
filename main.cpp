#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include "globals.h"
#include <SDL3/SDL_main.h>
#include <iostream>
#include <chrono>
#include "instructions.hpp"

/* We will use this renderer to draw into this window every frame. */
static SDL_AudioStream *stream = NULL;
static int current_sine_sample = 0;

using namespace std;

Machine m;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_AudioSpec spec;

    srand(time(NULL)); // Seed rand

    if (argc < 2) {
        SDL_Log("Usage: ./main <rom path>");
        return SDL_APP_FAILURE;
    }

    // Load ROM to memory from argv
    FILE *rom = fopen(argv[1], "rb");
    fread(m.memory + 0x200, 1, 0x1000 - 0x200, rom);
    fclose(rom);

    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/renderer/clear", 640, 320, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    spec.channels = 1;
    spec.format = SDL_AUDIO_F32;
    spec.freq = 8000;
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!stream) {
        SDL_Log("Couldn't create audio stream: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    /* SDL_OpenAudioDeviceStream starts the device paused. You have to tell it to start! */
    SDL_ResumeAudioStreamDevice(stream);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP) {
        handle_input(event, m);
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    uint64_t now = SDL_GetTicks(); // milliseconds since start

    if (now - m.last_tick >= 1000 / 60) { // ~16ms per tick
        if (m.delay_timer > 0) m.delay_timer--;
        if (m.sound_timer > 0) m.sound_timer--;
        if (m.sound_timer == 0) SDL_FlushAudioStream(stream); // flush audio immediately
        m.last_tick = now;
    }

    const int minimum_audio = (512 * sizeof(float)) / 2;
    if (SDL_GetAudioStreamQueued(stream) < minimum_audio && m.sound_timer > 0) {
        static float samples[512];
        for (int i = 0; i < SDL_arraysize(samples); i++) {
            const int freq = 440;
            const float phase = current_sine_sample * freq / 8000.0f;
            samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
            current_sine_sample++;
        }
        current_sine_sample %= 8000;
        SDL_PutAudioStreamData(stream, samples, sizeof(samples));
    }

    if (now - m.last_cpu_tick >= 1000 / 700) {
        uint16_t instruction = m.fetch();

        decode(instruction, m);

        m.last_cpu_tick = now;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* new color, full alpha. */

    if(m.display_dirty) {
        /* clear the window to the draw color. */
        SDL_RenderClear(renderer);
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 64; x++) {
                if (m.display[y * 64 + x]) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // on
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // off
                }
                SDL_RenderPoint(renderer, x, y);
            }
        }
        /* put the newly-cleared rendering on the screen. */
        SDL_RenderPresent(renderer);
        m.display_dirty = false;
    }
    

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}