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
#include "ObjectManager.hpp"
#include "CommandManager.hpp"
#include "MemoryManager.hpp"
#include "DescriptorManager.hpp"
#include "RenderpassManager.hpp"
#include "PipelineManagers.hpp"
#include "VulkanWindow.hpp"
#define MemoryBarrier __faststorefence