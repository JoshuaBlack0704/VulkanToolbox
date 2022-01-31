#pragma once
namespace vkt
{
	class VulkanWindow
	{
	public:
		GLFWwindow* window = nullptr;
		vk::SurfaceKHR surface = nullptr;
		bool resized = true;
		int width, height;
		uint32_t extensionCount;
		const char** extensions;
		VulkanWindow(uint32_t width, uint32_t height)
		{
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			window = glfwCreateWindow(width, height, "Game", NULL, NULL);

			glfwSetWindowUserPointer(window, this);
			glfwSetWindowSizeCallback(window, Resized);
		};
		VulkanWindow(GLFWmonitor* monitor)
		{
			window = glfwCreateWindow({}, {}, "Game", glfwGetPrimaryMonitor(), NULL);
			glfwSetWindowSizeCallback(window, Resized);
		};
		VulkanWindow() = default;

		void CreateSurface(vk::Instance instance)
		{
			if (glfwCreateWindowSurface(instance, window, nullptr, (VkSurfaceKHR*)&surface) != VkResult::VK_SUCCESS)
			{
				spdlog::error("\nCould not create glfw surface\nInstance pointer: {}\nWindow pointer: {}\nOutput surface pointer: {}\n", (uint64_t)(VkInstance)instance, (void*)window, (uint64_t)(VkSurfaceKHR)surface);
				abort();
			}
		}

		bool Open()
		{
			return !glfwWindowShouldClose(window);
		}

		~VulkanWindow()
		{
			glfwDestroyWindow(window);
		}


	private:
		static void Resized(GLFWwindow* window, int width, int height)
		{
			VulkanWindow* obj = static_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
			obj->resized = true;
			width = width; height = height;
			spdlog::info("Window resized! New width: {} New height: {}", width, height);
		}

	};
}
