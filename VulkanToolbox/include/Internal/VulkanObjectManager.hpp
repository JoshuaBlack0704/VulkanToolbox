#pragma once
namespace vkt
{
	/**
	 * \brief A struct used by the VulkanObjectManager class to store buffers created by a vmaAllocator
	 */
	struct VmaBuffer
	{
		/**
		 * \brief The allocations buffer
		 */
		vk::Buffer buffer;

		/**
		 * \brief The allocation
		 */
		VmaAllocation allocation;

		/**
		 * \brief The allocation info
		 */
		VmaAllocationInfo allocationInfo;
	};

	/**
	 * \brief A struct used by the VulkanObjectManager class to store images created by a vmaAllocator
	 */
	struct VmaImage
	{
		/**
		 * \brief The allocations image
		 */
		vk::Image image;

		/**
		 * \brief A view that is created by the VulkanObjectManager right after allocation
		 */
		vk::ImageView view;

		/**
		 * \brief The allocation
		 */
		VmaAllocation allocation;

		/**
		 * \brief The allocations info
		 */
		VmaAllocationInfo allocationInfo;
	};

	/**
	 * \brief A struct to pass a queue and its index
	 */
	struct QueueData
	{
		/**
		 * \brief The index of the queue
		 */
		uint32_t index;

		/**
		 * \brief The queue
		 */
		vk::Queue queue;
	};

	/**
	 * \brief A struct used by many managers to store the data needed to wait on either a normal or timelinesemaphore
	 */
	struct WaitData
	{
		/**
		 * \brief This is a pointer to a value that can later be dereferenced to provide an accurate wait value
		 */
		std::shared_ptr<uint64_t> waitValuePtr;

		/**
		 * \brief The semaphore of this wait data, if can be infered either a normal or timeline semaphore by the if the waitValuePtr is nullptr or not
		 */
		vk::Semaphore waitSemaphore;

		/**
		 * \brief The stage that accompanies a waitSemaphore submission for a vk::submit
		 */
		vk::PipelineStageFlags waitStage;


		/**
		 * \brief The main constructor
		 * \param _waitValuePtr This can be either null or a real pointer which can help distinguish the between a normal or timelineSemaphore
		 * \param _waitSemaphore Must be a valid semaphore
		 * \param _waitStage The accompanying wait stage
		 */
		WaitData(std::shared_ptr<uint64_t> _waitValuePtr, vk::Semaphore _waitSemaphore, vk::PipelineStageFlags _waitStage)
		{
			waitValuePtr = _waitValuePtr;
			waitSemaphore = _waitSemaphore;
			waitStage = _waitStage;
		}
	};

	/**
	 * \brief A struct used by the VulkanObjectManager class to store all the data it needs to abstract a swapchain
	 */
	struct SwapchainData
	{
		/**
		 * \brief The managed swapchain
		 */
		vk::SwapchainKHR swapchain;

		/**
		 * \brief The format chosen for the current swapchain
		 */
		vk::Format imageFormat;

		/**
		 * \brief The extent of the current swapchain
		 */
		vk::Extent2D extent;

		/**
		 * \brief All of the images wrapped in a VmaImage struct
		 */
		std::vector<VmaImage> images;

		/**
		 * \brief Can be used to return a certain swapchain image by index
		 * \param index The index of the requested swapchain index
		 * \return A VmaImage struct containing both the swapchain image and its view
		 */
		VmaImage GetImage(uint64_t index)
		{
			return images[index];
		}

		/**
		 * \brief Gets the managed swapchain
		 * \return 
		 */
		vk::SwapchainKHR GetSwapchain()
		{
			return swapchain;
		}

		/**
		 * \brief Gets the managed swapchain's format
		 * \return 
		 */
		vk::Format GetFormat()
		{
			return imageFormat;
		}

		/**
		 * \brief Gets the managed swapchains's extent
		 * \return 
		 */
		vk::Extent2D GetExtent()
		{
			return extent;
		}

		/**
		 * \brief Returns the whole images vector 
		 * \return A vector of VmaImage structs
		 */
		std::vector<VmaImage>& GetImages()
		{
			return images;
		}
	};


	/**
	 * \brief The key component of all the other managers, a system that handles the creation of other vulkan objects as well as their lifetimes in RAII style
	 */
	class VulkanObjectManager
	{
	public:
		/**
		 * \brief This constructor uses a parent vom, NOTE this will make the new vom a derivative of the parent and will share the devices
		 * \param _vom The parent vom whose device will be used to initialize the new vom
		 */
		VulkanObjectManager(VulkanObjectManager& _vom)
		{
			SetDevice(_vom.GetDevice());
		}

		/**
		 * \brief This constructor uses an external device handle to initialize its self
		 * \param deviceHandle The external device handle
		 * \param manage Whether to tie the device handle's lifetime to the new vom
		 */
		VulkanObjectManager(vk::Device deviceHandle, bool manage = false)
		{
			SetDevice(deviceHandle);
			if (manage)
			{
				Manage(deviceHandle);
			}
		}

		/**
		 * \brief returns the currently mounted device
		 * \return The currently mounted device
		 */
		vk::Device GetDevice()
		{
			assert(device != NULL);
			return device;
		}
		/**
		 * \brief returns the currently mounted Instance
		 * \return The currently mounted Instance
		 */
		vk::Instance GetInstance()
		{
			assert(instance != NULL);
			return instance;
		}
		/**
		 * \brief returns the currently mounted PhysicalDevice
		 * \return The currently mounted PhysicalDevice
		 */
		vk::PhysicalDevice GetPhysicalDevice()
		{
			assert(physicalDevice != NULL);
			return physicalDevice;
		}
		/**
		 * \brief returns the currently mounted Surface
		 * \return The currently mounted Surface
		 */
		vk::SurfaceKHR GetSurface()
		{
			assert(surface != NULL);
			return surface;
		}
		/**
		 * \brief returns the currently mounted SwapchainData struct
		 * \return The currently mounted SwapchainData struct
		 */
		SwapchainData& GetSwapchainData(bool ignoreCheck = false)
		{
			if (!ignoreCheck)
			{
				assert(swapchain.swapchain != NULL);
			}
			return swapchain;
		}

		/**
		 * \brief Sets the mounted device used to perform the vom's functions
		 * \param _deviceToMount The device to mount, NOTE this does not tie the devices lifetime to the vom
		 */
		void SetDevice(vk::Device _deviceToMount)
		{
			assert(_deviceToMount != NULL);
			device = _deviceToMount;
		}

		/**
		 * \brief Sets the mounted Instance used to perform the vom's functions
		 * \param _instanceToMount The Instance to mount, NOTE this does not tie the Instance's lifetime to the vom
		 */
		void SetInstance(vk::Instance _instanceToMount)
		{
			assert(_instanceToMount != NULL);
			instance = _instanceToMount;
		}
		/**
		 * \brief Sets the mounted device used to perform the vom's functions
		 * \param _deviceToMount The device to mount, NOTE this does not tie the devices lifetime to the vom
		 */
		void SetPhysicalDevice(vk::PhysicalDevice _physicalDeviceToMount)
		{
			assert(_physicalDeviceToMount != NULL);
			physicalDevice = _physicalDeviceToMount;
		}
		/**
		 * \brief Sets the mounted device used to perform the vom's functions
		 * \param _deviceToMount The device to mount, NOTE this does not tie the devices lifetime to the vom
		 */
		void SetQueues(QueueData _graphicsQueueToMount, QueueData _transferQueueToMount, QueueData _computeQueueToMount)
		{
			assert(_graphicsQueueToMount.queue != NULL);
			assert(_transferQueueToMount.queue != NULL);
			assert(_computeQueueToMount.queue != NULL);
			graphicsQueue = _graphicsQueueToMount;
			transferQueue = _transferQueueToMount;
			computeQueue = _computeQueueToMount;
		}
		void SetSurface(vk::SurfaceKHR _surfaceToMount)
		{
			assert(_surfaceToMount != NULL);
			surface = _surfaceToMount;
		}
		void SetSwapchain(SwapchainData swapchainData, bool manage = false)
		{
			assert(swapchainData.swapchain != NULL);
			swapchain = swapchainData;
			if (manage)
			{
				Manage(GetSwapchainData());
			}
			if (swapchainData.imageFormat != vk::Format())
			{
				swapchain.images.clear();
				auto images = GetDevice().getSwapchainImagesKHR(GetSwapchainData().GetSwapchain());
				for (size_t i = 0; i < images.size(); i++)
				{
					swapchain.images.emplace_back
					(images[i],
						MakeImageView
						(vk::ImageViewCreateInfo({},
							images[i],
							vk::ImageViewType::e2D,
							swapchainData.imageFormat,
							vk::ComponentMapping(),
							vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
						)
					);


				}
			}
		}

		QueueData GetTransferQueue()
		{
			assert(transferQueue.queue != NULL);
			return transferQueue;
		}
		QueueData GetGraphicsQueue()
		{
			assert(graphicsQueue.queue != NULL);
			return graphicsQueue;
		}
		QueueData GetComputeQueue()
		{
			assert(computeQueue.queue != NULL);
			return computeQueue;
		}
		QueueData GetGeneralQueue()
		{
			assert(generalQueue.queue != NULL);
			return generalQueue;
		}
		void SetTransferQueue(QueueData _transferQueue)
		{
			assert(_transferQueue.queue != NULL);
			transferQueue = _transferQueue;
		}
		void SetGraphicsQueue(QueueData _graphicsQueue)
		{
			assert(_graphicsQueue.queue != NULL);
			graphicsQueue = _graphicsQueue;
		}
		void SetComputeQueue(QueueData _computeQueue)
		{
			assert(_computeQueue.queue != NULL);
			computeQueue = _computeQueue;
		}
		void SetGeneralQueue(QueueData _generalQueue)
		{
			assert(_generalQueue.queue != NULL);
			generalQueue = _generalQueue;
		}

		vk::Semaphore MakeSemaphore(bool manage = true)
		{

			vk::SemaphoreCreateInfo createInfo;
			auto semaphoreHandle = GetDevice().createSemaphore(createInfo);
			if (manage)
			{
				Manage(semaphoreHandle);
			}
			return semaphoreHandle;
		}
		vk::Semaphore MakeTimelineSemaphore(uint64_t startingValue, bool manage = true)
		{
			vk::SemaphoreTypeCreateInfo timeCreateInfo(vk::SemaphoreType::eTimeline, startingValue);
			vk::SemaphoreCreateInfo createInfo;
			createInfo.pNext = &timeCreateInfo;
			auto semaphoreHandle = GetDevice().createSemaphore(createInfo);
			if (manage)
			{
				Manage(semaphoreHandle);
			}
			return semaphoreHandle;
		}
		vk::Fence MakeFence(bool signaled, bool manage = true)
		{

			vk::FenceCreateInfo createInfo;
			if (signaled)
			{
				createInfo.flags = vk::FenceCreateFlagBits::eSignaled;
			}
			auto fenceHandle = GetDevice().createFence(createInfo);
			if (manage)
			{
				Manage(fenceHandle);
			}
			return fenceHandle;
		}
		vk::CommandPool MakeCommandPool(vk::CommandPoolCreateInfo createInfo, bool manage = true)
		{
			auto commandPool = GetDevice().createCommandPool(createInfo);
			if (manage)
			{
				Manage(commandPool);
			}
			return commandPool;
		}
		std::vector<vk::CommandBuffer> MakeCommandBuffers(vk::CommandBufferAllocateInfo alocInfo)
		{
			return GetDevice().allocateCommandBuffers(alocInfo);
		}
		vk::RenderPass MakeRenderPass(vk::RenderPassCreateInfo createInfo, bool manage = true)
		{
			auto renderpass = GetDevice().createRenderPass(createInfo);
			if (manage)
			{
				Manage(renderpass);
			}
			return renderpass;
		}
		vk::Pipeline MakePipeline(vk::PipelineCache cache, vk::GraphicsPipelineCreateInfo createInfo, bool manage = true)
		{
			vk::ResultValue<vk::Pipeline> pipeline = GetDevice().createGraphicsPipeline(cache, createInfo);
			if (manage)
			{
				Manage(pipeline.value);
			}
			return pipeline.value;
		}
		vk::Pipeline MakePipeline(vk::PipelineCache cache, vk::ComputePipelineCreateInfo createInfo, bool manage = true)
		{
			vk::ResultValue<vk::Pipeline> pipeline = GetDevice().createComputePipeline(cache, createInfo);
			if (manage)
			{
				Manage(pipeline.value);
			}
			return pipeline.value;
		}
		vk::PipelineLayout MakePipelineLayout(vk::PipelineLayoutCreateInfo createInfo, bool manage = true)
		{
			auto layout = GetDevice().createPipelineLayout(createInfo);
			if (manage)
			{
				Manage(layout);
			}
			return layout;
		}
		vk::Buffer MakeBuffer(vk::BufferCreateInfo createInfo, bool manage = true)
		{
			auto buffer = GetDevice().createBuffer(createInfo);
			if (manage)
			{
				Manage(buffer);
			}
			return buffer;
		}
		vk::DeviceMemory MakeMemoryAllocation(vk::MemoryAllocateInfo alocInfo, bool manage = true)
		{
			auto aloc = GetDevice().allocateMemory(alocInfo);
			if (manage)
			{
				Manage(aloc);
			}
			return aloc;
		}
		vk::DescriptorPool MakeDescriptorPool(vk::DescriptorPoolCreateInfo createInfo, bool manage = true)
		{
			auto pool = GetDevice().createDescriptorPool(createInfo);
			if (manage)
			{
				Manage(pool);
			}
			return pool;
		}
		vk::DescriptorSetLayout MakeDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo createInfo, bool manage = true)
		{
			auto layout = GetDevice().createDescriptorSetLayout(createInfo);
			if (manage)
			{
				Manage(layout);
			}
			return layout;
		}
		std::vector<vk::DescriptorSet> MakeDescriptorSets(vk::DescriptorSetAllocateInfo allocateInfo)
		{
			return GetDevice().allocateDescriptorSets(allocateInfo);
		}
		vk::Image MakeImage(vk::ImageCreateInfo createInfo, bool manage = true)
		{
			auto image = GetDevice().createImage(createInfo);
			if (manage)
			{
				Manage(image);
			}
			return image;
		}
		vk::ImageView MakeImageView(vk::ImageViewCreateInfo createInfo, bool manage = true)
		{
			auto imageView = GetDevice().createImageView(createInfo);
			if (manage)
			{
				Manage(imageView);
			}
			return imageView;
		}
		VmaAllocator GetAllocator()
		{
			assert(allocator != NULL);
			return allocator;
		}
		VmaAllocator MakeAllocator(VmaAllocatorCreateInfo createInfo, bool mount = false, bool manage = true)
		{
			VmaAllocator allocatorHandle;
			vmaCreateAllocator(&createInfo, &allocatorHandle);
			if (manage)
			{
				Manage(allocatorHandle);
			}
			if (mount)
			{
				allocator = allocatorHandle;
			}
			return allocatorHandle;
		}
		VmaAllocator MakeAllocator(uint32_t apiVersion, bool mount = false, bool manage = true)
		{

			VmaAllocatorCreateInfo allocatorCreateInfo = {};
			allocatorCreateInfo.vulkanApiVersion = apiVersion;
			allocatorCreateInfo.physicalDevice = GetPhysicalDevice();
			allocatorCreateInfo.device = GetDevice();
			allocatorCreateInfo.instance = GetInstance();
			VmaAllocator allocatorHandle;
			vmaCreateAllocator(&allocatorCreateInfo, &allocatorHandle);
			if (manage)
			{
				Manage(allocatorHandle);
			}
			if (mount)
			{
				allocator = allocatorHandle;
			}
			return allocatorHandle;
		}
		void SetAllocator(VmaAllocator allocatorHandle)
		{
			allocator = allocatorHandle;
		}
		VmaBuffer VmaMakeBuffer(vk::BufferCreateInfo bufferInfo, VmaAllocationCreateInfo allocationCreateInfo, bool manage = true)
		{

			VmaBuffer allocation;
			VkBufferCreateInfo bufferCreateInfo = bufferInfo;
			VkBuffer buf;
			vmaCreateBuffer(GetAllocator(), &bufferCreateInfo, &allocationCreateInfo, &buf, &allocation.allocation, &allocation.allocationInfo);
			allocation.buffer = buf;
			if (manage)
			{
				Manage(allocation);
			}
			return allocation;
		}
		VmaImage VmaMakeImage(vk::ImageCreateInfo imageInfo, vk::ImageViewCreateInfo viewInfo, VmaAllocationCreateInfo allocationCreateInfo, bool manage = true)
		{
			VkImage image;
			VmaImage imageData;
			VkImageCreateInfo _imageInfo = imageInfo;
			vmaCreateImage(GetAllocator(), &_imageInfo, &allocationCreateInfo, &image, &imageData.allocation, &imageData.allocationInfo);
			imageData.image = image;
			viewInfo.image = imageData.image;
			imageData.view = MakeImageView(viewInfo);
			if (manage)
			{
				Manage(imageData);
			}
			return imageData;

		}
		vk::ShaderModule MakeShaderModule(const char* shaderPath, bool manage = true)
		{
			std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
			assert(file.is_open());
			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);
			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();
			vk::ShaderModuleCreateInfo moduleCreateInfo({}, buffer.size(), reinterpret_cast<const uint32_t*>(buffer.data()));
			auto module = GetDevice().createShaderModule(moduleCreateInfo);
			if (manage)
			{
				Manage(module);
			}
			return module;
		}
		vk::Framebuffer MakeFramebuffer(vk::FramebufferCreateInfo createInfo, bool manage = true)
		{
			auto frame = GetDevice().createFramebuffer(createInfo);
			if (true)
			{
				Manage(frame);
			}
			return frame;
		}

		void Manage(vk::Framebuffer frame)
		{
			framebuffersToDestroy.emplace_back(frame);
		}
		void Manage(vk::ShaderModule module)
		{
			assert(module != NULL);
			shaderModulesToDestroy.emplace_back(module);
		}
		void Manage(vk::Semaphore semaphore)
		{
			assert(semaphore != NULL);
			semaphoresToDestroy.emplace_back(semaphore);
		}
		void Manage(vk::Fence fence)
		{
			assert(fence != NULL);
			fencesToDestroy.emplace_back(fence);
		}
		void Manage(vk::CommandPool commandPool)
		{
			assert(commandPool != NULL);
			commandPoolsToDestroy.emplace_back(commandPool);
		}
		void Manage(vk::RenderPass renderPass)
		{
			assert(renderPass != NULL);
			renderPassesToDestroy.emplace_back(renderPass);
		}
		void Manage(vk::Pipeline pipeline)
		{
			assert(pipeline != NULL);
			pipelineToDestroy.emplace_back(pipeline);
		}
		void Manage(vk::PipelineLayout layout)
		{
			assert(layout != NULL);
			pipelineLayoutsToDestroy.emplace_back(layout);
		}
		void Manage(vk::Buffer buffer)
		{
			assert(buffer != NULL);
			buffersToDestroy.emplace_back(buffer);
		}
		void Manage(vk::DeviceMemory aloc)
		{
			assert(aloc != NULL);
			memoryAllocationsToDestroy.emplace_back(aloc);
		}
		void Manage(vk::Device device)
		{
			assert(device != NULL);
			devicesToDestroy.emplace_back(device);
		}
		void Manage(vk::DescriptorPool pool)
		{
			assert(pool != NULL);
			descriptorPoolsToDestroy.emplace_back(pool);
		}
		void Manage(vk::DescriptorSetLayout layout)
		{
			assert(layout != NULL);
			descriptorSetLayoutsToDestroy.emplace_back(layout);
		}
		void Manage(vk::Instance instance)
		{
			assert(instance != NULL);
			instancesToDestroy.emplace_back(instance);
		}
		void Manage(vk::SurfaceKHR surface)
		{
			assert(surface != NULL);
			surfacesToDestroy.emplace_back(surface);
		}
		void Manage(SwapchainData swapchainData)
		{
			assert(swapchainData.swapchain != NULL);
			swapchainsToDestroy.emplace_back(swapchainData.swapchain);
		}
		void Manage(vk::Image image)
		{
			assert(image != NULL);
			imagesToDestroy.emplace_back(image);
		}
		void Manage(vk::ImageView imageView)
		{
			assert(imageView != NULL);
			imageViewsToDestroy.emplace_back(imageView);
		}
		void Manage(VmaAllocator allocator)
		{
			assert(allocator != NULL);

			allocatorsToDestroy.emplace_back(allocator);
		}
		void Manage(VmaBuffer buffer)
		{
			assert(buffer.buffer != NULL);

			vmaBuffersToDestroy.emplace_back(buffer);
		}
		void Manage(VmaImage image)
		{
			assert(image.image != NULL);

			vmaImagesToDestroy.emplace_back(image);
		}

		void DestroyType(vk::Framebuffer)
		{
			while (!framebuffersToDestroy.empty())
			{
				device.destroyFramebuffer(framebuffersToDestroy.back());
				framebuffersToDestroy.pop_back();
			}
		}
		void DestroyType(vk::ShaderModule)
		{
			while (!shaderModulesToDestroy.empty())
			{
				device.destroyShaderModule(shaderModulesToDestroy.back());
				shaderModulesToDestroy.pop_back();
			}
		}
		void DestroyType(vk::Semaphore)
		{
			while (!semaphoresToDestroy.empty())
			{
				GetDevice().destroySemaphore(semaphoresToDestroy.front());
				semaphoresToDestroy.pop_front();
			}
		}
		void DestroyType(vk::Fence)
		{
			while (!fencesToDestroy.empty())
			{
				GetDevice().destroyFence(fencesToDestroy.front());
				fencesToDestroy.pop_front();
			}
		}
		void DestroyType(vk::CommandPool)
		{
			while (!commandPoolsToDestroy.empty())
			{
				GetDevice().destroyCommandPool(commandPoolsToDestroy.front());
				commandPoolsToDestroy.pop_front();
			}
		}
		void DestroyType(vk::RenderPass)
		{
			while (!renderPassesToDestroy.empty())
			{
				GetDevice().destroyRenderPass(renderPassesToDestroy.front());
				renderPassesToDestroy.pop_front();
			}
		}
		void DestroyType(vk::Pipeline)
		{
			while (!pipelineToDestroy.empty())
			{
				GetDevice().destroyPipeline(pipelineToDestroy.front());
				pipelineToDestroy.pop_front();
			}
		}
		void DestroyType(vk::PipelineLayout)
		{
			while (!pipelineLayoutsToDestroy.empty())
			{
				GetDevice().destroyPipelineLayout(pipelineLayoutsToDestroy.back());
				pipelineLayoutsToDestroy.pop_back();
			}
		}
		void DestroyType(vk::Buffer)
		{
			while (!buffersToDestroy.empty())
			{
				GetDevice().destroyBuffer(buffersToDestroy.front());
				buffersToDestroy.pop_front();
			}
		}
		void DestroyType(vk::DeviceMemory)
		{
			while (!memoryAllocationsToDestroy.empty())
			{
				GetDevice().freeMemory(memoryAllocationsToDestroy.front());
				memoryAllocationsToDestroy.pop_front();
			}
		}
		void DestroyType(vk::Device)
		{
			while (!devicesToDestroy.empty())
			{
				devicesToDestroy.back().waitIdle();
				devicesToDestroy.back().destroy();
				devicesToDestroy.pop_back();
			}
		}
		void DestroyType(vk::DescriptorPool)
		{
			while (!descriptorPoolsToDestroy.empty())
			{
				GetDevice().destroyDescriptorPool(descriptorPoolsToDestroy.front());
				descriptorPoolsToDestroy.pop_front();
			}
		}
		void DestroyType(vk::DescriptorSetLayout)
		{
			while (!descriptorSetLayoutsToDestroy.empty())
			{
				GetDevice().destroyDescriptorSetLayout(descriptorSetLayoutsToDestroy.front());
				descriptorSetLayoutsToDestroy.pop_front();
			}
		}
		void DestroyType(vk::Instance)
		{
			while (!instancesToDestroy.empty())
			{
				instancesToDestroy.front().destroy();
				instancesToDestroy.pop_front();
			}
		}
		void DestroyType(vk::SurfaceKHR)
		{
			while (!surfacesToDestroy.empty())
			{
				instance.destroySurfaceKHR(surfacesToDestroy.front());
				surfacesToDestroy.pop_front();
			}
		}
		void DestroyType(vk::SwapchainKHR)
		{
			while (!swapchainsToDestroy.empty())
			{
				GetDevice().destroySwapchainKHR(swapchainsToDestroy.front());
				swapchainsToDestroy.pop_front();
			}
		}
		void DestroyType(vk::Image)
		{
			while (!imagesToDestroy.empty())
			{
				GetDevice().destroyImage(imagesToDestroy.front());
				imagesToDestroy.pop_front();
			}
		}
		void DestroyType(vk::ImageView)
		{
			while (!imageViewsToDestroy.empty())
			{
				GetDevice().destroyImageView(imageViewsToDestroy.front());
				imageViewsToDestroy.pop_front();
			}
		}
		void DestroyType(VmaAllocator)
		{

			while (!allocatorsToDestroy.empty())
			{
				vmaDestroyAllocator(allocatorsToDestroy.front());
				allocatorsToDestroy.pop_front();
			}
		}
		void DestroyType(VmaBuffer)
		{
			while (!vmaBuffersToDestroy.empty())
			{
				vmaDestroyBuffer(GetAllocator(), vmaBuffersToDestroy.front().buffer, vmaBuffersToDestroy.front().allocation);
				vmaBuffersToDestroy.pop_front();
			}
		}
		void DestroyType(VmaImage)
		{
			while (!vmaImagesToDestroy.empty())
			{
				vmaDestroyImage(GetAllocator(), vmaImagesToDestroy.back().image, vmaImagesToDestroy.back().allocation);
				vmaImagesToDestroy.pop_back();
			}
		}

		void DisposeAll()
		{
			DestroyType(vk::Semaphore());
			DestroyType(vk::Fence());
			DestroyType(vk::CommandPool());
			DestroyType(vk::Framebuffer());
			DestroyType(vk::RenderPass());
			DestroyType(vk::ShaderModule());
			DestroyType(vk::Pipeline());
			DestroyType(vk::PipelineLayout());
			DestroyType(vk::Buffer());
			DestroyType(vk::DeviceMemory());
			DestroyType(vk::DescriptorPool());
			DestroyType(vk::DescriptorSetLayout());
			DestroyType(vk::Image());
			DestroyType(vk::ImageView());
			DestroyType(VmaBuffer());
			DestroyType(VmaImage());
			DestroyType(VmaAllocator());
			DestroyType(vk::SwapchainKHR());
			DestroyType(vk::Device());
			DestroyType(vk::SurfaceKHR());
			DestroyType(vk::Instance());
		}
		~VulkanObjectManager()
		{
			DisposeAll();
		}
	private:
		vk::Device device;
		vk::PhysicalDevice physicalDevice;
		vk::SurfaceKHR surface;
		vk::Instance instance;
		QueueData transferQueue;
		QueueData graphicsQueue;
		QueueData computeQueue;
		QueueData generalQueue;
		VmaAllocator allocator;
		
		
		std::deque<vk::Semaphore> semaphoresToDestroy;
		std::deque<vk::Fence> fencesToDestroy;
		std::deque<vk::CommandPool> commandPoolsToDestroy;
		std::deque<vk::RenderPass> renderPassesToDestroy;
		std::deque<vk::Pipeline> pipelineToDestroy;
		std::deque<vk::PipelineLayout> pipelineLayoutsToDestroy;
		std::deque<vk::Buffer> buffersToDestroy;
		std::deque<vk::DeviceMemory> memoryAllocationsToDestroy;
		std::deque<vk::Device> devicesToDestroy;
		std::deque<vk::DescriptorPool> descriptorPoolsToDestroy;
		std::deque<vk::DescriptorSetLayout> descriptorSetLayoutsToDestroy;
		std::deque<vk::Instance> instancesToDestroy;
		std::deque<vk::SurfaceKHR> surfacesToDestroy;
		std::deque<vk::SwapchainKHR> swapchainsToDestroy;
		std::deque<vk::Image> imagesToDestroy;
		std::deque<vk::ImageView> imageViewsToDestroy;
		std::deque<VmaBuffer> vmaBuffersToDestroy;
		std::deque<VmaAllocator> allocatorsToDestroy;
		std::deque<VmaImage> vmaImagesToDestroy;
		std::deque<vk::ShaderModule> shaderModulesToDestroy;
		std::deque<vk::Framebuffer> framebuffersToDestroy;
		SwapchainData swapchain;
	};

	class TimelineSemaphore
	{
	public:
		TimelineSemaphore(vk::Device deviceHandle, vk::Semaphore externalSemaphore, std::shared_ptr<uint64_t> nextValPtr)
		{
			device = deviceHandle;
			timelineSemaphore = externalSemaphore;
			nextValue = nextValPtr;
		}
		TimelineSemaphore(VulkanObjectManager& _vom, std::shared_ptr<uint64_t> nextValPtr, bool manage = true)
		{
			device = _vom.GetDevice();
			timelineSemaphore = _vom.MakeTimelineSemaphore(*nextValPtr, manage);
			nextValue = nextValPtr;
		}
		TimelineSemaphore() = default;
		void WaitOn()
		{
			vk::SemaphoreWaitInfo waitInfo({}, 1, &timelineSemaphore, nextValue.get());
			auto res = device.waitSemaphores(waitInfo, UINT64_MAX);
		};
		bool Signaled()
		{
			vk::SemaphoreWaitInfo waitInfo({}, 1, &timelineSemaphore, nextValue.get());
			vk::Result res = device.waitSemaphores(waitInfo, 0);
			return res == vk::Result::eSuccess;
		}
		vk::Semaphore Use(bool increment = true)
		{
			if (increment)
			{
				(*nextValue)++;
			}
			return timelineSemaphore;
		}
		uint64_t GetNextValue()
		{
			return *nextValue;
		}
		void SetNextValue(uint64_t newVal)
		{
			*nextValue = newVal;
		}
		void Reset(uint64_t startingValue)
		{
			device.destroySemaphore(timelineSemaphore);
			vk::SemaphoreTypeCreateInfo timeCreateInfo(vk::SemaphoreType::eTimeline, startingValue);
			vk::SemaphoreCreateInfo createInfo;
			createInfo.pNext = &timeCreateInfo;
			timelineSemaphore = device.createSemaphore(createInfo);
			*nextValue = startingValue + 1;
		}
		vk::Semaphore GetTimelineSemaphore()
		{
			return timelineSemaphore;
		}
	private:
		vk::Device device;
		std::shared_ptr<uint64_t> nextValue;
		vk::Semaphore timelineSemaphore;
	};

	class CommandBufferCache
	{
	public:
		CommandBufferCache(vk::Device deviceHandle, vk::CommandPool externalCommandPool)
		{
			assert(deviceHandle != NULL);
			assert(externalCommandPool != NULL);
			device = deviceHandle;
			commandPool = externalCommandPool;
		}
		CommandBufferCache(VulkanObjectManager& _vom, vk::CommandPool externalPool)
		{
			assert(externalPool != NULL);
			device = _vom.GetDevice();
			commandPool = externalPool;
		}
		CommandBufferCache(vk::Device deviceHandle)
		{
			assert(deviceHandle != NULL);
			device = deviceHandle;
		};
		CommandBufferCache(VulkanObjectManager& _vom)
		{
			device = _vom.GetDevice();
		};

		CommandBufferCache() = default;
		vk::CommandPool GetCommandPool()
		{
			assert(commandPool != NULL);
			return commandPool;
		}
		vk::CommandBuffer NextCommandBuffer(bool manage = true)
		{
			if (!freeCommandBuffers->empty())
			{
				vk::CommandBuffer buffer = freeCommandBuffers->back();
				freeCommandBuffers->pop_back();
				if (manage)
				{
					usedCommandBuffers->emplace_back(buffer);
				}
				return buffer;
			}
			else
			{
				assert(commandPool != NULL);
				vk::CommandBufferAllocateInfo allocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
				vk::CommandBuffer buffer = device.allocateCommandBuffers(allocateInfo)[0];
				if (manage)
				{
					usedCommandBuffers->emplace_back(buffer);
				}
				return buffer;
			}
		}
		void ResetCommandPool()
		{
			device.resetCommandPool(commandPool);
			while (!usedCommandBuffers->empty())
			{
				freeCommandBuffers->emplace_back(usedCommandBuffers->back());
				usedCommandBuffers->pop_back();
			}
		}
		void ResetBuffers()
		{
			while (!usedCommandBuffers->empty())
			{
				usedCommandBuffers->back().reset();
				freeCommandBuffers->emplace_back(usedCommandBuffers->back());
				usedCommandBuffers->pop_back();
			}
		}
		void AddUsedBuffers(std::vector<vk::CommandBuffer> buffersToAdd)
		{
			usedCommandBuffers->insert(usedCommandBuffers->end(), buffersToAdd.begin(), buffersToAdd.end());
		}
		void AddFreeBuffers(std::vector<vk::CommandBuffer> buffersToAdd)
		{
			freeCommandBuffers->insert(freeCommandBuffers->end(), buffersToAdd.begin(), buffersToAdd.end());
		}
		std::shared_ptr<std::vector<vk::CommandBuffer>> usedCommandBuffers = std::make_shared<std::vector<vk::CommandBuffer>>();
		std::shared_ptr<std::vector<vk::CommandBuffer>> freeCommandBuffers = std::make_shared<std::vector<vk::CommandBuffer>>();
	private:
		//We use a device because all it does is allocate command buffer which are managed by the command pool either created by an external vom or passed as an argument
		vk::Device device;
		vk::CommandPool commandPool;
		
	};

	struct SemaphoreDataEntity
	{
		vk::Semaphore& semaphore;
		std::shared_ptr<uint64_t>& signalValue;
		vk::PipelineStageFlags& waitStage;
		uint64_t index;

		SemaphoreDataEntity(uint64_t _index, vk::Semaphore& semaphorePtr, std::shared_ptr<uint64_t>& signalValuePtr, vk::PipelineStageFlags& waitStagePtr)
			: semaphore(semaphorePtr), signalValue(signalValuePtr), waitStage(waitStagePtr), index(_index)
		{}

		void operator=(const SemaphoreDataEntity& ref)
		{
			semaphore = ref.semaphore;
			signalValue = ref.signalValue;
			waitStage = ref.waitStage;
			index = ref.index;
		}
	};

	struct SemaphoreData
	{
		uint64_t timelineCount = 0;
		std::vector<vk::Semaphore> semaphores;
		std::vector<std::shared_ptr<uint64_t>> timelineValues;
		std::vector<vk::PipelineStageFlags> waitStages;
		std::vector<uint64_t> extractedTimelineValues;
		SemaphoreDataEntity operator[](uint64_t index)
		{
			return SemaphoreDataEntity(index, semaphores[index], timelineValues[index], waitStages[index]);
		}
		SemaphoreDataEntity EmplaceBack(vk::Semaphore semaphore, std::shared_ptr<uint64_t> signalValue, vk::PipelineStageFlags waitStage)
		{
			if (signalValue == nullptr)
			{
				semaphores.emplace_back(semaphore);
				timelineValues.emplace_back(signalValue);
				waitStages.emplace_back(waitStage);
				return SemaphoreDataEntity(semaphores.size(), semaphores.back(), timelineValues.back(), waitStages.back());

			}
			else
			{
				timelineCount++;
				for (size_t i = 0; i < timelineValues.size(); i++)
				{
					if (timelineValues[i] == nullptr)
					{
						semaphores.emplace(semaphores.begin() + i, semaphore);
						timelineValues.emplace(timelineValues.begin() + i, signalValue);
						waitStages.emplace(waitStages.begin()+ i, waitStage);
						return SemaphoreDataEntity(i, semaphores[i], timelineValues[i], waitStages[i]);

					}
				}
				semaphores.emplace_back(semaphore);
				timelineValues.emplace_back(signalValue);
				waitStages.emplace_back(waitStage);
				return SemaphoreDataEntity(semaphores.size(), semaphores.back(), timelineValues.back(), waitStages.back());
			}
		}
		std::vector<SemaphoreDataEntity> EmplaceBack(std::vector<WaitData> datas)
		{
			std::vector<SemaphoreDataEntity> createdEntities;
			for(auto& data : datas)
			{
				
				createdEntities.emplace_back(EmplaceBack(data.waitSemaphore, data.waitValuePtr, data.waitStage));
			}
			return createdEntities;
		}
		uint64_t size()
		{
			return semaphores.size();
		}
		void ExtractTimelineValues()
		{
			extractedTimelineValues.clear();
			for (size_t i = 0; i < timelineValues.size(); i++)
			{
				if (timelineValues[i] != nullptr)
				{
					extractedTimelineValues.emplace_back(*timelineValues[i]);
				}
			}
		}
		uint32_t GetTimelineCount()
		{
			uint32_t count;
			for (size_t i = 0; i < timelineValues.size(); i++)
			{
				if (timelineValues[i] != nullptr)
				{
					count++;
				}
			}
			return count;
		}
	};

	class SyncManager
	{
	public:
		SyncManager(VulkanObjectManager& _vom) : vom(_vom){}
		SyncManager(vk::Device deviceHandle) : vom(deviceHandle){}

		vk::TimelineSemaphoreSubmitInfo timelineSubmitInfo;
		SemaphoreData waitSemaphores;
		SemaphoreData signalSemaphores;


		void AttachWaitData(std::vector<WaitData> datas)
		{
			waitSemaphores.EmplaceBack(datas);
		}
		void CreateTimelineSignalSemaphore(uint64_t startValue)
		{
			signalSemaphores.EmplaceBack(vom.MakeTimelineSemaphore(startValue), std::make_shared<uint64_t>(startValue + 1), {});
		}
		void CreateTimelineSignalSemaphore(std::shared_ptr<uint64_t> startAndSignalValuePtr)
		{
			signalSemaphores.EmplaceBack(vom.MakeTimelineSemaphore(*startAndSignalValuePtr), startAndSignalValuePtr, {});
		}
		void CreateSignalSemaphore()
		{
			signalSemaphores.EmplaceBack(vom.MakeSemaphore(), {}, {});
		}
		vk::TimelineSemaphoreSubmitInfo GetTimelineSubmitInfo(bool withNormalWaits = true, bool withNormalSignals = true)
		{
			waitSemaphores.ExtractTimelineValues();
			signalSemaphores.ExtractTimelineValues();
			timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo(
				(withNormalWaits)? waitSemaphores.size() : waitSemaphores.timelineCount,
				waitSemaphores.extractedTimelineValues.data(),
				(withNormalSignals)? signalSemaphores.size() : signalSemaphores.timelineCount,
				signalSemaphores.extractedTimelineValues.data());

			return timelineSubmitInfo;
		}
		vk::TimelineSemaphoreSubmitInfo* GetTimelineSubmitInfoPtr(bool withNormalWaits = true, bool withNormalSignals = true)
		{
			waitSemaphores.ExtractTimelineValues();
			signalSemaphores.ExtractTimelineValues();
			timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo(
				(withNormalWaits) ? waitSemaphores.size() : waitSemaphores.timelineCount,
				waitSemaphores.extractedTimelineValues.data(),
				(withNormalSignals) ? signalSemaphores.size() : signalSemaphores.timelineCount,
				signalSemaphores.extractedTimelineValues.data());

			return &timelineSubmitInfo;
		}
		void Clear()
		{
			waitSemaphores = SemaphoreData();
			signalSemaphores = SemaphoreData();
			vom.DestroyType(vk::Semaphore());
		}
		void ClearWaits()
		{
			waitSemaphores = SemaphoreData();
		}
		void ClearSignals()
		{
			signalSemaphores = SemaphoreData();
		}

	private:
		VulkanObjectManager vom;
	};

}



