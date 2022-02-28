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
#include "Internal/ObjectManager.hpp"
#include "Internal/CommandManager.hpp"
#include "Internal/MemoryManager.hpp"
#include "Internal/DescriptorManager.hpp"
#include "Internal/RenderpassManager.hpp"
#include "Internal/PipelineManagers.hpp"
#include "Internal/VulkanWindow.hpp"
#define MemoryBarrier __faststorefence
