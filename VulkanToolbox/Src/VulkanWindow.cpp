#include "VulkanToolbox.hpp"
namespace vkt
{

	KeyMap::KeyMap(int code, int _action, std::function<void()> function)
	{
		scanCode = code;
		action = _action;
		callback = function;
	};


	VulkanWindow::VulkanWindow(uint32_t width, uint32_t height)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(width, height, "Game", NULL, NULL);

		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, Resized);
		glfwSetKeyCallback(window, ExecuteKeys);
	};
	VulkanWindow::VulkanWindow(GLFWmonitor* monitor)
	{
		window = glfwCreateWindow({}, {}, "Game", glfwGetPrimaryMonitor(), NULL);
		glfwSetWindowSizeCallback(window, Resized);
	};

	void VulkanWindow::CreateSurface(vk::Instance instance)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, (VkSurfaceKHR*)&surface) != VkResult::VK_SUCCESS)
		{
			spdlog::error("\nCould not create glfw surface\nInstance pointer: {}\nWindow pointer: {}\nOutput surface pointer: {}\n", (uint64_t)(VkInstance)instance, (void*)window, (uint64_t)(VkSurfaceKHR)surface);
			abort();
		}
	}

	void VulkanWindow::AttachResizeAction(std::function<void(GLFWwindow* window, int width, int height)> function)
	{
		resizeActions.emplace_back(function);
	}

	bool VulkanWindow::Open()
	{
		return !glfwWindowShouldClose(window);
	}

	std::shared_ptr<KeyMap> VulkanWindow::AddKeyMap(int scanCode, int glfwAction, std::function<void()> function)
	{
		keyMaps.emplace_back(std::make_shared<KeyMap>(scanCode, glfwAction, function));
		return keyMaps.back();
	}

	VulkanWindow::~VulkanWindow()
	{
		glfwDestroyWindow(window);
	}


	void VulkanWindow::Resized(GLFWwindow * window, int width, int height)
	{
		VulkanWindow* obj = static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		obj->resized = true;
		obj->width = width; obj->height = height;

		for (auto& action : obj->resizeActions)
		{
			action(window, width, height);
		}

	}
	void VulkanWindow::ExecuteKeys(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		VulkanWindow* obj = static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		for (auto const& map : obj->keyMaps)
		{
			if (map->scanCode == scancode && map->action == action)
			{
				map->callback();
			}
		}
	}
}
