#include <iostream>
#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_set>

#include <lvk/LVK.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

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

		// Depth texture
		lvk::TextureDesc depthTexDesc{};
		depthTexDesc.type = lvk::TextureType_2D;
		depthTexDesc.format = lvk::Format_Z_F32;
		depthTexDesc.dimensions = { (uint32_t)width, (uint32_t)height };
		depthTexDesc.usage = lvk::TextureUsageBits_Attachment;
		depthTexDesc.debugName = "Depth Buffer";
		lvk::Holder<lvk::TextureHandle> depthTexture = ctx->createTexture(depthTexDesc);

		// Pipelines
		lvk::RenderPipelineDesc pipelineDesc{};
		pipelineDesc.smVert = vert;
		pipelineDesc.smFrag = frag;
		pipelineDesc.color[0].format = ctx->getSwapchainFormat();
		pipelineDesc.depthFormat = ctx->getFormat(depthTexture);

		lvk::Holder<lvk::RenderPipelineHandle> soildPipeline = ctx->createRenderPipeline(pipelineDesc);

		LVK_ASSERT(soildPipeline.valid());

		// Render Loop
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			glfwGetFramebufferSize(window, &width, &height);

			if (!width || !height)
				continue;

			const float ratio = width / (float)height;

			const glm::mat4 m = glm::mat4(1.0f);
			const glm::mat4 v = glm::lookAt(
				glm::vec3(0.0f, 1.0f, 3.0f),   // camera position
				glm::vec3(0.0f, 0.0f, 0.0f),   // look at model
				glm::vec3(0.0f, 1.0f, 0.0f)    // up direction
			);
			const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);

			lvk::RenderPass renderPass;
			renderPass.color[0].loadOp = lvk::LoadOp_Clear;
			renderPass.color[0].clearColor[0] = 0.5f;
			renderPass.color[0].clearColor[1] = 0.5f;
			renderPass.color[0].clearColor[2] = 0.5f;
			renderPass.color[0].clearColor[3] = 1.0f;
			renderPass.depth.loadOp = lvk::LoadOp_Clear; // Depth
			renderPass.depth.clearDepth = 1.0f;

			// Frame buffer
			lvk::Framebuffer framebuffer;
			framebuffer.color[0].texture = ctx->getCurrentSwapchainTexture();
			framebuffer.depthStencil.texture = depthTexture;

			// Perframe data
			const struct PerFrameData
			{
				glm::mat4 mvp;
			} pc = { .mvp = p * v * m };

			// Command buffer
			lvk::ICommandBuffer& buff = ctx->acquireCommandBuffer();
			// Begin Rendering
			buff.cmdBeginRendering(renderPass, framebuffer);
			buff.cmdPushDebugGroupLabel("Render Triangle", 0xff0000ff);
			{
				// Bindings
				buff.cmdBindRenderPipeline(soildPipeline);
				buff.cmdBindDepthState({ .compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true });
				buff.cmdPushConstants(pc);
				// Draw elements
				buff.cmdDraw(3);
			}
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