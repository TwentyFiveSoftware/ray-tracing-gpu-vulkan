#include <chrono>
#include <thread>
#include <iostream>
#include "vulkan.h"

int main() {
    // SETUP
    const uint32_t renderCalls = 50;
    const uint32_t samples = 10000;

    VulkanSettings settings = {
            .windowWidth = 1920,
            .windowHeight = 1080
    };

    Vulkan vulkan(settings, generateRandomScene());


    // RENDERING
    auto renderBeginTime = std::chrono::steady_clock::now();

    for (uint32_t number = 1; number <= renderCalls; number++) {
        RenderCallInfo renderCallInfo = {
                .number = number,
                .totalRenderCalls = renderCalls,
                .totalSamples = samples
        };

        std::cout << "Render call " << number << " / " << renderCalls << " (" << (number * samples / renderCalls)
                  << " / " << samples << " samples)";
        auto renderCallBeginTime = std::chrono::steady_clock::now();

        vulkan.render(renderCallInfo);

        auto renderCallTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - renderCallBeginTime).count();
        std::cout << " - Completed in " << renderCallTime << " ms" << std::endl;

        vulkan.update();
    }

    auto renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - renderBeginTime).count();
    std::cout << "Rendering completed: " << samples << " samples rendered in " << renderTime << " ms"
              << std::endl << std::endl;


    // WINDOW
    while (!vulkan.shouldExit()) {
        vulkan.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
