#pragma once
namespace vkt
{
	/**
	 * \brief The command Manager is a system that allows the easy management of command pools and external synchronization
	 */
	class CommandManager
	{
	public:
		/**
		 * \brief Constructor overload that extracts the data it needs from an external vom
		 * \param _vom The parent vom, note that the command managers vom will use the same device the this vom uses
		 * \param targetQueue This is the queue that ALL command buffers that this command manager will control will need to be submitted to
		 * \param createInternalCommandPool Set this to true if you would like the command managers internal command buffer cache to create its own 
		 * \param _targetStages 
		 * \param startingSubmitCount 
		 */
		CommandManager(VulkanObjectManager& _vom, QueueData targetQueue, bool createInternalCommandPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0)
		: syncManager(_vom), vom(_vom)
		{
			vom.SetGeneralQueue(targetQueue);
			if (createInternalCommandPool)
			{
				cmdCache = CommandBufferCache(vom, vom.MakeCommandPool(vk::CommandPoolCreateInfo({}, vom.GetGeneralQueue().index)));
			}
			else
			{
				cmdCache = CommandBufferCache(vom);
			}
			submitCount = std::make_shared<uint64_t>(startingSubmitCount);
			syncManager.CreateTimelineSignalSemaphore(submitCount);
			syncManager.CreateSignalSemaphore();
			fence = vom.MakeFence(false);
			targetStages = _targetStages;
		}
		CommandManager(VulkanObjectManager& _vom, QueueData targetQueue, vk::CommandPool externalPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0)
		: syncManager(_vom), vom(_vom)
		{
			vom.SetDevice(_vom.GetDevice());
			vom.SetGeneralQueue(targetQueue);
			cmdCache = CommandBufferCache(vom, externalPool);
			externalCommandPool = externalPool;
			submitCount = std::make_shared<uint64_t>(startingSubmitCount);
			syncManager.CreateTimelineSignalSemaphore(submitCount);
			syncManager.CreateSignalSemaphore();
			fence = vom.MakeFence(false);
			targetStages = _targetStages;
		}
		CommandManager(vk::Device deviceHandle, QueueData targetQueue, bool createInternalCommandPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0)
		: syncManager(deviceHandle), vom(deviceHandle)
		{
			vom.SetGeneralQueue(targetQueue);
			if (createInternalCommandPool)
			{
				cmdCache = CommandBufferCache(vom, vom.MakeCommandPool(vk::CommandPoolCreateInfo({}, vom.GetGeneralQueue().index)));
			}
			else
			{
				cmdCache = CommandBufferCache(vom);
			}
			submitCount = std::make_shared<uint64_t>(startingSubmitCount);
			syncManager.CreateTimelineSignalSemaphore(submitCount);
			syncManager.CreateSignalSemaphore();
			fence = vom.MakeFence(false);
			targetStages = _targetStages;
		}
		CommandManager(vk::Device deviceHandle, QueueData targetQueue, vk::CommandPool externalPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0)
		: syncManager(deviceHandle), vom(deviceHandle)
		{
			vom.SetGeneralQueue(targetQueue);
			cmdCache = CommandBufferCache(vom, externalPool);
			externalCommandPool = externalPool;
			submitCount = std::make_shared<uint64_t>(startingSubmitCount);
			syncManager.CreateTimelineSignalSemaphore(submitCount);
			syncManager.CreateSignalSemaphore();
			fence = vom.MakeFence(false);
			targetStages = _targetStages;
		}

		vk::CommandBuffer RecordNew()
		{
			return cmdCache.NextCommandBuffer();
		}
		void DependsOn(std::vector<WaitData> waits)
		{
			syncManager.AttachWaitData(waits);
		}
		void DependsOn(std::vector<CommandManager*> managers)
		{
			std::vector<WaitData> waits;
			for (auto manager : managers)
			{
				auto signal = manager->GetMainTimelineSignal();
				waits.emplace_back(WaitData(signal.signalValue, signal.semaphore, manager->targetStages));
			}
			DependsOn(waits);
		}
		void ClearDepends()
		{
			syncManager.ClearWaits();
		}
		void AddFreeBuffers(std::vector<vk::CommandBuffer> buffers)
		{
			cmdCache.AddFreeBuffers(buffers);
		}

		vk::SubmitInfo GetSubmitInfo(bool incrementSubmitCount, bool withNormalWaits = true, bool withNormalSignals = true)
		{
			if (incrementSubmitCount)
			{
				(* submitCount)++;
			}
			auto timelineSubmitInfoPtr = syncManager.GetTimelineSubmitInfoPtr(withNormalWaits, withNormalSignals);
			submitInfo = vk::SubmitInfo(timelineSubmitInfoPtr->waitSemaphoreValueCount, syncManager.waitSemaphores.semaphores.data(),
				syncManager.waitSemaphores.waitStages.data(),
				cmdCache.usedCommandBuffers->size(), cmdCache.usedCommandBuffers->data(), timelineSubmitInfoPtr->signalSemaphoreValueCount,
				syncManager.signalSemaphores.semaphores.data());
			submitInfo.pNext = timelineSubmitInfoPtr;

			return submitInfo;
		}
		void Execute(bool incrementSubmitCount, bool wait, bool useNormalSignal, bool withNormalWaits = true)
		{
			if (incrementSubmitCount)
			{
				(*submitCount)++;
			}
			auto timelineSubmitInfoPtr = syncManager.GetTimelineSubmitInfoPtr(withNormalWaits, useNormalSignal);
			submitInfo = vk::SubmitInfo(timelineSubmitInfoPtr->waitSemaphoreValueCount, syncManager.waitSemaphores.semaphores.data(),
				syncManager.waitSemaphores.waitStages.data(),
				cmdCache.usedCommandBuffers->size(), cmdCache.usedCommandBuffers->data(), timelineSubmitInfoPtr->signalSemaphoreValueCount,
				syncManager.signalSemaphores.semaphores.data());
			submitInfo.pNext = timelineSubmitInfoPtr;

			auto res = vom.GetGeneralQueue().queue.submit(1, &submitInfo, (wait) ? fence : VK_NULL_HANDLE);
			if (wait)
			{
				res = vom.GetDevice().waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
				res = vom.GetDevice().resetFences(1, &fence);
			}

		}
		bool IsFinished()
		{
			vk::SemaphoreWaitInfo waitInfo({}, 1, &syncManager.signalSemaphores.semaphores[0], submitCount.get());
			if(vom.GetDevice().waitSemaphores(waitInfo, 0) == vk::Result::eSuccess)
			{
				return true;
			}
			return false;
		}
		void Wait()
		{
			vk::SemaphoreWaitInfo waitInfo({}, 1, &syncManager.signalSemaphores.semaphores[0], submitCount.get());
			auto res = vom.GetDevice().waitSemaphores(waitInfo, UINT64_MAX);
		}
		void Reset()
		{
			if (externalCommandPool == NULL)
			{
				cmdCache.ResetCommandPool();
			}
			else
			{
				cmdCache.ResetBuffers();
			}
		}
		uint64_t GetSubmitCount()
		{
			return *submitCount;
		}
		std::shared_ptr<uint64_t> GetSubmitCountPtr()
		{
			return submitCount;
		}

		void IncrementSubmitCount()
		{
			(*submitCount)++;
		}
		SemaphoreDataEntity GetMainTimelineSignal()
		{
			return syncManager.signalSemaphores[0];
		}
		CommandBufferCache cmdCache;
		SyncManager syncManager;

	private:
		VulkanObjectManager vom;
		vk::CommandPool externalCommandPool;
		std::shared_ptr<uint64_t> submitCount;
		vk::SubmitInfo submitInfo;
		vk::Fence fence;
		vk::PipelineStageFlags targetStages;
	};
}
