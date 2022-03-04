#include <iostream>
#define VMA_IMPLEMENTATION
#include <VulkanToolbox.hpp>
#include <VkBootstrap.h>

inline glm::vec3 GetSurfaceNormal(glm::vec3 V1, glm::vec3 V2, glm::vec3 V3)
{
	return glm::cross((V2 - V1), (V3 - V2));
}

struct VertexData
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;


	static std::vector<vk::VertexInputBindingDescription> vertexInputBinding()
	{
		std::vector<vk::VertexInputBindingDescription> bindings;
		bindings.emplace_back(vk::VertexInputBindingDescription(0, sizeof(VertexData), vk::VertexInputRate::eVertex));
		return bindings;
	}
	static std::vector<vk::VertexInputAttributeDescription> vertexInputAttribute()
	{
		std::vector<vk::VertexInputAttributeDescription> attributes;
		attributes.emplace_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, pos)));
		attributes.emplace_back(vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, color)));
		attributes.emplace_back(vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, normal)));
		return attributes;
	}
};
struct Triangle
{
	glm::vec3 left = glm::vec3(-1, 1, 0);
	glm::vec3 top = glm::vec3(0, -1, 0);
	glm::vec3 right = glm::vec3(1.0f, 1.0f, 0.0f);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	std::vector<VertexData> vertices = {
		{right, color, GetSurfaceNormal(right, top, left)},
		{top, color, GetSurfaceNormal(right, top, left)},
		{left, color, GetSurfaceNormal(right, top, left)}
	};
};

int main()
{
	vkt::ObjectManager vom({}, false, true);
	uint32_t width = 500;
	uint32_t height = 500;
	vkt::VulkanWindow window(width, height);


	//Initialization
	{
		vkb::Instance bootInstance;
		vkb::PhysicalDevice bootPDevice;
		vkb::Device bootDevice;

		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder.set_app_name("VMLTester")
			.set_engine_name("Tester")
			.require_api_version(1, 3);
#ifndef NDEBUG
		instanceBuilder.enable_validation_layers();
#endif


		auto bootInstanceReturn = instanceBuilder.build();
		if (!bootInstanceReturn)
		{
			std::cout << "ERROR CODE OCCURRED:" << bootInstanceReturn.vk_result() << std::endl;
			abort();
		}
		bootInstance = bootInstanceReturn.value();
		window.CreateSurface(bootInstance.instance);
		bootDevice.surface = window.surface;

		vkb::PhysicalDeviceSelector physicalDeviceSelector(bootInstance);
		VkPhysicalDeviceFeatures features{};
		VkPhysicalDeviceVulkan11Features features11{};
		VkPhysicalDeviceVulkan12Features features12{};
		VkPhysicalDeviceVulkan13Features features13{}; features13.dynamicRendering = VK_TRUE;

		auto pDeviceRet = physicalDeviceSelector
		.set_surface(window.surface)
		.add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
		.add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
		.set_required_features(features)
		.set_required_features_11(features11)
		.set_required_features_12(features12)
		.set_required_features_13(features13)
		.require_present()
		.select();

		std::cout << "Selected device named: " << pDeviceRet->properties.deviceName << "\n";
		if (!pDeviceRet)
		{
			std::cout << "ERROR CODE OCCURRED:" << pDeviceRet.vk_result() << std::endl;
			throw std::logic_error(" ");
		}
		bootPDevice = pDeviceRet.value();

		vkb::DeviceBuilder deviceBuilder{ bootPDevice };
		auto lDeviceRet = deviceBuilder.build();
		if (!lDeviceRet)
		{
			std::cout << "ERROR CODE OCCURRED:" << lDeviceRet.vk_result() << std::endl;
			throw std::logic_error(" ");
		}
		bootDevice = lDeviceRet.value();

		vom.SetDevice(bootDevice.device);
		vom.Manage(vom.GetDevice());
		vom.SetInstance(bootInstanceReturn.value().instance);
		vom.Manage(bootInstanceReturn.value().instance);
		vom.SetSurface(window.surface);
		vom.Manage(window.surface);
		vom.SetPhysicalDevice(pDeviceRet.value().physical_device);

		vom.MakeAllocator(VK_API_VERSION_1_2, true, true);

		auto graphicsQueueRet = bootDevice.get_queue(vkb::QueueType::graphics);
		if (!graphicsQueueRet)
		{
			std::cout << "No Graphics Queue for you!! HAHAHA" << std::endl;
			throw std::logic_error(" ");
		}
		auto transferQueueRet = bootDevice.get_queue(vkb::QueueType::transfer);
		if (!transferQueueRet)
		{
			std::cout << "No Transfer Queue for you!! HAHAHA" << std::endl;
			throw std::logic_error(" ");
		}
		auto computeQueueRet = bootDevice.get_queue(vkb::QueueType::compute);
		if (!computeQueueRet)
		{
			std::cout << "No Compute Queue for you!! HAHAHA" << std::endl;
			throw std::logic_error(" ");
		}

		vom.SetQueues({ bootDevice.get_queue_index(vkb::QueueType::graphics).value(), graphicsQueueRet.value() },
			{ bootDevice.get_queue_index(vkb::QueueType::transfer).value(), transferQueueRet.value() },
			{ bootDevice.get_queue_index(vkb::QueueType::compute).value(), computeQueueRet.value() });
	}
	
	//Game scope
	{
		vkt::ObjectManager pVom(vom);
		pVom.SetSurface(vom.GetSurface());
		pVom.SetPhysicalDevice(vom.GetPhysicalDevice());
		pVom.SetGraphicsQueue(vom.GetGraphicsQueue());
		vk::Pipeline graphicsPipeline;
		vk::PipelineLayoutCreateInfo gPipelineLayoutCreateInfo;
		vk::PipelineLayout gPipelineLayout;
		std::shared_ptr<uint64_t> resizeCount = std::make_shared<uint64_t>(0);



		auto BuildPresentationData = [&pVom, &resizeCount](GLFWwindow* window,int,int)
		{
			if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
			{
				*resizeCount = *resizeCount + 1;
				vkb::SwapchainBuilder swapchainBuilder(pVom.GetPhysicalDevice(), pVom.GetDevice(), pVom.GetSurface());
				auto ret = swapchainBuilder.set_old_swapchain(pVom.GetSwapchainData(true).GetSwapchain()).build();
				std::cout<<"Shaw chain builder chose format: " << ret->image_format << std::endl;
				pVom.DestroyAll();
				assert(ret);
				pVom.SetSwapchain(ret->swapchain, static_cast<vk::Format>(ret->image_format), ret->extent, true);
			}
		};
		auto BuildGraphicsPipeline = [&pVom, &graphicsPipeline, &gPipelineLayoutCreateInfo, &gPipelineLayout](GLFWwindow* window,int,int)
		{
			if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
			{
				auto vert = pVom.MakeShaderModule("shaders/Vertex.spv");
				auto frag = pVom.MakeShaderModule("shaders/Fragment.spv");

				std::vector<vk::PipelineShaderStageCreateInfo> shaders = {
					vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex, vert, "main"),
					vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, frag, "main") };

				auto vertexBindings = VertexData::vertexInputBinding();
				auto vertexAttributes = VertexData::vertexInputAttribute();

				vk::PipelineVertexInputStateCreateInfo vertexState(
					{},
					vertexBindings.size(),
					vertexBindings.data(),
					vertexAttributes.size(),
					vertexAttributes.data());
				vk::PipelineInputAssemblyStateCreateInfo assemblyState(
					{},
					vk::PrimitiveTopology::eTriangleList,
					VK_FALSE);
				vk::PipelineTessellationStateCreateInfo tessellationState;

				vk::Viewport renderViewport(0, 0, pVom.GetSwapchainData().extent.width, pVom.GetSwapchainData().GetExtent().height, 0, 1);
				vk::Rect2D scissor(0, pVom.GetSwapchainData().GetExtent());

				vk::PipelineViewportStateCreateInfo viewportState(
					{},
					1,
					& renderViewport,
					1,
					& scissor);
				vk::PipelineRasterizationStateCreateInfo rasterizationState(
					{},
					VK_FALSE,
					VK_FALSE,
					vk::PolygonMode::eFill,
					vk::CullModeFlagBits::eBack,
					vk::FrontFace::eCounterClockwise,
					VK_FALSE,
					{},
					{},
					{},
					1);

				vk::PipelineMultisampleStateCreateInfo multisampleState(
					{},
					vk::SampleCountFlagBits::e1,
					VK_FALSE,
					1.0f,
					nullptr,
					VK_FALSE,
					VK_FALSE
				);

				vk::PipelineDepthStencilStateCreateInfo depthStencilState(
					{},
					VK_TRUE,
					VK_TRUE,
					vk::CompareOp::eLess,
					VK_FALSE,
					VK_FALSE,
					{},
					{},
					0,
					1.0f
				);

				gPipelineLayout = pVom.MakePipelineLayout(gPipelineLayoutCreateInfo);

				vk::Format colorFormat = pVom.GetSwapchainData().GetFormat();
				vk::PipelineRenderingCreateInfo dynamicRenderState(
					0,
					1,
					&colorFormat,
					depthImage.imageFormat,
					vk::Format::eUndefined);

				vk::PipelineColorBlendAttachmentState blendAttachment(VK_FALSE, {}, {}, {}, {}, {}, {}, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
				vk::PipelineColorBlendStateCreateInfo colorBlendState({}, VK_FALSE, {}, 1, & blendAttachment);

				vk::GraphicsPipelineCreateInfo gCreateInfo(
					{},
					shaders.size(),
					shaders.data(),
					& vertexState,
					& assemblyState,
					& tessellationState,
					& viewportState,
					& rasterizationState,
					& multisampleState,
					& depthStencilState,
					& colorBlendState,
					{},
					gPipelineLayout,
					{},
					0,
					{},
					{}
				);

				gCreateInfo.pNext = &dynamicRenderState;

				graphicsPipeline = pVom.MakePipeline({}, gCreateInfo);
			}
		};
		window.AttachResizeAction(BuildPresentationData);
		window.AttachResizeAction(BuildPresentaionDependentDescrtiptors);
		window.AttachResizeAction(BuildGraphicsPipeline);

		BuildPresentationData(window.window, {}, {});


		vkt::BufferManager gpuStorage(vom, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_GPU_ONLY);
		vkt::BufferManager gpuUniformStorage(vom, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_GPU_ONLY);
		vkt::BufferManager vboStorage(vom, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_GPU_ONLY);
		auto positionsSector = gpuStorage.GetSector();
		positionsSector->neededSize = sizeof(Translation) * objectCount;
		auto staticsSector = gpuStorage.GetSector();
		staticsSector->neededSize = sizeof(glm::vec4) * countData.staticsCount;
		auto matrixSector = gpuStorage.GetSector();
		matrixSector->neededSize = sizeof(glm::mat4) * objectCount;
		auto camdataSector = gpuUniformStorage.GetSector();
		camdataSector->neededSize = sizeof(CamData);
		auto modelMatrixSector = gpuStorage.GetSector();
		modelMatrixSector->neededSize = sizeof(glm::mat4) * countData.objectCount;

		auto vbo = vboStorage.GetSector();

		//Memory Transfer scope
		{
			vkt::MemoryOperationsBuffer ops(vom);
			ops.RamToSector(objectData.vertices.data(), vbo, sizeof(objectData.vertices[0])* objectData.vertices.size());
			gpuStorage.Update(true);
			vboStorage.Update(true);
			gpuUniformStorage.Update(true);
			ops.Execute({}, true);
		}

		//Random set scope
		{
			vkt::DescriptorManager randomGenDescPool(vom);
			auto randomGenDescSet = randomGenDescPool.GetNewSet();
			randomGenDescSet->AttachSector(staticsSector, vk::ShaderStageFlagBits::eCompute);
			randomGenDescPool.Update();

			
			vkt::ComputePipelineManager randomGenStage(vom, vk::PipelineLayoutCreateInfo({}, 1, &randomGenDescSet->layout, 1, &countRange), "shaders/RandomGen.spv");
			vkt::CommandManager cmdManager(vom, vom.GetComputeQueue(), true, vk::PipelineStageFlagBits::eComputeShader);
			auto cmd = cmdManager.RecordNew();
			cmd.begin(vk::CommandBufferBeginInfo());
			cmd.bindPipeline(vk::PipelineBindPoint::eCompute, randomGenStage.computePipeline);
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, randomGenStage.layout, 0, 1, &randomGenDescSet->set, 0, {});
			cmd.pushConstants(randomGenStage.layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(CountData), &countData);
			cmd.dispatch((countData.staticsCount/64)+1, 1, 1);
			cmd.end();
			cmdManager.Execute(true, true, false, false);

			vkt::MemoryOperationsBuffer ops(vom);

			std::vector<glm::vec4> staticsReturned;
			staticsReturned.resize(countData.staticsCount);
			auto returnStatics = ops.SectorToRam(staticsSector, staticsReturned.data());
			ops.Execute({}, true);
			returnStatics.Execute();


		}

		stateUpdateSet->AttachSector(positionsSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(staticsSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(matrixSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(camdataSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(modelMatrixSector, vk::ShaderStageFlagBits::eCompute);
		graphicsSet->AttachSector(matrixSector, vk::ShaderStageFlagBits::eVertex);
		graphicsSet->AttachSector(modelMatrixSector, vk::ShaderStageFlagBits::eVertex);
		descriptorManager.Update();


		LightData lightData{ glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f,1.0f,1.0f), glm::vec3(1.0f,1.0f,1.0f), 1.0f, 0.1f};
		vk::PushConstantRange lightRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(lightData));
		gPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo({}, 1, &graphicsSet->layout, 1, &lightRange);
		BuildGraphicsPipeline(window.window, 0, 0);

		vkt::ComputePipelineManager stateUpdate(vom, vk::PipelineLayoutCreateInfo({}, 1, &stateUpdateSet->layout, 1, &countRange), "shaders/StateUpdate.spv");

		vkt::MemoryOperationsBuffer frameOps(vom);
		uint32_t imageIndex = 0;
		auto imgAvailable = vom.MakeSemaphore();
		vkt::CommandManager cmdManager(vom, vom.GetGraphicsQueue(), true, vk::PipelineStageFlagBits::eAllGraphics);
		cmdManager.DependsOn({ 
			{nullptr, imgAvailable, vk::PipelineStageFlagBits::eColorAttachmentOutput }
			,{frameOps.cmdManager.GetSubmitCountPtr(), frameOps.cmdManager.GetMainTimelineSignal().semaphore, vk::PipelineStageFlagBits::eTransfer}
		});
		auto fence = vom.MakeFence(true);
		spdlog::stopwatch sw;
		float timeSpeedFactor = 1;
		Character character(window, pVom, countData.deltaTime, timeSpeedFactor);


		float accumulatedTime = 0;

		while (window.Open())
		{
			sw.reset();
			if (!glfwGetWindowAttrib(window.window, GLFW_ICONIFIED))
			{
				character.Move(countData.deltaTime/timeSpeedFactor);
				CamData camData{character.camera.GetClip(), character.camera.Project(), character.camera.View(), character.camera.PVMatrix(), character.camera.GetPosition()};
				imageIndex = vom.GetDevice().acquireNextImageKHR(pVom.GetSwapchainData().GetSwapchain(), UINT64_MAX, imgAvailable).value;
				frameOps.Clear(true);
				frameOps.RamToSector(&camData, camdataSector, sizeof(camData));
				frameOps.Execute();
				descriptorManager.Update();
				cmdManager.Reset();
				auto cmd = cmdManager.RecordNew();
				cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
				cmd.bindPipeline(vk::PipelineBindPoint::eCompute, stateUpdate.computePipeline);
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, stateUpdate.layout, 0, 1, &stateUpdateSet->set, 0, {});
				cmd.pushConstants(stateUpdate.layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(CountData), &countData);
				cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
				cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, gPipelineLayout, 0, 1, &graphicsSet->set, 0, {});
				cmd.pushConstants(gPipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(LightData), &lightData);
				cmd.dispatch((countData.objectCount/64) + 1, 1, 1);
				vk::MemoryBarrier stateBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
				cmd.pipelineBarrier(
					vk::PipelineStageFlagBits::eComputeShader,
					vk::PipelineStageFlagBits::eVertexShader,
					{},
					1,
					& stateBarrier,
					0,
					{},
					0,
					{});
				
				auto currentImage = pVom.GetSwapchainData().GetImage(imageIndex);

				//Attachment setup
				vk::ClearColorValue clearColorValue;
				clearColorValue.setFloat32({ 0.0f, 0.0f, 0.0f });
				vk::ClearValue colorClear(clearColorValue);
				vk::ClearValue depthClear(vk::ClearDepthStencilValue(1, {}));
				vk::RenderingAttachmentInfo colorAttachment(
					currentImage.view,
					vk::ImageLayout::eColorAttachmentOptimal,
					{},
					{},
					{},
					vk::AttachmentLoadOp::eClear,
					vk::AttachmentStoreOp::eStore,
					colorClear);
				vk::RenderingAttachmentInfoKHR depthAttachment(
					depthImage.view,
					depthImage.layout,
					{},
					{},
					{},
					vk::AttachmentLoadOp::eClear,
					vk::AttachmentStoreOp::eNone,
					depthClear
				);
				vk::RenderingInfoKHR renderingInfo(
					{},
					vk::Rect2D(0, pVom.GetSwapchainData().GetExtent()),
					1,
					0,
					1,
					& colorAttachment,
					&depthAttachment,
					{});
				vk::ImageMemoryBarrier colorRenderBarrier(
					vk::AccessFlagBits::eNoneKHR,
					vk::AccessFlagBits::eColorAttachmentWrite,
					vk::ImageLayout::eUndefined,
					currentImage.layout,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					currentImage.image,
					vk::ImageSubresourceRange(
						vk::ImageAspectFlagBits::eColor,
						0,
						1,
						0,
						1));
				cmd.pipelineBarrier(
					vk::PipelineStageFlagBits::eTopOfPipe,
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					{},
					0,
					{},
					0,
					{},
					1,
					& colorRenderBarrier);

				cmd.bindVertexBuffers(0, 1, &vbo->bufferAllocation->bufferData.buffer, &vbo->allocationOffset);
				cmd.beginRendering(&renderingInfo);
				cmd.draw(objectData.vertices.size(), countData.objectCount, 0, 0);
				cmd.endRendering();
				vk::ImageMemoryBarrier colorPresentBarrier(
					vk::AccessFlagBits::eColorAttachmentWrite,
					vk::AccessFlagBits::eNone,
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageLayout::ePresentSrcKHR,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					currentImage.image,
					vk::ImageSubresourceRange(
						vk::ImageAspectFlagBits::eColor,
						0,
						1,
						0,
						1));

				cmd.pipelineBarrier(
					vk::PipelineStageFlagBits::eAllCommands,
					vk::PipelineStageFlagBits::eBottomOfPipe,
					{},
					0,
					{},
					0,
					{},
					1,
					& colorPresentBarrier);

				cmd.end();

				cmdManager.Execute(true, true, true, true);
				auto res = vom.GetGraphicsQueue().queue.presentKHR(vk::PresentInfoKHR(1, &cmdManager.GetMainSignal().semaphore, 1, &pVom.GetSwapchainData().swapchain, &imageIndex));

			}
			countData.deltaTime = sw.elapsed().count();
			glfwPollEvents();
			countData.frameCount++;
			accumulatedTime += countData.deltaTime;
			if (accumulatedTime > 1)
			{
				spdlog::info("Frame Delta Time: {} FPS: {:.3f}", countData.deltaTime, 1 / countData.deltaTime);

				accumulatedTime = 0;
			}
			countData.deltaTime *= timeSpeedFactor;
		}
	}
}
