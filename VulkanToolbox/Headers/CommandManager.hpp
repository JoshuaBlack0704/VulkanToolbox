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
		CommandManager(ObjectManager& _vom, QueueData targetQueue, bool createInternalCommandPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0);
		CommandManager(ObjectManager& _vom, QueueData targetQueue, vk::CommandPool externalPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0);
		CommandManager(vk::Device deviceHandle, QueueData targetQueue, bool createInternalCommandPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0);
		CommandManager(vk::Device deviceHandle, QueueData targetQueue, vk::CommandPool externalPool, vk::PipelineStageFlags _targetStages, uint64_t startingSubmitCount = 0);

		vk::CommandBuffer RecordNew();
		void DependsOn(std::vector<WaitData> waits);
		void DependsOn(std::vector<CommandManager*> managers);
		void ClearDepends();
		void AddFreeBuffers(std::vector<vk::CommandBuffer> buffers);

		vk::SubmitInfo GetSubmitInfo(bool incrementSubmitCount, bool withNormalWaits = true, bool withNormalSignals = true);
		void Execute(bool incrementSubmitCount, bool wait, bool useNormalSignal, bool withNormalWaits = true);
		bool IsFinished();
		void Wait();
		void Reset();
		uint64_t GetSubmitCount();
		std::shared_ptr<uint64_t> GetSubmitCountPtr();

		void IncrementSubmitCount();
		SemaphoreDataEntity GetMainTimelineSignal();
		SemaphoreDataEntity GetMainSignal();
		CommandBufferCache cmdCache;
		SyncManager syncManager;

	private:
		ObjectManager vom;
		vk::CommandPool externalCommandPool;
		std::shared_ptr<uint64_t> submitCount;
		vk::SubmitInfo submitInfo;
		vk::Fence fence;
		vk::PipelineStageFlags targetStages;
	};
}
