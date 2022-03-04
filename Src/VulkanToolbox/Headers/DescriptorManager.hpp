#pragma once
namespace vkt
{
	//The descriptor manager's update function will allocate sets if needed and then will ask the set managers if they need a rewrite
	//if so the manager will aquire the write data for the set from the set and then perform the rewrite
	//A rewrite is needed if an sector descriptor no longer has a matching versions number with its sector's buffer manager
	// meaning the sectors buffer and offset and maybe even size has changed

	struct BufferDescriptorEntity
	{
		const uint64_t index;
		vk::DescriptorType& type;
		std::shared_ptr<uint64_t>& srcVersion;
		uint64_t& version;
		vk::ShaderStageFlags& targetStage;
		vk::DescriptorBufferInfo& bInfo;

		BufferDescriptorEntity(
			const uint64_t index,
			vk::DescriptorType& type,
			std::shared_ptr<uint64_t>& srcVersion,
			uint64_t& version,
			vk::ShaderStageFlags& targetStage,
			vk::DescriptorBufferInfo& bInfo
		);
	};

	struct ImageDescriptorEntity
	{
		uint64_t const index;
		vk::DescriptorType& type;
		std::shared_ptr<uint64_t>& srcVersion;
		uint64_t& version;
		vk::ShaderStageFlags& targetStage;
		vk::DescriptorImageInfo& iInfo;

		ImageDescriptorEntity(
			const uint64_t index,
			vk::DescriptorType& type,
			std::shared_ptr<uint64_t>& srcVersion,
			uint64_t& version,
			vk::ShaderStageFlags& targetStage,
			vk::DescriptorImageInfo& iInfo
		);
	};

	struct DescriptorEntity
	{
		uint64_t const index;
		vk::DescriptorType type;
		std::shared_ptr<uint64_t> srcVersion;
		uint64_t version;
		vk::ShaderStageFlags targetStage;
		vk::Buffer* buffer;
		uint64_t* allocationOffset;
		uint64_t* range;
		vk::ImageLayout* imageLayout;
		vk::ImageView* view;
		vk::Sampler* sampler;
		vk::DescriptorBufferInfo& bInfo;
		vk::DescriptorImageInfo& iInfo;

		DescriptorEntity(
			uint64_t const index,
			vk::DescriptorType type,
			std::shared_ptr<uint64_t> srcVersion,
			uint64_t version,
			vk::ShaderStageFlags targetStage,
			vk::Buffer* buffer,
			uint64_t* allocationOffset,
			uint64_t* range,
			vk::ImageLayout* imageLayout,
			vk::ImageView* view,
			vk::Sampler* sampler,
			vk::DescriptorBufferInfo& bInfo,
			vk::DescriptorImageInfo& iInfo
		);

		BufferDescriptorEntity AsBufferDescriptor();
		ImageDescriptorEntity AsImageDescriptor();
		bool IsBufferDescriptor();
		bool IsImageDescriptor();
	};

	//This is a data oriented approach!
	struct SectorDescriptorData
	{
		std::vector <vk::DescriptorType> types;
		std::vector <std::shared_ptr<uint64_t>> srcVersions;
		std::vector <uint64_t> versions;
		std::vector<vk::ShaderStageFlags> targetStages;
		std::vector<vk::Buffer*> buffers;
		std::vector<uint64_t*> allocationOffsets;
		std::vector<uint64_t*> ranges;
		std::vector<vk::ImageLayout*> imageLayouts;
		std::vector<vk::ImageView*> views;
		std::vector<vk::Sampler*> samplers;
		std::vector<vk::DescriptorBufferInfo> bInfos;
		std::vector<vk::DescriptorImageInfo> iInfos;


		DescriptorEntity operator[](uint64_t index);
		DescriptorEntity EmplaceBack(vk::DescriptorType type, std::shared_ptr<uint64_t> srcVersion, vk::ShaderStageFlags targetStage, vk::Buffer* buffer, uint64_t* allocationOffset, uint64_t* range, vk::ImageLayout* layout, vk::ImageView* view, vk::Sampler* sampler);
		uint32_t size();
	};

	struct DescriptorSetData
	{
		SectorDescriptorData descData;
		vk::DescriptorSet set = nullptr;
		uint32_t layoutBindingCount = 0;
		vk::DescriptorSetLayout layout;
		std::vector < vk::DescriptorSetLayoutBinding > bindings;
		std::vector<vk::WriteDescriptorSet> writes;
		bool NeedsRewrite();
		bool NeedsAllocation();
		void AddDescriptor(vk::DescriptorType type, std::shared_ptr<uint64_t> srcVersion, vk::ShaderStageFlags targetStage, vk::Buffer* buffer, uint64_t* offset, uint64_t* range);
		void AddDescriptor(vk::DescriptorType type, std::shared_ptr<uint64_t> srcVersion, vk::ShaderStageFlags targetStage, vk::ImageLayout* layout, vk::ImageView* view, vk::Sampler* sampler);
		void AttachSector(std::shared_ptr<SectorData> sector, vk::ShaderStageFlags targetStage);
		void ProduceTypeCounts(std::vector<vk::DescriptorPoolSize>& typeCounts);
		vk::DescriptorSetLayoutCreateInfo ProduceSetLayout();
		std::vector<vk::WriteDescriptorSet>& Write();
	};

	class DescriptorManager
	{
	public:
		DescriptorManager(ObjectManager& _vom);
		DescriptorManager(vk::Device device);

		std::shared_ptr<DescriptorSetData> GetNewSet();

		void Update();

	private:
		ObjectManager vom;
		vk::DescriptorPool mainPool;
		std::vector<std::shared_ptr<DescriptorSetData>> sets;

	};
}
