#include "VulkanToolbox.hpp"

namespace vkt
{

		ComputePipelineManager::ComputePipelineManager(ObjectManager& _vom, vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo, const char* shaderPath)
			: vom(_vom)
		{
			auto compiledShader = vom.MakeShaderModule(shaderPath);
			layout = vom.MakePipelineLayout(pipelineLayoutCreateInfo);
			vk::PipelineShaderStageCreateInfo computeStage({}, vk::ShaderStageFlagBits::eCompute, compiledShader, "main");
			vk::ComputePipelineCreateInfo computePipelineCreateInfo({}, computeStage, layout);
			computePipeline = vom.MakePipeline({}, computePipelineCreateInfo);
			vom.DestroyType(vk::ShaderModule());
		}
		ComputePipelineManager::ComputePipelineManager(vk::Device deviceHandle, vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo, const char* shaderPath)
			: vom(deviceHandle)
		{
			auto compiledShader = vom.MakeShaderModule(shaderPath);
			layout = vom.MakePipelineLayout(pipelineLayoutCreateInfo);
			vk::PipelineShaderStageCreateInfo computeStage({}, vk::ShaderStageFlagBits::eCompute, compiledShader, "main");
			vk::ComputePipelineCreateInfo computePipelineCreateInfo({}, computeStage, layout);
			computePipeline = vom.MakePipeline({}, computePipelineCreateInfo);
			vom.DestroyType(vk::ShaderModule());
		}



		GraphicsPipelineManager::GraphicsPipelineManager(ObjectManager& _vom,
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
			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo)
			: vom(_vom)
		{
			vom.SetDevice(_vom.GetDevice());

			//Adding Renderpass Group
			renderPass = _renderPass;

			//Adding Shaders
			for (size_t i = 0; i < shaderPaths.size(); i++)
			{
				vk::PipelineShaderStageCreateInfo stageCreateInfo({}, shaderStageFlagBits[i], vom.MakeShaderModule(shaderPaths[i]), "main");
				state.shaderCreateInfos.push_back(stageCreateInfo);
			}

			//Graphics pipeline state
			state.inputAssemblyState = inputAssemblyState;
			state.viewport = viewport;
			state.scissor = scissor;
			state.viewportState = vk::PipelineViewportStateCreateInfo({}, 1, &state.viewport, 1, &state.scissor);
			state.rasterizationState = rasterizationState;
			state.multisampleState = multisampleState;
			state.depthStencilState = depthStencilState;
			state.colorBlendAttachments = colorBlendAttachments;
			state.colorBlendState = colorBlendState;
			state.colorBlendState.attachmentCount = colorBlendAttachments.size();
			state.colorBlendState.pAttachments = state.colorBlendAttachments.data();
			state.pipelineLayoutCreateInfo = pipelineLayoutCreateInfo;

			//Building vertex state
			state.bindings = vertexBindings;
			state.attributes = vertexAttributes;
			state.vertexInputState = vk::PipelineVertexInputStateCreateInfo({}, state.bindings.size(), state.bindings.data(), state.attributes.size(), state.attributes.data());

			//Creating graphics pipeline
			layout = vom.MakePipelineLayout(state.pipelineLayoutCreateInfo);

			state.graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo({}, state.shaderCreateInfos.size(),
				state.shaderCreateInfos.data(), &state.vertexInputState, &state.inputAssemblyState, {}, &state.viewportState,
				&state.rasterizationState, &state.multisampleState, &state.depthStencilState, &state.colorBlendState, {}, layout,
				renderPass, 0);
			pipeline = vom.MakePipeline({}, state.graphicsPipelineCreateInfo);
		}

		GraphicsPipelineManager::GraphicsPipelineManager(vk::Device deviceHandle,
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
			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo)
			: vom(deviceHandle)
		{
			//Adding Renderpass Group
			renderPass = _renderPass;

			//Adding Shaders
			for (size_t i = 0; i < shaderPaths.size(); i++)
			{
				vk::PipelineShaderStageCreateInfo stageCreateInfo({}, shaderStageFlagBits[i], vom.MakeShaderModule(shaderPaths[i]), "main");
				state.shaderCreateInfos.push_back(stageCreateInfo);
			}

			//Graphics pipeline state
			state.inputAssemblyState = inputAssemblyState;
			state.viewport = viewport;
			state.scissor = scissor;
			state.viewportState = vk::PipelineViewportStateCreateInfo({}, 1, &state.viewport, 1, &state.scissor);
			state.rasterizationState = rasterizationState;
			state.multisampleState = multisampleState;
			state.depthStencilState = depthStencilState;
			state.colorBlendAttachments = colorBlendAttachments;
			state.colorBlendState = colorBlendState;
			state.colorBlendState.attachmentCount = colorBlendAttachments.size();
			state.colorBlendState.pAttachments = state.colorBlendAttachments.data();
			state.pipelineLayoutCreateInfo = pipelineLayoutCreateInfo;

			//Building vertex state
			state.bindings = vertexBindings;
			state.attributes = vertexAttributes;
			state.vertexInputState = vk::PipelineVertexInputStateCreateInfo({}, state.bindings.size(), state.bindings.data(), state.attributes.size(), state.attributes.data());

			//Creating graphics pipeline
			layout = vom.MakePipelineLayout(state.pipelineLayoutCreateInfo);

			state.graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo({}, state.shaderCreateInfos.size(),
				state.shaderCreateInfos.data(), &state.vertexInputState, &state.inputAssemblyState, {}, &state.viewportState,
				&state.rasterizationState, &state.multisampleState, &state.depthStencilState, &state.colorBlendState, {}, layout,
				renderPass, 0);
			pipeline = vom.MakePipeline({}, state.graphicsPipelineCreateInfo);
		}



}
