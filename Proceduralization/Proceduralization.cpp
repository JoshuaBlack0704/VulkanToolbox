// Proceduralization.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#define VMA_IMPLEMENTATION
#include <VulkanToolbox.hpp>
#define GLM_FORCE_RANDIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <VkBootstrap.h>


class Character
{
public:
	const glm::mat4 clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);
	glm::mat4 PVmat;
	glm::mat4 Vmat;
	glm::mat4 BuildProjection;
	glm::vec4 pos = glm::vec4(0, 0, -50, 1);
	glm::vec4 vel = glm::vec4(0, 0, 0, 1.0f);
	glm::vec4 accel;
	glm::vec4 center = glm::vec4(0, 0, 1, 1);
	glm::vec3 rotAxis;
	glm::mat4 rotMatrix = glm::mat4(1.0f);
	glm::mat4 transMatrix;
	glm::vec4 target;
	double& deltaTime;
	float& timeSpeedFactor;
	vkt::VulkanObjectManager& vom;

	Character(vkt::VulkanWindow& window, vkt::VulkanObjectManager& _vom, double& _deltaTime, float& _timeSpeedFactor) : deltaTime(_deltaTime), vom(_vom), timeSpeedFactor(_timeSpeedFactor)
	{

		pos = glm::vec4(0, 0, -50, 1);

		wKeyMap = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_W), GLFW_PRESS, [&]() {accel[2] = 1; spdlog::info("Moving ship forward"); });
		wKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_W), GLFW_RELEASE, [&]() {accel[2] = 0;  vel[2] = 0; spdlog::info("Stopping ship"); });

		sKeyMap = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_S), GLFW_PRESS, [&]() {accel[2] = -1; spdlog::info("Moving ship back"); });
		sKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_S), GLFW_RELEASE, [&]() {accel[2] = 0;  vel[2] = 0; spdlog::info("Stopping ship"); });

		aKeyMap = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_A), GLFW_PRESS, [&]() {accel[0] = -1; spdlog::info("Moving ship left"); });
		aKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_A), GLFW_RELEASE, [&]() {accel[0] = 0;  vel[0] = 0; spdlog::info("Stopping ship"); });

		dKeyMap = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_D), GLFW_PRESS, [&]() {accel[0] = 1; spdlog::info("Moving ship right"); });
		dKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_D), GLFW_RELEASE, [&]() {accel[0] = 0;  vel[0] = 0; spdlog::info("Stopping ship"); });

		shiftKeyMap = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_LEFT_SHIFT), GLFW_PRESS, [&]() {accel[1] = -1; spdlog::info("Moving ship up"); });
		shiftKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_LEFT_SHIFT), GLFW_RELEASE, [&]() {accel[1] = 0;  vel[1] = 0; spdlog::info("Stopping ship"); });

		ctrlKeyMap = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_LEFT_CONTROL), GLFW_PRESS, [&]() {accel[1] = 1; spdlog::info("Moving ship down"); });
		ctrlKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_LEFT_CONTROL), GLFW_RELEASE, [&]() {accel[1] = 0;  vel[1] = 0; spdlog::info("Stopping ship"); });

		upKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_UP), GLFW_PRESS, [&]() {     rotAxis[0] = 1; spdlog::info("Rotating ship up"); });
		upKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_UP), GLFW_RELEASE, [&]() {   rotAxis[0] = 0; spdlog::info("Stopping ship"); });

		downKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_DOWN), GLFW_PRESS, [&]() { rotAxis[0] = -1; spdlog::info("Rotating ship down"); });
		downKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_DOWN), GLFW_RELEASE, [&]() { rotAxis[0] = 0; spdlog::info("Stopping ship"); });

		leftKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_LEFT), GLFW_PRESS, [&]() { rotAxis[1] = -1; spdlog::info("Rotating ship left"); });
		leftKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_LEFT), GLFW_RELEASE, [&]() { rotAxis[1] = 0; spdlog::info("Stopping ship"); });

		rightKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_RIGHT), GLFW_PRESS, [&]() {rotAxis[1] = 1; spdlog::info("Rotating ship right"); });
		rightKeyStop = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_RIGHT), GLFW_RELEASE, [&]() {rotAxis[1] = 0; spdlog::info("Stopping ship"); });

		backspaceKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_BACKSPACE), GLFW_PRESS, [&]() {deltaTime *= -1; spdlog::info("Reversing time"); });
		spaceKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_SPACE), GLFW_PRESS, [&]()
			{
				if (deltaTime != 0)
				{
					spdlog::info("stopping time");
					deltaTime = 0;
				}
				else
				{
					spdlog::info("starting time");
					deltaTime = 1;
				}
			});

		numEnterKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_KP_ENTER), GLFW_PRESS, [&]()
			{
				spdlog::info("\nPlease enter new time speed factor:");
				std::cin >> timeSpeedFactor;
				spdlog::info(" New time speed factor set to: {}", timeSpeedFactor);
			});
		altKey = window.AddKeyMap(glfwGetKeyScancode(GLFW_KEY_LEFT_ALT), GLFW_PRESS, [&]()
			{
				spdlog::info("\nPlease enter new time speed factor:");
				std::cin >> timeSpeedFactor;
				spdlog::info(" New time speed factor set to: {}", timeSpeedFactor);

			});

	}

	Character* Move()
	{
		if (rotAxis != glm::vec3(0))
		{
			rotMatrix = glm::rotate(rotMatrix, glm::radians(20 * static_cast<float>(deltaTime)), (rotAxis));
			center = rotMatrix * glm::vec4(0, 0, 1, 1);
		}


		vel += accel;
		pos += rotMatrix * vel * glm::vec4(deltaTime, deltaTime, deltaTime, 1);

		return this;
	}
	Character* Move(double _externalDeltaTime)
	{
		if (rotAxis != glm::vec3(0))
		{
			rotMatrix = glm::rotate(rotMatrix, glm::radians(20 * static_cast<float>(_externalDeltaTime)), (rotAxis));
			center = rotMatrix * glm::vec4(0, 0, 1, 1);
		}


		vel += accel;
		pos += rotMatrix * vel * glm::vec4(_externalDeltaTime, _externalDeltaTime, _externalDeltaTime, 1);

		return this;
	}
	glm::mat4 GetPVMat()
	{
		float aspect = static_cast<float>(vom.GetSwapchainData().GetExtent().width) / static_cast<float>(vom.GetSwapchainData().GetExtent().height);
		return clip * glm::perspective(glm::radians(70.0f), aspect, .1f, 1000.0f) * glm::lookAt(static_cast<glm::vec3>(pos), static_cast<glm::vec3>(pos + center), glm::vec3(0, -1, 0));
	}

private:
	std::shared_ptr<vkt::KeyMap> wKeyMap;
	std::shared_ptr<vkt::KeyMap> wKeyStop;
	std::shared_ptr<vkt::KeyMap> sKeyMap;
	std::shared_ptr<vkt::KeyMap> sKeyStop;
	std::shared_ptr<vkt::KeyMap> aKeyMap;
	std::shared_ptr<vkt::KeyMap> aKeyStop;
	std::shared_ptr<vkt::KeyMap> dKeyMap;
	std::shared_ptr<vkt::KeyMap> dKeyStop;
	std::shared_ptr<vkt::KeyMap> shiftKeyMap;
	std::shared_ptr<vkt::KeyMap> shiftKeyStop;
	std::shared_ptr<vkt::KeyMap> ctrlKeyMap;
	std::shared_ptr<vkt::KeyMap> ctrlKeyStop;

	std::shared_ptr<vkt::KeyMap> upKey;
	std::shared_ptr<vkt::KeyMap> upKeyStop;

	std::shared_ptr<vkt::KeyMap> downKey;
	std::shared_ptr<vkt::KeyMap> downKeyStop;

	std::shared_ptr<vkt::KeyMap> leftKey;
	std::shared_ptr<vkt::KeyMap> leftKeyStop;

	std::shared_ptr<vkt::KeyMap> rightKey;
	std::shared_ptr<vkt::KeyMap> rightKeyStop;

	std::shared_ptr<vkt::KeyMap> backspaceKey;
	std::shared_ptr<vkt::KeyMap> spaceKey;
	std::shared_ptr<vkt::KeyMap> numEnterKey;
	std::shared_ptr<vkt::KeyMap> altKey;

};
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
		{bottomRight,     color, GetSurfaceNormal(frontTop, bottomLeft, bottomRight)},
		{frontTop, color, GetSurfaceNormal(frontTop, bottomLeft, bottomRight)},
		{bottomLeft,  color, GetSurfaceNormal(frontTop, bottomLeft, bottomRight)},
		//Bottom face
		{bottomLeft,  color, GetSurfaceNormal(bottomLeft, back, bottomRight)},
		{back,        color, GetSurfaceNormal(bottomLeft, back, bottomRight)},
		{bottomRight, color, GetSurfaceNormal(bottomLeft, back, bottomRight)},
		//Top Left face
		{bottomLeft, color, GetSurfaceNormal(back, bottomLeft, frontTop)},
		{frontTop,        color, GetSurfaceNormal(back, bottomLeft, frontTop)},
		{back,       color, GetSurfaceNormal(back, bottomLeft, frontTop)},
		//Top Right face
		{bottomRight,       color, GetSurfaceNormal(back, frontTop, bottomRight)},
		{back,        color, GetSurfaceNormal(back, frontTop, bottomRight)},
		{frontTop,  color, GetSurfaceNormal(back, frontTop, bottomRight)}
	};
};
struct PaperAirplane
{
	glm::vec3 nose{ 0, -1, 2 };
	glm::vec3 backBottom{ 0, .25, -1 };
	glm::vec3 backMiddleLeft{ -.25, -.75, -1 };
	glm::vec3 backMiddleRight{ .25, -.75, -1 };
	glm::vec3 backTop{ 0, -1, -1 };
	glm::vec3 leftWingTip{ -1, -1.15, -1.25 };
	glm::vec3 rightWingTip{ 1, -1.15, -1.25 };
	glm::vec3 color{ 1,1,1 };

	std::vector<VertexData> vertices
	{
		//Right fuselage
		{nose, color, GetSurfaceNormal(nose, backMiddleRight, backBottom)},
		{backMiddleRight, color, GetSurfaceNormal(nose, backMiddleRight, backBottom)},
		{backBottom, color, GetSurfaceNormal(nose, backMiddleRight, backBottom)},

		//Right underwing
		{nose, color, GetSurfaceNormal(nose, rightWingTip, backMiddleRight)},
		{rightWingTip, color, GetSurfaceNormal(nose, rightWingTip, backMiddleRight)},
		{backMiddleRight, color, GetSurfaceNormal(nose, rightWingTip, backMiddleRight)},

		//Right top wing
		{nose, color, GetSurfaceNormal(nose, backTop, rightWingTip)},
		{backTop, color, GetSurfaceNormal(nose, backTop, rightWingTip)},
		{rightWingTip, color, GetSurfaceNormal(nose, backTop, rightWingTip)},

		//Left fuselage
		{nose, color, GetSurfaceNormal(nose, backBottom, backMiddleLeft)},
		{backBottom, color, GetSurfaceNormal(nose, backBottom, backMiddleLeft)},
		{backMiddleLeft, color, GetSurfaceNormal(nose, backBottom, backMiddleLeft)},

		//Left underwing
		{nose, color, GetSurfaceNormal(nose, backMiddleLeft, leftWingTip)},
		{backMiddleLeft, color, GetSurfaceNormal(nose, backMiddleLeft, leftWingTip)},
		{leftWingTip, color, GetSurfaceNormal(nose, backMiddleLeft, leftWingTip)},

		//Left top wing
		{nose, color, GetSurfaceNormal(nose, leftWingTip, backTop)},
		{leftWingTip, color, GetSurfaceNormal(nose, leftWingTip, backTop)},
		{backTop, color, GetSurfaceNormal(nose, leftWingTip, backTop)},

		//Back bottom
		{backMiddleLeft, color, GetSurfaceNormal(backMiddleLeft, backBottom, backMiddleRight)},
		{backBottom, color, GetSurfaceNormal(backMiddleLeft, backBottom, backMiddleRight)},
		{backMiddleRight, color, GetSurfaceNormal(backMiddleLeft, backBottom, backMiddleRight)},

		//Left Wing Back
		{leftWingTip, color, GetSurfaceNormal(leftWingTip, backMiddleLeft, backTop)},
		{backMiddleLeft, color, GetSurfaceNormal(leftWingTip, backMiddleLeft, backTop)},
		{backTop, color, GetSurfaceNormal(leftWingTip, backMiddleLeft, backTop)},

		//Right wing back
		{rightWingTip, color, GetSurfaceNormal(rightWingTip, backTop, backMiddleRight)},
		{backTop, color, GetSurfaceNormal(rightWingTip, backTop, backMiddleRight)},
		{backMiddleRight, color, GetSurfaceNormal(rightWingTip, backTop, backMiddleRight)},

		//Center Back
		{backTop, color, GetSurfaceNormal(backTop, backMiddleLeft, backMiddleRight)},
		{backMiddleLeft, color, GetSurfaceNormal(backTop, backMiddleLeft, backMiddleRight)},
		{backMiddleRight, color, GetSurfaceNormal(backTop, backMiddleLeft, backMiddleRight)},
	};



};
struct Velocity
{
	alignas(16)glm::vec4 unitDirection;
	alignas(4)float speed;
};


int main()
{
	vkt::VulkanObjectManager vom({}, false, true);
	uint32_t width = 500;
	uint32_t height = 500;
	vkt::VulkanWindow window(width, height);

#ifndef NDEBUG
	float objectCount = 1000000;
	spdlog::info("Please Enter Object Count");
	//std::cin >> objectCount;
#else
	float objectCount = 0;
	spdlog::info("Please Enter Object Count");
	std::cin >> objectCount;
#endif

	//Initialization
	{
		vkb::Instance bootInstance;
		vkb::PhysicalDevice bootPDevice;
		vkb::Device bootDevice;

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
		std::shared_ptr<vkt::RenderPassManager> renderPassManager;
		std::shared_ptr<vkt::GraphicsPipelineManager> graphicsPipeline;
		std::vector<std::shared_ptr<vkt::DescriptorSetData>> graphicsDescSets;
		std::vector<vk::PushConstantRange> graphicsPushRanges;
		std::vector<const char*> shaderPaths = {"shaders/vert.spv", "shaders/frag.spv" };
		std::vector<vk::ShaderStageFlagBits> shaderStageFlags = {vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment};

		auto buildPresentationData = [&vom, &renderPassManager, &graphicsPipeline, &graphicsDescSets, &graphicsPushRanges, &shaderPaths, & shaderStageFlags](GLFWwindow* window, int width, int height)
		{
			spdlog::info("Rebuilding graphics data with Width: {} and Height: {}", width, height);

			vkb::SwapchainBuilder swapchainBuilder{ vom.GetPhysicalDevice(), vom.GetDevice(), vom.GetSurface() };
			auto swapchainRet = swapchainBuilder.set_old_swapchain(vom.GetSwapchainData(true).GetSwapchain()).build();
			if (!swapchainRet)
			{
				std::cout << "\n\nNo swapchain for you!!\n\n";
				throw std::logic_error("");
			}
			else
			{
				vkb::Swapchain bootSwapchain = swapchainRet.value();

				//Delete old presentation objects
				vom.DestroyType(vk::SwapchainKHR());
				vom.DestroyType(vk::ImageView());
				vom.DestroyType(vk::Image());
				vom.DestroyType(vkt::VmaImage());

				vom.SetSwapchain({ bootSwapchain.swapchain, (vk::Format)bootSwapchain.image_format, bootSwapchain.extent }, true);

				auto gQueue = vom.GetGraphicsQueue();

				auto depthImage = vom.VmaMakeImage(
					vk::ImageCreateInfo({}, vk::ImageType::e2D, vk::Format::eD32Sfloat, vk::Extent3D(bootSwapchain.extent.width, bootSwapchain.extent.height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
						vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, 1, &gQueue.index, vk::ImageLayout::eUndefined),
					vk::ImageViewCreateInfo({}, {}, vk::ImageViewType::e2D, vk::Format::eD32Sfloat, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)),
					VmaAllocationCreateInfo{ {}, VMA_MEMORY_USAGE_GPU_ONLY });

				vk::ClearColorValue clearColorValue;
				clearColorValue.setFloat32({ 1.0f,0.0f,0.0f,1.0f });
				vk::ClearValue colorClear;
				colorClear.setColor(clearColorValue);
				vk::ClearValue depthClear;
				depthClear.setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));
				renderPassManager = std::make_shared<vkt::RenderPassManager>(vom.GetDevice(), vom.GetSwapchainData().GetExtent().width, vom.GetSwapchainData().GetExtent().height);
				auto colorAttachment = renderPassManager->CreateAttachment(vk::AttachmentDescription({}, vom.GetSwapchainData().GetFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear
					, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR),
					vk::ImageLayout::eColorAttachmentOptimal, colorClear, vk::ImageView());
				auto depthAttachment = renderPassManager->CreateAttachment(vk::AttachmentDescription({}, vk::Format::eD32Sfloat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear
					, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal),
					vk::ImageLayout::eDepthStencilAttachmentOptimal, depthClear, depthImage.view);
				renderPassManager->CreateSubpass(vk::PipelineBindPoint::eGraphics, { colorAttachment }, {}, depthAttachment, { vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput
					, vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eColorAttachmentWrite, {})
					,vk::SubpassDependency(VK_SUBPASS_EXTERNAL, {},
						vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
						vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::AccessFlagBits::eNoneKHR,
						vk::AccessFlagBits::eDepthStencilAttachmentWrite)
					});
				renderPassManager->CreateRenderPass();
				for (auto& image : vom.GetSwapchainData().GetImages())
				{
					colorAttachment->view = image.view;
					renderPassManager->CreateFramebuffer();
				}

				std::vector<::vk::DescriptorSetLayout> setLayouts;
				for (auto const& set : graphicsDescSets)
				{
					setLayouts.emplace_back(set->layout);
				}

				graphicsPipeline = std::make_unique<vkt::GraphicsPipelineManager>(
					vom, renderPassManager->renderPass,
					shaderPaths, shaderStageFlags,
					VertexData::vertexInputBinding(), VertexData::vertexInputAttribute(),
					vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE),
					vk::Viewport(0, 0, bootSwapchain.extent.width, bootSwapchain.extent.height, 0, 1),
					vk::Rect2D({ 0,0, }, bootSwapchain.extent),
					vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, {}, {}, {}, 1.0f),
					vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE),
					vk::PipelineDepthStencilStateCreateInfo({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE, {}, {}, 0.0f, 1.0f),
					std::vector{ vk::PipelineColorBlendAttachmentState(VK_FALSE, {}, {}, {}, {}, {}, {}, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) },
					vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy),
					vk::PipelineLayoutCreateInfo({}, setLayouts.size(), setLayouts.data(), graphicsPushRanges.size(), graphicsPushRanges.data()));

			}

		};
		buildPresentationData(nullptr, static_cast<int>(width), static_cast<int>(height));

		window.AttachResizeAction(buildPresentationData);

		uint32_t imageIndex = 0;
		auto imgAvailable = vom.MakeSemaphore();
		vkt::CommandManager cmdManager(vom, vom.GetGraphicsQueue(), true, vk::PipelineStageFlagBits::eAllGraphics);
		cmdManager.DependsOn({ {nullptr, imgAvailable, vk::PipelineStageFlagBits::eEarlyFragmentTests } });
		auto fence = vom.MakeFence(true);

		while (window.Open())
		{
			imageIndex = vom.GetDevice().acquireNextImageKHR(vom.GetSwapchainData().GetSwapchain(), UINT64_MAX, imgAvailable).value;

			cmdManager.Reset();
			auto cmd = cmdManager.RecordNew();
			cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
			vk::RenderPassBeginInfo renderPassBeginInfo(graphicsPipeline->renderPass, renderPassManager->frames[imageIndex].frameBuffer, vk::Rect2D(0, vom.GetSwapchainData().GetExtent()), 2, renderPassManager->clearValues.data());
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->pipeline);
			cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
			cmd.endRenderPass();
			cmd.end();

			cmdManager.Execute(true, true, true, true);
			auto res = vom.GetGraphicsQueue().queue.presentKHR(vk::PresentInfoKHR(1, &cmdManager.GetMainSignal().semaphore, 1, &vom.GetSwapchainData().swapchain, &imageIndex));

			glfwPollEvents();
		}
	}
}
