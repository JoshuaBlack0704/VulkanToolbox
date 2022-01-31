#pragma once

#include <memory>
#include <cassert>
#include <vulkan\vulkan.hpp>
#include <deque>
#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <taskflow/taskflow.hpp>

#undef MemoryBarrier
#include "Internal/VulkanObjectManager.hpp"
#include "Internal/VulkanCommandManager.hpp"
#include "Internal/VulkanMemoryManager.hpp"
#include "Internal/VulkanDescriptorManager.hpp"
#include "Internal/VulkanRenderpassManager.hpp"
#include "Internal/VulkanPipelineManagers.hpp"
#include "Internal/VulkanWindow.hpp"
#define MemoryBarrier __faststorefence
