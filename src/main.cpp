#include <chrono>
#include <thread>
#include <iostream>
#include "vulkan.h"

int main() {
    VulkanSettings settings = {
            .windowWidth = 1200,
            .windowHeight = 675
    };

    Vulkan vulkan(settings, generateRandomScene());

    auto renderBeginTime = std::chrono::steady_clock::now();

    vulkan.render();

    auto renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - renderBeginTime).count();
    std::cout << "Rendered in " << renderTime << " ms" << std::endl;


    while (!vulkan.shouldExit()) {
        vulkan.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
