#include "../Headers/VulkanToolbox.hpp"
namespace vkt
{
	void SubpassData::AttachInputs(std::vector<AttachmentData*> attachments)
	{
		for (auto data : attachments)
		{
			inputTargets.emplace_back(data->ref);
		}
	};
	void SubpassData::AttachColors(std::vector<AttachmentData*> attachments)
	{
		for (auto data : attachments)
		{
			colorTargets.emplace_back(data->ref);
		}
	};
	void SubpassData::AttachDepth(AttachmentData* attachment)
	{
		depthTarget = attachment->ref;
	};
	void SubpassData::SetDependency(vk::SubpassDependency _deps)
	{
		_deps.dstSubpass = index;
		deps.emplace_back(_deps);
	};

	AttachmentData* RenderPassManager::CreateAttachment(vk::AttachmentDescription desc, vk::ImageLayout interimLayout, vk::ClearValue clearValue, vk::ImageView _view)
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
	SubpassData* RenderPassManager::CreateSubpass(vk::PipelineBindPoint bindPoint, std::vector<AttachmentData*> colorAttachments, std::vector<AttachmentData*> inputAttachments, AttachmentData* _depthTarget, std::vector<vk::SubpassDependency> deps)
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
	vk::RenderPass RenderPassManager::CreateRenderPass()
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
	FrameData RenderPassManager::CreateFramebuffer()
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

	RenderPassManager::RenderPassManager(vk::Device deviceHandle, uint32_t _width, uint32_t _height)
		: vom(deviceHandle), width(_width), height(_height)
	{
	}
	RenderPassManager::RenderPassManager(ObjectManager& _vom, uint32_t _width, uint32_t _height)
		: vom(_vom), width(_width), height(_height)
	{
	}
	RenderPassManager::RenderPassManager(vk::Device deviceHandle)
		: vom(deviceHandle)
	{
	}

	void RenderPassManager::Dispose()
	{
		vom.DestroyAll();
	};
}

