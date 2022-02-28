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
		void Reset();
		void SetSize(uint64_t _neededSize);
	};

	class BufferManager
	{
	public:
		ObjectManager vom;
		CommandManager cmdManager;
		vk::PhysicalDeviceProperties deviceProperties;
		vk::BufferCreateInfo bufferCreateInfo;
		VmaAllocationCreateInfo allocationCreateInfo;
		VmaBuffer bufferData;
		void* map;
		uint64_t alignment;

		std::vector<std::shared_ptr<SectorData>> sectors;

		BufferManager(vk::Device deviceHandle, VmaAllocator allocator, QueueData _transferQueue, vk::BufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsageFlags);
		BufferManager(ObjectManager& _vom, vk::BufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsageFlags);


		std::shared_ptr<SectorData> GetSector();

		void Update(bool wait = false);

		void RemoveSector(std::shared_ptr<SectorData> sector);

		void Clear();

		void Free();

		~BufferManager();
	};

	enum class TransferType
	{
		SECTORTOSECTOR = 2, IMAGETOSECTOR = 3, SECTORTOIMAGE = 4, IMAGETOIMAGE = 5
	};

	struct SectorToSectorEntity
	{
		uint64_t index;
		TransferType type = TransferType::SECTORTOSECTOR;
		std::shared_ptr<SectorData>& srcSector;
		uint64_t& srcVersion;
		std::shared_ptr<SectorData>& dstSector;
		uint64_t& dstVersion;
		uint64_t& size;


		SectorToSectorEntity(
			uint64_t _index,
			std::shared_ptr<SectorData>& _srcSector,
			uint64_t& _srcVersion,
			std::shared_ptr<SectorData>& _dstSector,
			uint64_t& _dstVersion,
			uint64_t& _size);

		std::vector<WaitData> Record(vk::CommandBuffer cmd);
		bool NeedsRecording();
	};
	struct ImageToSectorEntity
	{
		uint64_t index;
		TransferType type = TransferType::IMAGETOSECTOR;
		vk::Image& srcImage;
		std::shared_ptr<SectorData>& dstSector;
		uint64_t& dstVersion;
		vk::BufferImageCopy& bufferImageCopy;
		vk::ImageLayout& srcImageFormat;
		vk::ImageSubresourceRange& subresourceRange;

		ImageToSectorEntity(
			uint64_t _index,
			vk::Image& _srcImage,
			std::shared_ptr<SectorData>& _dstSector,
			uint64_t& _dstVersion,
			vk::BufferImageCopy& _copyData,
			vk::ImageLayout& _srcImageFormat,
			vk::ImageSubresourceRange& _subresourceRange
		);

		std::vector<WaitData> Record(vk::CommandBuffer cmd);

		bool NeedsRecording();

	};
	struct SectorToImageEntity
	{
		uint64_t index;
		TransferType type = TransferType::SECTORTOIMAGE;
		vk::Image& dstImage;
		std::shared_ptr<SectorData>& srcSector;
		uint64_t& srcVersion;
		vk::BufferImageCopy& bufferImageCopy;
		vk::ImageLayout& dstImageFormat;
		vk::ImageSubresourceRange& subresourceRange;


		SectorToImageEntity(
			uint64_t _index,
			vk::Image& _dstImage,
			std::shared_ptr<SectorData>& _srcSector,
			uint64_t& _srcVersion,
			vk::BufferImageCopy& _copyData,
			vk::ImageLayout& _dstImageFormat,
			vk::ImageSubresourceRange& _subresourceRange
		);

		std::vector<WaitData> Record(vk::CommandBuffer cmd);

		bool NeedsRecording();
	};
	struct ImageToImageEntity
	{
		uint64_t index;
		TransferType type = TransferType::IMAGETOIMAGE;
		vk::Image& srcImage;
		vk::Image& dstImage;
		vk::ImageLayout& srcImageLayout;
		vk::ImageLayout& dstImageLayout;
		vk::ImageCopy& imageCopy;
		vk::ImageSubresourceRange& subresourceRange;

		ImageToImageEntity(
			uint64_t _index,
			vk::Image& _srcImage,
			vk::Image& _dstImage,
			vk::ImageLayout& _srcImageLayout,
			vk::ImageLayout& _dstImageLayout,
			vk::ImageCopy& _copyData,
			vk::ImageSubresourceRange& _subresourceRange);

		std::vector<WaitData> Record(vk::CommandBuffer cmd);

		bool NeedsRecording();
	};
	struct TransferEntity
	{
		uint64_t index;
		TransferType& type;
		uint64_t& size;
		std::shared_ptr<SectorData>& srcSector;
		uint64_t& srcVersion;
		std::shared_ptr<SectorData>& dstSector;
		uint64_t& dstVersion;
		vk::Image& srcImage;
		vk::Image& dstImage;
		vk::BufferImageCopy& bufferImageCopy;
		vk::ImageCopy& imageCopy;
		vk::ImageLayout& srcImageLayout;
		vk::ImageLayout& dstImageLayout;
		vk::ImageSubresourceRange& subresourceRange;

		TransferEntity(uint64_t _index,
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
		);

		SectorToSectorEntity AsSectorToSector();
		ImageToSectorEntity AsImageToSector();
		SectorToImageEntity AsSectorToImage();
		ImageToImageEntity AsImageToImage();

		bool IsSectorToSector();
		bool IsImageToSector();
		bool IsSectorToImage();
		bool IsImageToImage();

		bool NeedsRecording();

		std::vector<WaitData> Record(vk::CommandBuffer cmd);

	};

	class TransferData
	{
	public:
		std::vector<TransferType> types;
		std::vector<uint64_t> sizes;
		std::vector<uint64_t> srcVersion;
		std::vector<uint64_t> dstVersion;
		std::vector<std::shared_ptr<SectorData>> srcSectors;
		std::vector<std::shared_ptr<SectorData>> dstSectors;
		std::vector<vk::Image> srcImages;
		std::vector<vk::Image> dstImages;
		std::vector<vk::BufferImageCopy> bufferImageCopies;
		std::vector<vk::ImageCopy> imageCopies;
		std::vector<vk::ImageLayout> srcImageLayouts;
		std::vector<vk::ImageLayout> dstImageLayouts;
		std::vector<vk::ImageSubresourceRange> subresourceRanges;

		bool IsSectorToSector(uint64_t index);
		bool IsImageToSector(uint64_t index);
		bool IsSectorToImage(uint64_t index);
		bool IsImageToImage(uint64_t index);

		TransferEntity operator[](uint64_t index);
		uint64_t EmplaceBack(
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
		);
		
		uint64_t EmplaceSectorToSector(std::shared_ptr<SectorData> srcSector, std::shared_ptr<SectorData> dstSector, uint64_t size);
		uint64_t EmplaceImageToSector(vk::Image srcImage, std::shared_ptr<SectorData> dstSector, vk::BufferImageCopy imageToBufferCopy, vk::ImageLayout srcImageLayout, vk::ImageSubresourceRange subresourceRange);
		uint64_t EmplaceSectorToImage(std::shared_ptr<SectorData> srcSector, vk::Image dstImage, vk::BufferImageCopy bufferToImageCopy, vk::ImageLayout dstImageLayout, vk::ImageSubresourceRange subresourceRange);
		uint64_t EmplaceImageToImage(vk::Image srcImage, vk::Image dstImage, vk::ImageCopy imageCopy, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::ImageSubresourceRange subresourceRange);

		uint64_t Size();

	};

	inline void CopyFromRam(void* src, std::shared_ptr<SectorData> dstSector, uint64_t size)
	{
		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(dstSector->bufferAllocation->vom.GetAllocator(), dstSector->bufferAllocation->bufferData.allocation, &flags);
		assert((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) || (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

		if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		{
			vmaMapMemory(dstSector->bufferAllocation->vom.GetAllocator(), dstSector->bufferAllocation->bufferData.allocation, &dstSector->bufferAllocation->map);
			memcpy(reinterpret_cast<char*>(dstSector->bufferAllocation->map) + dstSector->allocationOffset, src, size);
			vmaUnmapMemory(dstSector->bufferAllocation->vom.GetAllocator(), dstSector->bufferAllocation->bufferData.allocation);
		}
		else if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			vmaInvalidateAllocation(dstSector->bufferAllocation->vom.GetAllocator(), dstSector->bufferAllocation->bufferData.allocation, dstSector->allocationOffset, dstSector->allocatedSize);
			vmaMapMemory(dstSector->bufferAllocation->vom.GetAllocator(), dstSector->bufferAllocation->bufferData.allocation, &dstSector->bufferAllocation->map);
			memcpy(reinterpret_cast<char*>(dstSector->bufferAllocation->map) + dstSector->allocationOffset, src, size);
			vmaUnmapMemory(dstSector->bufferAllocation->vom.GetAllocator(), dstSector->bufferAllocation->bufferData.allocation);
			vmaFlushAllocation(dstSector->bufferAllocation->vom.GetAllocator(), dstSector->bufferAllocation->bufferData.allocation, dstSector->allocationOffset, dstSector->allocatedSize);
		}
	}
	inline void CopyToRam(std::shared_ptr<SectorData> srcSector, void* dst)
	{


		VkMemoryPropertyFlags flags;
		vmaGetAllocationMemoryProperties(srcSector->bufferAllocation->vom.GetAllocator(), srcSector->bufferAllocation->bufferData.allocation, &flags);
		assert(!(flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) || (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

		if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		{
			vmaMapMemory(srcSector->bufferAllocation->vom.GetAllocator(), srcSector->bufferAllocation->bufferData.allocation, &srcSector->bufferAllocation->map);
			memcpy(const_cast<void*>(dst), reinterpret_cast<char*>(srcSector->bufferAllocation->map) + srcSector->allocationOffset, srcSector->neededSize);
			vmaUnmapMemory(srcSector->bufferAllocation->vom.GetAllocator(), srcSector->bufferAllocation->bufferData.allocation);
		}
		else if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			vmaInvalidateAllocation(srcSector->bufferAllocation->vom.GetAllocator(), srcSector->bufferAllocation->bufferData.allocation, srcSector->allocationOffset, srcSector->allocatedSize);
			vmaMapMemory(srcSector->bufferAllocation->vom.GetAllocator(), srcSector->bufferAllocation->bufferData.allocation, &srcSector->bufferAllocation->map);
			memcpy(const_cast<void*>(dst), reinterpret_cast<char*>(srcSector->bufferAllocation->map) + srcSector->allocationOffset, srcSector->neededSize);
			vmaUnmapMemory(srcSector->bufferAllocation->vom.GetAllocator(), srcSector->bufferAllocation->bufferData.allocation);
			vmaFlushAllocation(srcSector->bufferAllocation->vom.GetAllocator(), srcSector->bufferAllocation->bufferData.allocation, srcSector->allocationOffset, srcSector->allocatedSize);
		}
	}

	class ToRamTransferExecutor
	{
	public:
		ToRamTransferExecutor(vk::Device deviceHandle, std::shared_ptr<SectorData> _stagingBuffer, void* _dst, WaitData _wait);
		void Execute();
	private:
		vk::Device device;
		std::shared_ptr<SectorData> stagingBuffer;
		void* dst;
		WaitData waitData;
	};

	struct TransferStep
	{
		std::vector<std::shared_ptr<SectorData>> dstSectors;
		std::vector<vk::Image> dstImages;
		std::vector<uint64_t> transferIndecies;
	};

	class MemoryOperationsBuffer
	{
	public:
		ObjectManager vom;
		CommandManager cmdManager;
		std::vector<TransferStep> steps;
		bool record = false;
		BufferManager transferBuffer;
		TransferData transferData;

		MemoryOperationsBuffer(ObjectManager& _vom);
		MemoryOperationsBuffer(vk::Device deviceHandle, VmaAllocator allocatorHandle, QueueData transferQueueData);

		void FindStep(uint64_t transferIndex);
		

		void RamToSector(void* src, std::shared_ptr<SectorData> dst, uint64_t size);
		ToRamTransferExecutor SectorToRam(std::shared_ptr<SectorData> src, void* dst);
		void SectorToSector(std::shared_ptr<SectorData> src, std::shared_ptr<SectorData> dst, uint64_t size);
		void SectorToImage(std::shared_ptr<SectorData> src, vk::Image dst, vk::ImageLayout dstImageLayout, vk::BufferImageCopy copyData, vk::ImageSubresourceRange subresourceRange);
		void ImageToSector(vk::Image src, std::shared_ptr<SectorData> dst, vk::ImageLayout srcImageLayout, vk::BufferImageCopy copyData, vk::ImageSubresourceRange subresourceRange);
		void ImageToImage(vk::Image src, vk::Image dst, vk::ImageLayout srcImageLayout, vk::ImageLayout dstImageLayout, vk::ImageCopy copyData, vk::ImageSubresourceRange subresourceRange);
		void RamToImage(void* src, vk::Image dstImage, vk::BufferImageCopy copyData, vk::ImageLayout dstImageLayout, uint64_t size, vk::ImageSubresourceRange subresourceRange);
		ToRamTransferExecutor ImageToRam(vk::Image srcImage, void* dst, vk::BufferImageCopy copyData, vk::ImageLayout srcImageLayout, vk::ImageSubresourceRange subresourceRange);
		void DependsOn(WaitData wait);
		void Clear(bool freeInternalBuffer = true);

		void Execute(std::vector<WaitData> transientWaits = {}, bool wait = false, bool useNormalSignal = false, bool useNormalWaits = false);

		void WaitOn();
		~MemoryOperationsBuffer();
	};

}
