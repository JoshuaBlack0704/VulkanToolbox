#include "VulkanToolbox.hpp"

namespace vkt
{
		
		WaitData::WaitData(std::shared_ptr<uint64_t> _waitValuePtr, vk::Semaphore _waitSemaphore, vk::PipelineStageFlags _waitStage)
		{
			waitValuePtr = _waitValuePtr;
			waitSemaphore = _waitSemaphore;
			waitStage = _waitStage;
		}



		
		VmaImage SwapchainData::GetImage(uint64_t index)
		{
			return images[index];
		}

		
		vk::SwapchainKHR SwapchainData::GetSwapchain()
		{
			return swapchain;
		}

		
		vk::Format SwapchainData::GetFormat()
		{
			return imageFormat;
		}

		
		vk::Extent2D SwapchainData::GetExtent()
		{
			return extent;
		}

		
		std::vector<VmaImage>& SwapchainData::GetImages()
		{
			return images;
		}




		ObjectManager::ObjectManager(ObjectManager& _vom)
		{
			SetDevice(_vom.GetDevice());
		}


		ObjectManager::ObjectManager(vk::Device deviceHandle, bool manage, bool skipNullCheck)
		{
			if (!skipNullCheck)
			{
				SetDevice(deviceHandle);
			}
			else
			{
				device = deviceHandle;
			}
			if (manage)
			{
				Manage(deviceHandle);
			}
		}


		vk::Device ObjectManager::GetDevice()
		{
			assert(device != NULL);
			return device;
		}

		vk::Instance ObjectManager::GetInstance()
		{
			assert(instance != NULL);
			return instance;
		}

		vk::PhysicalDevice ObjectManager::GetPhysicalDevice()
		{
			assert(physicalDevice != NULL);
			return physicalDevice;
		}

		vk::SurfaceKHR ObjectManager::GetSurface()
		{
			assert(surface != NULL);
			return surface;
		}

		SwapchainData& ObjectManager::GetSwapchainData(bool ignoreCheck)
		{
			if (!ignoreCheck)
			{
				assert(swapchainData.swapchain != NULL);
			}
			return swapchainData;
		}

		
		void ObjectManager::SetDevice(vk::Device _deviceToMount)
		{
			assert(_deviceToMount != NULL);
			device = _deviceToMount;
		}


		void ObjectManager::SetInstance(vk::Instance _instanceToMount)
		{
			assert(_instanceToMount != NULL);
			instance = _instanceToMount;
		}

		void ObjectManager::SetPhysicalDevice(vk::PhysicalDevice _physicalDeviceToMount)
		{
			assert(_physicalDeviceToMount != NULL);
			physicalDevice = _physicalDeviceToMount;
		}

		void ObjectManager::SetQueues(QueueData _graphicsQueueToMount, QueueData _transferQueueToMount, QueueData _computeQueueToMount)
		{
			assert(_graphicsQueueToMount.queue != NULL);
			assert(_transferQueueToMount.queue != NULL);
			assert(_computeQueueToMount.queue != NULL);
			graphicsQueue = _graphicsQueueToMount;
			transferQueue = _transferQueueToMount;
			computeQueue = _computeQueueToMount;
		}
		void ObjectManager::SetSurface(vk::SurfaceKHR _surfaceToMount)
		{
			assert(_surfaceToMount != NULL);
			surface = _surfaceToMount;
		}
		void ObjectManager::SetSwapchain(vk::SwapchainKHR swapchain, vk::Format imageFormat, vk::Extent2D extent, bool manage)
		{
			assert(swapchain != NULL);
			swapchainData = SwapchainData{ swapchain, imageFormat, extent };
			auto images = GetDevice().getSwapchainImagesKHR(GetSwapchainData().GetSwapchain());
			for (auto& image : images)
			{
				swapchainData.images.emplace_back(image, MakeImageView
				(vk::ImageViewCreateInfo({},
					image,
					vk::ImageViewType::e2D,
					swapchainData.imageFormat,
					vk::ComponentMapping(),
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
				));

				swapchainData.GetImages().back().layout = vk::ImageLayout::eColorAttachmentOptimal;
				swapchainData.GetImages().back().imageFormat = imageFormat;
			}
			if (manage)
			{
				Manage(GetSwapchainData());
			}
		}

		QueueData ObjectManager::GetTransferQueue()
		{
			assert(transferQueue.queue != NULL);
			return transferQueue;
		}
		QueueData ObjectManager::GetGraphicsQueue()
		{
			assert(graphicsQueue.queue != NULL);
			return graphicsQueue;
		}
		QueueData ObjectManager::GetComputeQueue()
		{
			assert(computeQueue.queue != NULL);
			return computeQueue;
		}
		QueueData ObjectManager::GetGeneralQueue()
		{
			assert(generalQueue.queue != NULL);
			return generalQueue;
		}
		void ObjectManager::SetTransferQueue(QueueData _transferQueue)
		{
			assert(_transferQueue.queue != NULL);
			transferQueue = _transferQueue;
		}
		void ObjectManager::SetGraphicsQueue(QueueData _graphicsQueue)
		{
			assert(_graphicsQueue.queue != NULL);
			graphicsQueue = _graphicsQueue;
		}
		void ObjectManager::SetComputeQueue(QueueData _computeQueue)
		{
			assert(_computeQueue.queue != NULL);
			computeQueue = _computeQueue;
		}
		void ObjectManager::SetGeneralQueue(QueueData _generalQueue)
		{
			assert(_generalQueue.queue != NULL);
			generalQueue = _generalQueue;
		}

		vk::Semaphore ObjectManager::MakeSemaphore(bool manage)
		{

			vk::SemaphoreCreateInfo createInfo;
			auto semaphoreHandle = GetDevice().createSemaphore(createInfo);
			if (manage)
			{
				Manage(semaphoreHandle);
			}
			return semaphoreHandle;
		}
		vk::Semaphore ObjectManager::MakeTimelineSemaphore(uint64_t startingValue, bool manage)
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
		vk::Fence ObjectManager::MakeFence(bool signaled, bool manage)
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
		vk::CommandPool ObjectManager::MakeCommandPool(vk::CommandPoolCreateInfo createInfo, bool manage)
		{
			auto commandPool = GetDevice().createCommandPool(createInfo);
			if (manage)
			{
				Manage(commandPool);
			}
			return commandPool;
		}
		std::vector<vk::CommandBuffer> ObjectManager::MakeCommandBuffers(vk::CommandBufferAllocateInfo alocInfo)
		{
			return GetDevice().allocateCommandBuffers(alocInfo);
		}
		vk::RenderPass ObjectManager::MakeRenderPass(vk::RenderPassCreateInfo createInfo, bool manage)
		{
			auto renderpass = GetDevice().createRenderPass(createInfo);
			if (manage)
			{
				Manage(renderpass);
			}
			return renderpass;
		}
		vk::Pipeline ObjectManager::MakePipeline(vk::PipelineCache cache, vk::GraphicsPipelineCreateInfo createInfo, bool manage)
		{
			vk::ResultValue<vk::Pipeline> pipeline = GetDevice().createGraphicsPipeline(cache, createInfo);
			if (manage)
			{
				Manage(pipeline.value);
			}
			return pipeline.value;
		}
		vk::Pipeline ObjectManager::MakePipeline(vk::PipelineCache cache, vk::ComputePipelineCreateInfo createInfo, bool manage)
		{
			vk::ResultValue<vk::Pipeline> pipeline = GetDevice().createComputePipeline(cache, createInfo);
			if (manage)
			{
				Manage(pipeline.value);
			}
			return pipeline.value;
		}
		vk::PipelineLayout ObjectManager::MakePipelineLayout(vk::PipelineLayoutCreateInfo createInfo, bool manage)
		{
			auto layout = GetDevice().createPipelineLayout(createInfo);
			if (manage)
			{
				Manage(layout);
			}
			return layout;
		}
		vk::Buffer ObjectManager::MakeBuffer(vk::BufferCreateInfo createInfo, bool manage)
		{
			auto buffer = GetDevice().createBuffer(createInfo);
			if (manage)
			{
				Manage(buffer);
			}
			return buffer;
		}
		vk::DeviceMemory ObjectManager::MakeMemoryAllocation(vk::MemoryAllocateInfo alocInfo, bool manage)
		{
			auto aloc = GetDevice().allocateMemory(alocInfo);
			if (manage)
			{
				Manage(aloc);
			}
			return aloc;
		}
		vk::DescriptorPool ObjectManager::MakeDescriptorPool(vk::DescriptorPoolCreateInfo createInfo, bool manage)
		{
			auto pool = GetDevice().createDescriptorPool(createInfo);
			if (manage)
			{
				Manage(pool);
			}
			return pool;
		}
		vk::DescriptorSetLayout ObjectManager::MakeDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo createInfo, bool manage)
		{
			auto layout = GetDevice().createDescriptorSetLayout(createInfo);
			if (manage)
			{
				Manage(layout);
			}
			return layout;
		}
		std::vector<vk::DescriptorSet> ObjectManager::MakeDescriptorSets(vk::DescriptorSetAllocateInfo allocateInfo)
		{
			return GetDevice().allocateDescriptorSets(allocateInfo);
		}
		vk::Image ObjectManager::MakeImage(vk::ImageCreateInfo createInfo, bool manage)
		{
			auto image = GetDevice().createImage(createInfo);
			if (manage)
			{
				Manage(image);
			}
			return image;
		}
		vk::ImageView ObjectManager::MakeImageView(vk::ImageViewCreateInfo createInfo, bool manage)
		{
			auto imageView = GetDevice().createImageView(createInfo);
			if (manage)
			{
				Manage(imageView);
			}
			return imageView;
		}
		VmaAllocator ObjectManager::GetAllocator()
		{
			assert(allocator != NULL);
			return allocator;
		}
		VmaAllocator ObjectManager::MakeAllocator(VmaAllocatorCreateInfo createInfo, bool mount, bool manage)
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
		VmaAllocator ObjectManager::MakeAllocator(uint32_t apiVersion, bool mount, bool manage)
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
		void ObjectManager::SetAllocator(VmaAllocator allocatorHandle)
		{
			allocator = allocatorHandle;
		}
		VmaBuffer ObjectManager::VmaMakeBuffer(vk::BufferCreateInfo bufferInfo, VmaAllocationCreateInfo allocationCreateInfo, bool manage)
		{

			VmaBuffer allocation;
			VkBufferCreateInfo bufferCreateInfo = (VkBufferCreateInfo)bufferInfo;
			VkBuffer buf;
			vmaCreateBuffer(GetAllocator(), &bufferCreateInfo, &allocationCreateInfo, &buf, &allocation.allocation, &allocation.allocationInfo);
			allocation.buffer = buf;
			if (manage)
			{
				Manage(allocation);
			}
			return allocation;
		}
		VmaImage ObjectManager::VmaMakeImage(vk::ImageCreateInfo imageInfo, vk::ImageViewCreateInfo viewInfo, VmaAllocationCreateInfo allocationCreateInfo, bool transition, bool manage)
		{
			VkImage image;
			VmaImage imageData;
			VkImageCreateInfo _imageInfo = (VkImageCreateInfo)imageInfo;
			_imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageData.layout = imageInfo.initialLayout;
			imageData.extent = imageInfo.extent;
			vmaCreateImage(GetAllocator(), &_imageInfo, &allocationCreateInfo, &image, &imageData.allocation, &imageData.allocationInfo);
			imageData.image = image;
			viewInfo.image = imageData.image;
			imageData.view = MakeImageView(viewInfo);
			imageData.imageFormat = imageInfo.format;

			if (transition)
			{
				auto pool = MakeCommandPool(vk::CommandPoolCreateInfo({}, GetGraphicsQueue().index), false);
				auto cmd = MakeCommandBuffers(vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1))[0];
				cmd.begin(vk::CommandBufferBeginInfo());
				TransitionImages(
					cmd,
					{ vk::ImageMemoryBarrier(
						vk::AccessFlagBits::eNone,
						vk::AccessFlagBits::eMemoryWrite,
						vk::ImageLayout::eUndefined,
						imageInfo.initialLayout,
						VK_QUEUE_FAMILY_IGNORED,
						VK_QUEUE_FAMILY_IGNORED,
						imageData.image,
						viewInfo.subresourceRange) },
					vk::PipelineStageFlagBits::eAllCommands,
					vk::PipelineStageFlagBits::eAllCommands);
				cmd.end();
				auto fence = MakeFence(false, false);
				vk::SubmitInfo submit(
					{},
					{},
					{},
					1,
					&cmd,
					{},
					{});
				auto res = GetGraphicsQueue().queue.submit(1, &submit, fence);
				res = GetDevice().waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
				GetDevice().destroyFence(fence);
				GetDevice().destroyCommandPool(pool);
			}
			if (manage)
			{
				Manage(imageData);
			}
			return imageData;

		}
		vk::Sampler ObjectManager::MakeImageSampler(vk::SamplerCreateInfo createInfo, bool manage)
		{
			vk::Sampler sampler = GetDevice().createSampler(createInfo);
			if (manage)
			{
				Manage(sampler);
			}
			return sampler;
		}


		void ObjectManager::TransitionImages(vk::CommandBuffer cmd, std::vector<vk::ImageMemoryBarrier> imageTransitions, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage)
		{
			cmd.pipelineBarrier(
				srcStage,
				dstStage,
				{},
				0,
				{},
				0,
				{},
				imageTransitions.size(),
				imageTransitions.data());
		}
		void ObjectManager::TransitionImages(std::vector<vk::ImageMemoryBarrier> imageTransitions, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage)
		{
			auto pool = MakeCommandPool(vk::CommandPoolCreateInfo({}, GetGraphicsQueue().index), false);
			auto cmd = MakeCommandBuffers(vk::CommandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1))[0];
			cmd.begin(vk::CommandBufferBeginInfo());
			TransitionImages(cmd, imageTransitions, srcStage, dstStage);
			cmd.end();
			auto fence = MakeFence(false, false);
			vk::SubmitInfo submit(
				{},
				{},
				{},
				1,
				&cmd,
				{},
				{});
			auto res = GetGraphicsQueue().queue.submit(1, &submit, fence);
			res = GetDevice().waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
			GetDevice().destroyFence(fence);
			GetDevice().destroyCommandPool(pool);
		}

		vk::ShaderModule ObjectManager::MakeShaderModule(const char* shaderPath, bool manage)
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
		vk::Framebuffer ObjectManager::MakeFramebuffer(vk::FramebufferCreateInfo createInfo, bool manage)
		{
			auto frame = GetDevice().createFramebuffer(createInfo);
			if (manage)
			{
				Manage(frame);
			}
			return frame;
		}

		void ObjectManager::Manage(vk::Framebuffer frame)
		{
			framebuffersToDestroy.emplace_back(frame);
		}
		void ObjectManager::Manage(vk::ShaderModule module)
		{
			assert(module != NULL);
			shaderModulesToDestroy.emplace_back(module);
		}
		void ObjectManager::Manage(vk::Semaphore semaphore)
		{
			assert(semaphore != NULL);
			semaphoresToDestroy.emplace_back(semaphore);
		}
		void ObjectManager::Manage(vk::Fence fence)
		{
			assert(fence != NULL);
			fencesToDestroy.emplace_back(fence);
		}
		void ObjectManager::Manage(vk::CommandPool commandPool)
		{
			assert(commandPool != NULL);
			commandPoolsToDestroy.emplace_back(commandPool);
		}
		void ObjectManager::Manage(vk::RenderPass renderPass)
		{
			assert(renderPass != NULL);
			renderPassesToDestroy.emplace_back(renderPass);
		}
		void ObjectManager::Manage(vk::Pipeline pipeline)
		{
			assert(pipeline != NULL);
			pipelineToDestroy.emplace_back(pipeline);
		}
		void ObjectManager::Manage(vk::PipelineLayout layout)
		{
			assert(layout != NULL);
			pipelineLayoutsToDestroy.emplace_back(layout);
		}
		void ObjectManager::Manage(vk::Buffer buffer)
		{
			assert(buffer != NULL);
			buffersToDestroy.emplace_back(buffer);
		}
		void ObjectManager::Manage(vk::DeviceMemory aloc)
		{
			assert(aloc != NULL);
			memoryAllocationsToDestroy.emplace_back(aloc);
		}
		void ObjectManager::Manage(vk::Device device)
		{
			assert(device != NULL);
			devicesToDestroy.emplace_back(device);
		}
		void ObjectManager::Manage(vk::DescriptorPool pool)
		{
			assert(pool != NULL);
			descriptorPoolsToDestroy.emplace_back(pool);
		}
		void ObjectManager::Manage(vk::DescriptorSetLayout layout)
		{
			assert(layout != NULL);
			descriptorSetLayoutsToDestroy.emplace_back(layout);
		}
		void ObjectManager::Manage(vk::Instance instance)
		{
			assert(instance != NULL);
			instancesToDestroy.emplace_back(instance);
		}
		void ObjectManager::Manage(vk::SurfaceKHR surface)
		{
			assert(surface != NULL);
			surfacesToDestroy.emplace_back(surface);
		}
		void ObjectManager::Manage(SwapchainData swapchainData)
		{
			assert(swapchainData.swapchain != NULL);
			swapchainsToDestroy.emplace_back(swapchainData.swapchain);
		}
		void ObjectManager::Manage(vk::Image image)
		{
			assert(image != NULL);
			imagesToDestroy.emplace_back(image);
		}
		void ObjectManager::Manage(vk::ImageView imageView)
		{
			assert(imageView != NULL);
			imageViewsToDestroy.emplace_back(imageView);
		}
		void ObjectManager::Manage(VmaAllocator allocator)
		{
			assert(allocator != NULL);

			allocatorsToDestroy.emplace_back(allocator);
		}
		void ObjectManager::Manage(VmaBuffer buffer)
		{
			assert(buffer.buffer != NULL);

			vmaBuffersToDestroy.emplace_back(buffer);
		}
		void ObjectManager::Manage(VmaImage image)
		{
			assert(image.image != NULL);

			vmaImagesToDestroy.emplace_back(image);
		}
		void ObjectManager::Manage(vk::Sampler sampler)
		{
			assert(sampler != NULL);
			samplersToDestroy.emplace_back(sampler);
		}

		void ObjectManager::DestroyType(vk::Framebuffer)
		{
			while (!framebuffersToDestroy.empty())
			{
				device.destroyFramebuffer(framebuffersToDestroy.back());
				framebuffersToDestroy.pop_back();
			}
		}
		void ObjectManager::DestroyType(vk::ShaderModule)
		{
			while (!shaderModulesToDestroy.empty())
			{
				device.destroyShaderModule(shaderModulesToDestroy.back());
				shaderModulesToDestroy.pop_back();
			}
		}
		void ObjectManager::DestroyType(vk::Semaphore)
		{
			while (!semaphoresToDestroy.empty())
			{
				GetDevice().destroySemaphore(semaphoresToDestroy.front());
				semaphoresToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::Fence)
		{
			while (!fencesToDestroy.empty())
			{
				GetDevice().destroyFence(fencesToDestroy.front());
				fencesToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::CommandPool)
		{
			while (!commandPoolsToDestroy.empty())
			{
				GetDevice().destroyCommandPool(commandPoolsToDestroy.front());
				commandPoolsToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::RenderPass)
		{
			while (!renderPassesToDestroy.empty())
			{
				GetDevice().destroyRenderPass(renderPassesToDestroy.front());
				renderPassesToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::Pipeline)
		{
			while (!pipelineToDestroy.empty())
			{
				GetDevice().destroyPipeline(pipelineToDestroy.front());
				pipelineToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::PipelineLayout)
		{
			while (!pipelineLayoutsToDestroy.empty())
			{
				GetDevice().destroyPipelineLayout(pipelineLayoutsToDestroy.back());
				pipelineLayoutsToDestroy.pop_back();
			}
		}
		void ObjectManager::DestroyType(vk::Buffer)
		{
			while (!buffersToDestroy.empty())
			{
				GetDevice().destroyBuffer(buffersToDestroy.front());
				buffersToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::DeviceMemory)
		{
			while (!memoryAllocationsToDestroy.empty())
			{
				GetDevice().freeMemory(memoryAllocationsToDestroy.front());
				memoryAllocationsToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::Device)
		{
			while (!devicesToDestroy.empty())
			{
				devicesToDestroy.back().waitIdle();
				devicesToDestroy.back().destroy();
				devicesToDestroy.pop_back();
			}
		}
		void ObjectManager::DestroyType(vk::DescriptorPool)
		{
			while (!descriptorPoolsToDestroy.empty())
			{
				GetDevice().destroyDescriptorPool(descriptorPoolsToDestroy.front());
				descriptorPoolsToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::DescriptorSetLayout)
		{
			while (!descriptorSetLayoutsToDestroy.empty())
			{
				GetDevice().destroyDescriptorSetLayout(descriptorSetLayoutsToDestroy.front());
				descriptorSetLayoutsToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::Instance)
		{
			while (!instancesToDestroy.empty())
			{
				instancesToDestroy.front().destroy();
				instancesToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::SurfaceKHR)
		{
			while (!surfacesToDestroy.empty())
			{
				instance.destroySurfaceKHR(surfacesToDestroy.front());
				surfacesToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::SwapchainKHR)
		{
			while (!swapchainsToDestroy.empty())
			{
				GetDevice().destroySwapchainKHR(swapchainsToDestroy.front());
				swapchainsToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::Image)
		{
			while (!imagesToDestroy.empty())
			{
				GetDevice().destroyImage(imagesToDestroy.front());
				imagesToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(vk::ImageView)
		{
			while (!imageViewsToDestroy.empty())
			{
				GetDevice().destroyImageView(imageViewsToDestroy.front());
				imageViewsToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(VmaAllocator)
		{

			while (!allocatorsToDestroy.empty())
			{
				vmaDestroyAllocator(allocatorsToDestroy.front());
				allocatorsToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(VmaBuffer)
		{
			while (!vmaBuffersToDestroy.empty())
			{
				vmaDestroyBuffer(GetAllocator(), vmaBuffersToDestroy.front().buffer, vmaBuffersToDestroy.front().allocation);
				vmaBuffersToDestroy.pop_front();
			}
		}
		void ObjectManager::DestroyType(VmaImage)
		{
			while (!vmaImagesToDestroy.empty())
			{
				vmaDestroyImage(GetAllocator(), vmaImagesToDestroy.back().image, vmaImagesToDestroy.back().allocation);
				vmaImagesToDestroy.pop_back();
			}
		}
		void ObjectManager::DestroyType(vk::Sampler)
		{
			while (!samplersToDestroy.empty())
			{
				GetDevice().destroySampler(samplersToDestroy.back());
				samplersToDestroy.pop_back();
			}
		}


		void ObjectManager::DestroyAll()
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
			DestroyType(vk::Sampler());
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
		ObjectManager::~ObjectManager()
		{
			DestroyAll();
		}


		TimelineSemaphore::TimelineSemaphore(vk::Device deviceHandle, vk::Semaphore externalSemaphore, std::shared_ptr<uint64_t> nextValPtr)
		{
			device = deviceHandle;
			timelineSemaphore = externalSemaphore;
			nextValue = nextValPtr;
		}
		TimelineSemaphore::TimelineSemaphore(ObjectManager& _vom, std::shared_ptr<uint64_t> nextValPtr, bool manage)
		{
			device = _vom.GetDevice();
			timelineSemaphore = _vom.MakeTimelineSemaphore(*nextValPtr, manage);
			nextValue = nextValPtr;
		}
		void TimelineSemaphore::WaitOn()
		{
			vk::SemaphoreWaitInfo waitInfo({}, 1, &timelineSemaphore, nextValue.get());
			auto res = device.waitSemaphores(waitInfo, UINT64_MAX);
		};
		bool TimelineSemaphore::Signaled()
		{
			vk::SemaphoreWaitInfo waitInfo({}, 1, &timelineSemaphore, nextValue.get());
			vk::Result res = device.waitSemaphores(waitInfo, 0);
			return res == vk::Result::eSuccess;
		}
		vk::Semaphore TimelineSemaphore::Use(bool increment)
		{
			if (increment)
			{
				(*nextValue)++;
			}
			return timelineSemaphore;
		}
		uint64_t TimelineSemaphore::GetNextValue()
		{
			return *nextValue;
		}
		void TimelineSemaphore::SetNextValue(uint64_t newVal)
		{
			*nextValue = newVal;
		}
		void TimelineSemaphore::Reset(uint64_t startingValue)
		{
			device.destroySemaphore(timelineSemaphore);
			vk::SemaphoreTypeCreateInfo timeCreateInfo(vk::SemaphoreType::eTimeline, startingValue);
			vk::SemaphoreCreateInfo createInfo;
			createInfo.pNext = &timeCreateInfo;
			timelineSemaphore = device.createSemaphore(createInfo);
			*nextValue = startingValue + 1;
		}
		vk::Semaphore TimelineSemaphore::GetTimelineSemaphore()
		{
			return timelineSemaphore;
		}



		CommandBufferCache::CommandBufferCache(vk::Device deviceHandle, vk::CommandPool externalCommandPool)
		{
			assert(deviceHandle != NULL);
			assert(externalCommandPool != NULL);
			device = deviceHandle;
			commandPool = externalCommandPool;
		}
		CommandBufferCache::CommandBufferCache(ObjectManager& _vom, vk::CommandPool externalPool)
		{
			assert(externalPool != NULL);
			device = _vom.GetDevice();
			commandPool = externalPool;
		}
		CommandBufferCache::CommandBufferCache(vk::Device deviceHandle)
		{
			assert(deviceHandle != NULL);
			device = deviceHandle;
		};
		CommandBufferCache::CommandBufferCache(ObjectManager& _vom)
		{
			device = _vom.GetDevice();
		};

		vk::CommandPool CommandBufferCache::GetCommandPool()
		{
			assert(commandPool != NULL);
			return commandPool;
		}
		vk::CommandBuffer CommandBufferCache::NextCommandBuffer(bool manage)
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
		void CommandBufferCache::ResetCommandPool()
		{
			device.resetCommandPool(commandPool);
			while (!usedCommandBuffers->empty())
			{
				freeCommandBuffers->emplace_back(usedCommandBuffers->back());
				usedCommandBuffers->pop_back();
			}
		}
		void CommandBufferCache::ResetBuffers()
		{
			while (!usedCommandBuffers->empty())
			{
				usedCommandBuffers->back().reset();
				freeCommandBuffers->emplace_back(usedCommandBuffers->back());
				usedCommandBuffers->pop_back();
			}
		}
		void CommandBufferCache::AddUsedBuffers(std::vector<vk::CommandBuffer> buffersToAdd)
		{
			usedCommandBuffers->insert(usedCommandBuffers->end(), buffersToAdd.begin(), buffersToAdd.end());
		}


	
		SemaphoreDataEntity::SemaphoreDataEntity(uint64_t _index, vk::Semaphore& semaphorePtr, std::shared_ptr<uint64_t>& signalValuePtr, vk::PipelineStageFlags& waitStagePtr)
			: semaphore(semaphorePtr), signalValue(signalValuePtr), waitStage(waitStagePtr), index(_index)
		{
		}

		void SemaphoreDataEntity::operator=(const SemaphoreDataEntity& ref)
		{
			semaphore = ref.semaphore;
			signalValue = ref.signalValue;
			waitStage = ref.waitStage;
			index = ref.index;
		}



		SemaphoreDataEntity SemaphoreData::operator[](uint64_t index)
		{
			return SemaphoreDataEntity(index, semaphores[index], timelineValues[index], waitStages[index]);
		}
		SemaphoreDataEntity SemaphoreData::EmplaceBack(vk::Semaphore semaphore, std::shared_ptr<uint64_t> signalValue, vk::PipelineStageFlags waitStage)
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
						waitStages.emplace(waitStages.begin() + i, waitStage);
						return SemaphoreDataEntity(i, semaphores[i], timelineValues[i], waitStages[i]);

					}
				}
				semaphores.emplace_back(semaphore);
				timelineValues.emplace_back(signalValue);
				waitStages.emplace_back(waitStage);
				return SemaphoreDataEntity(semaphores.size(), semaphores.back(), timelineValues.back(), waitStages.back());
			}
		}
		std::vector<SemaphoreDataEntity> SemaphoreData::EmplaceBack(std::vector<WaitData> datas)
		{
			std::vector<SemaphoreDataEntity> createdEntities;
			for (auto& data : datas)
			{

				createdEntities.emplace_back(EmplaceBack(data.waitSemaphore, data.waitValuePtr, data.waitStage));
			}
			return createdEntities;
		}
		uint64_t SemaphoreData::size()
		{
			return semaphores.size();
		}
		void SemaphoreData::ExtractTimelineValues()
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
		uint32_t SemaphoreData::GetTimelineCount()
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
	

		void SyncManager::AttachWaitData(std::vector<WaitData> datas)
		{
			waitSemaphores.EmplaceBack(datas);
		}
		void SyncManager::CreateTimelineSignalSemaphore(uint64_t startValue)
		{
			signalSemaphores.EmplaceBack(vom.MakeTimelineSemaphore(startValue), std::make_shared<uint64_t>(startValue + 1), {});
		}
		void SyncManager::CreateTimelineSignalSemaphore(std::shared_ptr<uint64_t> startAndSignalValuePtr)
		{
			signalSemaphores.EmplaceBack(vom.MakeTimelineSemaphore(*startAndSignalValuePtr), startAndSignalValuePtr, {});
		}
		void SyncManager::CreateSignalSemaphore()
		{
			signalSemaphores.EmplaceBack(vom.MakeSemaphore(), {}, {});
		}
		vk::TimelineSemaphoreSubmitInfo SyncManager::GetTimelineSubmitInfo(bool withNormalWaits, bool withNormalSignals)
		{
			waitSemaphores.ExtractTimelineValues();
			signalSemaphores.ExtractTimelineValues();
			timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo(
				(withNormalWaits && waitSemaphores.timelineCount > 0) ? waitSemaphores.size() : waitSemaphores.timelineCount,
				waitSemaphores.extractedTimelineValues.data(),
				(withNormalSignals) ? signalSemaphores.size() : signalSemaphores.timelineCount,
				signalSemaphores.extractedTimelineValues.data());

			return timelineSubmitInfo;
		}
		vk::TimelineSemaphoreSubmitInfo* SyncManager::GetTimelineSubmitInfoPtr(bool withNormalWaits, bool withNormalSignals)
		{
			GetTimelineSubmitInfo(withNormalWaits, withNormalSignals);
			return &timelineSubmitInfo;
		}
		void SyncManager::Clear()
		{
			waitSemaphores = SemaphoreData();
			signalSemaphores = SemaphoreData();
			vom.DestroyType(vk::Semaphore());
		}
		void SyncManager::ClearWaits()
		{
			waitSemaphores = SemaphoreData();
		}
		void SyncManager::ClearSignals()
		{
			signalSemaphores = SemaphoreData();
		}


}



