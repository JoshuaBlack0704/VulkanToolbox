#include "../Headers/VulkanToolbox.hpp"

namespace vkt
{
	CommandManager::CommandManager(ObjectManager& _vom, QueueData targetQueue, bool createInternalCommandPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount)
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
	CommandManager::CommandManager(ObjectManager& _vom, QueueData targetQueue, vk::CommandPool externalPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount)
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
	CommandManager::CommandManager(vk::Device deviceHandle, QueueData targetQueue, bool createInternalCommandPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount)
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
	CommandManager::CommandManager(vk::Device deviceHandle, QueueData targetQueue, vk::CommandPool externalPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount)
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

	vk::CommandBuffer CommandManager::RecordNew()
	{
		return cmdCache.NextCommandBuffer();
	}
	void CommandManager::DependsOn(std::vector<WaitData> waits)
	{
		syncManager.AttachWaitData(waits);
	}
	void CommandManager::DependsOn(std::vector<CommandManager*> managers)
	{
		std::vector<WaitData> waits;
		for (auto manager : managers)
		{
			auto signal = manager->GetMainTimelineSignal();
			waits.emplace_back(WaitData(signal.signalValue, signal.semaphore, manager->targetStages));
		}
		DependsOn(waits);
	}
	void CommandManager::ClearDepends()
	{
		syncManager.ClearWaits();
	}
	void CommandManager::AddFreeBuffers(std::vector<vk::CommandBuffer> buffers)
	{
		cmdCache.AddFreeBuffers(buffers);
	}

	vk::SubmitInfo CommandManager::GetSubmitInfo(bool incrementSubmitCount, bool withNormalWaits, bool withNormalSignals)
	{
		if (incrementSubmitCount)
		{
			(*submitCount)++;
		}
		auto timelineSubmitInfoPtr = syncManager.GetTimelineSubmitInfoPtr(withNormalWaits, withNormalSignals);
		submitInfo = vk::SubmitInfo(timelineSubmitInfoPtr->waitSemaphoreValueCount, syncManager.waitSemaphores.semaphores.data(),
			syncManager.waitSemaphores.waitStages.data(),
			cmdCache.usedCommandBuffers->size(), cmdCache.usedCommandBuffers->data(), timelineSubmitInfoPtr->signalSemaphoreValueCount,
			syncManager.signalSemaphores.semaphores.data());
		submitInfo.pNext = timelineSubmitInfoPtr;

		return submitInfo;
	}
	void CommandManager::Execute(bool incrementSubmitCount, bool wait, bool useNormalSignal, bool withNormalWaits)
	{
		if (incrementSubmitCount)
		{
			(*submitCount)++;
		}
		auto timelineSubmitInfoPtr = syncManager.GetTimelineSubmitInfoPtr(withNormalWaits, useNormalSignal);
		submitInfo = vk::SubmitInfo(syncManager.waitSemaphores.size(), syncManager.waitSemaphores.semaphores.data(),
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
	bool CommandManager::IsFinished()
	{
		vk::SemaphoreWaitInfo waitInfo({}, 1, &syncManager.signalSemaphores.semaphores[0], submitCount.get());
		if (vom.GetDevice().waitSemaphores(waitInfo, 0) == vk::Result::eSuccess)
		{
			return true;
		}
		return false;
	}
	void CommandManager::Wait()
	{
		vk::SemaphoreWaitInfo waitInfo({}, 1, &syncManager.signalSemaphores.semaphores[0], submitCount.get());
		auto res = vom.GetDevice().waitSemaphores(waitInfo, UINT64_MAX);
	}
	void CommandManager::Reset()
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
	uint64_t CommandManager::GetSubmitCount()
	{
		return *submitCount;
	}
	std::shared_ptr<uint64_t> CommandManager::GetSubmitCountPtr()
	{
		return submitCount;
	}

	void CommandManager::IncrementSubmitCount()
	{
		(*submitCount)++;
	}
	SemaphoreDataEntity CommandManager::GetMainTimelineSignal()
	{
		return syncManager.signalSemaphores[0];
	}
	SemaphoreDataEntity CommandManager::GetMainSignal()
	{
		return syncManager.signalSemaphores[1];
	}
}
