// VulkanToolbox.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define VMA_IMPLEMENTATION
#include <iostream>
#define GLM_FORCE_RANDIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <VkBootstrap.h>
#include <random>
#include <VulkanToolbox.hpp>
#include <spdlog/stopwatch.h>

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

struct CamData
{
	alignas(16) glm::mat4 MVmat;
	alignas(4) float objectCount;
	alignas(4) float deltaTime;
};

struct LightData
{
	alignas(16)glm::vec3 lightColor;
	alignas(16)glm::vec3 lightDirection;
	alignas(16)glm::vec3 ambientColor;
	alignas(4)float directionIntensity;
	alignas(4)float ambientIntensity;
};

inline glm::vec3 GetSurfaceNormal(glm::vec3 V1, glm::vec3 V2, glm::vec3 V3)
{
	return glm::cross((V2 - V1), (V3 - V2));
}

struct Tetrahedron
{
	glm::vec3 bottomLeft{ -1, 1, -1 };
	glm::vec3 bottomRight{ 1, 1, -1 };
	glm::vec3 frontTop{ 0, -1, -1 };
	glm::vec3 back{ 0, 0, 3 };
	glm::vec3 color{ 1,1,1 };
	std::vector<VertexData> vertices{
		//Front face
	{bottomLeft,     color, GetSurfaceNormal(frontTop, bottomLeft, bottomRight)},
	{frontTop, color, GetSurfaceNormal(frontTop, bottomLeft, bottomRight)},
	{bottomRight,  color, GetSurfaceNormal(frontTop, bottomLeft, bottomRight)},
	//Bottom face
	{bottomLeft,  color, GetSurfaceNormal(bottomLeft, back, bottomRight)},
	{bottomRight,        color, GetSurfaceNormal(bottomLeft, back, bottomRight)},
	{back, color, GetSurfaceNormal(bottomLeft, back, bottomRight)},
	//Top Left face
	{bottomLeft, color, GetSurfaceNormal(back, bottomLeft, frontTop)},
	{back,        color, GetSurfaceNormal(back, bottomLeft, frontTop)},
	{frontTop,       color, GetSurfaceNormal(back, bottomLeft, frontTop)},
	//Top Right face
	{bottomRight,       color, GetSurfaceNormal(back, frontTop, bottomRight)},
	{frontTop,        color, GetSurfaceNormal(back, frontTop, bottomRight)},
	{back,  color, GetSurfaceNormal(back, frontTop, bottomRight)}
	};
};

struct MatrixData
{
	alignas(16)glm::mat4 rotMatrix;
};
struct Velocity
{
	alignas(16)glm::vec4 unitDirection;
	alignas(4)float speed;
};

std::unique_ptr < vkt::GraphicsPipelineManager > CreatePresentation(vkt::VulkanObjectManager& _vom, vkb::Device& bootLDevice, vkt::RenderPassManager& renderpassManager, std::shared_ptr<vkt::DescriptorSetData> graphiscDescSet, std::vector<vk::PushConstantRange> pushConstantRanges)
{
	vkb::SwapchainBuilder swapchainBuilder{ bootLDevice };
	auto swapchainRet = swapchainBuilder.set_old_swapchain(_vom.GetSwapchainData(true).GetSwapchain()).build();
	if (!swapchainRet)
	{
		std::cout << "\n\nNo swapchain for you!!\n\n";
		throw std::logic_error("");
	}
	else
	{
		vkb::Swapchain bootSwapchain;
		_vom.DestroyType(vk::SwapchainKHR());
		bootSwapchain = swapchainRet.value();
		_vom.DestroyType(vk::ImageView());
		_vom.DestroyType(vk::Image());
		_vom.DestroyType(vkt::VmaImage());
		_vom.SetSwapchain({ bootSwapchain.swapchain, (vk::Format)bootSwapchain.image_format, bootSwapchain.extent }, true);

		auto gQueue = _vom.GetGraphicsQueue();

		auto depthImage = _vom.VmaMakeImage(
			vk::ImageCreateInfo({}, vk::ImageType::e2D, vk::Format::eD32Sfloat, vk::Extent3D(bootSwapchain.extent.width, bootSwapchain.extent.height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, 1, &gQueue.index, vk::ImageLayout::eUndefined),
			vk::ImageViewCreateInfo({}, {}, vk::ImageViewType::e2D, vk::Format::eD32Sfloat, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)),
			VmaAllocationCreateInfo{ {}, VMA_MEMORY_USAGE_GPU_ONLY });

		vk::ClearColorValue clearColorValue;
		clearColorValue.setFloat32({ 0.0f,0.0f,0.0f,1.0f });
		vk::ClearValue colorClear;
		colorClear.setColor(clearColorValue);
		vk::ClearValue depthClear;
		depthClear.setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));
		renderpassManager.Dispose();
		renderpassManager = vkt::RenderPassManager(_vom.GetDevice(), _vom.GetSwapchainData().GetExtent().width, _vom.GetSwapchainData().GetExtent().height);
		auto colorAttachment = renderpassManager.CreateAttachment(vk::AttachmentDescription({}, _vom.GetSwapchainData().GetFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear
			, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR),
			vk::ImageLayout::eColorAttachmentOptimal, colorClear, vk::ImageView());
		auto depthAttachment = renderpassManager.CreateAttachment(vk::AttachmentDescription({}, vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear
			, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal),
			vk::ImageLayout::eDepthStencilAttachmentOptimal, depthClear, depthImage.view);
		renderpassManager.CreateSubpass(vk::PipelineBindPoint::eGraphics, { colorAttachment }, {}, depthAttachment, { vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput
			, vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eColorAttachmentWrite, {})
			,vk::SubpassDependency(VK_SUBPASS_EXTERNAL, {},
				vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
				vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::AccessFlagBits::eNoneKHR,
				vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			});
		renderpassManager.CreateRenderPass();
		for (auto& image : _vom.GetSwapchainData().GetImages())
		{
			colorAttachment->view = image.view;
			renderpassManager.CreateFramebuffer();
		}

		return std::make_unique<vkt::GraphicsPipelineManager>(
			_vom, renderpassManager.renderPass,
			std::vector{ "shaders/vert.spv", "shaders/frag.spv" }, std::vector{ vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment },
			VertexData::vertexInputBinding(), VertexData::vertexInputAttribute(),
			vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE),
			vk::Viewport(0, 0, bootSwapchain.extent.width, bootSwapchain.extent.height, 0, 1),
			vk::Rect2D({ 0,0, }, bootSwapchain.extent),
			vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise, VK_FALSE, {}, {}, {}, 1.0f),
			vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE),
			vk::PipelineDepthStencilStateCreateInfo({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1.0f),
			std::vector{ vk::PipelineColorBlendAttachmentState(VK_FALSE, {}, {}, {}, {}, {}, {}, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) },
			vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy),
			vk::PipelineLayoutCreateInfo({}, 1, &(*graphiscDescSet).layout, pushConstantRanges.size(), pushConstantRanges.data()));

	}
}

int main()
{
	vkb::Instance bootInstance;
	vkb::PhysicalDevice bootPDevice;
	vkb::Device bootDevice;

#ifndef NDEBUG
	uint32_t width = 500;
	//spdlog::info("Set width: ");
	//std::cin >> width;
	uint32_t height = 500;
	//spdlog::info("Set height: ");
	//std::cin >> height;
#else
	uint32_t width = 500;
	spdlog::info("Set width: ");
	std::cin >> width;
	uint32_t height = 500;
	spdlog::info("Set height: ");
	std::cin >> height;
#endif

#ifndef NDEBUG
	float objectCount = 1000000;
	spdlog::info("Please Enter Object Count");
	//std::cin >> objectCount;
#else
	float objectCount = 0;
	spdlog::info("Please Enter Object Count");
	std::cin >> objectCount;
#endif


	vkt::VulkanWindow window(width, height);

	vkb::InstanceBuilder instanceBuilder;
	instanceBuilder.set_app_name("VMLTester")
		.set_engine_name("Tester")
		.require_api_version(1, 2);
#ifndef NDEBUG
	instanceBuilder.enable_validation_layers();
#endif


	auto bootInstanceReturn = instanceBuilder.build();
	if (!bootInstanceReturn)
	{
		std::cout << "ERROR CODE OCCURRED:" << bootInstanceReturn.vk_result() << std::endl;
		abort();
		throw std::logic_error(" ");
	}
	bootInstance = bootInstanceReturn.value();
	window.CreateSurface(bootInstance.instance);
	bootDevice.surface = window.surface;

	vkb::PhysicalDeviceSelector physicalDeviceSelector(bootInstance);
	vk::PhysicalDeviceVulkan12Features features; features.timelineSemaphore = VK_TRUE;
	auto pDeviceRet = physicalDeviceSelector.set_surface(window.surface).add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME).set_required_features_12(features).require_present().select();
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
	vkt::VulkanObjectManager vom(bootDevice.device);
	vom.SetInstance(bootInstanceReturn.value().instance);
	vom.Manage(bootInstanceReturn.value().instance);
	vom.SetSurface(window.surface);
	vom.Manage(window.surface);
	vom.SetPhysicalDevice(pDeviceRet.value().physical_device);
	vom.Manage(bootDevice.device);

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





	window.resized = false;





	//Game Scope
	{


		vkt::BufferManager vboStorage(vom, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_GPU_ONLY);
		vkt::BufferManager cpuStorage(vom, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_ONLY);
		vkt::BufferManager gpuStorage(vom, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_GPU_ONLY);
		vkt::MemoryOperationsBuffer swapBuffer(vom);
		auto transferSector = cpuStorage.GetSector();
		auto postitionsSector = gpuStorage.GetSector();
		auto velocitiesSector = gpuStorage.GetSector();
		auto matrixSector = gpuStorage.GetSector();
		auto vbo = vboStorage.GetSector();
		Tetrahedron objectData;

		{
			float objectSpeed = 100;
			float bounds = 500;
			std::vector<glm::vec4> positions;
			positions.resize(objectCount);
			std::vector<Velocity> velocities;
			velocities.resize(objectCount);
			std::vector<MatrixData> matricies;
			matricies.resize(objectCount);
			std::default_random_engine engine;
			std::uniform_real_distribution<float> distribution(-bounds, bounds);
			std::uniform_real_distribution<float> zdistribution(0.0f, 148000);
			std::uniform_real_distribution<float> veloctyDist(-1, 1);
			std::uniform_real_distribution<float> speedDist(0, objectSpeed);
			for (auto& pos : positions)
			{
				//pos = glm::vec4(distribution(engine), distribution(engine), zdistribution(engine), 1);
				pos = glm::vec4(0, 0, 1000, 1);
			}
			for (auto& vel : velocities)
			{
				vel.unitDirection = glm::normalize(glm::vec4(veloctyDist(engine), veloctyDist(engine), veloctyDist(engine), 0));
				vel.speed = speedDist(engine);
				//vel = glm::normalize(glm::vec4(1, -1, 1, 0))*objectSpeed;


			}

			swapBuffer.CopyFromRam(sizeof(positions[0]) * positions.size(), positions.data(), transferSector);
			swapBuffer.CopyToSector(transferSector, postitionsSector);
			swapBuffer.CopyFromRam(sizeof(velocities[0]) * velocities.size(), velocities.data(), transferSector);
			swapBuffer.CopyToSector(transferSector, velocitiesSector);
			swapBuffer.CopyFromRam(sizeof(matricies[0]) * matricies.size(), matricies.data(), transferSector);
			swapBuffer.CopyToSector(transferSector, matrixSector);
			cpuStorage.Update();
			gpuStorage.Update();
			swapBuffer.Execute({}, true);
			swapBuffer.Clear();
			transferSector->neededSize = 0;
			swapBuffer.CopyFromRam(sizeof(objectData.vertices[0]) * objectData.vertices.size(), objectData.vertices.data(), transferSector);
			swapBuffer.CopyToSector(transferSector, vbo);
			vboStorage.Update();
			swapBuffer.Execute({}, true);
			swapBuffer.Clear();


		}



		vkt::DescriptorManager descriptorManager(vom);
		auto computeSet = descriptorManager.GetNewSet();
		computeSet->AttachSector(postitionsSector, vk::ShaderStageFlagBits::eCompute);
		computeSet->AttachSector(velocitiesSector, vk::ShaderStageFlagBits::eCompute);
		computeSet->AttachSector(matrixSector, vk::ShaderStageFlagBits::eCompute);
		auto graphicsSet = descriptorManager.GetNewSet();
		graphicsSet->AttachSector(matrixSector, vk::ShaderStageFlagBits::eVertex);
		descriptorManager.Update();

		vk::PushConstantRange camPush(vk::ShaderStageFlagBits::eCompute, 0, sizeof(CamData));
		vk::PushConstantRange lightPush(vk::ShaderStageFlagBits::eFragment, sizeof(CamData), sizeof(LightData));
		vk::PushConstantRange lightPush2(vk::ShaderStageFlagBits::eFragment, 0, sizeof(LightData));
		vk::PushConstantRange camPush2(vk::ShaderStageFlagBits::eVertex, sizeof(LightData), sizeof(CamData));

		vkt::RenderPassManager renderpassManager(vom.GetDevice());

		vkt::ComputePipelineManager computePipeline(vom, vk::PipelineLayoutCreateInfo({}, 1, &computeSet->layout, 1, &camPush), "shaders/StateUpdate.spv");

		auto graphicsPipeline = CreatePresentation(vom, bootDevice, renderpassManager, graphicsSet, { lightPush2, camPush2 });


		vkt::CommandManager cmdManager(vom, vom.GetGraphicsQueue(), true, vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eAllGraphics);

		const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f);
		glm::vec3 camPos(0, 0, -50);
		const glm::vec3 center(0, 0, 1);
		const glm::vec3 up(0.f, -1.0f, 0);
		LightData lightData{ glm::vec3(1.0f,1.0f,1.0f), glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(1.0f,1.0f,1.0f), 1.0f, 0.1f };

		spdlog::stopwatch sw;
		spdlog::stopwatch deltaWatch;
		uint64_t frame = 0;
		double cpuDeltaTime = 0;
		double gpuDeltaTime = 0;
		double accumulatedWaitTime = 0;
		double deltaTime = 0;
		uint32_t imageIndex;
		auto imgAvailable = vom.MakeSemaphore();
		cmdManager.DependsOn({ vkt::WaitData(std::make_shared<uint64_t>(0), imgAvailable, vk::PipelineStageFlagBits::eEarlyFragmentTests) });
		while (window.Open())
		{
			deltaTime = deltaWatch.elapsed().count();
			deltaWatch.reset();

			if (window.resized)
			{
				window.resized = false;
				graphicsPipeline->vom.DisposeAll();
				graphicsPipeline = CreatePresentation(vom, bootDevice, renderpassManager, graphicsSet, { lightPush2, camPush2 });
			}

			imageIndex = vom.GetDevice().acquireNextImageKHR(vom.GetSwapchainData().GetSwapchain(), UINT64_MAX, imgAvailable, VK_NULL_HANDLE).value;
			sw.reset();
			descriptorManager.Update();
			const glm::mat4 view = glm::lookAt(camPos, camPos + center, up);
			//camPos[2] += 10*deltaTime;
			float aspect = static_cast<float>(vom.GetSwapchainData().GetExtent().width) / static_cast<float>(vom.GetSwapchainData().GetExtent().height);
			glm::mat4 proj = glm::perspective(glm::radians(70.0f), aspect, .1f, 500.0f);
			glm::mat4 PVmat = clip * proj * view;

			CamData camData{ PVmat, objectCount, static_cast<float>(deltaTime) };


			cmdManager.Reset();
			auto cmd = cmdManager.RecordNew();
			cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
			cmd.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline.computePipeline);
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->pipeline);
			cmd.pushConstants(computePipeline.layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(CamData), &camData);
			cmd.pushConstants(graphicsPipeline->layout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(LightData), &lightData);
			cmd.pushConstants(graphicsPipeline->layout, vk::ShaderStageFlagBits::eVertex, sizeof(LightData), sizeof(CamData), &camData);
			cmd.bindVertexBuffers(0, 1, &vboStorage.bufferData.buffer, &vbo->allocationOffset);
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, computePipeline.layout, 0, 1, &(*computeSet).set, {}, {});
			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline->layout, 0, 1, &(*graphicsSet).set, {}, {});
			vk::RenderPassBeginInfo renderPassBeginInfo(graphicsPipeline->renderPass, renderpassManager.frames[imageIndex].frameBuffer, vk::Rect2D(0, vom.GetSwapchainData().GetExtent()), 2, renderpassManager.clearValues.data());
			cmd.dispatch((objectCount / 64) + 1, 1, 1);
			vk::MemoryBarrier computeMemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
			cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eVertexShader, {}, 1, &computeMemoryBarrier, {}, {}, {}, {});
			cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
			cmd.draw(objectData.vertices.size(), objectCount, 0, 0);
			cmd.endRenderPass();
			cmd.end();
			cpuDeltaTime = sw.elapsed().count() * 1000;
			sw.reset();
			cmdManager.Execute(true, true, true, true);
			auto res = vom.GetGraphicsQueue().queue.presentKHR(vk::PresentInfoKHR(1, &cmdManager.syncManager.signalSemaphores[1].semaphore, 1, &vom.GetSwapchainData().swapchain, &imageIndex));
			gpuDeltaTime = sw.elapsed().count() * 1000;

			accumulatedWaitTime += cpuDeltaTime;
			accumulatedWaitTime += gpuDeltaTime;

			if (accumulatedWaitTime > 500)
			{
				accumulatedWaitTime = 0;
				spdlog::info("Frame: {} Fps: {:.3f}\nCpuTime: {:.3f} [ms] GpuDeltaTime: {:.3f} [ms]\n", frame, 1 / ((cpuDeltaTime + gpuDeltaTime) / 1000), cpuDeltaTime, gpuDeltaTime);
			}

			glfwPollEvents();

			frame++;
		}

	}
}
