#pragma once
namespace vkt
{
	//The goal is to have a single manager and api that can handle all memory ops behind an abstraction based on sectors
//and on MemoryOperationsBuffers that behaves like a traditional vulkan command buffer

//To achieve the design goal will will have a few classes
//The buffer class which will handle the physical allocations of the sectors
//The sector which is an abstraction for a set of data in a buffer
//The memory command buffer which will be an abstraction for transfer recording and will set the sectors state in a linear increaising order as they are recorded
	//the memory command buffer will also, upon execution handle buffer managers to ensure the sectors will be allocated with enough size to complete the transfers
	//As it stands memory command buffers will launch in a seperate thread by default that will wait for semaphores to do ram transfers
	//NOTE: Look into adding memory mapping capabilities to avoid always having to copy back to ram
	class BufferManager;
	struct SectorData;
	class MemoryOperationsBuffer;

	

	struct SectorData
	{
		uint64_t neededSize;
		uint64_t allocatedSize;
		uint64_t allocationOffset;
		BufferManager* bufferAllocation;
		void Reset()
		{
			neededSize = 0;
		}
		void SetSize(uint64_t _neededSize)
		{
			neededSize = _neededSize;
		}
	};

	class BufferManager
	{
	public:
		VulkanObjectManager vom;
		CommandManager cmdManager;
		vk::PhysicalDeviceProperties deviceProperties;
		vk::BufferCreateInfo bufferCreateInfo;
		VmaAllocationCreateInfo allocationCreateInfo;
		VmaBuffer bufferData;
		void* map;
		uint64_t alignment;

		std::vector<std::shared_ptr<SectorData>> sectors;

		BufferManager(vk::Device deviceHandle, VmaAllocator allocator, QueueData _transferQueue, vk::BufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsageFlags)
		: cmdManager(deviceHandle, _transferQueue, true, vk::PipelineStageFlagBits::eTransfer), vom(deviceHandle)
		{
			vom.SetAllocator(allocator);
			vom.SetTransferQueue(_transferQueue);
			const VkPhysicalDeviceProperties* props;
			vmaGetPhysicalDeviceProperties(allocator, &props);
			deviceProperties = *props;
			bufferCreateInfo.setUsage(bufferUsageFlags);
			allocationCreateInfo.usage = memoryUsageFlags;

			if (bufferCreateInfo.usage & vk::BufferUsageFlagBits::eStorageBuffer)
			{
				alignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
			}
			else if (bufferCreateInfo.usage & vk::BufferUsageFlagBits::eUniformBuffer)
			{
				alignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
			}
			else if (bufferCreateInfo.usage & vk::BufferUsageFlagBits::eStorageTexelBuffer)
			{
				alignment = deviceProperties.limits.minTexelBufferOffsetAlignment;
			}
			else
			{
				alignment = 0;
			}

		}
		BufferManager(VulkanObjectManager& _vom, vk::BufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsageFlags)
		: cmdManager(_vom, _vom.GetTransferQueue(), true, vk::PipelineStageFlagBits::eTransfer), vom(_vom)
		{
			vom.SetDevice(_vom.GetDevice());
			vom.SetAllocator(_vom.GetAllocator());
			vom.SetTransferQueue(_vom.GetTransferQueue());
			const VkPhysicalDeviceProperties* props;
			vmaGetPhysicalDeviceProperties(vom.GetAllocator(), &props);
			deviceProperties = *props;
			bufferCreateInfo.setUsage(bufferUsageFlags | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc);
			allocationCreateInfo.usage = memoryUsageFlags;

			if (bufferCreateInfo.usage & vk::BufferUsageFlagBits::eStorageBuffer)
			{
				alignment = deviceProperties.limits.minStorageBufferOffsetAlignment;
			}
			else if (bufferCreateInfo.usage & vk::BufferUsageFlagBits::eUniformBuffer)
			{
				alignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
			}
			else if (bufferCreateInfo.usage & vk::BufferUsageFlagBits::eStorageTexelBuffer)
			{
				alignment = deviceProperties.limits.minTexelBufferOffsetAlignment;
			}
			else
			{
				alignment = 0;
			}

		}


		std::shared_ptr<SectorData> GetSector()
		{
			sectors.emplace_back(new SectorData());
			sectors.back()->bufferAllocation = this;
			return sectors.back();
		}

		void Update()
		{
			if (cmdManager.GetSubmitCount() == 0)
			{
				cmdManager.IncrementSubmitCount();
				uint64_t currentOffset = 0;

				for (auto& sector : sectors)
				{
					uint64_t memoryBlock = (alignment != 0)? ((sector->neededSize / alignment) + 1)*alignment : sector->neededSize;
					sector->allocationOffset = currentOffset;
					sector->allocatedSize = memoryBlock;
					currentOffset += memoryBlock;
				}
				bufferCreateInfo.size = currentOffset;
				bufferData = vom.VmaMakeBuffer(bufferCreateInfo, allocationCreateInfo, false);

				return;
			}
			bool record = false;
			for (auto sector : sectors)
			{
				if (sector->neededSize > sector->allocatedSize)
				{
					record = true;
					break;
				}
			}

			if (record)
			{
				std::vector<vk::BufferCopy> copyOps;
				uint64_t currentOffset = 0;

				for (auto& sector : sectors)
				{
					uint64_t memoryBlock = (alignment != 0) ? ((sector->neededSize / alignment) + 1) * alignment : sector->neededSize;
					copyOps.emplace_back(vk::BufferCopy(sector->allocationOffset, currentOffset, (sector->neededSize < sector->allocatedSize) ? sector->neededSize : sector->allocatedSize));
					sector->allocationOffset = currentOffset;
					sector->allocatedSize = memoryBlock;
					currentOffset += memoryBlock;
				}
				
				bufferCreateInfo.size = currentOffset;

				auto newBufferAllocation = vom.VmaMakeBuffer(bufferCreateInfo, allocationCreateInfo, false);

				cmdManager.Reset();
				auto transferBuffer = cmdManager.RecordNew();
				transferBuffer.begin(vk::CommandBufferBeginInfo({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }));
				transferBuffer.copyBuffer(bufferData.buffer, newBufferAllocation.buffer, copyOps.size(), copyOps.data());
				transferBuffer.end();

				cmdManager.Execute(true, false, false, false);
				vom.Manage(bufferData);
				bufferData = newBufferAllocation;
			}
		}

		void RemoveSector(std::shared_ptr<SectorData> sector)
		{
			auto iter = sectors.begin();
			while (*iter != sector)
			{
				++iter;
			}
			sectors.erase(iter);
		}
		~BufferManager()
		{
			vom.Manage(bufferData);
		}
	};

	struct TransferDetails
	{
		const uint64_t size = 0;
		const void* ramSrc = nullptr;
		const void* ramDst = nullptr;

		/**
		 * \brief This tells what buffer version the commands have been recorded for for the src memory buffer
		 */
		std::shared_ptr<uint64_t> srcVersion = std::make_shared<uint64_t>(-1);
		/**
		 * \brief This tells what buffer version the commands have been recorded for for the dst memory buffer
		 */
		std::shared_ptr<uint64_t> dstVersion = std::make_shared<uint64_t>(-1);

		const std::shared_ptr<SectorData> srcSector = nullptr;
		const std::shared_ptr<SectorData> dstSector = nullptr;

		const vk::Image srcImage;
		const vk::Image dstImage;

		///\brief Copy from ram
		TransferDetails(uint64_t _size, void* _src, std::shared_ptr<SectorData> _dst) : size(_size), ramSrc(_src), dstSector(_dst){}
		///\brief Copy to sector
		TransferDetails(std::shared_ptr<SectorData> src, std::shared_ptr<SectorData> dst, uint64_t _size) : srcSector(src), dstSector(dst), size(_size){}
		///\brief Copy to image
		TransferDetails(std::shared_ptr<SectorData> src, vk::Image dst) : srcSector(src), dstImage(dst) {}
		///\brief Copy from image
		TransferDetails(vk::Image src, std::shared_ptr<SectorData> dst) : srcImage(src), dstSector(dst) {}
		///\brief Copy to ram
		TransferDetails(std::shared_ptr<SectorData> src, void* dst) : srcSector(src), ramDst(dst) {}

		bool IsCopyFromRam()
		{
			if (ramSrc != nullptr && dstSector != nullptr)
			{
				return true;
			}
			return false;
		}
		bool IsCopyToSector()
		{
			if (srcSector != nullptr && dstSector != nullptr)
			{
				return true;
			}
			return false;
		}
		bool IsCopyToImage()
		{
			if (srcSector != nullptr && dstImage != NULL)
			{
				return true;
			}
			return false;
		}
		bool IsCopyFromImage()
		{
			if (srcImage != NULL && dstSector != nullptr)
			{
				return true;
			}
			return false;
		}
		bool IsCopyToRam()
		{
			if (srcSector != nullptr && ramDst != nullptr)
			{
				return true;
			}
			return false;
		}
		bool NeedsRecording()
		{
			if (!IsCopyFromRam() && *srcVersion != srcSector->bufferAllocation->cmdManager.GetSubmitCount())
			{
				return true;
			}
			if (!IsCopyToRam() && *dstVersion != dstSector->bufferAllocation->cmdManager.GetSubmitCount())
			{
				return true;
			}
			return false;
		}

	};

	struct Section
	{
		CommandManager cmdManager;
		std::vector<TransferDetails> transfers;
		Section(VulkanObjectManager& _vom, vk::CommandBuffer externalBuffer)
		: cmdManager(_vom, _vom.GetTransferQueue(), false, vk::PipelineStageFlagBits::eTransfer)
		{
			cmdManager.AddFreeBuffers({ externalBuffer });
		}
		Section(VulkanObjectManager& _vom, vk::CommandBuffer externalBuffer, std::vector<WaitData> waits)
			: cmdManager(_vom, _vom.GetTransferQueue(), false, vk::PipelineStageFlagBits::eTransfer)
		{
			cmdManager.DependsOn(waits);
			cmdManager.AddFreeBuffers({externalBuffer});
		}
	};

	enum class TransferType
	{
		FROMRAM = 0, TORAM = 1, SECTOR = 2, FROMIMAGE = 3, TOIMAGE = 4
	};

	struct FromRamEntity
	{
		TransferType type = TransferType::FROMRAM;
		uint64_t const size;
		void* const srcRam;
		std::shared_ptr<SectorData> const dstSector;
		FromRamEntity(uint64_t const _size, void* const _srcRam, std::shared_ptr<SectorData> const _dstSector)
		: size(_size), srcRam(_srcRam), dstSector(_dstSector){}
	};
	struct ToRamEntity
	{
		TransferType type = TransferType::TORAM;
		void* const dstRam;
		std::shared_ptr<SectorData> const srcSector;
		ToRamEntity(std::shared_ptr<SectorData> const _srcSector, void* const _dstRam)
		: dstRam(_dstRam), srcSector(_srcSector){}
	};
	struct SectorTransferEntity
	{
		TransferType type = TransferType::SECTOR;
		std::shared_ptr<SectorData> const srcSector;
		std::shared_ptr<SectorData> const dstSector;
		SectorTransferEntity(std::shared_ptr<SectorData> const _srcSector, std::shared_ptr<SectorData> const _dstSector)
		: srcSector(_srcSector), dstSector(_dstSector){}
	};
	struct FromImageEntity
	{
		TransferType type = TransferType::FROMIMAGE;
		vk::Image const srcImage;
		std::shared_ptr<SectorData> const dstSector;
		vk::BufferImageCopy copyData;
		vk::ImageLayout const srcImageFormat;
		FromImageEntity(vk::Image const _srcImage,
			std::shared_ptr<SectorData> const _dstSector,
			vk::BufferImageCopy _copyData,
			vk::ImageLayout const _srcImageFormat
		) : srcImage(_srcImage), dstSector(_dstSector), copyData(_copyData), srcImageFormat(_srcImageFormat){}
	};
	struct ToImageEntity
	{
		TransferType type = TransferType::TOIMAGE;
		vk::Image const dstImage;
		std::shared_ptr<SectorData> const srcSector;
		vk::BufferImageCopy copyData;
		vk::ImageLayout const dstImageFormat;
		ToImageEntity(vk::Image const _dstImage,
			std::shared_ptr<SectorData> const _srcSector,
			vk::BufferImageCopy _copyData,
			vk::ImageLayout const _dstImageFormat
		) : dstImage(_dstImage), srcSector(_srcSector), copyData(_copyData), dstImageFormat(_dstImageFormat){}
	};
	struct TransferEntity
	{
		TransferType const type;
		uint64_t const size;
		std::shared_ptr<SectorData> const srcSector;
		std::shared_ptr<SectorData> const dstSector;
		void* const srcRam;
		void* const dstRam;
		vk::Image const srcImage;
		vk::Image const dstImage;
		vk::BufferImageCopy imageCopy;
		vk::ImageLayout const imageLayout;

		TransferEntity(TransferType const _type,
			uint64_t const _size,
			std::shared_ptr<SectorData> const _srcSector,
			std::shared_ptr<SectorData> const _dstSector,
			void* const _srcRam,
			void* const _dstRam,
			vk::Image const _srcImage,
			vk::Image const _dstImage,
			vk::BufferImageCopy _imageCopy,
			vk::ImageLayout const _imageLayout
		) : type(_type),
		size(_size),
		srcSector(_srcSector),
		dstSector(_dstSector),
		srcRam(_srcRam),
		dstRam(_dstRam),
		srcImage(_srcImage),
		dstImage(_dstImage),
		imageCopy(_imageCopy),
		imageLayout(_imageLayout){}

		FromRamEntity AsFromRamTransfer()
		{
			assert(srcRam != nullptr);
			assert(dstSector != nullptr);
			return FromRamEntity(size, srcRam, dstSector);
		}
		ToRamEntity AsToRamTransfer()
		{
			assert(dstRam != nullptr);
			assert(srcSector != nullptr);
			return ToRamEntity(srcSector, dstRam);
		}
		SectorTransferEntity AsSectorTransfer()
		{
			assert(srcSector != nullptr);
			assert(dstSector != nullptr);
			return SectorTransferEntity(srcSector, dstSector);
		}
		FromImageEntity AsFromImageTransfer()
		{
			assert(srcImage != NULL);
			assert(dstSector != nullptr);
			assert(imageCopy != vk::BufferImageCopy());
			return FromImageEntity(srcImage, dstSector, imageCopy, imageLayout);
		}
		ToImageEntity AsToImageTransfer()
		{
			assert(dstImage != NULL);
			assert(srcSector != nullptr);
			assert(imageCopy != vk::BufferImageCopy());
			return ToImageEntity(dstImage, srcSector, imageCopy, imageLayout);
		}

	};

	class TransferData
	{
		std::vector<TransferType const> types;
		std::vector<uint64_t const> sizes;
		std::vector<uint64_t> srcVersion;
		std::vector<uint64_t> dstVersion;
		std::vector<std::shared_ptr<SectorData> const> srcSectors;
		std::vector<std::shared_ptr<SectorData> const> dstSectors;
		std::vector<void* const> srcRam;
		std::vector<void* const> dstRam;
		std::vector<vk::Image const> srcImage;
		std::vector<vk::Image const> dstImage;
		std::vector<vk::BufferImageCopy> imageCopies;
		std::vector<vk::ImageLayout const> imageLayouts;

		bool IsFromRamTransfer(uint64_t index)
		{
			return types[index] == TransferType::FROMRAM;
		}
		bool IsToRamTransfer(uint64_t index)
		{
			return types[index] == TransferType::TORAM;
		}
		bool IsSectorTransfer(uint64_t index)
		{
			return types[index] == TransferType::SECTOR;
		}
		bool IsFromImageTransfer(uint64_t index)
		{
			return types[index] == TransferType::FROMIMAGE;
		}
		bool IsToImageTransfer(uint64_t index)
		{
			return types[index] == TransferType::TOIMAGE;
		}

		TransferEntity operator[](uint64_t index)
		{
			return TransferEntity(
				types[index],
				sizes[index],
				srcSectors[index],
				dstSectors[index],
				srcRam[index],
				dstRam[index],
				srcImage[index],
				dstImage[index],
				imageCopies[index],
				imageLayouts[index]);
		}
		void EmplaceBack(TransferEntity entity)
		{
			types.emplace_back(entity.type);
			sizes.emplace_back(entity.size);
			srcVersion.emplace_back(-1);
			dstVersion.emplace_back(-1);
			srcSectors.emplace_back(entity.srcSector);
			dstSectors.emplace_back(entity.dstSector);
			srcRam.emplace_back(entity.srcRam);
			dstRam.emplace_back(entity.dstRam);
			srcImage.emplace_back(entity.srcImage);
			dstImage.emplace_back(entity.dstImage);
			imageCopies.emplace_back(entity.imageCopy);
			imageLayouts.emplace_back(entity.imageLayout);
		}
		void EmplaceBack(FromRamEntity entity)
		{
			EmplaceBack(TransferEntity(
				entity.type,
				entity.size,
				{},
				entity.dstSector,
				entity.srcRam,
				{}, {}, {}, {}, {}));
		}
		void EmplaceBack(ToRamEntity entity)
		{
			EmplaceBack(TransferEntity(
				entity.type,
				{},
				entity.srcSector,
				{},
				{},
				entity.dstRam, {}, {}, {}, {}));
		}
		void EmplaceBack(SectorTransferEntity entity)
		{
			EmplaceBack(TransferEntity(
				entity.type,
				{},
				entity.srcSector,
				entity.dstSector,
				{},
				{}, {}, {}, {}, {}));
		}
		void EmplaceBack(FromImageEntity entity)
		{
			EmplaceBack(TransferEntity(
				entity.type,
				{},
				{},
				entity.dstSector,
				{},
				{},
				entity.srcImage,
				{},
				entity.copyData,
				entity.srcImageFormat));
		}
		void EmplaceBack(ToImageEntity entity)
		{
			EmplaceBack(TransferEntity(
				entity.type,
				{},
				entity.srcSector,
				{},
				{},
				{},
				{},
				entity.dstImage,
				entity.copyData,
				entity.dstImageFormat));
		}


	};

	class MemoryOperationsBuffer
	{
	public:
		VulkanObjectManager vom;
		CommandBufferCache commandBufferCache;
		std::list<Section> sections;
		std::vector<vk::SubmitInfo> submits;
		std::vector<TransferDetails> transfers;
		std::vector<WaitData> waitDatas;
		bool record = false;
		BufferManager transferBuffer;

		tf::Executor transferExecutor = tf::Executor(1);
		tf::Taskflow flow;

		MemoryOperationsBuffer(VulkanObjectManager& _vom)
		: vom(_vom), transferBuffer(_vom.GetDevice(), _vom.GetAllocator(), _vom.GetTransferQueue(), vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_ONLY)
		{
			vom.SetTransferQueue(_vom.GetTransferQueue());
			commandBufferCache = CommandBufferCache(vom, vom.MakeCommandPool(vk::CommandPoolCreateInfo({}, vom.GetTransferQueue().index)));
		}
		MemoryOperationsBuffer(vk::Device deviceHandle, VmaAllocator allocatorHandle, QueueData transferQueueData)
		: vom(deviceHandle), transferBuffer(deviceHandle, allocatorHandle, transferQueueData, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_ONLY)
		{
			assert(transferQueueData.queue != NULL);
			vom.SetTransferQueue(transferQueueData);
			commandBufferCache = CommandBufferCache(vom, vom.MakeCommandPool(vk::CommandPoolCreateInfo({}, vom.GetTransferQueue().index)));
		}

		void CopyFromRam(uint64_t size, void* src, std::shared_ptr<SectorData> dst)
		{
			assert(src != nullptr);
			assert(dst != nullptr);
			transfers.emplace_back(TransferDetails(size, src, dst));
			if (dst->neededSize < size)
			{
				dst->neededSize = size;
			}
		}
		void CopyToSector(std::shared_ptr<SectorData> src, std::shared_ptr<SectorData> dst, uint64_t size)
		{
			assert(src != nullptr);
			assert(dst != nullptr);
			transfers.emplace_back(TransferDetails(src, dst, size));
			if (dst->neededSize < src->neededSize)
			{
				dst->neededSize = src->neededSize;
			}
		}
		void CopyToImage(std::shared_ptr<SectorData> src, vk::Image dst)
		{
			assert(src != nullptr);
			assert(dst != NULL);
			transfers.emplace_back(TransferDetails(src, dst));
			vk::MemoryRequirements reqs = vom.GetDevice().getImageMemoryRequirements(dst);
			assert(reqs.size == src->neededSize);
		}
		void CopyFromImage(vk::Image src, std::shared_ptr<SectorData> dst)
		{
			assert(src != NULL);
			assert(dst != nullptr);
			transfers.emplace_back(TransferDetails(src, dst));
			vk::MemoryRequirements reqs = vom.GetDevice().getImageMemoryRequirements(src);
			if (dst->neededSize < reqs.size)
			{
				dst->neededSize = reqs.size;
			}
		}
		void CopyToRam(std::shared_ptr<SectorData> src, void* dst)
		{
			assert(src != NULL);
			assert(dst != nullptr);
			transfers.emplace_back(TransferDetails(src, dst));
		}
		void DependsOn(WaitData wait)
		{
			waitDatas.emplace_back(wait);
		}
		void Clear()
		{
			transfers.clear();
			waitDatas.clear();
		};

		void Execute(std::vector<WaitData> transientWaits = {}, bool wait = false)
		{
			for (auto transfer : transfers)
			{
				if (transfer.NeedsRecording())
				{
					record = true;
					break;
				}
			}

			if (record)
			{
				commandBufferCache.ResetCommandPool();
				sections.clear();
				sections.emplace_back(vom, commandBufferCache.NextCommandBuffer(), transientWaits);
				flow.clear();


				for (auto& transfer : transfers)
				{
					if (transfer.IsCopyFromRam() || transfer.IsCopyToRam())
					{
						if (!sections.back().transfers.empty())
						{
							auto previousSignal = sections.back().cmdManager.GetMainTimelineSignal();
							std::vector<WaitData> waits = { WaitData(previousSignal.signalValue, previousSignal.semaphore, vk::PipelineStageFlagBits::eTransfer) };
							sections.emplace_back(vom, commandBufferCache.NextCommandBuffer(), waits);
						}

						sections.back().transfers.emplace_back(transfer);

						auto previousSignal = sections.back().cmdManager.GetMainTimelineSignal();
						std::vector<WaitData> waits = { WaitData(previousSignal.signalValue, previousSignal.semaphore, vk::PipelineStageFlagBits::eTransfer) };
						sections.emplace_back(vom, commandBufferCache.NextCommandBuffer(), waits);

					}
					else
					{
						sections.back().transfers.emplace_back(transfer);
					}
				}
				for (auto& section : sections)
				{
					if (section.transfers[0].IsCopyFromRam() || section.transfers[0].IsCopyToRam())
					{
						assert(section.cmdManager.syncManager.waitSemaphores.timelineCount == section.cmdManager.syncManager.waitSemaphores.semaphores.size());
						section.cmdManager.IncrementSubmitCount();
						flow.emplace([device = vom.GetDevice(), &cmdManager = section.cmdManager, &ramTransfer = section.transfers[0]]()
						{
							if (cmdManager.syncManager.waitSemaphores.size() > 0)
							{
								cmdManager.syncManager.waitSemaphores.ExtractTimelineValues();
								vk::SemaphoreWaitInfo waitInfo({}, cmdManager.syncManager.waitSemaphores.size(), cmdManager.syncManager.waitSemaphores.semaphores.data(), cmdManager.syncManager.waitSemaphores.extractedTimelineValues.data());
								auto res = device.waitSemaphores(waitInfo, UINT64_MAX);
							}

							if (ramTransfer.IsCopyFromRam())
							{
								*ramTransfer.dstVersion = ramTransfer.dstSector->bufferAllocation->cmdManager.GetSubmitCount();

								VkMemoryPropertyFlags flags;
								vmaGetAllocationMemoryProperties(ramTransfer.dstSector->bufferAllocation->vom.GetAllocator(), ramTransfer.dstSector->bufferAllocation->bufferData.allocation, &flags);
								assert((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) || (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

								if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
								{
									vmaMapMemory(ramTransfer.dstSector->bufferAllocation->vom.GetAllocator(), ramTransfer.dstSector->bufferAllocation->bufferData.allocation, &ramTransfer.dstSector->bufferAllocation->map);
									memcpy(reinterpret_cast<char*>(ramTransfer.dstSector->bufferAllocation->map) + ramTransfer.dstSector->allocationOffset, ramTransfer.ramSrc, ramTransfer.size);
									vmaUnmapMemory(ramTransfer.dstSector->bufferAllocation->vom.GetAllocator(), ramTransfer.dstSector->bufferAllocation->bufferData.allocation);
								}
								else if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
								{
									vmaInvalidateAllocation(ramTransfer.dstSector->bufferAllocation->vom.GetAllocator(), ramTransfer.dstSector->bufferAllocation->bufferData.allocation, ramTransfer.dstSector->allocationOffset, ramTransfer.dstSector->allocatedSize);
									vmaMapMemory(ramTransfer.dstSector->bufferAllocation->vom.GetAllocator(), ramTransfer.dstSector->bufferAllocation->bufferData.allocation, &ramTransfer.dstSector->bufferAllocation->map);
									memcpy(reinterpret_cast<char*>(ramTransfer.dstSector->bufferAllocation->map) + ramTransfer.dstSector->allocationOffset, ramTransfer.ramSrc, ramTransfer.size);
									vmaUnmapMemory(ramTransfer.dstSector->bufferAllocation->vom.GetAllocator(), ramTransfer.dstSector->bufferAllocation->bufferData.allocation);
									vmaFlushAllocation(ramTransfer.dstSector->bufferAllocation->vom.GetAllocator(), ramTransfer.dstSector->bufferAllocation->bufferData.allocation, ramTransfer.dstSector->allocationOffset, ramTransfer.dstSector->allocatedSize);
								}

							}
							else if (ramTransfer.IsCopyToRam())
							{
								*ramTransfer.srcVersion = ramTransfer.srcSector->bufferAllocation->cmdManager.GetSubmitCount();


								VkMemoryPropertyFlags flags;
								vmaGetAllocationMemoryProperties(ramTransfer.srcSector->bufferAllocation->vom.GetAllocator(), ramTransfer.srcSector->bufferAllocation->bufferData.allocation, &flags);
								assert(!(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) || (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

								if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
								{
									vmaMapMemory(ramTransfer.srcSector->bufferAllocation->vom.GetAllocator(), ramTransfer.srcSector->bufferAllocation->bufferData.allocation, &ramTransfer.srcSector->bufferAllocation->map);
									memcpy(const_cast<void*>(ramTransfer.ramDst), reinterpret_cast<char*>(ramTransfer.srcSector->bufferAllocation->map) + ramTransfer.srcSector->allocationOffset, ramTransfer.srcSector->neededSize);
									vmaUnmapMemory(ramTransfer.srcSector->bufferAllocation->vom.GetAllocator(), ramTransfer.srcSector->bufferAllocation->bufferData.allocation);
								}
								else if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
								{
									vmaInvalidateAllocation(ramTransfer.srcSector->bufferAllocation->vom.GetAllocator(), ramTransfer.srcSector->bufferAllocation->bufferData.allocation, ramTransfer.srcSector->allocationOffset, ramTransfer.srcSector->allocatedSize);
									vmaMapMemory(ramTransfer.srcSector->bufferAllocation->vom.GetAllocator(), ramTransfer.srcSector->bufferAllocation->bufferData.allocation, &ramTransfer.srcSector->bufferAllocation->map);
									memcpy(const_cast<void*>(ramTransfer.ramDst), reinterpret_cast<char*>(ramTransfer.srcSector->bufferAllocation->map) + ramTransfer.srcSector->allocationOffset, ramTransfer.srcSector->neededSize);
									vmaUnmapMemory(ramTransfer.srcSector->bufferAllocation->vom.GetAllocator(), ramTransfer.srcSector->bufferAllocation->bufferData.allocation);
									vmaFlushAllocation(ramTransfer.srcSector->bufferAllocation->vom.GetAllocator(), ramTransfer.srcSector->bufferAllocation->bufferData.allocation, ramTransfer.srcSector->allocationOffset, ramTransfer.srcSector->allocatedSize);
								}
							}

							auto mainSignal = cmdManager.GetMainTimelineSignal();
							vk::SemaphoreSignalInfo signalInfo(mainSignal.semaphore, *mainSignal.signalValue);
							device.signalSemaphore(signalInfo);



						});
					}
					else
					{
						std::vector<std::shared_ptr<SectorData>> dstSectors;
						auto cmd = section.cmdManager.RecordNew();
						cmd.begin(vk::CommandBufferBeginInfo());
						for (auto& transfer : section.transfers)
						{
							for (auto dst : dstSectors)
							{
								if (dst == transfer.srcSector || dst == transfer.dstSector)
								{
									vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferRead);
									cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 1, &memoryBarrier, 0, {}, 0, {});
									dstSectors.clear();
									break;
								}
							}
							if (transfer.IsCopyToSector())
							{
								*transfer.srcVersion = transfer.srcSector->bufferAllocation->cmdManager.GetSubmitCount();
								*transfer.dstVersion = transfer.dstSector->bufferAllocation->cmdManager.GetSubmitCount();
								vk::BufferCopy copy(transfer.srcSector->allocationOffset, transfer.dstSector->allocationOffset, transfer.size);
								cmd.copyBuffer(transfer.srcSector->bufferAllocation->bufferData.buffer, transfer.dstSector->bufferAllocation->bufferData.buffer, 1, &copy);
								dstSectors.emplace_back(transfer.dstSector);
							}
							if (transfer.IsCopyToImage())
							{
								assert(false);
								/*vk::BufferImageCopy copy(transfer.srcSector->allocationOffset, 0, transfer.srcSector->neededSize);
								cmd.copyBufferToImage(transfer.srcSector->bufferAllocation->buffer, transfer.dstImage, 1, &copy);*/
							}
							if (transfer.IsCopyFromImage())
							{
								assert(false);
							}
						}
						cmd.end();
					}
				}
			}

			submits.clear();
			for (auto& section : sections)
			{
				if (!section.transfers[0].IsCopyFromRam() && !section.transfers[0].IsCopyToRam())
				{
					submits.emplace_back(section.cmdManager.GetSubmitInfo(true, false, false));
				}
			}

			auto res = vom.GetTransferQueue().queue.submit(submits.size(), submits.data(), VK_NULL_HANDLE);
			transferExecutor.run(flow);

			if (wait)
			{
				sections.back().cmdManager.Wait();
				transferExecutor.wait_for_all();
			}

			record = false;

		}

		static void GroupSubmit(std::vector<MemoryOperationsBuffer*> ops, std::vector<WaitData> transientWaits, bool wait = false)
		{
			assert(ops.size() > 0);
			for (auto op : ops)
			{
				op->Execute(transientWaits, wait);
			}
		}

		void WaitOn()
		{
			sections.back().cmdManager.Wait();
		}
		~MemoryOperationsBuffer()
		{
			WaitOn();
		}
	};

}
