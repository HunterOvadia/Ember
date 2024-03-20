#include "Ember.h"

#ifdef RENDERER_VULKAN
#include "Core/Renderer/Renderer.h"
#include "Core/Renderer/Renderer_Vulkan.h"
#include "Core/Window.h"
#include "Core/Memory/Memory.h"
#include "Imgui/imgui.h"
#include "Containers/DynamicArray.h"
#include "Core/Platform/Platform.h"

static bool IsExtensionAvailable(const DynamicArray<VkExtensionProperties>& Properties, const char* Extension)
{
    for(const VkExtensionProperties& Property : Properties)
    {
        if(CStringCompare(Property.extensionName, Extension) == 0)
        {
            return true;
        }
    }

    return false;
}

static void CreateRenderPass(ember_renderer_vulkan_t* Renderer)
{
    VkAttachmentDescription Attachment = {
        .format = Renderer->Context.SurfaceFormat.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference ColorAttachment = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription Subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &ColorAttachment
    };

    VkSubpassDependency Dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo Info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &Attachment,
        .subpassCount = 1,
        .pSubpasses = &Subpass,
        .dependencyCount = 1,
        .pDependencies = &Dependency
    };
    
    CheckVkResult(vkCreateRenderPass(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Renderer->Context.RenderPass));
}

static void CreateImageViews(ember_renderer_vulkan_t* Renderer)
{
    VkImageSubresourceRange ImageRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
        
    VkImageViewCreateInfo Info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = Renderer->Context.SurfaceFormat.format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_R,
            .g = VK_COMPONENT_SWIZZLE_G,
            .b = VK_COMPONENT_SWIZZLE_B,
            .a = VK_COMPONENT_SWIZZLE_A
        },
        .subresourceRange = ImageRange
    };

    for(u32 i = 0; i < Renderer->Context.ImageCount; ++i)
    {
        vk_frame_t* Frame = &Renderer->Context.Frames[i];
        Info.image = Frame->Backbuffer;
        CheckVkResult(vkCreateImageView(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Frame->BackbufferView));
    }
}

static void CreateFrameBuffers(ember_renderer_vulkan_t* Renderer)
{
    VkImageView Attachment[1];
    VkFramebufferCreateInfo Info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = Renderer->Context.RenderPass,
        .attachmentCount = 1,
        .pAttachments = Attachment,
        .width = (u32)Renderer->Context.Width,
        .height = (u32)Renderer->Context.Height,
        .layers = 1
    };

    for(u32 i = 0; i < Renderer->Context.ImageCount; ++i)
    {
        vk_frame_t* Frame = &Renderer->Context.Frames[i];
        Attachment[0] = Frame->BackbufferView;
        CheckVkResult(vkCreateFramebuffer(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Frame->Framebuffer));
    }
}

static void ResetFences(ember_renderer_vulkan_t* Renderer, vk_frame_t* Frame)
{
    EMBER_ASSERT(Frame != nullptr);
    CheckVkResult(vkWaitForFences(Renderer->Context.Device, 1, &Frame->Fence, VK_TRUE, UINT64_MAX));
    CheckVkResult(vkResetFences(Renderer->Context.Device, 1, &Frame->Fence));
}

static void BeginCommandBuffer(ember_renderer_vulkan_t* Renderer, vk_frame_t* Frame)
{
    EMBER_ASSERT(Frame != nullptr);

    CheckVkResult(vkResetCommandPool(Renderer->Context.Device, Frame->CommandPool, 0));

    VkCommandBufferBeginInfo Info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
        
    CheckVkResult(vkBeginCommandBuffer(Frame->CommandBuffer, &Info));
}

static void BeginRenderPass(ember_renderer_vulkan_t* Renderer, vk_frame_t* Frame)
{
    EMBER_ASSERT(Frame != nullptr);

    VkRenderPassBeginInfo Info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = Renderer->Context.RenderPass,
        .framebuffer = Frame->Framebuffer,
        .renderArea = {
            .extent = {
                .width = (u32)Renderer->Context.Width,
                .height = (u32)Renderer->Context.Height
            }
        },
        .clearValueCount = 1,
        .pClearValues = &Renderer->Context.ClearValue
    };
    
    vkCmdBeginRenderPass(Frame->CommandBuffer, &Info, VK_SUBPASS_CONTENTS_INLINE);
}

static void EndRenderPass(vk_frame_t* Frame)
{
    EMBER_ASSERT(Frame != nullptr);
    vkCmdEndRenderPass(Frame->CommandBuffer);
}

static void EndCommandBuffer(vk_frame_t* Frame)
{
    EMBER_ASSERT(Frame != nullptr);
    CheckVkResult(vkEndCommandBuffer(Frame->CommandBuffer));
}

static void SubmitRenderQueue(ember_renderer_vulkan_t* Renderer, vk_frame_t* Frame, VkSemaphore* ImageAcquiredSemaphore, VkSemaphore* RenderCompleteSemaphore)
{
    EMBER_ASSERT(Frame != nullptr && ImageAcquiredSemaphore != nullptr && RenderCompleteSemaphore != nullptr);

    VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo SubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = ImageAcquiredSemaphore,
        .pWaitDstStageMask = &WaitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &Frame->CommandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = RenderCompleteSemaphore
    };
    
    CheckVkResult(vkQueueSubmit(Renderer->Context.Queue, 1, &SubmitInfo, Frame->Fence));
}

static vk_frame_t* GetCurrentFrame(ember_renderer_vulkan_t* Renderer)
{
    return &Renderer->Context.Frames[Renderer->Context.FrameIndex];
}

static vk_semaphores_t GetCurrentSemaphores(ember_renderer_vulkan_t* Renderer)
{
    return Renderer->Context.Semaphores[Renderer->Context.SemaphoreIndex];
}

void DestroySemaphore(ember_renderer_vulkan_t* Renderer, vk_semaphores_t* Semaphores)
{
    vkDestroySemaphore(Renderer->Context.Device, Semaphores->ImageAcquiredSemaphore, Renderer->Context.Allocator);
    vkDestroySemaphore(Renderer->Context.Device, Semaphores->RenderCompleteSemaphore, Renderer->Context.Allocator);
    Semaphores->ImageAcquiredSemaphore = Semaphores->RenderCompleteSemaphore = VK_NULL_HANDLE;
}

static void DestroyFrame(ember_renderer_vulkan_t* Renderer, vk_frame_t* Frame)
{
    vkDestroyFence(Renderer->Context.Device, Frame->Fence, Renderer->Context.Allocator);
    vkFreeCommandBuffers(Renderer->Context.Device, Frame->CommandPool, 1, &Frame->CommandBuffer);
    vkDestroyCommandPool(Renderer->Context.Device, Frame->CommandPool, Renderer->Context.Allocator);
    Frame->Fence = VK_NULL_HANDLE;
    Frame->CommandBuffer = VK_NULL_HANDLE;
    Frame->CommandPool = VK_NULL_HANDLE;

    vkDestroyImageView(Renderer->Context.Device, Frame->BackbufferView, Renderer->Context.Allocator);
    vkDestroyFramebuffer(Renderer->Context.Device, Frame->Framebuffer, Renderer->Context.Allocator);
}

static void DestroyFrames(ember_renderer_vulkan_t* Renderer)
{
    CheckVkResult(vkDeviceWaitIdle(Renderer->Context.Device));

    for(u32 i = 0; i < Renderer->Context.ImageCount; ++i)
    {
        DestroyFrame(Renderer, &Renderer->Context.Frames[i]);
    }

    for(u32 i = 0; i < Renderer->Context.SemaphoreCount; ++i)
    {
        DestroySemaphore(Renderer, &Renderer->Context.Semaphores[i]);
    }

    EmberMemoryFree(Renderer->Context.Frames);
    EmberMemoryFree(Renderer->Context.Semaphores);

    Renderer->Context.Frames = nullptr;
    Renderer->Context.Semaphores = nullptr;
    Renderer->Context.ImageCount = 0;
    if(Renderer->Context.RenderPass)
    {
        vkDestroyRenderPass(Renderer->Context.Device, Renderer->Context.RenderPass, Renderer->Context.Allocator);
    }
}

static void CreateVulkanInstance(ember_renderer_t* Renderer, ember_window_t* Window)
{
    ember_renderer_vulkan_t* VkRenderer = (ember_renderer_vulkan_t*)Renderer->Internal;
    DynamicArray<const char*> Extensions = EmberPlatformRendererVulkanGetInstanceExtensions(Window);
    
    VkInstanceCreateInfo CreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
    };

    u32 PropertiesCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &PropertiesCount, nullptr);

    DynamicArray<VkExtensionProperties> Properties(PropertiesCount);
    CheckVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &PropertiesCount, Properties.GetData()));

    if(IsExtensionAvailable(Properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
    {
        Extensions.Add(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    if(IsExtensionAvailable(Properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
    {
        Extensions.Add(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        CreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    const char* Layers[] = { "VK_LAYER_KHRONOS_validation" };
    CreateInfo.enabledLayerCount = 1;
    CreateInfo.ppEnabledLayerNames = Layers;
    Extensions.Add("VK_EXT_debug_report");
#endif

    CreateInfo.enabledExtensionCount = (u32)Extensions.Size();
    CreateInfo.ppEnabledExtensionNames = Extensions.GetData();
    CheckVkResult(vkCreateInstance(&CreateInfo, VkRenderer->Context.Allocator, &VkRenderer->Context.Instance));

#ifdef APP_USE_VULKAN_DEBUG_REPORT
    VK_GET_INSTANCE_PROC_ADDR(vkCreateDebugReportCallbackEXT, VkRenderer->Context.Instance);
    EMBER_ASSERT(vkCreateDebugReportCallbackEXT != nullptr);
    VkDebugReportCallbackCreateInfoEXT DebugReportCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
        .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
        .pfnCallback = DebugReportFn,
        .pUserData = nullptr
    };
    CheckVkResult(vkCreateDebugReportCallbackEXT(VkRenderer->Context.Instance, &DebugReportCreateInfo, VkRenderer->Context.Allocator, &VkRenderer->Context.DebugReport));
#endif
}

static void SelectGraphicsQueueFamily(ember_renderer_vulkan_t* Renderer)
{
    u32 Count;
    vkGetPhysicalDeviceQueueFamilyProperties(Renderer->Context.PhysicalDevice, &Count, nullptr);
    VkQueueFamilyProperties* Queues = (VkQueueFamilyProperties*)EmberMemoryAllocate(sizeof(VkQueueFamilyProperties) * Count);
    vkGetPhysicalDeviceQueueFamilyProperties(Renderer->Context.PhysicalDevice, &Count, Queues);
    for(u32 i = 0; i < Count; ++i)
    {
        if(Queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            Renderer->Context.QueueFamily = i;
            break;
        }
    }

    EmberMemoryFree(Queues);
    EMBER_ASSERT(Renderer->Context.QueueFamily != (u32)-1);
}

static VkPhysicalDevice SelectPhysicalDevice(ember_renderer_vulkan_t* Renderer)
{
    u32 GPUCount = 0;
    CheckVkResult(vkEnumeratePhysicalDevices(Renderer->Context.Instance, &GPUCount, nullptr));
    EMBER_ASSERT(GPUCount > 0);

    DynamicArray<VkPhysicalDevice> GPUs(GPUCount);
    CheckVkResult(vkEnumeratePhysicalDevices(Renderer->Context.Instance, &GPUCount, GPUs.GetData()));
    for(VkPhysicalDevice& CurrentDevice : GPUs)
    {
        VkPhysicalDeviceProperties Properties;
        vkGetPhysicalDeviceProperties(CurrentDevice, &Properties);
        if(Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            return CurrentDevice;
        }
    }

    if(GPUCount > 0)
    {
        return GPUs[0];
    }

    return nullptr;
}

static void CreatePhysicalDevice(ember_renderer_vulkan_t* Renderer)
{
    Renderer->Context.PhysicalDevice = SelectPhysicalDevice(Renderer);
}

void CreateLogicalDevice(ember_renderer_vulkan_t* Renderer)
{
    DynamicArray<const char*> DeviceExtensions;
    DeviceExtensions.Add("VK_KHR_swapchain");

    u32 PropertiesCount;
    vkEnumerateDeviceExtensionProperties(Renderer->Context.PhysicalDevice, nullptr, &PropertiesCount, nullptr);
        
    DynamicArray<VkExtensionProperties> Properties(PropertiesCount);
    vkEnumerateDeviceExtensionProperties(Renderer->Context.PhysicalDevice, nullptr, &PropertiesCount, Properties.GetData());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if(IsExtensionAvailable(Properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
    {
        DeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
#endif

    constexpr float QueuePriority[] = { 1.0f };
    VkDeviceQueueCreateInfo QueueInfo[1] = {};
    QueueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueInfo[0].queueFamilyIndex = Renderer->Context.QueueFamily;
    QueueInfo[0].queueCount = 1;
    QueueInfo[0].pQueuePriorities = QueuePriority;

    VkDeviceCreateInfo CreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = ARRAY_SIZE(QueueInfo),
        .pQueueCreateInfos = QueueInfo,
        .enabledExtensionCount = (u32)DeviceExtensions.Size(),
        .ppEnabledExtensionNames = DeviceExtensions.GetData()
    };
    CheckVkResult(vkCreateDevice(Renderer->Context.PhysicalDevice, &CreateInfo, Renderer->Context.Allocator, &Renderer->Context.Device));
    vkGetDeviceQueue(Renderer->Context.Device, Renderer->Context.QueueFamily, 0, &Renderer->Context.Queue);
}

static void CreateDescriptorPool(ember_renderer_vulkan_t* Renderer)
{
    VkDescriptorPoolSize PoolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    };

    VkDescriptorPoolCreateInfo PoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1,
        .poolSizeCount = ARRAY_SIZE(PoolSizes),
        .pPoolSizes = PoolSizes
    };
    
    CheckVkResult(vkCreateDescriptorPool(Renderer->Context.Device, &PoolInfo, Renderer->Context.Allocator, &Renderer->Context.DescriptorPool));
}

static VkSurfaceFormatKHR SelectSurfaceFormat(ember_renderer_vulkan_t* Renderer, const VkFormat* RequestFormats, int RequestFormatsCount, VkColorSpaceKHR RequestColorSpace)
{
    EMBER_ASSERT(RequestFormats != nullptr);
    EMBER_ASSERT(RequestFormatsCount > 0);

    u32 AvailableCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer->Context.PhysicalDevice, Renderer->Context.Surface, &AvailableCount, nullptr);
    DynamicArray<VkSurfaceFormatKHR> AvailableFormats(AvailableCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Renderer->Context.PhysicalDevice, Renderer->Context.Surface, &AvailableCount, AvailableFormats.GetData());

    if(AvailableCount == 1)
    {
        if(AvailableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            VkSurfaceFormatKHR Result = {
                .format = RequestFormats[0],
                .colorSpace = RequestColorSpace
            };
            return Result;
        }

        return AvailableFormats[0];
    }

    for(int RequestIndex = 0; RequestIndex < RequestFormatsCount; ++RequestIndex)
    {
        for(int AvailableIndex = 0; AvailableIndex < (int)AvailableCount; ++AvailableIndex)
        {
            if(AvailableFormats[AvailableIndex].format == RequestFormats[RequestIndex] && AvailableFormats[AvailableIndex].colorSpace == RequestColorSpace)
            {
                return AvailableFormats[AvailableIndex];
            }
        }
    }

    return AvailableFormats[0];
}

static void CreateSurface(ember_renderer_t* Renderer, ember_window_t* Window)
{
    ember_renderer_vulkan_t* VkRenderer = (ember_renderer_vulkan_t*)Renderer->Internal;
    if(EmberPlatformRendererCreateVulkanSurface(VkRenderer, Window) == 0)
    {
        EMBER_LOG(Error, "Failed to create Vulkan Surface!");
        return;
    }
    
    VkBool32 Res;
    vkGetPhysicalDeviceSurfaceSupportKHR(VkRenderer->Context.PhysicalDevice, VkRenderer->Context.QueueFamily, VkRenderer->Context.Surface, &Res);
    if(Res != VK_TRUE)
    {
        EMBER_LOG(Error, "[Vulkan]: No WSI Support on Physical Device 0");
        return;
    }

    constexpr VkFormat RequestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    constexpr VkColorSpaceKHR RequestSurfaceColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkRenderer->Context.SurfaceFormat = SelectSurfaceFormat(VkRenderer, RequestSurfaceImageFormat, ARRAY_SIZE(RequestSurfaceImageFormat), RequestSurfaceColorSpace);
}

static VkPresentModeKHR SelectPresentMode(ember_renderer_vulkan_t* Renderer, VkPresentModeKHR* RequestModes, int RequestModesCount)
{
    EMBER_ASSERT(RequestModes != nullptr);
    EMBER_ASSERT(RequestModesCount > 0);

    u32 AvailableCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer->Context.PhysicalDevice, Renderer->Context.Surface, &AvailableCount, nullptr);
    DynamicArray<VkPresentModeKHR> AvailableModes(AvailableCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(Renderer->Context.PhysicalDevice, Renderer->Context.Surface, &AvailableCount, AvailableModes.GetData());

    for(int RequestIndex = 0; RequestIndex < RequestModesCount; ++RequestIndex)
    {
        for(int AvailIndex = 0; AvailIndex < (int)AvailableCount; ++AvailIndex)
        {
            if(RequestModes[RequestIndex] == AvailableModes[AvailIndex])
            {
                return RequestModes[RequestIndex];
            }
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static void SetPresentMode(ember_renderer_vulkan_t* Renderer)
{
#ifdef APP_UNLIMITED_FRAME_RATE
    VkPresentModeKHR PresentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
    VkPresentModeKHR PresentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
    Renderer->Context.PresentMode = SelectPresentMode(Renderer, &PresentModes[0], ARRAY_SIZE(PresentModes));
}

static u32 GetMinImageCountFromPresentMode(VkPresentModeKHR PresentMode)
{
    switch(PresentMode)
    {
        case VK_PRESENT_MODE_MAILBOX_KHR:
        {
            return 3;
        }
        case VK_PRESENT_MODE_FIFO_KHR:
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        {
            return 2;
        }
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
        {
            return 1;
        }
        default:
        {
            EMBER_ASSERT(false);
            return 1;
        }
    }
}

void InternalCreateSwapChain(ember_renderer_vulkan_t* Renderer, u32 Width, u32 Height)
{
    VkSwapchainKHR OldSwapchain = Renderer->Context.Swapchain;

    Renderer->Context.Swapchain = VK_NULL_HANDLE;

    DestroyFrames(Renderer);

    if(Renderer->MinImageCount == 0)
    {
        Renderer->MinImageCount = GetMinImageCountFromPresentMode(Renderer->Context.PresentMode);
    }

    VkSwapchainCreateInfoKHR Info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = Renderer->Context.Surface,
        .minImageCount = Renderer->MinImageCount,
        .imageFormat = Renderer->Context.SurfaceFormat.format,
        .imageColorSpace = Renderer->Context.SurfaceFormat.colorSpace,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = Renderer->Context.PresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = OldSwapchain
    };

    VkSurfaceCapabilitiesKHR Capabilities;
    CheckVkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Renderer->Context.PhysicalDevice, Renderer->Context.Surface, &Capabilities));
    if(Info.minImageCount < Capabilities.minImageCount)
    {
        Info.minImageCount = Capabilities.minImageCount;
    }
    else if(Capabilities.maxImageCount != 0 && Info.minImageCount > Capabilities.maxImageCount)
    {
        Info.minImageCount = Capabilities.maxImageCount;
    }

    if(Capabilities.currentExtent.width == 0xffffffff)
    {
        Info.imageExtent.width = Renderer->Context.Width = Width;
        Info.imageExtent.height = Renderer->Context.Height = Height;
    }
    else
    {
        Info.imageExtent.width = Renderer->Context.Width = Capabilities.currentExtent.width;
        Info.imageExtent.height = Renderer->Context.Height = Capabilities.currentExtent.height;
    }

    CheckVkResult(vkCreateSwapchainKHR(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Renderer->Context.Swapchain));
    CheckVkResult(vkGetSwapchainImagesKHR(Renderer->Context.Device, Renderer->Context.Swapchain, &Renderer->Context.ImageCount, nullptr));

    VkImage Backbuffers[16] = {};
    EMBER_ASSERT(Renderer->Context.ImageCount >= Renderer->MinImageCount);
    EMBER_ASSERT(Renderer->Context.ImageCount < ARRAY_SIZE(Backbuffers));
    CheckVkResult(vkGetSwapchainImagesKHR(Renderer->Context.Device, Renderer->Context.Swapchain, &Renderer->Context.ImageCount, Backbuffers));

    EMBER_ASSERT(Renderer->Context.Frames == nullptr && Renderer->Context.Semaphores == nullptr);
    Renderer->Context.SemaphoreCount = (Renderer->Context.ImageCount + 1);
    Renderer->Context.Frames = EmberMemoryAllocateType<vk_frame_t, true>(Renderer->Context.ImageCount);
    Renderer->Context.Semaphores = EmberMemoryAllocateType<vk_semaphores_t, true>(Renderer->Context.SemaphoreCount);
    for(u32 i = 0; i < Renderer->Context.ImageCount; ++i)
    {
        Renderer->Context.Frames[i].Backbuffer = Backbuffers[i];
    }

    if(OldSwapchain)
    {
        vkDestroySwapchainKHR(Renderer->Context.Device, OldSwapchain, Renderer->Context.Allocator);
    }

    CreateRenderPass(Renderer);
    CreateImageViews(Renderer);
    CreateFrameBuffers(Renderer);
}

void CreateCommandBuffers(ember_renderer_vulkan_t* Renderer)
{
    EMBER_ASSERT(Renderer->Context.PhysicalDevice != VK_NULL_HANDLE && Renderer->Context.Device != VK_NULL_HANDLE);
    for(u32 i = 0; i < Renderer->Context.ImageCount; ++i)
    {
        vk_frame_t* Frame = &Renderer->Context.Frames[i];
        {
            VkCommandPoolCreateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = 0,
                .queueFamilyIndex = Renderer->Context.QueueFamily
            };
            CheckVkResult(vkCreateCommandPool(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Frame->CommandPool));
        }
        {
            VkCommandBufferAllocateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = Frame->CommandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
            CheckVkResult(vkAllocateCommandBuffers(Renderer->Context.Device, &Info, &Frame->CommandBuffer));
        }
        {
            VkFenceCreateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
            };
            CheckVkResult(vkCreateFence(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Frame->Fence));
        }
    }

    for(u32 i = 0; i < Renderer->Context.SemaphoreCount; ++i)
    {
        vk_semaphores_t* Semaphores = &Renderer->Context.Semaphores[i];
        {
            VkSemaphoreCreateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
            };
            CheckVkResult(vkCreateSemaphore(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Semaphores->ImageAcquiredSemaphore));
            CheckVkResult(vkCreateSemaphore(Renderer->Context.Device, &Info, Renderer->Context.Allocator, &Semaphores->RenderCompleteSemaphore));
        }
    }
}

static void CreateOrResizeSwapChain(ember_renderer_vulkan_t* Renderer, int Width, int Height)
{
    InternalCreateSwapChain(Renderer, Width, Height);
    CreateCommandBuffers(Renderer);
}

static void RecreateSwapchainIfNecessary(ember_renderer_t* Renderer, ember_window_t* Window)
{
    ember_renderer_vulkan_t* VkRenderer = (ember_renderer_vulkan_t*)Renderer->Internal;
    if(!VkRenderer->SwapChainRebuild)
    {
        return;
    }
    
    u32 Width, Height;
    if(EmberPlatformGetWindowSize(Window, &Width, &Height))
    {
        if(Width > 0 && Height > 0)
        {
            ImGui_ImplVulkan_SetMinImageCount(VkRenderer->MinImageCount);
            CreateOrResizeSwapChain(VkRenderer, Width, Height);
            VkRenderer->Context.FrameIndex = 0;
            VkRenderer->SwapChainRebuild = false;
        }
    }
}

static void FrameRender(ember_renderer_vulkan_t* Renderer, ImDrawData* DrawData)
{
    VkSemaphore ImageAcquiredSemaphore = GetCurrentSemaphores(Renderer).ImageAcquiredSemaphore;
    VkSemaphore RenderCompleteSemaphore = GetCurrentSemaphores(Renderer).RenderCompleteSemaphore;
    VkResult Error = vkAcquireNextImageKHR(Renderer->Context.Device, Renderer->Context.Swapchain, UINT64_MAX, ImageAcquiredSemaphore, VK_NULL_HANDLE, &Renderer->Context.FrameIndex);
    if(Error == VK_ERROR_OUT_OF_DATE_KHR || Error == VK_SUBOPTIMAL_KHR)
    {
        Renderer->SwapChainRebuild = true;
        return;
    }
    CheckVkResult(Error);

    vk_frame_t* Frame = GetCurrentFrame(Renderer);
    
    ResetFences(Renderer, Frame);
    BeginCommandBuffer(Renderer, Frame);
    BeginRenderPass(Renderer, Frame);
    {
        ImGui_ImplVulkan_RenderDrawData(DrawData, Frame->CommandBuffer);
    }
    EndRenderPass(Frame);
    EndCommandBuffer(Frame);
    SubmitRenderQueue(Renderer, Frame, &ImageAcquiredSemaphore, &RenderCompleteSemaphore);
}

static void FramePresent(ember_renderer_vulkan_t* Renderer)
{
    if(Renderer->SwapChainRebuild)
    {
        return;
    }

    VkSemaphore RenderCompleteSemaphore = GetCurrentSemaphores(Renderer).RenderCompleteSemaphore;

    VkPresentInfoKHR PresentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &RenderCompleteSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &Renderer->Context.Swapchain,
        .pImageIndices = &Renderer->Context.FrameIndex,
    };

    VkResult Error = vkQueuePresentKHR(Renderer->Context.Queue, &PresentInfo);
    if(Error == VK_ERROR_OUT_OF_DATE_KHR || Error == VK_SUBOPTIMAL_KHR)
    {
        Renderer->SwapChainRebuild = true;
        return;
    }
	
    CheckVkResult(Error);
    Renderer->Context.SemaphoreIndex = (Renderer->Context.SemaphoreIndex + 1) % Renderer->Context.SemaphoreCount;
}

bool EmberRendererInit(ember_renderer_t* Renderer, ember_window_t* Window)
{
    Renderer->Internal = EmberMemoryAllocateType<ember_renderer_vulkan_t, true>();

    ember_renderer_vulkan_t* VkRenderer = (ember_renderer_vulkan_t*)Renderer->Internal;
    CreateVulkanInstance(Renderer, Window);
    CreatePhysicalDevice(VkRenderer);
    SelectGraphicsQueueFamily(VkRenderer);
    CreateLogicalDevice(VkRenderer);
    CreateDescriptorPool(VkRenderer);
    CreateSurface(Renderer, Window);
    SetPresentMode(VkRenderer);

    u32 Width, Height;
    if(EmberPlatformGetWindowSize(Window, &Width, &Height))
    {
        CreateOrResizeSwapChain(VkRenderer, (int)Width, (int)Height);
    }
    
    return true;
}

void EmberRendererShutdown(ember_renderer_t* Renderer)
{
    ember_renderer_vulkan_t* VkRenderer = (ember_renderer_vulkan_t*)Renderer->Internal;

    CheckVkResult(vkDeviceWaitIdle(VkRenderer->Context.Device));
    
    DestroyFrames(VkRenderer);
    
    vkDestroySwapchainKHR(VkRenderer->Context.Device, VkRenderer->Context.Swapchain, VkRenderer->Context.Allocator);
    vkDestroySurfaceKHR(VkRenderer->Context.Instance, VkRenderer->Context.Surface, VkRenderer->Context.Allocator);
    
    vkDestroyDescriptorPool(VkRenderer->Context.Device, VkRenderer->Context.DescriptorPool, VkRenderer->Context.Allocator);
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    VK_GET_INSTANCE_PROC_ADDR(vkDestroyDebugReportCallbackEXT, VkRenderer->Context.Instance);
    vkDestroyDebugReportCallbackEXT(VkRenderer->Context.Instance, VkRenderer->Context.DebugReport, VkRenderer->Context.Allocator);
#endif
    vkDestroyDevice(VkRenderer->Context.Device, VkRenderer->Context.Allocator);
    vkDestroyInstance(VkRenderer->Context.Instance, VkRenderer->Context.Allocator);
}

void EmberRendererBeginFrame(ember_renderer_t* Renderer, ember_window_t* Window)
{
    RecreateSwapchainIfNecessary(Renderer, Window);
    ImGui_ImplVulkan_NewFrame();
}

void EmberRendererEndFrame(ember_renderer_t* Renderer)
{
    ImDrawData* MainDrawData = ImGui::GetDrawData();
    const bool IsMinimized = (MainDrawData->DisplaySize.x <= 0.0f || MainDrawData->DisplaySize.y <= 0.0f);
    if(!IsMinimized)
    {
        ember_renderer_vulkan_t* VkRenderer = (ember_renderer_vulkan_t*)Renderer->Internal;
        FrameRender(VkRenderer, MainDrawData);

        if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        
        FramePresent(VkRenderer);
    }
}

bool EmberRendererImGuiInit(ember_renderer_t* Renderer)
{
    EMBER_ASSERT(Renderer);
    
    ember_renderer_vulkan_t* VkRenderer = (ember_renderer_vulkan_t*)Renderer->Internal;
    EMBER_ASSERT(VkRenderer->MinImageCount >= 2);
    
    ImGui_ImplVulkan_InitInfo InitInfo = {
        .Instance = VkRenderer->Context.Instance,
        .PhysicalDevice = VkRenderer->Context.PhysicalDevice,
        .Device = VkRenderer->Context.Device,
        .QueueFamily = VkRenderer->Context.QueueFamily,
        .Queue = VkRenderer->Context.Queue,
        .DescriptorPool = VkRenderer->Context.DescriptorPool,
        .RenderPass = VkRenderer->Context.RenderPass,
        .MinImageCount = VkRenderer->MinImageCount,
        .ImageCount = VkRenderer->Context.ImageCount,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineCache = VkRenderer->Context.PipelineCache,
        .Subpass = 0,
        .Allocator = VkRenderer->Context.Allocator,
        .CheckVkResultFn = CheckVkResult
    };
    
    return ImGui_ImplVulkan_Init(&InitInfo);
}

void EmberRendererImGuiShutdown()
{
    ImGui_ImplVulkan_Shutdown();
}
#endif