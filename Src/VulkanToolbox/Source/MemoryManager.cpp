#include "../Headers/VulkanToolbox.hpp"
#undef MemoryBarrier

namespace vkt
{

	void SectorData::Reset()
	{
		neededSize = 0;
	}
	void SectorData::SetSize(uint64_t _neededSize)
	{
		neededSize = _neededSize;
	}

	BufferManager::BufferManager(vk::Device deviceHandle, VmaAllocator allocator, QueueData _transferQueue, vk::BufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsageFlags)
		: cmdManager(deviceHandle, _transferQueue, true, vk::PipelineStageFlagBits::eTransfer), vom(deviceHandle)
	{
		vom.SetAllocator(allocator);
		vom.SetTransferQueue(_transferQueue);
		const VkPhysicalDeviceProperties* props;
		vmaGetPhysicalDeviceProperties(allocator, &props);
		deviceProperties = *props;
		bufferCreateInfo.setUsage(bufferUsageFlags | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
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
	BufferManager::BufferManager(ObjectManager& _vom, vk::BufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsageFlags)
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


	std::shared_ptr<SectorData> BufferManager::GetSector()
	{
		sectors.emplace_back(new SectorData());
		sectors.back()->bufferAllocation = this;
		return sectors.back();
	}

	void BufferManager::Update(bool wait)
	{
		if (bufferCreateInfo.size == 0)
		{
			uint64_t currentOffset = 0;

			for (auto& sector : sectors)
			{
				uint64_t memoryBlock = (alignment != 0) ? ((sector->neededSize / alignment) + 1) * alignment : sector->neededSize;
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
				if (sector->allocatedSize != 0)
				{
					uint64_t memoryBlock = (alignment != 0) ? ((sector->neededSize / alignment) + 1) * alignment : sector->neededSize;
					copyOps.emplace_back(vk::BufferCopy(sector->allocationOffset, currentOffset, (sector->neededSize < sector->allocatedSize) ? sector->neededSize : sector->allocatedSize));
					sector->allocationOffset = currentOffset;
					sector->allocatedSize = memoryBlock;
					currentOffset += memoryBlock;
				}
				else
				{
					uint64_t memoryBlock = (alignment != 0) ? ((sector->neededSize / alignment) + 1) * alignment : sector->neededSize;
					sector->allocationOffset = currentOffset;
					sector->allocatedSize = memoryBlock;
					currentOffset += memoryBlock;
				}

			}

			bufferCreateInfo.size = currentOffset;

			auto newBufferAllocation = vom.VmaMakeBuffer(bufferCreateInfo, allocationCreateInfo, false);

			if (copyOps.size() > 0)
			{
				cmdManager.Reset();
				auto transferBuffer = cmdManager.RecordNew();
				transferBuffer.begin(vk::CommandBufferBeginInfo({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit }));
				transferBuffer.copyBuffer(bufferData.buffer, newBufferAllocation.buffer, copyOps.size(), copyOps.data());
				transferBuffer.end();
				cmdManager.Execute(true, wait, false, false);
			}

			vom.Manage(bufferData);
			bufferData = newBufferAllocation;
		}
	}

	void BufferManager::RemoveSector(std::shared_ptr<SectorData> sector)
	{
		auto iter = sectors.begin();
		while (*iter != sector)
		{
			++iter;
		}
		sectors.erase(iter);
	}

	void BufferManager::Clear()
	{
		//bufferCreateInfo.size = 0;
		sectors.clear();
	}

	void BufferManager::Free()
	{
		bufferCreateInfo.size = 0;
		sectors.clear();
		if (bufferData.buffer != NULL)
		{
			vom.Manage(bufferData);
		}
		vom.DestroyAll();
	}

	BufferManager::~BufferManager()
	{
		vom.Manage(bufferData);
	}

	SectorToSectorEntity::SectorToSectorEntity(
		uint64_t _index,
		std::shared_ptr<SectorData>& _srcSector,
		uint64_t& _srcVersion,
		std::shared_ptr<SectorData>& _dstSector,
		uint64_t& _dstVersion,
		uint64_t& _size)
		: index(_index), srcSector(_srcSector), srcVersion(_srcVersion), dstSector(_dstSector), dstVersion(_dstVersion), size(_size)
	{
	}

	std::vector<WaitData> SectorToSectorEntity::Record(vk::CommandBuffer cmd)
	{
		srcVersion = srcSector->bufferAllocation->cmdManager.GetSubmitCount();
		dstVersion = dstSector->bufferAllocation->cmdManager.GetSubmitCount();

		vk::BufferCopy copy(srcSector->allocationOffset, dstSector->allocationOffset, size);
		cmd.copyBuffer(srcSector->bufferAllocation->bufferData.buffer, dstSector->bufferAllocation->bufferData.buffer, 1, &copy);

		return { WaitData(srcSector->bufferAllocation->cmdManager.GetSubmitCountPtr(), srcSector->bufferAllocation->cmdManager.GetMainTimelineSignal().semaphore, vk::PipelineStageFlagBits::eTransfer),
		WaitData(dstSector->bufferAllocation->cmdManager.GetSubmitCountPtr(), dstSector->bufferAllocation->cmdManager.GetMainTimelineSignal().semaphore, vk::PipelineStageFlagBits::eTransfer) };
	}
	bool SectorToSectorEntity::NeedsRecording()
	{
		return !(srcVersion == srcSector->bufferAllocation->cmdManager.GetSubmitCount() && dstVersion == dstSector->bufferAllocation->cmdManager.GetSubmitCount());
	}

	ImageToSectorEntity::ImageToSectorEntity(
		uint64_t _index,
		vk::Image& _srcImage,
		std::shared_ptr<SectorData>& _dstSector,
		uint64_t& _dstVersion,
		vk::BufferImageCopy& _copyData,
		vk::ImageLayout& _srcImageFormat,
		vk::ImageSubresourceRange& _subresourceRange
	) : index(_index), srcImage(_srcImage), dstSector(_dstSector), dstVersion(_dstVersion), bufferImageCopy(_copyData), srcImageFormat(_srcImageFormat), subresourceRange(_subresourceRange)
	{
	}

	std::vector<WaitData> ImageToSectorEntity::Record(vk::CommandBuffer cmd)
	{
		dstVersion = dstSector->bufferAllocation->cmdManager.GetSubmitCount();

		bufferImageCopy.bufferOffset = dstSector->allocationOffset;
		vk::ImageMemoryBarrier imageMemoryBarrier(vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferWrite, srcImageFormat,
			vk::ImageLayout::eTransferSrcOptimal, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, srcImage,
			subresourceRange);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eTransfer,
			{},
			0, {},
			0, {},
			1, &imageMemoryBarrier);
		cmd.copyImageToBuffer(srcImage, vk::ImageLayout::eTransferSrcOptimal, dstSector->bufferAllocation->bufferData.buffer, 1, &bufferImageCopy);
		imageMemoryBarrier = vk::ImageMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eNoneKHR, vk::ImageLayout::eTransferSrcOptimal,
			srcImageFormat, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, srcImage,
			subresourceRange);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
			{},
			0, {},
			0, {},
			1, &imageMemoryBarrier);

		return { WaitData(dstSector->bufferAllocation->cmdManager.GetSubmitCountPtr(), dstSector->bufferAllocation->cmdManager.GetMainTimelineSignal().semaphore, vk::PipelineStageFlagBits::eTransfer) };
	}

	bool ImageToSectorEntity::NeedsRecording()
	{
		return !(dstVersion == dstSector->bufferAllocation->cmdManager.GetSubmitCount());
	}


	SectorToImageEntity::SectorToImageEntity(
		uint64_t _index,
		vk::Image& _dstImage,
		std::shared_ptr<SectorData>& _srcSector,
		uint64_t& _srcVersion,
		vk::BufferImageCopy& _copyData,
		vk::ImageLayout& _dstImageFormat,
		vk::ImageSubresourceRange& _subresourceRange
	) : index(_index), dstImage(_dstImage), srcSector(_srcSector), srcVersion(_srcVersion), bufferImageCopy(_copyData), dstImageFormat(_dstImageFormat), subresourceRange(_subresourceRange)
	{
	}

	std::vector<WaitData> SectorToImageEntity::Record(vk::CommandBuffer cmd)
	{
		srcVersion = srcSector->bufferAllocation->cmdManager.GetSubmitCount();

		bufferImageCopy.bufferOffset = srcSector->allocationOffset;
		vk::ImageMemoryBarrier imageMemoryBarrier(vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferWrite, dstImageFormat,
			vk::ImageLayout::eTransferDstOptimal, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, dstImage,
			subresourceRange);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eTransfer,
			{},
			0, {},
			0, {},
			1, &imageMemoryBarrier);
		cmd.copyBufferToImage(srcSector->bufferAllocation->bufferData.buffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy);
		imageMemoryBarrier = vk::ImageMemoryBarrier(
			vk::AccessFlagBits::eTransferWrite,
			vk::AccessFlagBits::eNoneKHR,
			vk::ImageLayout::eTransferDstOptimal,
			dstImageFormat, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, dstImage,
			subresourceRange);
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
			{},
			0, {},
			0, {},
			1, &imageMemoryBarrier);

		return { WaitData(srcSector->bufferAllocation->cmdManager.GetSubmitCountPtr(), srcSector->bufferAllocation->cmdManager.GetMainTimelineSignal().semaphore, vk::PipelineStageFlagBits::eTransfer) };
	}

	bool SectorToImageEntity::NeedsRecording()
	{
		return !(srcVersion == srcSector->bufferAllocation->cmdManager.GetSubmitCount());
	}

	ImageToImageEntity::ImageToImageEntity(
		uint64_t _index,
		vk::Image& _srcImage,
		vk::Image& _dstImage,
		vk::ImageLayout& _srcImageLayout,
		vk::ImageLayout& _dstImageLayout,
		vk::ImageCopy& _copyData,
		vk::ImageSubresourceRange& _subresourceRange)
		: index(_index), srcImage(_srcImage), dstImage(_dstImage), srcImageLayout(_srcImageLayout), dstImageLayout(_dstImageLayout), imageCopy(_copyData), subresourceRange(_subresourceRange)
	{
	}

	std::vector<WaitData> ImageToImageEntity::Record(vk::CommandBuffer cmd)
	{
		vk::ImageMemoryBarrier srcImageTransition(vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferRead, srcImageLayout, vk::ImageLayout::eTransferSrcOptimal,
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, srcImage, subresourceRange);
		vk::ImageMemoryBarrier dstImageTransition(vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferWrite, dstImageLayout, vk::ImageLayout::eTransferDstOptimal,
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, dstImage, subresourceRange);

		vk::ImageMemoryBarrier srcImageDeTransition(vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eNoneKHR, vk::ImageLayout::eTransferSrcOptimal, srcImageLayout,
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, srcImage, subresourceRange);
		vk::ImageMemoryBarrier dstImageDeTransition(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eNoneKHR, vk::ImageLayout::eTransferDstOptimal, dstImageLayout,
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, dstImage, subresourceRange);

		vk::ImageMemoryBarrier transitionsBarriers[] = { srcImageTransition, dstImageTransition };
		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eTransfer,
			{},
			0, {},
			0, {},
			2, transitionsBarriers);
		cmd.copyImage(srcImage, vk::ImageLayout::eTransferSrcOptimal, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &imageCopy);
		vk::ImageMemoryBarrier DeTransitionsBarriers[] = { srcImageDeTransition, dstImageDeTransition };

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
			{},
			0, {},
			0, {},
			2, DeTransitionsBarriers);
		return {};
	}

	bool ImageToImageEntity::NeedsRecording()
	{
		return false;
	}


	TransferEntity::TransferEntity(uint64_t _index,
		TransferType& _type,
		uint64_t& _size,
		std::shared_ptr<SectorData>& _srcSector,
		uint64_t& _srcVersion,
		std::shared_ptr<SectorData>& _dstSector,
		uint64_t& _dstVersion,
		vk::Image& _srcImage,
		vk::Image& _dstImage,
		vk::BufferImageCopy& _bufferImageCopy,
		vk::ImageCopy& _imageCopy,
		vk::ImageLayout& _srcImageLayout,
		vk::ImageLayout& _dstImageLayout,
		vk::ImageSubresourceRange& _subresourceRange
	) : index(_index),
		type(_type),
		size(_size),
		srcSector(_srcSector),
		srcVersion(_srcVersion),
		dstSector(_dstSector),
		dstVersion(_dstVersion),
		srcImage(_srcImage),
		dstImage(_dstImage),
		bufferImageCopy(_bufferImageCopy),
		imageCopy(_imageCopy),
		srcImageLayout(_srcImageLayout),
		dstImageLayout(_dstImageLayout),
		subresourceRange(_subresourceRange)
	{
	}

	SectorToSectorEntity TransferEntity::AsSectorToSector()
	{
		assert(srcSector != nullptr);
		assert(dstSector != nullptr);
		assert(IsSectorToSector());
		return SectorToSectorEntity(index, srcSector, srcVersion, dstSector, dstVersion, size);
	}
	ImageToSectorEntity TransferEntity::AsImageToSector()
	{
		assert(srcImage != NULL);
		assert(dstSector != nullptr);
		assert(bufferImageCopy != vk::BufferImageCopy());
		assert(IsImageToSector());
		return ImageToSectorEntity(index, srcImage, dstSector, dstVersion, bufferImageCopy, srcImageLayout, subresourceRange);
	}
	SectorToImageEntity TransferEntity::AsSectorToImage()
	{
		assert(dstImage != NULL);
		assert(srcSector != nullptr);
		assert(bufferImageCopy != vk::BufferImageCopy());
		assert(IsSectorToImage());
		return SectorToImageEntity(index, dstImage, srcSector, srcVersion, bufferImageCopy, dstImageLayout, subresourceRange);
	}
	ImageToImageEntity TransferEntity::AsImageToImage()
	{
		assert(srcImage != NULL);
		assert(dstImage != NULL);
		assert(imageCopy != vk::ImageCopy());
		assert(IsImageToImage());
		return ImageToImageEntity(index, srcImage, dstImage, srcImageLayout, dstImageLayout, imageCopy, subresourceRange);
	}

	bool TransferEntity::IsSectorToSector()
	{
		return type == TransferType::SECTORTOSECTOR;
	}
	bool TransferEntity::IsImageToSector()
	{
		return type == TransferType::IMAGETOSECTOR;
	}
	bool TransferEntity::IsSectorToImage()
	{
		return type == TransferType::SECTORTOIMAGE;
	}
	bool TransferEntity::IsImageToImage()
	{
		return type == TransferType::IMAGETOIMAGE;
	}

	bool TransferEntity::NeedsRecording()
	{
		assert(IsSectorToSector() || IsSectorToImage() || IsImageToSector() || IsImageToImage());
		if (IsSectorToSector())
		{
			return AsSectorToSector().NeedsRecording();
		}
		else if (IsSectorToImage())
		{
			return AsSectorToImage().NeedsRecording();
		}
		else if (IsImageToSector())
		{
			return AsImageToSector().NeedsRecording();
		}
		else if (IsImageToImage())
		{
			return AsImageToImage().NeedsRecording();
		}
		return true;
	}

	std::vector<WaitData> TransferEntity::Record(vk::CommandBuffer cmd)
	{
		assert(IsSectorToSector() || IsSectorToImage() || IsImageToSector() || IsImageToImage());
		if (IsSectorToSector())
		{
			return AsSectorToSector().Record(cmd);
		}
		else if (IsSectorToImage())
		{
			return AsSectorToImage().Record(cmd);
		}
		else if (IsImageToSector())
		{
			return AsImageToSector().Record(cmd);
		}
		else if (IsImageToImage())
		{
			return AsImageToImage().Record(cmd);
		}
		return {};
	}



	TransferEntity TransferData::operator[](uint64_t index)
	{
		return TransferEntity(
			index,
			types[index],
			sizes[index],
			srcSectors[index],
			srcVersion[index],
			dstSectors[index],
			dstVersion[index],
			srcImages[index],
			dstImages[index],
			bufferImageCopies[index],
			imageCopies[index],
			srcImageLayouts[index],
			dstImageLayouts[index],
			subresourceRanges[index]);
	}
	uint64_t TransferData::EmplaceBack(
		TransferType type,
		uint64_t size,
		std::shared_ptr<SectorData> srcSector,
		std::shared_ptr<SectorData> dstSector,
		vk::Image srcImage,
		vk::Image dstImage,
		vk::BufferImageCopy bufferImageCopy,
		vk::ImageCopy imageCopy,
		vk::ImageLayout srcImageLayout,
		vk::ImageLayout dstImageLayout,
		vk::ImageSubresourceRange subresourceRange
	)
	{
		types.emplace_back(type);
		sizes.emplace_back(size);
		srcVersion.emplace_back(-1);
		dstVersion.emplace_back(-1);
		srcSectors.emplace_back(srcSector);
		dstSectors.emplace_back(dstSector);
		srcImages.emplace_back(srcImage);
		dstImages.emplace_back(dstImage);
		bufferImageCopies.emplace_back(bufferImageCopy);
		imageCopies.emplace_back(imageCopy);
		srcImageLayouts.emplace_back(srcImageLayout);
		dstImageLayouts.emplace_back(dstImageLayout);
		subresourceRanges.emplace_back(subresourceRange);
		return types.size() - 1;
	}

	uint64_t TransferData::EmplaceSectorToSector(std::shared_ptr<SectorData> srcSector, std::shared_ptr<SectorData> dstSector, uint64_t size)
	{
		return EmplaceBack(
			TransferType::SECTORTOSECTOR,
			size,
			srcSector,
			dstSector,
			{},
			{},
			{},
			{},
			{},
			{},
			{});
	}
	uint64_t TransferData::EmplaceImageToSector(vk::Image srcImage, std::shared_ptr<SectorData> dstSector, vk::BufferImageCopy imageToBufferCopy, vk::ImageLayout srcImageLayout, vk::ImageSubresourceRange subresourceRange)
	{
		return EmplaceBack(
			TransferType::IMAGETOSECTOR,
			{},
			{},
			dstSector,
			srcImage,
			{},
			imageToBufferCopy,
			{},
			srcImageLayout,
			{},
			subresourceRange);
	}
	uint64_t TransferData::EmplaceSectorToImage(std::shared_ptr<SectorData> srcSector, vk::Image dstImage, vk::BufferImageCopy bufferToImageCopy, vk::ImageLayout dstImageLayout, vk::ImageSubresourceRange subresourceRange)
	{
		return EmplaceBack(
			TransferType::SECTORTOIMAGE,
			{},
			srcSector,
			{},
			{},
			dstImage,
			bufferToImageCopy,
			{},
			{},
			dstImageLayout,
			subresourceRange);
	}
	uint64_t TransferData::EmplaceImageToImage(vk::Image srcImage, vk::Image dstImage, vk::ImageCopy imageCopy, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::ImageSubresourceRange subresourceRange)
	{
		return EmplaceBack(
			TransferType::IMAGETOIMAGE,
			{},
			{},
			{},
			srcImage,
			dstImage,
			{},
			imageCopy,
			srcImageLayout,
			dstImageLayout,
			subresourceRange
		);
	}

	uint64_t TransferData::Size()
	{
		return types.size();
	}




	ToRamTransferExecutor::ToRamTransferExecutor(vk::Device deviceHandle, std::shared_ptr<SectorData> _stagingBuffer, void* _dst, WaitData _wait) :device(deviceHandle), stagingBuffer(_stagingBuffer), dst(_dst), waitData(_wait.waitValuePtr, _wait.waitSemaphore, vk::PipelineStageFlagBits::eTransfer) {}
	void ToRamTransferExecutor::Execute()
	{
		vk::SemaphoreWaitInfo waitInfo({}, 1, &waitData.waitSemaphore, waitData.waitValuePtr.get());
		auto res = device.waitSemaphores(waitInfo, UINT64_MAX);
		CopyToRam(stagingBuffer, dst);
	}




	MemoryOperationsBuffer::MemoryOperationsBuffer(ObjectManager& _vom)
		: vom(_vom), transferBuffer(_vom.GetDevice(), _vom.GetAllocator(), _vom.GetTransferQueue(), vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_ONLY),
		cmdManager(_vom, _vom.GetTransferQueue(), true, vk::PipelineStageFlagBits::eTransfer)
	{
		vom.SetTransferQueue(_vom.GetTransferQueue());
	}
	MemoryOperationsBuffer::MemoryOperationsBuffer(vk::Device deviceHandle, VmaAllocator allocatorHandle, QueueData transferQueueData)
		: vom(deviceHandle), transferBuffer(deviceHandle, allocatorHandle, transferQueueData, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_CPU_ONLY),
		cmdManager(deviceHandle, transferQueueData, true, vk::PipelineStageFlagBits::eTransfer)
	{
		assert(transferQueueData.queue != NULL);
		vom.SetTransferQueue(transferQueueData);
	}

	void MemoryOperationsBuffer::FindStep(uint64_t transferIndex)
	{
		auto transfer = transferData[transferIndex];
		if (transfer.IsSectorToSector() || transfer.IsSectorToImage())
		{
			for (auto& step : steps)
			{
				bool noDependency = true;
				for (auto& dstSector : step.dstSectors)
				{
					if (dstSector == transfer.AsSectorToSector().srcSector)
					{
						noDependency = false;
						break;
					}
				}
				if (noDependency)
				{
					step.transferIndecies.emplace_back(transfer.index);
					if (transfer.IsSectorToSector())
					{
						step.dstSectors.emplace_back(transfer.AsSectorToSector().dstSector);
					}
					else if (transfer.IsSectorToImage())
					{
						step.dstImages.emplace_back(transfer.AsSectorToImage().dstImage);
					}
					return;
				}
			}

			steps.emplace_back();
			auto& step = steps.back();
			step.transferIndecies.emplace_back(transfer.index);
			if (transfer.IsSectorToSector())
			{
				step.dstSectors.emplace_back(transfer.AsSectorToSector().dstSector);
			}
			else if (transfer.IsSectorToImage())
			{
				step.dstImages.emplace_back(transfer.AsSectorToImage().dstImage);
			}

		}
		else if (transfer.IsImageToSector())
		{
			for (auto& step : steps)
			{
				bool noDependency = true;
				for (auto& dstImage : step.dstImages)
				{
					if (dstImage == transfer.AsImageToSector().srcImage)
					{
						noDependency = false;
						break;
					}
				}
				if (noDependency)
				{
					step.transferIndecies.emplace_back(transfer.index);
					step.dstSectors.emplace_back(transfer.AsImageToSector().dstSector);
					return;
				}
			}

			steps.emplace_back();
			auto& step = steps.back();
			step.transferIndecies.emplace_back(transfer.index);
			step.dstSectors.emplace_back(transfer.AsImageToSector().dstSector);
		}
		else if (transfer.IsImageToImage())
		{
			for (auto& step : steps)
			{
				bool noDependency = true;
				for (auto& dstImage : step.dstImages)
				{
					if (dstImage == transfer.AsImageToImage().srcImage)
					{
						noDependency = false;
						break;
					}
				}
				if (noDependency)
				{
					step.transferIndecies.emplace_back(transfer.index);
					step.dstImages.emplace_back(transfer.AsImageToImage().dstImage);
					return;
				}
			}
			steps.emplace_back();
			auto& step = steps.back();
			step.transferIndecies.emplace_back(transfer.index);
			step.dstImages.emplace_back(transfer.AsImageToImage().dstImage);
		}

	}


	void MemoryOperationsBuffer::RamToSector(void* src, std::shared_ptr<SectorData> dst, uint64_t size)
	{
		assert(src != nullptr);
		assert(dst != nullptr);

		auto stagingBuffer = transferBuffer.GetSector();
		stagingBuffer->neededSize = size;
		transferBuffer.Update(true);
		CopyFromRam(src, stagingBuffer, size);
		if (dst->neededSize < stagingBuffer->neededSize)
		{
			dst->neededSize = stagingBuffer->neededSize;
		}
		FindStep(transferData.EmplaceSectorToSector(stagingBuffer, dst, stagingBuffer->neededSize));
	}
	ToRamTransferExecutor MemoryOperationsBuffer::SectorToRam(std::shared_ptr<SectorData> src, void* dst)
	{
		assert(src != NULL);
		assert(dst != nullptr);
		auto stagingBuffer = transferBuffer.GetSector();
		stagingBuffer->neededSize = src->neededSize;
		transferBuffer.Update(true);
		FindStep(transferData.EmplaceSectorToSector(src, stagingBuffer, src->neededSize));
		return ToRamTransferExecutor(vom.GetDevice(), stagingBuffer, dst, WaitData(cmdManager.GetSubmitCountPtr(), cmdManager.GetMainTimelineSignal().semaphore, vk::PipelineStageFlagBits::eTransfer));
	}
	void MemoryOperationsBuffer::SectorToSector(std::shared_ptr<SectorData> src, std::shared_ptr<SectorData> dst, uint64_t size)
	{
		assert(src != nullptr);
		assert(dst != nullptr);
		if (dst->neededSize < size)
		{
			dst->neededSize = size;
		}
		FindStep(transferData.EmplaceSectorToSector(src, dst, size));
	}
	void MemoryOperationsBuffer::SectorToImage(std::shared_ptr<SectorData> src, vk::Image dst, vk::ImageLayout dstImageLayout, vk::BufferImageCopy copyData, vk::ImageSubresourceRange subresourceRange)
	{
		assert(src != nullptr);
		assert(dst != NULL);
		assert(copyData != vk::BufferImageCopy());
		FindStep(transferData.EmplaceSectorToImage(src, dst, copyData, dstImageLayout, subresourceRange));
	}
	void MemoryOperationsBuffer::ImageToSector(vk::Image src, std::shared_ptr<SectorData> dst, vk::ImageLayout srcImageLayout, vk::BufferImageCopy copyData, vk::ImageSubresourceRange subresourceRange)
	{
		assert(src != NULL);
		assert(dst != nullptr);
		assert(copyData != vk::BufferImageCopy());

		auto reqs = vom.GetDevice().getImageMemoryRequirements(src);
		if (dst->neededSize < reqs.size)
		{
			dst->neededSize = reqs.size;
		}
		FindStep(transferData.EmplaceImageToSector(src, dst, copyData, srcImageLayout, subresourceRange));

	}
	void MemoryOperationsBuffer::ImageToImage(vk::Image src, vk::Image dst, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::ImageCopy copyData, vk::ImageSubresourceRange subresourceRange)
	{
		assert(src != NULL);
		assert(dst != NULL);
		assert(copyData != vk::ImageCopy());
		FindStep(transferData.EmplaceImageToImage(src, dst, copyData, srcImageLayout, dstImageLayout, subresourceRange));


	}
	void MemoryOperationsBuffer::RamToImage(void* src, vk::Image dstImage, vk::BufferImageCopy copyData, vk::ImageLayout dstImageLayout, uint64_t size, vk::ImageSubresourceRange subresourceRange)
	{
		auto stagingBuffer = transferBuffer.GetSector();
		stagingBuffer->neededSize = size;
		transferBuffer.Update(true);
		CopyFromRam(src, stagingBuffer, size);
		FindStep(transferData.EmplaceSectorToImage(stagingBuffer, dstImage, copyData, dstImageLayout, subresourceRange));
	}
	ToRamTransferExecutor MemoryOperationsBuffer::ImageToRam(vk::Image srcImage, void* dst, vk::BufferImageCopy copyData, vk::ImageLayout srcImageLayout, vk::ImageSubresourceRange subresourceRange)
	{
		auto stagingBuffer = transferBuffer.GetSector();
		auto reqs = vom.GetDevice().getImageMemoryRequirements(srcImage);
		stagingBuffer->neededSize = reqs.size;
		transferBuffer.Update(true);
		FindStep(transferData.EmplaceImageToSector(srcImage, stagingBuffer, copyData, srcImageLayout, subresourceRange));
		return ToRamTransferExecutor(vom.GetDevice(), stagingBuffer, dst, WaitData(cmdManager.GetSubmitCountPtr(), cmdManager.GetMainTimelineSignal().semaphore, vk::PipelineStageFlagBits::eTransfer));
	}
	void MemoryOperationsBuffer::DependsOn(WaitData wait)
	{
		cmdManager.DependsOn({ wait });
	}
	void MemoryOperationsBuffer::Clear(bool freeInternalBuffer)
	{
		steps.clear();
		transferData = TransferData();
		cmdManager.Reset();
		transferBuffer.Clear();
		if (freeInternalBuffer)
		{
			transferBuffer.Free();
		}
	};

	void MemoryOperationsBuffer::Execute(std::vector<WaitData> transientWaits, bool wait, bool useNormalSignal, bool useNormalWaits)
	{
		bool record = false;
		for (size_t i = 0; i < transferData.Size(); i++)
		{
			if (transferData[i].NeedsRecording())
			{
				record = true;
				break;
			}
		}

		std::vector<WaitData> submitWaits;

		if (record)
		{
			cmdManager.Reset();
			auto cmd = cmdManager.RecordNew();
			cmd.begin(vk::CommandBufferBeginInfo());
			for (auto& step : steps)
			{
				for (auto transferIndex : step.transferIndecies)
				{
					auto transfer = transferData[transferIndex];
					auto waits = transfer.Record(cmd);
					for (auto& wait : waits)
					{
						bool needsAdd = true;
						for (auto& finalWait : submitWaits)
						{
							if (wait.waitSemaphore == finalWait.waitSemaphore)
							{
								needsAdd = false;
								break;
							}
						}
						if (needsAdd)
						{
							submitWaits.emplace_back(wait);
						}
					}
				}
				vk::MemoryBarrier memoryBarrier(vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eTransferWrite);
				cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
					{},
					1, &memoryBarrier,
					0, {},
					0, {});
			}
			cmd.end();

		}

		submitWaits.insert(submitWaits.end(), transientWaits.begin(), transientWaits.end());
		cmdManager.DependsOn(submitWaits);
		cmdManager.Execute(true, wait, useNormalSignal, useNormalWaits);
		cmdManager.ClearDepends();
	}

	void MemoryOperationsBuffer::WaitOn()
	{
		cmdManager.Wait();
	}
	MemoryOperationsBuffer::~MemoryOperationsBuffer()
	{
		WaitOn();
		transferBuffer.cmdManager.Wait();
		//transferBuffer.Free();
	}
	

}
