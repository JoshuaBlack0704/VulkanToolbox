#pragma once

#include <memory>

#include <vulkan\vulkan.hpp>
#include <deque>

#include <vk_mem_alloc.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#undef MemoryBarrier
#include "ObjectManager.hpp"
#include "CommandManager.hpp"
#include "MemoryManager.hpp"
#include "DescriptorManager.hpp"
#include "RenderpassManager.hpp"
#include "PipelineManagers.hpp"
#include "VulkanWindow.hpp"
#define MemoryBarrier __faststorefence
