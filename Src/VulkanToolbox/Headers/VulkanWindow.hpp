#pragma once
namespace vkt
{
	struct KeyMap
	{
		int scanCode;
		int action;
		std::function<void()> callback;
		KeyMap(int code, int _action, std::function<void()> function);
	};

	class VulkanWindow
	{
	public:
		GLFWwindow* window = nullptr;
		vk::SurfaceKHR surface = nullptr;
		bool resized = true;
		int width, height;
		uint32_t extensionCount;
		const char** extensions;

		VulkanWindow(uint32_t width, uint32_t height);
		VulkanWindow(GLFWmonitor* monitor);
		VulkanWindow() = default;

		void CreateSurface(vk::Instance instance);

		void AttachResizeAction(std::function<void(GLFWwindow* window, int width, int height)> function);

		bool Open();

		std::shared_ptr<KeyMap> AddKeyMap(int scanCode, int glfwAction, std::function<void()> function);

		~VulkanWindow();


	private:
		static void Resized(GLFWwindow* window, int width, int height);
		static void ExecuteKeys(GLFWwindow* window, int key, int scancode, int action, int mods);

		std::list<std::function<void(GLFWwindow* window, int width, int height)>> resizeActions;
		std::list<std::shared_ptr<KeyMap>> keyMaps;
	};
}
