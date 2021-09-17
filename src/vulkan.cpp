#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "vulkan.h"
#include <iostream>
#include <set>
#include <fstream>
#include <stb_image_write.h>

Vulkan::Vulkan(VulkanSettings settings) :
        settings(settings) {
    createWindow();
    createInstance();
    createSurface();
    pickPhysicalDevice();
    findQueueFamilies();
    createLogicalDevice();

    dynamicDispatchLoader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr, device);

    createCommandPool();
    createSwapChain();
    createRenderTargetImage();
    createSphereBuffer();
    createBottomAccelerationStructure();
    createTopAccelerationStructure();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();
    createPipelineLayout();
    createComputePipeline();
    createRTPipeline();
    createCommandBuffer();
    createFence();
    createSemaphore();
}

Vulkan::~Vulkan() {
    device.destroySemaphore(semaphore);
    device.destroyFence(fence);

    device.destroyPipeline(computePipeline);
    device.destroyPipeline(rtPipeline);
    device.destroyPipelineLayout(computePipelineLayout);
    device.destroyPipelineLayout(rtPipelineLayout);
    device.destroyDescriptorSetLayout(computeDescriptorSetLayout);
    device.destroyDescriptorSetLayout(rtDescriptorSetLayout);
    device.destroyDescriptorPool(computeDescriptorPool);
    device.destroyDescriptorPool(rtDescriptorPool);

    destroyAccelerationStructure(topAccelerationStructure);
    destroyAccelerationStructure(bottomAccelerationStructure);

    destroyBuffer(sphereBuffer);

    device.destroyImageView(swapChainImageView);
    device.destroySwapchainKHR(swapChain);
    device.destroyCommandPool(commandPool);
    destroyImage(renderTargetImage);
    device.destroy();
    instance.destroySurfaceKHR(surface);
    instance.destroy();
    window.destroy();
    vkfw::terminate();
}

void Vulkan::update() {
    vkfw::pollEvents();
}

void Vulkan::render() {
    uint32_t swapChainImageIndex = device.acquireNextImageKHR(swapChain, UINT64_MAX, semaphore).value;

    device.resetFences(fence);

    vk::SubmitInfo submitInfo = {
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
    };

    computeQueue.submit(1, &submitInfo, fence);

    device.waitForFences(1, &fence, true, UINT64_MAX);
    device.resetFences(fence);

    vk::PresentInfoKHR presentInfo = {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &semaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapChain,
            .pImageIndices = &swapChainImageIndex
    };

    graphicsQueue.presentKHR(presentInfo);
}

bool Vulkan::shouldExit() const {
    return window.shouldClose();
}

void Vulkan::createWindow() {
    vkfw::init();

    vkfw::WindowHints hints = {
            .clientAPI = vkfw::ClientAPI::eNone
    };

    window = vkfw::createWindow(settings.windowWidth, settings.windowHeight, "Ray Tracing (Vulkan)", hints);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                                void* pUserData) {
    std::cout << "["
              << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << " | "
              << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>( messageTypes )) << "]:\n"
              << "id      : " << pCallbackData->pMessageIdName << "\n"
              << "message : " << pCallbackData->pMessage << "\n"
              << std::endl;

    return false;
}

void Vulkan::createInstance() {
    vk::ApplicationInfo applicationInfo = {
            .pApplicationName = "Ray Tracing (Vulkan)",
            .applicationVersion = 1,
            .pEngineName = "Ray Tracing (Vulkan)",
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_2
    };

    std::vector<const char*> enabledExtensions;
    enabledExtensions.insert(enabledExtensions.end(), vkfw::getRequiredInstanceExtensions().begin(),
                             vkfw::getRequiredInstanceExtensions().end());
    enabledExtensions.insert(enabledExtensions.end(), requiredInstanceExtensions.begin(),
                             requiredInstanceExtensions.end());

    std::vector<const char*> enabledLayers =
            {"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor", "VK_LAYER_KHRONOS_synchronization2"};

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                           vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral,
            .pfnUserCallback = &debugMessageFunc,
            .pUserData = nullptr
    };

    vk::InstanceCreateInfo instanceCreateInfo = {
            .pNext = &debugMessengerInfo,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
            .ppEnabledLayerNames = enabledLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
            .ppEnabledExtensionNames = enabledExtensions.data(),
    };

    instance = vk::createInstance(instanceCreateInfo);
}

void Vulkan::createSurface() {
    surface = vkfw::createWindowSurface(instance, window);
}

void Vulkan::pickPhysicalDevice() {
    std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    if (physicalDevices.empty()) {
        throw std::runtime_error("No GPU with Vulkan support found!");
    }

    for (const vk::PhysicalDevice &d: physicalDevices) {
        std::vector<vk::ExtensionProperties> availableExtensions = d.enumerateDeviceExtensionProperties();
        std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

        for (const vk::ExtensionProperties &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        if (requiredExtensions.empty()) {
            physicalDevice = d;
            return;
        }
    }

    throw std::runtime_error("No GPU supporting all required features found!");
}

void Vulkan::findQueueFamilies() {
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    bool computeFamilyFound = false;
    bool graphicsFamilyFound = false;

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        bool supportsGraphics = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
                                == vk::QueueFlagBits::eGraphics;
        bool supportsCompute = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
                               == vk::QueueFlagBits::eCompute;
        bool supportsPresenting = physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface);

        if (supportsCompute && !supportsGraphics && !computeFamilyFound) {
            computeQueueFamily = i;
            computeFamilyFound = true;
            continue;
        }

        if (supportsGraphics && supportsPresenting && !graphicsFamilyFound) {
            graphicsQueueFamily = i;
            graphicsFamilyFound = true;
        }

        if (computeFamilyFound && graphicsFamilyFound)
            break;
    }
}

void Vulkan::createLogicalDevice() {
    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {
            {
                    .queueFamilyIndex = graphicsQueueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority
            },
            {
                    .queueFamilyIndex = computeQueueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority
            }
    };

    vk::PhysicalDeviceFeatures deviceFeatures = {};

    vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {
            .bufferDeviceAddress = true,
            .bufferDeviceAddressCaptureReplay = false,
            .bufferDeviceAddressMultiDevice = false
    };

    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {
            .pNext = &bufferDeviceAddressFeatures,
            .rayTracingPipeline = true
    };

    vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {
            .pNext = &rayTracingPipelineFeatures,
            .accelerationStructure = true,
            .accelerationStructureCaptureReplay = true,
            .accelerationStructureIndirectBuild = false,
            .accelerationStructureHostCommands = false,
            .descriptorBindingAccelerationStructureUpdateAfterBind = false
    };

    vk::DeviceCreateInfo deviceCreateInfo = {
            .pNext = &accelerationStructureFeatures,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = requiredDeviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures
    };

    device = physicalDevice.createDevice(deviceCreateInfo);

    graphicsQueue = device.getQueue(graphicsQueueFamily, 0);
    computeQueue = device.getQueue(computeQueueFamily, 0);
}

void Vulkan::createCommandPool() {
    commandPool = device.createCommandPool({.queueFamilyIndex = computeQueueFamily});
}

void Vulkan::createSwapChain() {
    vk::SwapchainCreateInfoKHR swapChainCreateInfo = {
            .surface = surface,
            .minImageCount = 1,
            .imageFormat = swapChainImageFormat,
            .imageColorSpace = colorSpace,
            .imageExtent = {.width = settings.windowWidth, .height = settings.windowHeight},
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = physicalDevice.getSurfaceCapabilitiesKHR(surface).currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = presentMode,
            .clipped = true,
            .oldSwapchain = nullptr
    };

    swapChain = device.createSwapchainKHR(swapChainCreateInfo);

    // swap chain images
    std::vector<vk::Image> swapChainImages = device.getSwapchainImagesKHR(swapChain);
    swapChainImage = swapChainImages.front();
    swapChainImageView = createImageView(swapChainImage, swapChainImageFormat);
}

vk::ImageView Vulkan::createImageView(const vk::Image &image, const vk::Format &format) const {
    return device.createImageView(
            {
                    .image = image,
                    .viewType = vk::ImageViewType::e2D,
                    .format = format,
                    .subresourceRange = {
                            .aspectMask = vk::ImageAspectFlagBits::eColor,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                    }
            });
}

void Vulkan::createDescriptorSetLayout() {

    // COMPUTE PIPELINE
    {
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                {
                        .binding = 0,
                        .descriptorType = vk::DescriptorType::eStorageImage,
                        .descriptorCount = 1,
                        .stageFlags = vk::ShaderStageFlagBits::eCompute
                }
        };

        computeDescriptorSetLayout = device.createDescriptorSetLayout(
                {
                        .bindingCount = static_cast<uint32_t>(bindings.size()),
                        .pBindings = bindings.data()
                });
    }

    // RAY TRACING PIPELINE
    {
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                {
                        .binding = 0,
                        .descriptorType = vk::DescriptorType::eStorageImage,
                        .descriptorCount = 1,
                        .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR
                },
                {
                        .binding = 1,
                        .descriptorType = vk::DescriptorType::eAccelerationStructureKHR,
                        .descriptorCount = 1,
                        .stageFlags = vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR
                }
        };

        rtDescriptorSetLayout = device.createDescriptorSetLayout(
                {
                        .bindingCount = static_cast<uint32_t>(bindings.size()),
                        .pBindings = bindings.data()
                });
    }

}

void Vulkan::createDescriptorPool() {

    // COMPUTE PIPELINE
    {
        std::vector<vk::DescriptorPoolSize> poolSizes = {
                {
                        .type = vk::DescriptorType::eStorageImage,
                        .descriptorCount = 1
                }
        };

        computeDescriptorPool = device.createDescriptorPool(
                {
                        .maxSets = 1,
                        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                        .pPoolSizes = poolSizes.data()
                });
    }

    // RAY TRACING PIPELINE
    {
        std::vector<vk::DescriptorPoolSize> poolSizes = {
                {
                        .type = vk::DescriptorType::eStorageImage,
                        .descriptorCount = 1
                },
                {
                        .type = vk::DescriptorType::eAccelerationStructureKHR,
                        .descriptorCount = 1
                }
        };

        rtDescriptorPool = device.createDescriptorPool(
                {
                        .maxSets = 1,
                        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                        .pPoolSizes = poolSizes.data()
                });
    }

}

void Vulkan::createDescriptorSet() {

    // COMPUTE PIPELINE
    {
        computeDescriptorSet = device.allocateDescriptorSets(
                {
                        .descriptorPool = computeDescriptorPool,
                        .descriptorSetCount = 1,
                        .pSetLayouts = &computeDescriptorSetLayout
                }).front();


        vk::DescriptorImageInfo renderTargetImageInfo = {
                .imageView = renderTargetImage.imageView,
                .imageLayout = vk::ImageLayout::eGeneral
        };

        std::vector<vk::WriteDescriptorSet> descriptorWrites = {
                {
                        .dstSet = computeDescriptorSet,
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eStorageImage,
                        .pImageInfo = &renderTargetImageInfo
                }
        };

        device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
                                    0, nullptr);
    }

    // RAY TRACING PIPELINE
    {
        rtDescriptorSet = device.allocateDescriptorSets(
                {
                        .descriptorPool = rtDescriptorPool,
                        .descriptorSetCount = 1,
                        .pSetLayouts = &rtDescriptorSetLayout
                }).front();


        vk::DescriptorImageInfo renderTargetImageInfo = {
                .imageView = renderTargetImage.imageView,
                .imageLayout = vk::ImageLayout::eGeneral
        };

        vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo = {
                .accelerationStructureCount = 1,
//                .pAccelerationStructures =
        };

        std::vector<vk::WriteDescriptorSet> descriptorWrites = {
                {
                        .dstSet = rtDescriptorSet,
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eStorageImage,
                        .pImageInfo = &renderTargetImageInfo
                },
                {
                        .pNext = &accelerationStructureInfo,
                        .dstSet = rtDescriptorSet,
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = vk::DescriptorType::eAccelerationStructureKHR
                }
        };

//        device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
//                                    0, nullptr);
    }

}

void Vulkan::createPipelineLayout() {
    computePipelineLayout = device.createPipelineLayout(
            {
                    .setLayoutCount = 1,
                    .pSetLayouts = &computeDescriptorSetLayout,
                    .pushConstantRangeCount = 0,
                    .pPushConstantRanges = nullptr
            });

    rtPipelineLayout = device.createPipelineLayout(
            {
                    .setLayoutCount = 1,
                    .pSetLayouts = &rtDescriptorSetLayout,
                    .pushConstantRangeCount = 0,
                    .pPushConstantRanges = nullptr
            });
}

void Vulkan::createComputePipeline() {
    std::vector<char> computeShaderCode = readBinaryFile("shader.comp.spv");

    vk::ShaderModuleCreateInfo shaderModuleCreateInfo = {
            .codeSize = computeShaderCode.size(),
            .pCode = reinterpret_cast<const uint32_t*>(computeShaderCode.data())
    };

    vk::ShaderModule computeShaderModule = device.createShaderModule(shaderModuleCreateInfo);

    vk::PipelineShaderStageCreateInfo shaderStage = {
            .stage = vk::ShaderStageFlagBits::eCompute,
            .module = computeShaderModule,
            .pName = "main",
    };

    vk::ComputePipelineCreateInfo pipelineCreateInfo = {
            .stage = shaderStage,
            .layout = computePipelineLayout
    };

    computePipeline = device.createComputePipeline(nullptr, pipelineCreateInfo).value;

    device.destroyShaderModule(computeShaderModule);
}

void Vulkan::createRTPipeline() {

}

std::vector<char> Vulkan::readBinaryFile(const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("[Error] Failed to open file at '" + path + "'!");

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void Vulkan::createCommandBuffer() {
    commandBuffer = device.allocateCommandBuffers(
            {
                    .commandPool = commandPool,
                    .level = vk::CommandBufferLevel::ePrimary,
                    .commandBufferCount = 1
            }).front();

    vk::CommandBufferBeginInfo beginInfo = {};
    commandBuffer.begin(&beginInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline);

    std::vector<vk::DescriptorSet> descriptorSets = {computeDescriptorSet};
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipelineLayout, 0, descriptorSets,
                                     nullptr);

    // RENDER TARGET IMAGE: UNDEFINED -> GENERAL
    vk::ImageMemoryBarrier barrierRenderTargetToGeneral = getImagePipelineBarrier(
            vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eShaderWrite,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, renderTargetImage.image);

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader,
                                  vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                  0, nullptr, 1, &barrierRenderTargetToGeneral);


    // RENDER TO RENDER TARGET IMAGE
    commandBuffer.dispatch(
            static_cast<uint32_t>(std::ceil(float(settings.windowWidth) / 16.0f)),
            static_cast<uint32_t>(std::ceil(float(settings.windowHeight) / 16.0f)),
            1);


    // RENDER TARGET IMAGE: GENERAL -> TRANSFER SRC
    vk::ImageMemoryBarrier barrierRenderTargetToTransferSrc = getImagePipelineBarrier(
            vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead,
            vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal, renderTargetImage.image);

    // SWAP CHAIN IMAGE: UNDEFINED -> TRANSFER DST
    vk::ImageMemoryBarrier barrierSwapChainToTransferDst = getImagePipelineBarrier(
            vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, swapChainImage);

    vk::ImageMemoryBarrier barriers[2] = {barrierRenderTargetToTransferSrc, barrierSwapChainToTransferDst};

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer,
                                  vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                  0, nullptr, 2, barriers);


    // COPY RENDER TARGET IMAGE TO SWAP CHAIN IMAGE
    vk::ImageSubresourceLayers subresourceLayers = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
    };

    vk::ImageCopy imageCopy = {
            .srcSubresource = subresourceLayers,
            .srcOffset = {0, 0, 0},
            .dstSubresource = subresourceLayers,
            .dstOffset = {0, 0, 0},
            .extent = {
                    .width = settings.windowWidth,
                    .height = settings.windowHeight,
                    .depth = 1
            }
    };

    commandBuffer.copyImage(renderTargetImage.image, vk::ImageLayout::eTransferSrcOptimal, swapChainImage,
                            vk::ImageLayout::eTransferDstOptimal, 1, &imageCopy);


    // SWAP CHAIN IMAGE: TRANSFER DST -> PRESENT
    vk::ImageMemoryBarrier barrierSwapChainToPresent = getImagePipelineBarrier(
            vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead,
            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR, swapChainImage);

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                                  vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                  0, nullptr, 1, &barrierSwapChainToPresent);

    commandBuffer.end();
}

void Vulkan::createFence() {
    fence = device.createFence({});
}

void Vulkan::createSemaphore() {
    semaphore = device.createSemaphore({});
}

uint32_t Vulkan::findMemoryTypeIndex(const uint32_t &memoryTypeBits, const vk::MemoryPropertyFlags &properties) {
    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Unable to find suitable memory type!");
}

vk::ImageMemoryBarrier Vulkan::getImagePipelineBarrier(
        const vk::AccessFlagBits &srcAccessFlags, const vk::AccessFlagBits &dstAccessFlags,
        const vk::ImageLayout &oldLayout, const vk::ImageLayout &newLayout,
        const vk::Image &image) const {

    return {
            .srcAccessMask = srcAccessFlags,
            .dstAccessMask = dstAccessFlags,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = computeQueueFamily,
            .dstQueueFamilyIndex = computeQueueFamily,
            .image = image,
            .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            },
    };
}

VulkanImage Vulkan::createImage(const vk::Format &format, const vk::Flags<vk::ImageUsageFlagBits> &usageFlagBits) {
    vk::ImageCreateInfo imageCreateInfo = {
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {.width = settings.windowWidth, .height = settings.windowHeight, .depth = 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = usageFlagBits,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
    };

    vk::Image image = device.createImage(imageCreateInfo);

    vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(image);

    vk::MemoryAllocateInfo allocateInfo = {
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits,
                                                   vk::MemoryPropertyFlagBits::eDeviceLocal)
    };

    vk::DeviceMemory memory = device.allocateMemory(allocateInfo);

    device.bindImageMemory(image, memory, 0);

    return {
            .image = image,
            .memory = memory,
            .imageView = createImageView(image, format)
    };
}

void Vulkan::createRenderTargetImage() {
    renderTargetImage = createImage(swapChainImageFormat,
                                    vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc);
}

void Vulkan::destroyImage(const VulkanImage &image) const {
    device.destroyImageView(image.imageView);
    device.destroyImage(image.image);
    device.freeMemory(image.memory);
}

VulkanBuffer Vulkan::createBuffer(const vk::DeviceSize &size, const vk::Flags<vk::BufferUsageFlagBits> &usage,
                                  const vk::Flags<vk::MemoryPropertyFlagBits> &memoryProperty) {
    vk::BufferCreateInfo bufferCreateInfo = {
            .size = size,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive
    };

    vk::Buffer buffer = device.createBuffer(bufferCreateInfo);

    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);

    vk::MemoryAllocateFlagsInfo allocateFlagsInfo = {
            .flags = vk::MemoryAllocateFlagBits::eDeviceAddress
    };

    vk::MemoryAllocateInfo allocateInfo = {
            .pNext = &allocateFlagsInfo,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryProperty)
    };

    vk::DeviceMemory memory = device.allocateMemory(allocateInfo);

    device.bindBufferMemory(buffer, memory, 0);

    return {
            .buffer = buffer,
            .memory = memory,
    };
}

void Vulkan::destroyBuffer(const VulkanBuffer &buffer) const {
    device.destroyBuffer(buffer.buffer);
    device.freeMemory(buffer.memory);
}

void Vulkan::executeSingleTimeCommand(const std::function<void(const vk::CommandBuffer &singleTimeCommandBuffer)> &c) {
    vk::CommandBuffer singleTimeCommandBuffer = device.allocateCommandBuffers(
            {
                    .commandPool = commandPool,
                    .level = vk::CommandBufferLevel::ePrimary,
                    .commandBufferCount = 1
            }).front();

    vk::CommandBufferBeginInfo beginInfo = {
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    singleTimeCommandBuffer.begin(&beginInfo);

    c(singleTimeCommandBuffer);

    singleTimeCommandBuffer.end();


    vk::SubmitInfo submitInfo = {
            .commandBufferCount = 1,
            .pCommandBuffers = &singleTimeCommandBuffer
    };

    vk::Fence f = device.createFence({});
    computeQueue.submit(1, &submitInfo, f);
    device.waitForFences(1, &f, true, UINT64_MAX);

    device.destroyFence(f);
    device.freeCommandBuffers(commandPool, singleTimeCommandBuffer);
}

void Vulkan::createSphereBuffer() {
    sphereBuffer = createBuffer(sizeof(SpherePrimitive) * spheres.size(),
                                vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                                vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent |
                                vk::MemoryPropertyFlagBits::eDeviceLocal);
}

void Vulkan::createBottomAccelerationStructure() {
    // ACCELERATION STRUCTURE META INFO
    vk::AccelerationStructureGeometryKHR geometry = {
            .geometryType = vk::GeometryTypeKHR::eAabbs,
            .flags = vk::GeometryFlagBitsKHR::eOpaque
    };

    geometry.geometry.aabbs.sType = vk::StructureType::eAccelerationStructureGeometryAabbsDataKHR;

    vk::AccelerationStructureGeometryAabbsDataKHR geometryAABBsData = geometry.geometry.aabbs;
    geometryAABBsData.stride = sizeof(SpherePrimitive);
    geometryAABBsData.data.deviceAddress = device.getBufferAddress({.buffer = sphereBuffer.buffer})
                                           + offsetof(SpherePrimitive, bbox);


    vk::AccelerationStructureBuildGeometryInfoKHR buildInfo = {
            .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
            .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
            .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
            .srcAccelerationStructure = nullptr,
            .dstAccelerationStructure = nullptr,
            .geometryCount = 1,
            .pGeometries = &geometry,
            .scratchData = {}
    };


    // CALCULATE REQUIRED SIZE FOR THE ACCELERATION STRUCTURE
    std::vector<uint32_t> maxPrimitiveCounts = {static_cast<uint32_t>(spheres.size())};

    vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, maxPrimitiveCounts, dynamicDispatchLoader);


    // ALLOCATE BUFFERS FOR ACCELERATION STRUCTURE
    bottomAccelerationStructure.structureBuffer = createBuffer(buildSizesInfo.accelerationStructureSize,
                                                               vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
                                                               vk::MemoryPropertyFlagBits::eDeviceLocal);

    bottomAccelerationStructure.scratchBuffer = createBuffer(buildSizesInfo.buildScratchSize,
                                                             vk::BufferUsageFlagBits::eStorageBuffer |
                                                             vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                                             vk::MemoryPropertyFlagBits::eDeviceLocal);

    // CREATE THE ACCELERATION STRUCTURE
    vk::AccelerationStructureCreateInfoKHR createInfo = {
            .buffer = bottomAccelerationStructure.structureBuffer.buffer,
            .offset = 0,
            .size = buildSizesInfo.accelerationStructureSize,
            .type = vk::AccelerationStructureTypeKHR::eBottomLevel
    };

    bottomAccelerationStructure.accelerationStructure =
            device.createAccelerationStructureKHR(createInfo, nullptr, dynamicDispatchLoader);


    // FILL IN THE REMAINING META INFO
    buildInfo.dstAccelerationStructure = bottomAccelerationStructure.accelerationStructure;
    buildInfo.scratchData.deviceAddress =
            device.getBufferAddress({.buffer = bottomAccelerationStructure.scratchBuffer.buffer});


    // BUILD THE ACCELERATION STRUCTURE
    vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
            .primitiveCount = static_cast<uint32_t>(spheres.size()),
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0
    };

    const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos[] = {&buildRangeInfo};

    executeSingleTimeCommand([&](const vk::CommandBuffer &singleTimeCommandBuffer) {
        singleTimeCommandBuffer.buildAccelerationStructuresKHR(1, &buildInfo, pBuildRangeInfos, dynamicDispatchLoader);
    });
}

void Vulkan::createTopAccelerationStructure() {
    // ACCELERATION STRUCTURE META INFO
    vk::AccelerationStructureGeometryKHR geometry = {
            .geometryType = vk::GeometryTypeKHR::eInstances,
            .flags = vk::GeometryFlagBitsKHR::eOpaque
    };

    geometry.geometry.instances.sType = vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;

    vk::AccelerationStructureGeometryInstancesDataKHR geometryInstancesData = geometry.geometry.instances;
    geometryInstancesData.arrayOfPointers = false;
    geometryInstancesData.data = {};


    vk::AccelerationStructureBuildGeometryInfoKHR buildInfo = {
            .type = vk::AccelerationStructureTypeKHR::eTopLevel,
            .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
            .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
            .srcAccelerationStructure = nullptr,
            .dstAccelerationStructure = nullptr,
            .geometryCount = 1,
            .pGeometries = &geometry,
            .scratchData = {}
    };


    // CALCULATE REQUIRED SIZE FOR THE ACCELERATION STRUCTURE
    vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = device.getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, buildInfo, {1}, dynamicDispatchLoader);


    // ALLOCATE BUFFERS FOR ACCELERATION STRUCTURE
    topAccelerationStructure.structureBuffer = createBuffer(buildSizesInfo.accelerationStructureSize,
                                                            vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
                                                            vk::MemoryPropertyFlagBits::eDeviceLocal);

    topAccelerationStructure.scratchBuffer = createBuffer(buildSizesInfo.buildScratchSize,
                                                          vk::BufferUsageFlagBits::eStorageBuffer |
                                                          vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                                          vk::MemoryPropertyFlagBits::eDeviceLocal);

    // CREATE THE ACCELERATION STRUCTURE
    vk::AccelerationStructureCreateInfoKHR createInfo = {
            .buffer = topAccelerationStructure.structureBuffer.buffer,
            .offset = 0,
            .size = buildSizesInfo.accelerationStructureSize,
            .type = vk::AccelerationStructureTypeKHR::eTopLevel
    };

    topAccelerationStructure.accelerationStructure =
            device.createAccelerationStructureKHR(createInfo, nullptr, dynamicDispatchLoader);


    // CREATE INSTANCE INFO & WRITE IN NEW BUFFER
    std::array<std::array<float, 4>, 3> matrix = {
            {
                    {1.0f, 0.0f, 0.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f, 0.0f}
            }};

    vk::AccelerationStructureInstanceKHR accelerationStructureInstance = {
            .transform = {.matrix = matrix},
            .mask = 0xFF,
            .instanceShaderBindingTableRecordOffset = 0,
            .accelerationStructureReference = device.getAccelerationStructureAddressKHR(
                    {.accelerationStructure = bottomAccelerationStructure.accelerationStructure},
                    dynamicDispatchLoader),
    };

    topAccelerationStructure.instancesBuffer = createBuffer(
            sizeof(vk::AccelerationStructureInstanceKHR),
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
            vk::BufferUsageFlagBits::eShaderDeviceAddress,
            vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostCoherent |
            vk::MemoryPropertyFlagBits::eHostVisible);

    void* pInstancesBuffer = device.mapMemory(topAccelerationStructure.instancesBuffer.memory, 0,
                                              sizeof(vk::AccelerationStructureInstanceKHR));
    memcpy(pInstancesBuffer, &accelerationStructureInstance, sizeof(vk::AccelerationStructureInstanceKHR));
    device.unmapMemory(topAccelerationStructure.instancesBuffer.memory);


    // FILL IN THE REMAINING META INFO
    buildInfo.dstAccelerationStructure = topAccelerationStructure.accelerationStructure;
    buildInfo.scratchData.deviceAddress = device.getBufferAddress(
            {.buffer = topAccelerationStructure.scratchBuffer.buffer});

    geometryInstancesData.data.deviceAddress = device.getBufferAddress(
            {.buffer = topAccelerationStructure.instancesBuffer.buffer});


    // BUILD THE ACCELERATION STRUCTURE
    vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
            .primitiveCount = 1,
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0
    };

    const vk::AccelerationStructureBuildRangeInfoKHR* pBuildRangeInfos[] = {&buildRangeInfo};

    executeSingleTimeCommand([&](const vk::CommandBuffer &singleTimeCommandBuffer) {
        singleTimeCommandBuffer.buildAccelerationStructuresKHR(1, &buildInfo, pBuildRangeInfos, dynamicDispatchLoader);
    });
}

void Vulkan::destroyAccelerationStructure(const VulkanAccelerationStructure &accelerationStructure) {
    device.destroyAccelerationStructureKHR(accelerationStructure.accelerationStructure, nullptr, dynamicDispatchLoader);
    destroyBuffer(accelerationStructure.structureBuffer);
    destroyBuffer(accelerationStructure.scratchBuffer);
    destroyBuffer(accelerationStructure.instancesBuffer);
}
