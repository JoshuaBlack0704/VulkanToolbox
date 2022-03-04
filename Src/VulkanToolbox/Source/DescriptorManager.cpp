#include "../Headers/VulkanToolbox.hpp"

namespace vkt
{
	//The descriptor manager's update function will allocate sets if needed and then will ask the set managers if they need a rewrite
	//if so the manager will aquire the write data for the set from the set and then perform the rewrite
	//A rewrite is needed if an sector descriptor no longer has a matching versions number with its sector's buffer manager
	// meaning the sectors buffer and offset and maybe even size has changed

	BufferDescriptorEntity::BufferDescriptorEntity(
		const uint64_t index,
		vk::DescriptorType& type,
		std::shared_ptr<uint64_t>& srcVersion,
		uint64_t& version,
		vk::ShaderStageFlags& targetStage,
		vk::DescriptorBufferInfo& bInfo
	)
		: index(index), type(type), srcVersion(srcVersion), version(version), targetStage(targetStage), bInfo(bInfo)
	{
	};

	ImageDescriptorEntity::ImageDescriptorEntity(
		const uint64_t index,
		vk::DescriptorType& type,
		std::shared_ptr<uint64_t>& srcVersion,
		uint64_t& version,
		vk::ShaderStageFlags& targetStage,
		vk::DescriptorImageInfo& iInfo
	)
		: index(index), type(type), srcVersion(srcVersion), version(version), targetStage(targetStage), iInfo(iInfo)
	{
	};



	DescriptorEntity::DescriptorEntity(
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
	)
		:
		index(index),
		type(type),
		srcVersion(srcVersion),
		version(version),
		targetStage(targetStage),
		buffer(buffer),
		allocationOffset(allocationOffset),
		range(range),
		imageLayout(imageLayout),
		view(view),
		sampler(sampler),
		bInfo(bInfo),
		iInfo(iInfo)
	{
	};

	BufferDescriptorEntity DescriptorEntity::AsBufferDescriptor()
	{
		assert(buffer != nullptr);
		assert(allocationOffset != nullptr);
		assert(range != nullptr);
		bInfo = vk::DescriptorBufferInfo(*buffer, *allocationOffset, *range);
		return BufferDescriptorEntity(index, type, srcVersion, version, targetStage, bInfo);
	}
	ImageDescriptorEntity DescriptorEntity::AsImageDescriptor()
	{
		assert(view != nullptr);
		assert(sampler != nullptr);
		assert(imageLayout != nullptr);
		iInfo = vk::DescriptorImageInfo(*sampler, *view, *imageLayout);
		return ImageDescriptorEntity(index, type, srcVersion, version, targetStage, iInfo);
	}
	bool DescriptorEntity::IsBufferDescriptor()
	{
		return buffer != nullptr && allocationOffset != nullptr && range != nullptr;
	}
	bool DescriptorEntity::IsImageDescriptor()
	{
		return imageLayout != nullptr && view != nullptr && sampler != nullptr;
	}

	DescriptorEntity SectorDescriptorData::operator[](uint64_t index)
	{
		return DescriptorEntity(index, types[index], srcVersions[index], versions[index], targetStages[index], buffers[index], allocationOffsets[index], ranges[index], imageLayouts[index], views[index], samplers[index], bInfos[index], iInfos[index]);
	}
	DescriptorEntity SectorDescriptorData::EmplaceBack(vk::DescriptorType type, std::shared_ptr<uint64_t> srcVersion, vk::ShaderStageFlags targetStage, vk::Buffer* buffer, uint64_t* allocationOffset, uint64_t* range, vk::ImageLayout* layout, vk::ImageView* view, vk::Sampler* sampler)
	{
		types.emplace_back(type);
		srcVersions.emplace_back(srcVersion);
		versions.emplace_back(-1);
		targetStages.emplace_back(targetStage);
		buffers.emplace_back(buffer);
		allocationOffsets.emplace_back(allocationOffset);
		ranges.emplace_back(range);
		imageLayouts.emplace_back(layout);
		views.emplace_back(view);
		samplers.emplace_back(sampler);
		bInfos.emplace_back();
		iInfos.emplace_back();
		return DescriptorEntity(types.size(), types.back(), srcVersions.back(), versions.back(), targetStages.back(), buffers.back(), allocationOffsets.back(), ranges.back(), imageLayouts.back(), views.back(), samplers.back(), bInfos.back(), iInfos.back());
	}
	uint32_t SectorDescriptorData::size()
	{
		return versions.size();
	}

	bool DescriptorSetData::NeedsRewrite()
	{
		for (size_t i = 0; i < descData.size(); i++)
		{
			if (descData.versions[i] != *descData.srcVersions[i])
			{
				return true;
			}
		}
		return false;
	}
	bool DescriptorSetData::NeedsAllocation()
	{
		return set == NULL || layoutBindingCount != descData.size();
	}
	void DescriptorSetData::AddDescriptor(vk::DescriptorType type, std::shared_ptr<uint64_t> srcVersion, vk::ShaderStageFlags targetStage, vk::Buffer* buffer, uint64_t* offset, uint64_t* range)
	{
		descData.EmplaceBack(type, srcVersion, targetStage, buffer, offset, range, nullptr, nullptr, nullptr);
	}
	void DescriptorSetData::AddDescriptor(vk::DescriptorType type, std::shared_ptr<uint64_t> srcVersion, vk::ShaderStageFlags targetStage, vk::ImageLayout* layout, vk::ImageView* view, vk::Sampler* sampler)
	{
		descData.EmplaceBack(type, srcVersion, targetStage, nullptr, nullptr, nullptr, layout, view, sampler);
	}
	void DescriptorSetData::AttachSector(std::shared_ptr<SectorData> sector, vk::ShaderStageFlags targetStage)
	{
		auto bufferType = sector->bufferAllocation->bufferCreateInfo.usage;
		vk::DescriptorType descType = vk::DescriptorType::eStorageBuffer;
		assert(bufferType & vk::BufferUsageFlagBits::eStorageBuffer || bufferType & vk::BufferUsageFlagBits::eUniformBuffer);
		if (bufferType & vk::BufferUsageFlagBits::eStorageBuffer)
		{
			descType = vk::DescriptorType::eStorageBuffer;
		}
		else if (bufferType & vk::BufferUsageFlagBits::eUniformBuffer)
		{
			descType = vk::DescriptorType::eUniformBuffer;
		}



		AddDescriptor(descType, sector->bufferAllocation->cmdManager.GetSubmitCountPtr(), targetStage, &sector->bufferAllocation->bufferData.buffer, &sector->allocationOffset, &sector->neededSize);
	}
	void DescriptorSetData::ProduceTypeCounts(std::vector<vk::DescriptorPoolSize>& typeCounts)
	{
		for (size_t i = 0; i < descData.size(); i++)
		{
			bool foundType = false;
			for (auto& type : typeCounts)
			{
				if (descData.types[i] == type.type)
				{
					type.descriptorCount++;
					foundType = true;
					break;
				}
			}
			if (!foundType)
			{
				typeCounts.emplace_back(vk::DescriptorPoolSize(descData.types[i], 1));
			}
		}
	}
	vk::DescriptorSetLayoutCreateInfo DescriptorSetData::ProduceSetLayout()
	{
		if (bindings.size() != descData.size())
		{
			bindings.resize(descData.size());
			bindings.clear();
			for (size_t i = 0; i < descData.size(); i++)
			{
				auto desc = descData[i];
				bindings.emplace_back(vk::DescriptorSetLayoutBinding(desc.index, desc.type, 1, desc.targetStage));
			}
		}
		return vk::DescriptorSetLayoutCreateInfo({}, bindings.size(), bindings.data());
	}
	std::vector<vk::WriteDescriptorSet>& DescriptorSetData::Write()
	{
		writes.resize(descData.size());
		writes.clear();
		for (size_t i = 0; i < descData.size(); i++)
		{
			auto desc = descData[i];
			desc.version = *desc.srcVersion;
			if (desc.IsBufferDescriptor())
			{
				auto buffDesc = desc.AsBufferDescriptor();
				writes.emplace_back(vk::WriteDescriptorSet(set, buffDesc.index, 0, 1, buffDesc.type, {}, &buffDesc.bInfo, {}));
			}
			else if (desc.IsImageDescriptor())
			{
				auto imgDesc = desc.AsImageDescriptor();
				writes.emplace_back(vk::WriteDescriptorSet(set, imgDesc.index, 0, 1, imgDesc.type, &imgDesc.iInfo, {}, {}));
			}
		}
		return writes;

	}
	DescriptorManager::DescriptorManager(ObjectManager& _vom) : vom(_vom) {}
	DescriptorManager::DescriptorManager(vk::Device device) : vom(device) {}
	std::shared_ptr<DescriptorSetData> DescriptorManager::GetNewSet()
	{
		sets.emplace_back(std::make_shared<DescriptorSetData>());
		return sets.back();
	}
	void DescriptorManager::Update()
	{
		//Allocation Pass
		for (auto setData : sets)
		{
			if (setData->NeedsAllocation())
			{
				vom.DestroyType(vk::DescriptorPool());
				vom.DestroyType(vk::DescriptorSetLayout());
				std::vector<vk::DescriptorPoolSize> typeCounts;
				std::vector<vk::DescriptorSetLayout> layouts;
				for (auto& set : sets)
				{
					set->layout = vom.MakeDescriptorSetLayout(set->ProduceSetLayout());
					layouts.emplace_back(set->layout);
					set->ProduceTypeCounts(typeCounts);
					set->layoutBindingCount = set->descData.size();
				}
				mainPool = vom.MakeDescriptorPool(vk::DescriptorPoolCreateInfo({}, sets.size(), typeCounts.size(), typeCounts.data()));
				auto newSets = vom.MakeDescriptorSets(vk::DescriptorSetAllocateInfo(mainPool, sets.size(), layouts.data()));
				for (size_t i = 0; i < sets.size(); i++)
				{
					sets[i]->set = newSets[i];
				}
				break;
			}
		}


		//Write Pass
		for (auto setData : sets)
		{
			if (setData->NeedsRewrite())
			{
				auto& write = setData->Write();
				vom.GetDevice().updateDescriptorSets(write.size(), write.data(), 0, {});

			}
		}
	}
}
