#include <iostream>
#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_set>

#include <lvk/LVK.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "shader_processor.h"

int main()
{
	minilog::LogConfig configInfo{};
	configInfo.threadNames = false;
	minilog::initialize(nullptr, configInfo);

	int width = -65;
	int height = -90;

	GLFWwindow* window = lvk::initWindow("Example", width, height);

	// LVK Holder scope
	{
		// Context
		std::unique_ptr<lvk::IContext> ctx = lvk::createVulkanContextWithSwapchain(window, width, height, {});

		// Shaders
		lvk::Holder<lvk::ShaderModuleHandle> vert = loadShaderModule(ctx, std::filesystem::absolute(SHADER_DIR"/main.vert"));
		lvk::Holder<lvk::ShaderModuleHandle> frag = loadShaderModule(ctx, std::filesystem::absolute(SHADER_DIR"/main.frag"));

		// Pipelines
		lvk::RenderPipelineDesc pipelineDesc{};
		pipelineDesc.smVert = vert;
		pipelineDesc.smFrag = frag;
		pipelineDesc.color[0].format = ctx->getSwapchainFormat();

		lvk::Holder<lvk::RenderPipelineHandle> soildPipeline = ctx->createRenderPipeline(pipelineDesc);

		// Render Loop
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			glfwGetFramebufferSize(window, &width, &height);

			if (!width || !height)
				continue;

			// Command buffer
			lvk::ICommandBuffer& buff = ctx->acquireCommandBuffer();

			lvk::RenderPass renderPass;
			renderPass.color[0].loadOp = lvk::LoadOp_Clear;
			renderPass.color[0].clearColor[0] = 0.5f;
			renderPass.color[0].clearColor[1] = 0.5f;
			renderPass.color[0].clearColor[2] = 0.5f;
			renderPass.color[0].clearColor[3] = 1.0f;
			// Frame buffer
			lvk::Framebuffer framebuffer;
			framebuffer.color[0].texture = ctx->getCurrentSwapchainTexture();

			// Begin Rendering
			buff.cmdBeginRendering(renderPass, framebuffer);

			// Bindings
			buff.cmdBindRenderPipeline(soildPipeline);
			buff.cmdPushDebugGroupLabel("Render Triangle", 0xff0000ff);
			// Draw elements
			buff.cmdDraw(3);
			buff.cmdPopDebugGroupLabel();
			buff.cmdEndRendering();

			// Submission
			ctx->submit(buff, ctx->getCurrentSwapchainTexture());
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}