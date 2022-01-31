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

		void AttachInputs(std::vector<AttachmentData*> attachments)
		{
			for (auto data : attachments)
			{
				inputTargets.emplace_back(data->ref);
			}
		};
		void AttachColors(std::vector<AttachmentData*> attachments)
		{
			for (auto data : attachments)
			{
				colorTargets.emplace_back(data->ref);
			}
		};
		void AttachDepth(AttachmentData* attachment)
		{
			depthTarget = attachment->ref;
		};
		void SetDependency(vk::SubpassDependency _deps)
		{
			_deps.dstSubpass = index;
			deps.emplace_back(_deps);
		};
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

		AttachmentData* CreateAttachment(vk::AttachmentDescription desc, vk::ImageLayout interimLayout, vk::ClearValue clearValue, vk::ImageView _view)
		{
			auto data = new AttachmentData;
			data->desc = desc;
			data->ref = vk::AttachmentReference(attachments.size(), interimLayout);
			data->clearValue = clearValue;
			clearValues.emplace_back(clearValue);
			data->view = _view;
			attachments.emplace_back(data);
			return data;
		};
		SubpassData* CreateSubpass(vk::PipelineBindPoint bindPoint, std::vector<AttachmentData*> colorAttachments, std::vector<AttachmentData*> inputAttachments, AttachmentData* _depthTarget, std::vector<vk::SubpassDependency> deps)
		{
			auto subpass = new SubpassData();
			subpass->bindPoint = bindPoint;
			subpass->index = subpassDatas.size();
			for (auto data : colorAttachments)
			{
				subpass->colorTargets.emplace_back(data->ref);
			}
			for (auto data : inputAttachments)
			{
				subpass->inputTargets.emplace_back(data->ref);
			}
			if (_depthTarget != nullptr)
			{
				subpass->depthTarget = _depthTarget->ref;
			}
			if (!deps.empty())
			{
				subpass->deps = deps;
				for (auto& dep : subpass->deps)
				{
					dep.dstSubpass = subpass->index;
				}
			}
			subpassDatas.emplace_back(subpass);
			return subpass;
		};
		vk::RenderPass CreateRenderPass()
		{
			std::vector<vk::AttachmentDescription> descriptions;
			std::vector<vk::AttachmentReference> references;
			std::vector < vk::SubpassDescription > subpasses;
			std::vector<vk::SubpassDependency> dependencies;
			for (auto attachment : attachments)
			{
				descriptions.emplace_back(attachment->desc);
			}
			for (auto& subpassData : subpassDatas)
			{
				subpasses.emplace_back(vk::SubpassDescription({}, subpassData->bindPoint, subpassData->inputTargets.size(), subpassData->inputTargets.data(),
					subpassData->colorTargets.size(), subpassData->colorTargets.data(), {}, (subpassData->depthTarget != 0) ? &subpassData->depthTarget : nullptr,
					{}, {}));
				if (!subpassData->deps.empty())
				{
					dependencies.insert(dependencies.end(), subpassData->deps.begin(), subpassData->deps.end());
				}
			}
			vk::RenderPassCreateInfo createInfo({}, descriptions.size(), descriptions.data(), subpasses.size(), subpasses.data(), dependencies.size(), dependencies.data());
			renderPass = vom.MakeRenderPass(createInfo);
			return renderPass;
		};
		FrameData CreateFramebuffer()
		{
			FrameData frame;
			frame.renderPass = renderPass;
			frame.width = width; frame.height = height;
			for (auto attachment : attachments)
			{
				frame.views.emplace_back(attachment->view);
			}
			vk::FramebufferCreateInfo createInfo({}, renderPass, frame.views.size(), frame.views.data(), width, height, 1);
			frame.frameBuffer = vom.MakeFramebuffer(createInfo);
			frames.emplace_back(frame);
			return frame;
		};

		RenderPassManager(vk::Device deviceHandle, uint32_t _width, uint32_t _height)
		: vom(deviceHandle), width(_width), height(_height){}
		RenderPassManager(VulkanObjectManager& _vom, uint32_t _width, uint32_t _height)
		: vom(_vom), width(_width), height(_height){}
		RenderPassManager(vk::Device deviceHandle)
			: vom(deviceHandle) {}

		void Dispose()
		{
			vom.DisposeAll();
		}
	private:
		VulkanObjectManager vom;
	};
}

