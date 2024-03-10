#pragma once
#include "Ember.h"
#include "Core/Logging.h"
#include "Imgui/imgui_impl_vulkan.h"
#include <vulkan/vulkan.h>
#include "Containers/DynamicArray.h"


#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

static void CheckVkResult(VkResult ErrorCode)
{
    if(ErrorCode == 0)
    {
        return;
    }

    EMBER_LOG(Error, "[Vulkan]: Error: VkResult = %d\n", ErrorCode);
    EMBER_ASSERT(false);
}

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportFn(VkDebugReportFlagsEXT Flags, VkDebugReportObjectTypeEXT ObjectType, uint64_t Object, size_t Location, int32_t MessageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
    {
        UNUSED_ARG(Flags); UNUSED_ARG(Object); UNUSED_ARG(Location); UNUSED_ARG(MessageCode); UNUSED_ARG(pLayerPrefix); UNUSED_ARG(pUserData);
        EMBER_LOG(Debug, "[Vulkan]: Debug Report from ObjectType: %i\nMessages: %s\n\n", ObjectType, pMessage);
        return VK_FALSE;
    }
#endif


namespace Ember
{
    class Window;

    struct VkFrame
    {
        VkCommandPool       CommandPool;
        VkCommandBuffer     CommandBuffer;
        VkFence             Fence;
        VkImage             Backbuffer;
        VkImageView         BackbufferView;
        VkFramebuffer       Framebuffer;
    };

    struct VkSemaphores
    {
        VkSemaphore         ImageAcquiredSemaphore;
        VkSemaphore         RenderCompleteSemaphore;
    };
    
    struct VulkanContext
    {
        VkAllocationCallbacks* Allocator = nullptr;
        VkInstance Instance = nullptr;
        VkPhysicalDevice PhysicalDevice = nullptr;
        VkDevice Device = nullptr;
        u32 QueueFamily = (u32)-1;
        VkQueue Queue = nullptr;
        VkDebugReportCallbackEXT DebugReport = nullptr;
        VkPipelineCache PipelineCache = nullptr;
        VkDescriptorPool DescriptorPool = nullptr;

        int                 Width;
        int                 Height;
        VkSwapchainKHR      Swapchain;
        VkSurfaceKHR        Surface;
        VkSurfaceFormatKHR  SurfaceFormat;
        VkPresentModeKHR    PresentMode = (VkPresentModeKHR)~0;
        VkRenderPass        RenderPass;
        VkPipeline          Pipeline;
        bool                UseDynamicRendering;
        bool                ClearEnable = true;
        VkClearValue        ClearValue;
        uint32_t            FrameIndex;
        uint32_t            ImageCount;
        uint32_t            SemaphoreCount;
        uint32_t            SemaphoreIndex;
        VkFrame*            Frames;
        VkSemaphores*       Semaphores;
    };
    
    class VulkanRenderer
    {
    public:
        explicit VulkanRenderer(Window* Window);
        
        bool Initialize();
        void Shutdown();
        
        void BeginFrame();
        void EndFrame();
        
    private:
        VkPhysicalDevice SetupVulkan_SelectPhysicalDevice() const;
        void RecreateSwapchainIfNecessary();
        void FrameRender(ImDrawData* DrawData);
        void FramePresent();
        void CleanupVulkanWindow();
        void SetupVulkanWindow(VkSurfaceKHR Surface, int Width, int Height);

        void CreateOrResizeWindow(int Width, int Height);
        void CreateSwapChain(int Width, int Height);
        void CreateCommandBuffers();
        void DestroyFrame(VkFrame* Frame) const;
        void DestroySemaphore(VkSemaphores* Semaphores) const;
        VkSurfaceFormatKHR SelectSurfaceFormat(const VkFormat* RequestFormats, int RequestFormatsCount, VkColorSpaceKHR RequestColorSpace) const;
        VkPresentModeKHR SelectPresentMode(const VkPresentModeKHR* RequestModes, int RequestModesCount) const;

        static u32 GetMinImageCountFromPresentMode(VkPresentModeKHR PresentMode);
        static bool IsExtensionAvailable(const DynamicArray<VkExtensionProperties>& Properties, const char* Extension);

    private:
        Window* Window = nullptr;
        VulkanContext VkContext;
        
        u32 MinImageCount = 2;
        bool SwapChainRebuild = false;
    };
}
