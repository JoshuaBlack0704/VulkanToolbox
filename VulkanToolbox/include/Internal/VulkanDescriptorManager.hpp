#pragma once
namespace vkt
{
	//The descriptor manager's update function will allocate sets if needed and then will ask the set managers if they need a rewrite
	//if so the manager will aquire the write data for the set from the set and then perform the rewrite
	//A rewrite is needed if an sector descriptor no longer has a matching version number with its sector's buffer manager
	// meaning the sectors buffer and offset and maybe even size has changed

	struct SectorDescriptorEntity
	{
		SectorDescriptorEntity(int64_t& versionPtr, std::shared_ptr<SectorData>& sectorPtr, vk::DescriptorSetLayoutBinding& bindingPtr,
			vk::WriteDescriptorSet& writePtr, vk::DescriptorBufferInfo& bInfoPtr) : version(versionPtr), sector(sectorPtr), binding(bindingPtr), write(writePtr), bInfo(bInfoPtr){}

		int64_t& version;
		std::shared_ptr<SectorData>& sector;
		vk::DescriptorSetLayoutBinding& binding;
		vk::WriteDescriptorSet& write;
		vk::DescriptorBufferInfo& bInfo;
	};

	//This is a data oriented approach!
	struct SectorDescriptorData
	{
		SectorDescriptorData(uint64_t count)
		{
			version.resize(count);
			sector.resize(count);
			binding.resize(count);
			write.resize(count);
			bInfo.resize(count);
		}
		SectorDescriptorData() = default;

		std::vector<int64_t> version;
		std::vector < std::shared_ptr<SectorData>> sector;
		std::vector < vk::DescriptorSetLayoutBinding> binding;
		std::vector < vk::WriteDescriptorSet> write;
		std::vector < vk::DescriptorBufferInfo> bInfo;

		SectorDescriptorEntity operator[](uint64_t index)
		{
			return SectorDescriptorEntity(version[index], sector[index], binding[index], write[index], bInfo[index]);
		}
		SectorDescriptorEntity EmplaceBack(std::shared_ptr<SectorData> _sector, vk::ShaderStageFlags targetStage)
		{
			vk::DescriptorType type = vk::DescriptorType::eStorageBuffer;
			auto bufferType = _sector->bufferAllocation->bufferCreateInfo.usage;
			if (bufferType & vk::BufferUsageFlagBits::eStorageBuffer)
			{
				type = vk::DescriptorType::eStorageBuffer;
			}
			else if (bufferType & vk::BufferUsageFlagBits::eUniformBuffer)
			{
				type = vk::DescriptorType::eUniformBuffer;
			}

			version.emplace_back(-1);
			sector.emplace_back(_sector);
			binding.emplace_back(vk::DescriptorSetLayoutBinding(binding.size(), type, 1, targetStage));
			bInfo.emplace_back();
			write.emplace_back();
			return SectorDescriptorEntity(version.back(), sector.back(), binding.back(), write.back(), bInfo.back());
		}
		uint32_t size()
		{
			return version.size();
		}
	};

	struct DescriptorSetData
	{
		SectorDescriptorData descData;
		vk::DescriptorSet set = nullptr;
		uint32_t layoutBindingCount = 0;
		vk::DescriptorSetLayout layout;
		bool NeedsRewrite()
		{
			for (size_t i = 0; i < descData.size(); i++)
			{
				if (descData.version[i] != descData.sector[i]->bufferAllocation->cmdManager.GetSubmitCount())
				{
					return true;
				}
			}
			return false;
		}
		bool NeedsAllocation()
		{
			return set == NULL || layoutBindingCount != descData.size();
		}
		void AttachSector(std::shared_ptr<SectorData> sector, vk::ShaderStageFlags targetStage)
		{
			descData.version.resize(descData.version.size(), -1);
			descData.EmplaceBack(sector, targetStage);
		}
		void ProduceTypeCounts(std::vector<vk::DescriptorPoolSize>& typeCounts)
		{
			for (size_t i = 0; i < descData.size(); i++)
			{
				bool foundType = false;
				for (auto& type : typeCounts)
				{
					if (descData.binding[i].descriptorType == type.type)
					{
						type.descriptorCount++;
						foundType = true;
						break;
					}
				}
				if (!foundType)
				{
					typeCounts.emplace_back(vk::DescriptorPoolSize(descData.binding[i].descriptorType, 1));
				}
			}
		}
		vk::DescriptorSetLayoutCreateInfo ProduceSetLayout()
		{
			return vk::DescriptorSetLayoutCreateInfo({}, descData.size(), descData.binding.data());
		}
		std::vector<vk::WriteDescriptorSet>& Write()
		{
			for (size_t i = 0; i < descData.size(); i++)
			{
				auto desc = descData[i];
				desc.bInfo = vk::DescriptorBufferInfo(desc.sector->bufferAllocation->bufferData.buffer, desc.sector->allocationOffset, desc.sector->neededSize);
				desc.write = vk::WriteDescriptorSet(set, desc.binding.binding, 0, 1, desc.binding.descriptorType, {}, &desc.bInfo, {});
				desc.version = desc.sector->bufferAllocation->cmdManager.GetSubmitCount();
			}
			return descData.write;

		}
	};

	class DescriptorManager
	{
	public:
		DescriptorManager(VulkanObjectManager& _vom) : vom(_vom){}
		DescriptorManager(vk::Device device) : vom(device){}

		std::shared_ptr<DescriptorSetData> GetNewSet()
		{
			sets.emplace_back(std::make_shared<DescriptorSetData>());
			return sets.back();
		}

		void Update()
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

	private:
		VulkanObjectManager vom;
		vk::DescriptorPool mainPool;
		std::vector<std::shared_ptr<DescriptorSetData>> sets;

	};
}
