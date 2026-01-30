#include <iostream>
#include <memory>
#include <vector>

#include <lvk/LVK.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

int main()
{
	minilog::LogConfig configInfo{};
	configInfo.threadNames = false;
	minilog::initialize(nullptr, configInfo);

	int width = -65;
	int height = -90;

	GLFWwindow* window = lvk::initWindow("Example", width, height);

	// Context
	std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});

	// Shaders

	// Pipelines

	// Render Loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glfwGetFramebufferSize(window, &width, &height);

		if (!width || !height)
			continue;

		// Command buffer
	}

	ctx.reset();

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}