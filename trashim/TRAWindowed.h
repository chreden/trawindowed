#pragma once

#include <cstdint>

namespace trashim
{
    // Enables windowed mode for Tomb Raider Legend and Anniversary.
    void initialise_shim(HWND window, uint32_t back_buffer_width, uint32_t back_buffer_height, uint32_t display_width, uint32_t display_height, bool vsync, uint32_t framerate);
    void update_shim(uint32_t back_buffer_width, uint32_t back_buffer_height, uint32_t display_width, uint32_t display_height, bool vsync, uint32_t framerate);

    // Sleeps for the appropriate amount of time to emulate a fullscreen fixed framerate.
    // If vsync is disabled, this will immediately return.
    void wait_for_frame();

    void start_fps_emulation();
}