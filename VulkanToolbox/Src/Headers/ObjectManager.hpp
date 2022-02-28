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

		vk::Format imageFormat;
		vk::ImageLayout layout;
		vk::Extent3D extent;
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
		WaitData(std::shared_ptr<uint64_t> _waitValuePtr, vk::Semaphore _waitSemaphore, vk::PipelineStageFlags _waitStage);
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
		VmaImage GetImage(uint64_t index);

		/**
		 * \brief Gets the managed swapchain
		 * \return 
		 */
		vk::SwapchainKHR GetSwapchain();

		/**
		 * \brief Gets the managed swapchain's format
		 * \return 
		 */
		vk::Format GetFormat();

		/**
		 * \brief Gets the managed swapchains's extent
		 * \return 
		 */
		vk::Extent2D GetExtent();

		/**
		 * \brief Returns the whole images vector 
		 * \return A vector of VmaImage structs
		 */
		std::vector<VmaImage>& GetImages();
	};


	/**
	 * \brief The key component of all the other managers, a system that handles the creation of other vulkan objects as well as their lifetimes in RAII style
	 */
	class ObjectManager
	{
	public:
		/**
		 * \brief This constructor uses a parent vom, NOTE this will make the new vom a derivative of the parent and will share the devices
		 * \param _vom The parent vom whose device will be used to initialize the new vom
		 */
		ObjectManager(ObjectManager& _vom);

		/**
		 * \brief This constructor uses an external device handle to initialize its self
		 * \param deviceHandle The external device handle
		 * \param manage Whether to tie the device handle's lifetime to the new vom
		 */
		ObjectManager(vk::Device deviceHandle, bool manage = false, bool skipNullCheck = false);

		/**
		 * \brief returns the currently mounted device
		 * \return The currently mounted device
		 */
		vk::Device GetDevice();
		/**
		 * \brief returns the currently mounted Instance
		 * \return The currently mounted Instance
		 */
		vk::Instance GetInstance();
		/**
		 * \brief returns the currently mounted PhysicalDevice
		 * \return The currently mounted PhysicalDevice
		 */
		vk::PhysicalDevice GetPhysicalDevice();
		/**
		 * \brief returns the currently mounted Surface
		 * \return The currently mounted Surface
		 */
		vk::SurfaceKHR GetSurface();
		/**
		 * \brief returns the currently mounted SwapchainData struct
		 * \return The currently mounted SwapchainData struct
		 */
		SwapchainData& GetSwapchainData(bool ignoreCheck = false);

		/**
		 * \brief Sets the mounted device used to perform the vom's functions
		 * \param _deviceToMount The device to mount, NOTE this does not tie the devices lifetime to the vom
		 */
		void SetDevice(vk::Device _deviceToMount);

		/**
		 * \brief Sets the mounted Instance used to perform the vom's functions
		 * \param _instanceToMount The Instance to mount, NOTE this does not tie the Instance's lifetime to the vom
		 */
		void SetInstance(vk::Instance _instanceToMount);
		/**
		 * \brief Sets the mounted device used to perform the vom's functions
		 * \param _deviceToMount The device to mount, NOTE this does not tie the devices lifetime to the vom
		 */
		void SetPhysicalDevice(vk::PhysicalDevice _physicalDeviceToMount);
		/**
		 * \brief Sets the mounted device used to perform the vom's functions
		 * \param _deviceToMount The device to mount, NOTE this does not tie the devices lifetime to the vom
		 */
		void SetQueues(QueueData _graphicsQueueToMount, QueueData _transferQueueToMount, QueueData _computeQueueToMount);
		void SetSurface(vk::SurfaceKHR _surfaceToMount);
		void SetSwapchain(vk::SwapchainKHR swapchain, vk::Format imageFormat, vk::Extent2D extent, bool manage = false);

		QueueData GetTransferQueue();
		QueueData GetGraphicsQueue();
		QueueData GetComputeQueue();
		QueueData GetGeneralQueue();
		void SetTransferQueue(QueueData _transferQueue);
		void SetGraphicsQueue(QueueData _graphicsQueue);
		void SetComputeQueue(QueueData _computeQueue);
		void SetGeneralQueue(QueueData _generalQueue);

		vk::Semaphore MakeSemaphore(bool manage = true);
		vk::Semaphore MakeTimelineSemaphore(uint64_t startingValue, bool manage = true);
		vk::Fence MakeFence(bool signaled, bool manage = true);
		vk::CommandPool MakeCommandPool(vk::CommandPoolCreateInfo createInfo, bool manage = true);
		std::vector<vk::CommandBuffer> MakeCommandBuffers(vk::CommandBufferAllocateInfo alocInfo);
		vk::RenderPass MakeRenderPass(vk::RenderPassCreateInfo createInfo, bool manage = true);
		vk::Pipeline MakePipeline(vk::PipelineCache cache, vk::GraphicsPipelineCreateInfo createInfo, bool manage = true);
		vk::Pipeline MakePipeline(vk::PipelineCache cache, vk::ComputePipelineCreateInfo createInfo, bool manage = true);
		vk::PipelineLayout MakePipelineLayout(vk::PipelineLayoutCreateInfo createInfo, bool manage = true);
		vk::Buffer MakeBuffer(vk::BufferCreateInfo createInfo, bool manage = true);
		vk::DeviceMemory MakeMemoryAllocation(vk::MemoryAllocateInfo alocInfo, bool manage = true);
		vk::DescriptorPool MakeDescriptorPool(vk::DescriptorPoolCreateInfo createInfo, bool manage = true);
		vk::DescriptorSetLayout MakeDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo createInfo, bool manage = true);
		std::vector<vk::DescriptorSet> MakeDescriptorSets(vk::DescriptorSetAllocateInfo allocateInfo);
		vk::Image MakeImage(vk::ImageCreateInfo createInfo, bool manage = true);
		vk::ImageView MakeImageView(vk::ImageViewCreateInfo createInfo, bool manage = true);
		VmaAllocator GetAllocator();
		VmaAllocator MakeAllocator(VmaAllocatorCreateInfo createInfo, bool mount = false, bool manage = true);
		VmaAllocator MakeAllocator(uint32_t apiVersion, bool mount = false, bool manage = true);
		void SetAllocator(VmaAllocator allocatorHandle);
		VmaBuffer VmaMakeBuffer(vk::BufferCreateInfo bufferInfo, VmaAllocationCreateInfo allocationCreateInfo, bool manage = true);
		VmaImage VmaMakeImage(vk::ImageCreateInfo imageInfo, vk::ImageViewCreateInfo viewInfo, VmaAllocationCreateInfo allocationCreateInfo, bool transition = true, bool manage = true);
		vk::Sampler MakeImageSampler(vk::SamplerCreateInfo createInfo, bool manage = true);


		void TransitionImages(vk::CommandBuffer cmd, std::vector<vk::ImageMemoryBarrier> imageTransitions, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage);
		void TransitionImages(std::vector<vk::ImageMemoryBarrier> imageTransitions, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage);

		vk::ShaderModule MakeShaderModule(const char* shaderPath, bool manage = true);
		vk::Framebuffer MakeFramebuffer(vk::FramebufferCreateInfo createInfo, bool manage = true);

		void Manage(vk::Framebuffer frame);
		void Manage(vk::ShaderModule module);
		void Manage(vk::Semaphore semaphore);
		void Manage(vk::Fence fence);
		void Manage(vk::CommandPool commandPool);
		void Manage(vk::RenderPass renderPass);
		void Manage(vk::Pipeline pipeline);
		void Manage(vk::PipelineLayout layout);
		void Manage(vk::Buffer buffer);
		void Manage(vk::DeviceMemory aloc);
		void Manage(vk::Device device);
		void Manage(vk::DescriptorPool pool);
		void Manage(vk::DescriptorSetLayout layout);
		void Manage(vk::Instance instance);
		void Manage(vk::SurfaceKHR surface);
		void Manage(SwapchainData swapchainData);
		void Manage(vk::Image image);
		void Manage(vk::ImageView imageView);
		void Manage(VmaAllocator allocator);
		void Manage(VmaBuffer buffer);
		void Manage(VmaImage image);
		void Manage(vk::Sampler sampler);

		void DestroyType(vk::Framebuffer);
		void DestroyType(vk::ShaderModule);
		void DestroyType(vk::Semaphore);
		void DestroyType(vk::Fence);
		void DestroyType(vk::CommandPool);
		void DestroyType(vk::RenderPass);
		void DestroyType(vk::Pipeline);
		void DestroyType(vk::PipelineLayout);
		void DestroyType(vk::Buffer);
		void DestroyType(vk::DeviceMemory);
		void DestroyType(vk::Device);
		void DestroyType(vk::DescriptorPool);
		void DestroyType(vk::DescriptorSetLayout);
		void DestroyType(vk::Instance);
		void DestroyType(vk::SurfaceKHR);
		void DestroyType(vk::SwapchainKHR);
		void DestroyType(vk::Image);
		void DestroyType(vk::ImageView);
		void DestroyType(VmaAllocator);
		void DestroyType(VmaBuffer);
		void DestroyType(VmaImage);
		void DestroyType(vk::Sampler);


		void DestroyAll();
		~ObjectManager();
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
		std::deque<vk::Sampler> samplersToDestroy;
		SwapchainData swapchainData;
	};

	class TimelineSemaphore
	{
	public:
		TimelineSemaphore(vk::Device deviceHandle, vk::Semaphore externalSemaphore, std::shared_ptr<uint64_t> nextValPtr);
		TimelineSemaphore(ObjectManager& _vom, std::shared_ptr<uint64_t> nextValPtr, bool manage = true);
		TimelineSemaphore() = default;
		void WaitOn();
		bool Signaled();
		vk::Semaphore Use(bool increment = true);
		uint64_t GetNextValue();
		void SetNextValue(uint64_t newVal);
		void Reset(uint64_t startingValue);
		vk::Semaphore GetTimelineSemaphore();
	private:
		vk::Device device;
		std::shared_ptr<uint64_t> nextValue;
		vk::Semaphore timelineSemaphore;
	};

	class CommandBufferCache
	{
	public:
		CommandBufferCache(vk::Device deviceHandle, vk::CommandPool externalCommandPool);
		CommandBufferCache(ObjectManager& _vom, vk::CommandPool externalPool);;
		CommandBufferCache(vk::Device deviceHandle);
		CommandBufferCache(ObjectManager& _vom);

		CommandBufferCache() = default;
		vk::CommandPool GetCommandPool();
		vk::CommandBuffer NextCommandBuffer(bool manage = true);
		void ResetCommandPool();
		void ResetBuffers();
		void AddUsedBuffers(std::vector<vk::CommandBuffer> buffersToAdd);
		void AddFreeBuffers(std::vector<vk::CommandBuffer> buffersToAdd);
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

		SemaphoreDataEntity(uint64_t _index, vk::Semaphore& semaphorePtr, std::shared_ptr<uint64_t>& signalValuePtr, vk::PipelineStageFlags& waitStagePtr);

		void operator=(const SemaphoreDataEntity& ref);
	};

	struct SemaphoreData
	{
		uint64_t timelineCount = 0;
		std::vector<vk::Semaphore> semaphores;
		std::vector<std::shared_ptr<uint64_t>> timelineValues;
		std::vector<vk::PipelineStageFlags> waitStages;
		std::vector<uint64_t> extractedTimelineValues;
		SemaphoreDataEntity operator[](uint64_t index);
		SemaphoreDataEntity EmplaceBack(vk::Semaphore semaphore, std::shared_ptr<uint64_t> signalValue, vk::PipelineStageFlags waitStage);
		std::vector<SemaphoreDataEntity> EmplaceBack(std::vector<WaitData> datas);
		uint64_t size();
		void ExtractTimelineValues();
		uint32_t GetTimelineCount();
	};

	class SyncManager
	{
	public:
		SyncManager(ObjectManager& _vom) : vom(_vom){}
		SyncManager(vk::Device deviceHandle) : vom(deviceHandle){}

		vk::TimelineSemaphoreSubmitInfo timelineSubmitInfo;
		SemaphoreData waitSemaphores;
		SemaphoreData signalSemaphores;


		void AttachWaitData(std::vector<WaitData> datas);
		void CreateTimelineSignalSemaphore(uint64_t startValue);
		void CreateTimelineSignalSemaphore(std::shared_ptr<uint64_t> startAndSignalValuePtr);
		void CreateSignalSemaphore();
		vk::TimelineSemaphoreSubmitInfo GetTimelineSubmitInfo(bool withNormalWaits = true, bool withNormalSignals = true);
		vk::TimelineSemaphoreSubmitInfo* GetTimelineSubmitInfoPtr(bool withNormalWaits = true, bool withNormalSignals = true);
		void Clear();
		void ClearWaits();
		void ClearSignals();

	private:
		ObjectManager vom;
	};

}



