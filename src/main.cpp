#include <chrono>
#include <thread>
#include "vulkan.h"

int main() {
    VulkanSettings settings = {
            .windowWidth = 1200,
            .windowHeight = 675
    };

    Vulkan vulkan(settings, generateRandomScene());


    while (!vulkan.shouldExit()) {
        vulkan.render();
        vulkan.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
