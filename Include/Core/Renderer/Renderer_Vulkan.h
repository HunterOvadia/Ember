#pragma once
#include "Ember.h"
#include "Core/Logging.h"
#include "Imgui/imgui_impl_vulkan.h"
#include <vulkan/vulkan.h>

#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

#define VK_GET_INSTANCE_PROC_ADDR(name, instance) auto (name) = (PFN_##name)vkGetInstanceProcAddr(instance, #name);

inline void CheckVkResult(VkResult ErrorCode)
{
    if(ErrorCode == 0)
    {
        return;
    }

    EMBER_LOG(Error, "[Vulkan]: Error: VkResult = %d\n", ErrorCode);
    EMBER_ASSERT(false);
}
 
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    inline VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportFn(VkDebugReportFlagsEXT Flags, VkDebugReportObjectTypeEXT ObjectType, uint64_t Object, size_t Location, int32_t MessageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
    {
        UNUSED_ARG(Flags); UNUSED_ARG(Object); UNUSED_ARG(Location); UNUSED_ARG(MessageCode); UNUSED_ARG(pLayerPrefix); UNUSED_ARG(pUserData);
        EMBER_LOG(Debug, "[Vulkan]: Debug Report from ObjectType: %i\nMessages: %s\n\n", ObjectType, pMessage);
        return VK_FALSE;
    }
#endif

struct vk_frame_t
{
    VkCommandPool               CommandPool;
    VkCommandBuffer             CommandBuffer;
    VkFence                     Fence;
    VkImage                     Backbuffer;
    VkImageView                 BackbufferView;
    VkFramebuffer               Framebuffer;
};

struct vk_semaphores_t
{
    VkSemaphore                 ImageAcquiredSemaphore;
    VkSemaphore                 RenderCompleteSemaphore;
};

struct vk_context
{
    VkAllocationCallbacks*      Allocator = nullptr;
    VkInstance                  Instance = nullptr;
    VkPhysicalDevice            PhysicalDevice = nullptr;
    VkDevice                    Device = nullptr;
    u32                         QueueFamily = (u32)-1;
    VkQueue                     Queue = nullptr;
    VkDebugReportCallbackEXT    DebugReport = nullptr;
    VkPipelineCache             PipelineCache = nullptr;
    VkDescriptorPool            DescriptorPool = nullptr;

    u32                         Width;
    u32                         Height;
    VkSwapchainKHR              Swapchain;
    VkSurfaceKHR                Surface;
    VkSurfaceFormatKHR          SurfaceFormat;
    VkPresentModeKHR            PresentMode = (VkPresentModeKHR)~0;
    VkRenderPass                RenderPass;
    VkClearValue                ClearValue;
    uint32_t                    FrameIndex;
    uint32_t                    ImageCount;
    uint32_t                    SemaphoreCount;
    uint32_t                    SemaphoreIndex;
    vk_frame_t*                    Frames;
    vk_semaphores_t*               Semaphores;
};

struct ember_renderer_vulkan_t
{
    vk_context Context;
    u32 MinImageCount;
    bool SwapChainRebuild;
};
