// Proceduralization.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <random>
#define VMA_IMPLEMENTATION
#include <VulkanToolbox.hpp>
#define GLM_FORCE_RANDIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <VkBootstrap.h>
#include <spdlog/stopwatch.h>
#undef MemoryBarrier


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
	float& deltaTime;
	float& timeSpeedFactor;
	vkt::VulkanObjectManager& vom;

	Character(vkt::VulkanWindow& window, vkt::VulkanObjectManager& _vom, float& _deltaTime, float& _timeSpeedFactor) : deltaTime(_deltaTime), vom(_vom), timeSpeedFactor(_timeSpeedFactor)
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
			rotMatrix = glm::rotate(rotMatrix, glm::radians(90 * static_cast<float>(deltaTime)), (rotAxis));
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
			rotMatrix = glm::rotate(rotMatrix, glm::radians(90 * static_cast<float>(_externalDeltaTime)), (rotAxis));
			center = rotMatrix * glm::vec4(0, 0, 1, 1);
		}


		vel += accel;
		pos += rotMatrix * vel * glm::vec4(_externalDeltaTime, _externalDeltaTime, _externalDeltaTime, 1);

		return this;
	}
	glm::mat4 GetPMat()
	{
		float aspect = static_cast<float>(vom.GetSwapchainData().GetExtent().width) / static_cast<float>(vom.GetSwapchainData().GetExtent().height);
		return glm::perspective(glm::radians(70.0f), aspect, .1f, 1000.0f);
	}
	glm::mat4 GetVMat()
	{
		return glm::lookAt(static_cast<glm::vec3>(pos), static_cast<glm::vec3>(pos + center), glm::vec3(0, -1, 0));
	}
	glm::mat4 GetClipMat()
	{
		return clip;
	};
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
struct Translation
{
	alignas(16)glm::vec3 position;
	alignas(16)glm::vec3 target;
	alignas(4)float speed;
	alignas(1)bool enRoute;
};
struct CountData
{
	alignas(4)int objectCount;
	alignas(4)int staticsCount;
	alignas(4)float maxDimension;
	alignas(4)float deltaTime;
	alignas(4)int frameCount = 1;
	alignas(4)int vertexCount;
};
struct CamData
{
	alignas(16)glm::mat4 clipMatrix;
	alignas(16)glm::mat4 projectionMatrix;
	alignas(16)glm::mat4 viewMatrix;
	alignas(16)glm::vec3 pos;
};
struct LightData
{
	alignas(16)glm::vec3 lightColor;
	alignas(16)glm::vec3 lightDirection;
	alignas(16)glm::vec3 ambientColor;
	alignas(4)float lightIntensity;
	alignas(4)float ambientIntensity;
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
#ifndef NDEBUG
	int objectCount = 1000;
	int staticCount = 50;
	float maxDimension = 200;
#else
	int objectCount = 0;
	int staticCount = 0;
	float maxDimension = 0;
	spdlog::info("Please Enter Object Count");
	std::cin >> objectCount;
	spdlog::info("Please Enter static Count");
	std::cin >> staticCount;
	spdlog::info("Please Enter domain size");
	std::cin >> maxDimension;
#endif

	vkt::VulkanObjectManager vom({}, false, true);
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
		VkPhysicalDeviceFeatures features{}; features.samplerAnisotropy = VK_TRUE;
		VkPhysicalDeviceVulkan11Features features11{};
		VkPhysicalDeviceVulkan12Features features12{}; features12.timelineSemaphore = VK_TRUE; 
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
		Tetrahedron objectData;
		vk::PushConstantRange countRange(vk::ShaderStageFlagBits::eCompute, 0, sizeof(CountData));
		CountData countData{ objectCount, staticCount, maxDimension };
		countData.vertexCount = objectData.vertices.size();
		vkt::VulkanObjectManager pVom(vom);
		pVom.SetSurface(vom.GetSurface());
		pVom.SetPhysicalDevice(vom.GetPhysicalDevice());
		pVom.SetAllocator(vom.GetAllocator());
		pVom.SetGraphicsQueue(vom.GetGraphicsQueue());
		vkt::VmaImage depthImage;
		vk::Sampler depthImageSampler;
		vk::Pipeline graphicsPipeline;
		vk::PipelineLayoutCreateInfo gPipelineLayoutCreateInfo;
		vk::PipelineLayout gPipelineLayout;
		vkt::RenderPassManager renderpassManager(vom.GetDevice());
		vk::SamplerCreateInfo samplerCreate{};
		samplerCreate.magFilter = vk::Filter::eLinear;
		samplerCreate.minFilter = vk::Filter::eLinear;
		samplerCreate.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerCreate.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerCreate.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerCreate.anisotropyEnable = VK_TRUE;
		samplerCreate.maxAnisotropy = pVom.GetPhysicalDevice().getProperties().limits.maxSamplerAnisotropy;
		samplerCreate.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerCreate.unnormalizedCoordinates = VK_FALSE;
		samplerCreate.compareEnable = VK_FALSE;
		samplerCreate.compareOp = vk::CompareOp::eAlways;
		samplerCreate.mipmapMode = vk::SamplerMipmapMode::eLinear;
		depthImageSampler = pVom.MakeImageSampler(samplerCreate);


		auto BuildPresentationData = [&pVom, &depthImage, &depthImageSampler](GLFWwindow* window,int,int)
		{
			if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
			{
				vkb::SwapchainBuilder swapchainBuilder(pVom.GetPhysicalDevice(), pVom.GetDevice(), pVom.GetSurface());
				auto ret = swapchainBuilder.set_old_swapchain(pVom.GetSwapchainData(true).GetSwapchain()).build();
				spdlog::info("Shaw chain builder chose format: {}", ret->image_format);
				pVom.DestroyAll();
				assert(ret);
				pVom.SetSwapchain(ret->swapchain, static_cast<vk::Format>(ret->image_format), ret->extent, true);
				auto gQueue = pVom.GetGraphicsQueue();
				vk::Format depthFormat = vk::Format::eD32Sfloat;
				depthImage = pVom.VmaMakeImage(
					vk::ImageCreateInfo(
						{},
						vk::ImageType::e2D,
						depthFormat,
						vk::Extent3D(
							ret->extent.width,
							ret->extent.height,
							1),
						1,
						1,
						vk::SampleCountFlagBits::e1,
						vk::ImageTiling::eOptimal,
						vk::ImageUsageFlagBits::eDepthStencilAttachment,
						vk::SharingMode::eExclusive,
						1,
						&gQueue.index,
						vk::ImageLayout::eDepthStencilAttachmentOptimal),
					vk::ImageViewCreateInfo(
						{},
						{},
						vk::ImageViewType::e2D,
						depthFormat,
						vk::ComponentMapping(),
						vk::ImageSubresourceRange(
							vk::ImageAspectFlagBits::eDepth,
							0,
							1,
							0,
							1)),
					VmaAllocationCreateInfo{
						{},
						VMA_MEMORY_USAGE_GPU_ONLY });
				
			}
		};
		auto BuildGraphicsPipeline = [&pVom, &graphicsPipeline, &depthImage, &gPipelineLayoutCreateInfo, &gPipelineLayout](GLFWwindow* window,int,int)
		{
			if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
			{
				auto vert = pVom.MakeShaderModule("shaders/vert.spv");
				auto frag = pVom.MakeShaderModule("shaders/frag.spv");

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
		auto BuildGraphicsPipelineWithRenderPass = [&pVom, &graphicsPipeline, &depthImage, &gPipelineLayoutCreateInfo, &gPipelineLayout, &renderpassManager](GLFWwindow* window, int, int)
		{
			if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED))
			{
				auto vert = pVom.MakeShaderModule("shaders/vert.spv");
				auto frag = pVom.MakeShaderModule("shaders/frag.spv");

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
					&renderViewport,
					1,
					&scissor);
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
				vk::PipelineColorBlendStateCreateInfo colorBlendState({}, VK_FALSE, {}, 1, &blendAttachment);

				vk::ClearColorValue clearColorValue;
				clearColorValue.setFloat32({ 0.0f,0.0f,0.0f,1.0f });
				vk::ClearValue colorClear;
				colorClear.setColor(clearColorValue);
				vk::ClearValue depthClear;
				depthClear.setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));
				renderpassManager.Dispose();
				renderpassManager = vkt::RenderPassManager(pVom.GetDevice(), pVom.GetSwapchainData().GetExtent().width, pVom.GetSwapchainData().GetExtent().height);
				auto colorAttachment = renderpassManager.CreateAttachment(vk::AttachmentDescription({}, pVom.GetSwapchainData().GetFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear
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
				for (auto& image : pVom.GetSwapchainData().GetImages())
				{
					colorAttachment->view = image.view;
					renderpassManager.CreateFramebuffer();
				}

				vk::GraphicsPipelineCreateInfo gCreateInfo(
					{},
					shaders.size(),
					shaders.data(),
					&vertexState,
					&assemblyState,
					&tessellationState,
					&viewportState,
					&rasterizationState,
					&multisampleState,
					&depthStencilState,
					&colorBlendState,
					{},
					gPipelineLayout,
					renderpassManager.renderPass,
					0,
					{},
					{}
				);

				graphicsPipeline = pVom.MakePipeline({}, gCreateInfo);
			}
		};
		window.AttachResizeAction(BuildPresentationData);
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

			
			vkt::ComputePipelineManager randomGenStage(vom, vk::PipelineLayoutCreateInfo({}, 1, &randomGenDescSet->layout, 1, &countRange), "shaders/randomgen.spv");
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

		vkt::DescriptorManager descriptorManager(vom);
		auto stateUpdateSet = descriptorManager.GetNewSet();
		stateUpdateSet->AttachSector(positionsSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(staticsSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(matrixSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(camdataSector, vk::ShaderStageFlagBits::eCompute);
		stateUpdateSet->AttachSector(modelMatrixSector, vk::ShaderStageFlagBits::eCompute);
		auto graphicsSet = descriptorManager.GetNewSet();
		graphicsSet->AttachSector(matrixSector, vk::ShaderStageFlagBits::eVertex);
		graphicsSet->AttachSector(modelMatrixSector, vk::ShaderStageFlagBits::eVertex);
		descriptorManager.Update();
		LightData lightData{ glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f,1.0f,1.0f), glm::vec3(1.0f,1.0f,1.0f), 1.0f, 0.1f};
		vk::PushConstantRange lightRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(lightData));
		gPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo({}, 1, &graphicsSet->layout, 1, &lightRange);
		////BuildGraphicsPipeline(window.window, {}, {});
		BuildGraphicsPipeline(window.window, 0, 0);

		vkt::ComputePipelineManager stateUpdate(vom, vk::PipelineLayoutCreateInfo({}, 1, &stateUpdateSet->layout, 1, &countRange), "shaders/stateupdate.spv");

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

#ifndef NDEBUG
		std::vector<Translation> ships;
		std::vector<glm::mat4> matrices;
		matrices.resize(countData.objectCount);
		ships.resize(countData.objectCount);
		vkt::MemoryOperationsBuffer returnOps(vom);
		auto returnShips = returnOps.SectorToRam(positionsSector, ships.data());
		auto returnMatrices = returnOps.SectorToRam(matrixSector, matrices.data());
#endif

		float accumulatedTime = 0;

		while (window.Open())
		{
#ifndef NDEBUG
			returnOps.Execute({}, true);
			returnShips.Execute();
			returnMatrices.Execute();
#endif
			sw.reset();
			if (!glfwGetWindowAttrib(window.window, GLFW_ICONIFIED))
			{
				character.Move(countData.deltaTime/timeSpeedFactor);
				CamData camData{ character.GetClipMat(), character.GetPMat(), character.GetVMat(), glm::vec3(character.pos)};
				imageIndex = vom.GetDevice().acquireNextImageKHR(pVom.GetSwapchainData().GetSwapchain(), UINT64_MAX, imgAvailable).value;
				frameOps.Clear(true);
				frameOps.RamToSector(&camData, camdataSector, sizeof(camData));
				frameOps.Execute();
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
					vk::PipelineStageFlagBits::eComputeShader,
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
					vk::AttachmentLoadOp::eLoad,
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
				//vk::RenderPassBeginInfo renderPassBeginInfo(renderpassManager.renderPass, renderpassManager.frames[imageIndex].frameBuffer, vk::Rect2D({}, pVom.GetSwapchainData().GetExtent()), renderpassManager.clearValues.size(), renderpassManager.clearValues.data());
				cmd.beginRendering(&renderingInfo);
				//cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
				cmd.draw(objectData.vertices.size(), countData.objectCount, 0, 0);
				cmd.endRendering();
				//cmd.endRenderPass();
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
