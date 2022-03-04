#pragma once
namespace vkt
{
	struct ComputePipelineManager
	{
		ComputePipelineManager(ObjectManager& _vom, vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo, const char* shaderPath);
		ComputePipelineManager(vk::Device deviceHandle, vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo, const char* shaderPath);

		ObjectManager vom;
		vk::Pipeline computePipeline;
		vk::PipelineLayout layout;
	};

	struct PipelineState
	{
		//Implicit
		vk::PipelineCache cache;
		std::vector<vk::ShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderCreateInfos;
		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;
		vk::PipelineVertexInputStateCreateInfo vertexInputState;

		//Explicit
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		vk::Viewport viewport;
		vk::Rect2D scissor;
		vk::PipelineViewportStateCreateInfo viewportState;
		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		vk::PipelineMultisampleStateCreateInfo multisampleState;
		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
		vk::PipelineColorBlendStateCreateInfo colorBlendState;
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo;

	};

	struct GraphicsPipelineManager
	{
		GraphicsPipelineManager(ObjectManager& _vom,
			vk::RenderPass _renderPass,
			std::vector<const char*> shaderPaths,
			std::vector<vk::ShaderStageFlagBits> shaderStageFlagBits,
			std::vector<vk::VertexInputBindingDescription> vertexBindings, std::vector<vk::VertexInputAttributeDescription> vertexAttributes,
			vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState,
			vk::Viewport viewport, vk::Rect2D scissor,
			vk::PipelineRasterizationStateCreateInfo rasterizationState,
			vk::PipelineMultisampleStateCreateInfo multisampleState,
			vk::PipelineDepthStencilStateCreateInfo depthStencilState,
			std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments,
			vk::PipelineColorBlendStateCreateInfo colorBlendState,
			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo);

		GraphicsPipelineManager(vk::Device deviceHandle,
			vk::RenderPass _renderPass,
			std::vector<const char*> shaderPaths,
			std::vector<vk::ShaderStageFlagBits> shaderStageFlagBits,
			std::vector<vk::VertexInputBindingDescription> vertexBindings, std::vector<vk::VertexInputAttributeDescription> vertexAttributes,
			vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState,
			vk::Viewport viewport, vk::Rect2D scissor,
			vk::PipelineRasterizationStateCreateInfo rasterizationState,
			vk::PipelineMultisampleStateCreateInfo multisampleState,
			vk::PipelineDepthStencilStateCreateInfo depthStencilState,
			std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments,
			vk::PipelineColorBlendStateCreateInfo colorBlendState,
			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo);

		vk::PipelineLayout layout;
		vk::Pipeline pipeline;
		ObjectManager vom;
		PipelineState state;
		vk::RenderPass renderPass;
	};


}
