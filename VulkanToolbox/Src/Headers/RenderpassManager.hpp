#pragma once
namespace vkt
{
	struct AttachmentData
	{
		vk::AttachmentDescription desc;
		vk::AttachmentReference ref;
		vk::ClearValue clearValue;
		vk::ImageView view;
	};

	struct SubpassData
	{
		uint32_t index;
		vk::PipelineBindPoint bindPoint;
		std::vector<vk::AttachmentReference> inputTargets;
		std::vector<vk::AttachmentReference> colorTargets;
		vk::AttachmentReference depthTarget;
		std::vector<vk::SubpassDependency> deps;

		void AttachInputs(std::vector<AttachmentData*> attachments);
		void AttachColors(std::vector<AttachmentData*> attachments);
		void AttachDepth(AttachmentData* attachment);
		void SetDependency(vk::SubpassDependency _deps);
	};

	struct FrameData
	{
		std::vector<vk::ImageView> views;
		vk::RenderPass renderPass;
		vk::Framebuffer frameBuffer;
		uint32_t width, height;

	};

	class RenderPassManager
	{
	public:
		vk::RenderPass renderPass;
		std::vector<SubpassData*> subpassDatas;
		std::vector<AttachmentData*> attachments;
		std::vector<vk::ClearValue> clearValues;
		std::vector<FrameData> frames;

		uint32_t width, height;

		AttachmentData* CreateAttachment(vk::AttachmentDescription desc, vk::ImageLayout interimLayout, vk::ClearValue clearValue, vk::ImageView _view);
		SubpassData* CreateSubpass(vk::PipelineBindPoint bindPoint, std::vector<AttachmentData*> colorAttachments, std::vector<AttachmentData*> inputAttachments, AttachmentData* _depthTarget, std::vector<vk::SubpassDependency> deps);
		vk::RenderPass CreateRenderPass();
		FrameData CreateFramebuffer();

		RenderPassManager(vk::Device deviceHandle, uint32_t _width, uint32_t _height);
		RenderPassManager(ObjectManager& _vom, uint32_t _width, uint32_t _height);
		RenderPassManager(vk::Device deviceHandle);

		void Dispose();
	private:
		ObjectManager vom;
	};
}

