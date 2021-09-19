#include "scene.h"
#include <random>

float randomFloat(float min, float max) {
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(engine);
}

float randomFloat() {
    return randomFloat(0.0f, 1.0f);
}

Scene generateRandomScene() {
    Scene scene = {};

    scene.spheres[0] = {
            .geometry = glm::vec4(0.0f, -1000.0f, 1.0f, 1000.0f),
            .materialType = MaterialType::DIFFUSE,
            .textureType = TextureType::CHECKERED,
            .colors = {glm::vec4(0.05f, 0.05f, 0.05f, 1.0f), glm::vec4(0.95f, 0.95f, 0.95f, 1.0f)},
            .materialSpecificAttribute = 0.0f
    };

    scene.spheres[1] = {
            .geometry = glm::vec4(-4.0f, 1.0f, 0.0f, 1.0f),
            .materialType = MaterialType::DIFFUSE,
            .textureType = TextureType::SOLID,
            .colors = {glm::vec4(0.4f, 0.2f, 0.1f, 1.0f)},
            .materialSpecificAttribute = 0.0f
    };

    scene.spheres[2] = {
            .geometry = glm::vec4(4.0f, 1.0f, 0.0f, 1.0f),
            .materialType = MaterialType::METAL,
            .textureType = TextureType::SOLID,
            .colors = {glm::vec4(0.7f, 0.6f, 0.5f, 1.0f)},
            .materialSpecificAttribute = 0.0f
    };

    scene.spheres[3] = {
            .geometry = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
            .materialType = MaterialType::REFRACTIVE,
            .textureType = TextureType::SOLID,
            .colors = {glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
            .materialSpecificAttribute = 1.5f
    };

    uint32_t sphereIndex = 4;

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            scene.spheres[sphereIndex].geometry =
                    glm::vec4(float(a) + 0.9f * randomFloat(), 0.2f, float(b) + 0.9f * randomFloat(), 0.2f);

            const float materialProbability = randomFloat();

            if (materialProbability < 0.8) {
                scene.spheres[sphereIndex].materialType = MaterialType::DIFFUSE;
                scene.spheres[sphereIndex].textureType = TextureType::SOLID;
                scene.spheres[sphereIndex].colors[0] =
                        glm::vec4(randomFloat() * randomFloat(), randomFloat() * randomFloat(),
                                  randomFloat() * randomFloat(), 1.0f);
                scene.spheres[sphereIndex].materialSpecificAttribute = 0.0f;

            } else if (materialProbability < 0.95) {
                scene.spheres[sphereIndex].materialType = MaterialType::METAL;
                scene.spheres[sphereIndex].textureType = TextureType::SOLID;
                scene.spheres[sphereIndex].colors[0] =
                        glm::vec4(randomFloat() * randomFloat(), randomFloat() * randomFloat(),
                                  randomFloat() * randomFloat(), 1.0f);
                scene.spheres[sphereIndex].materialSpecificAttribute = randomFloat(0.0f, 0.5f);

            } else {
                scene.spheres[sphereIndex].materialType = MaterialType::REFRACTIVE;
                scene.spheres[sphereIndex].textureType = TextureType::SOLID;
                scene.spheres[sphereIndex].colors[0] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                scene.spheres[sphereIndex].materialSpecificAttribute = 1.5f;

            }

            sphereIndex++;
        }
    }

    scene.sphereAmount = sphereIndex;
    return scene;
}
