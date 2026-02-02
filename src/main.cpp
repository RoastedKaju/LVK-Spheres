#include <iostream>
#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_set>

#include <lvk/LVK.h>
#include <lvk/HelpersImGui.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "shader_processor.h"
#include "sphere_data.h"

#include <vulkan/vulkan.h>

enum class SphereType
{
	UVSphere = 0,
	IcoSphere = 1
};
SphereType currentSphereType = SphereType::UVSphere;
static const char* RenderShapeNames[] =
{
	"UV-Sphere",
	"IcoSphere"
};

void showUI(
	lvk::ImGuiRenderer& imgui,
	lvk::Framebuffer& framebuff,
	lvk::ICommandBuffer& cmdBuff,
	uint32_t& wireframe,
	bool& showSolid)
{
	bool wireframeState = wireframe;
	imgui.beginFrame(framebuff);
	ImGui::Begin("Render Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Checkbox("Wireframe", &wireframeState);
	ImGui::Checkbox("Solid Mode", &showSolid);
	int current = (int)currentSphereType;
	if (ImGui::Combo("Render Mode", &current, RenderShapeNames, 2))
	{
		currentSphereType = (SphereType)current;
	}
	ImGui::End();
	imgui.endFrame(cmdBuff);
	wireframe = (uint32_t)wireframeState;
}

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

		// ImGui Context
		std::unique_ptr<lvk::ImGuiRenderer> imgui = std::make_unique<lvk::ImGuiRenderer>(*ctx, FONTS_DIR"/Terminal.ttf", 13.0f);



		// Mouse callbacks
		glfwSetCursorPosCallback(window, [](auto* window, double x, double y) { ImGui::GetIO().MousePos = ImVec2(x, y); });
		glfwSetMouseButtonCallback(window, [](auto* window, int button, int action, int mods) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
				? ImGuiMouseButton_Left
				: (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = ImVec2((float)xpos, (float)ypos);
			io.MouseDown[imguiButton] = action == GLFW_PRESS;
			});

		// Vertex and index buffer along with mesh data
		std::vector<Vertex> verts;
		std::vector<uint32_t> indices;
		std::vector<Vertex> vertsIco;
		std::vector<uint32_t> indicesIco;
		// Generate UV sphere
		generateUVSphere(1.0f, 32, 64, verts, indices);
		// Vertex buffer
		lvk::BufferDesc vertBufDesc{};
		vertBufDesc.usage = lvk::BufferUsageBits_Vertex;
		vertBufDesc.storage = lvk::StorageType_Device;
		vertBufDesc.size = sizeof(Vertex) * verts.size();
		vertBufDesc.data = verts.data();
		vertBufDesc.debugName = "Buffer: vertex";
		lvk::Holder<lvk::BufferHandle> vertexBuffer = ctx->createBuffer(vertBufDesc);
		// Index Buffer
		lvk::BufferDesc indexBufDes{};
		indexBufDes.usage = lvk::BufferUsageBits_Index;
		indexBufDes.storage = lvk::StorageType_Device;
		indexBufDes.size = sizeof(uint32_t) * indices.size();
		indexBufDes.data = indices.data();
		indexBufDes.debugName = "Buffer: index";
		lvk::Holder<lvk::BufferHandle> indexBuffer = ctx->createBuffer(indexBufDes);
		// Generate Ico sphere
		generateIcoSphere(1.0f, 3, vertsIco, indicesIco);
		// Vertex buffer Ico
		lvk::BufferDesc vertBufIcoDesc{};
		vertBufIcoDesc.usage = lvk::BufferUsageBits_Vertex;
		vertBufIcoDesc.storage = lvk::StorageType_Device;
		vertBufIcoDesc.size = sizeof(Vertex) * vertsIco.size();
		vertBufIcoDesc.data = vertsIco.data();
		vertBufIcoDesc.debugName = "Buffer: vertex";
		lvk::Holder<lvk::BufferHandle> vertexBufferIco = ctx->createBuffer(vertBufIcoDesc);
		// Index Buffer Ico
		lvk::BufferDesc indexBufIcoDes{};
		indexBufIcoDes.usage = lvk::BufferUsageBits_Index;
		indexBufIcoDes.storage = lvk::StorageType_Device;
		indexBufIcoDes.size = sizeof(uint32_t) * indicesIco.size();
		indexBufIcoDes.data = indicesIco.data();
		indexBufIcoDes.debugName = "Buffer: index";
		lvk::Holder<lvk::BufferHandle> indexBufferIco = ctx->createBuffer(indexBufIcoDes);

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

		// Attributes
		const lvk::VertexInput vdesc = {
			.attributes = {
				{
					.location = 0,
					.format = lvk::VertexFormat::Float3,
					.offset = offsetof(Vertex, position)
				}
			},
			.inputBindings = { {.stride = sizeof(Vertex) } }
		};

		// Pipelines
		bool showSolid = true;
		lvk::RenderPipelineDesc pipelineDesc{};
		pipelineDesc.vertexInput = vdesc;
		pipelineDesc.smVert = vert;
		pipelineDesc.smFrag = frag;
		pipelineDesc.color[0].format = ctx->getSwapchainFormat();
		pipelineDesc.depthFormat = ctx->getFormat(depthTexture);

		lvk::Holder<lvk::RenderPipelineHandle> soildPipeline = ctx->createRenderPipeline(pipelineDesc);

		// Wireframe
		uint32_t isWireframe = 1;
		lvk::SpecializationConstantEntry wireframeSpecInfoEntry{};
		wireframeSpecInfoEntry.constantId = 0;
		wireframeSpecInfoEntry.size = sizeof(uint32_t);

		lvk::RenderPipelineDesc wireframePipelineDesc{};
		wireframePipelineDesc.vertexInput = vdesc;
		wireframePipelineDesc.smVert = vert;
		wireframePipelineDesc.smFrag = frag;
		wireframePipelineDesc.color[0].format = ctx->getSwapchainFormat();
		wireframePipelineDesc.depthFormat = ctx->getFormat(depthTexture);
		wireframePipelineDesc.polygonMode = lvk::PolygonMode_Line;
		wireframePipelineDesc.specInfo.entries[0] = wireframeSpecInfoEntry;
		wireframePipelineDesc.specInfo.data = &isWireframe;
		wireframePipelineDesc.specInfo.dataSize = sizeof(isWireframe);

		lvk::Holder<lvk::RenderPipelineHandle> wireframePipeline = ctx->createRenderPipeline(wireframePipelineDesc);

		LVK_ASSERT(soildPipeline.valid());
		LVK_ASSERT(wireframePipeline.valid());

		// Render Loop
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			glfwGetFramebufferSize(window, &width, &height);

			if (!width || !height)
				continue;

			const float ratio = width / (float)height;

			glm::mat4 model = glm::mat4(1.0f);          // identity
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians((float)glfwGetTime() * 15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

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
			} pc = { .mvp = p * v * model };

			// Command buffer
			lvk::ICommandBuffer& buff = ctx->acquireCommandBuffer();
			// Begin Rendering
			buff.cmdBeginRendering(renderPass, framebuffer);
			buff.cmdPushDebugGroupLabel("Render Triangle", 0xff0000ff);
			{
				// Bindings
				if (currentSphereType == SphereType::UVSphere)
				{
					buff.cmdBindVertexBuffer(0, vertexBuffer);
					buff.cmdBindIndexBuffer(indexBuffer, lvk::IndexFormat_UI32);
				}
				else
				{
					buff.cmdBindVertexBuffer(0, vertexBufferIco);
					buff.cmdBindIndexBuffer(indexBufferIco, lvk::IndexFormat_UI32);
				}

				// Bind Soild Pipeline
				if (showSolid)
				{
					buff.cmdBindRenderPipeline(soildPipeline);
					buff.cmdBindDepthState({ .compareOp = lvk::CompareOp_Less, .isDepthWriteEnabled = true });
					buff.cmdPushConstants(pc);
					if (currentSphereType == SphereType::UVSphere)
						buff.cmdDrawIndexed(indices.size());
					else
						buff.cmdDrawIndexed(indicesIco.size());
				}
				// Bind Wireframe Pipeline
				if (isWireframe)
				{
					buff.cmdBindRenderPipeline(wireframePipeline);
					// If we have skipped the upper pipeline then add push constants and depth state here
					if (!showSolid)
					{
						buff.cmdPushConstants(pc);
					}
					buff.cmdSetDepthBiasEnable(true);
					buff.cmdSetDepthBias(0.0f, -1.0f, 0.0f);
					if (currentSphereType == SphereType::UVSphere)
						buff.cmdDrawIndexed(indices.size());
					else
						buff.cmdDrawIndexed(indicesIco.size());
				}
				// UI
				showUI(*imgui, framebuffer, buff, isWireframe, showSolid);
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