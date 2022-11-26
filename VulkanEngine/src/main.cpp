#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

// std
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "vke_application.hpp"

int main() {
    vke::VkeApplication app{};

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}