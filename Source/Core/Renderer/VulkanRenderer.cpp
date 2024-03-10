#include "Core/Renderer/VulkanRenderer.h"
#include <Imgui/imgui_impl_sdl2.h>
#include <SDL/SDL_vulkan.h>
#include "Core/Window.h"

// TODO(HO): If we ever move away from SDL, we will need to remove the SDL properties
using namespace Ember;

VulkanRenderer::VulkanRenderer(Ember::Window* Window)
    : Window(Window)
    , VkContext()
{
}


bool VulkanRenderer::IsExtensionAvailable(const DynamicArray<VkExtensionProperties>& Properties, const char* Extension)
{
    for(const VkExtensionProperties& Property : Properties)
    {
        if(CStringUtil::StringCompare(Property.extensionName, Extension) == 0)
        {
            return true;
        }
    }

    return false;
}

VkPhysicalDevice VulkanRenderer::SelectPhysicalDevice() const
{
    u32 GPUCount;
    CheckVkResult(vkEnumeratePhysicalDevices(VkContext.Instance, &GPUCount, nullptr));
    EMBER_ASSERT(GPUCount > 0);

    DynamicArray<VkPhysicalDevice> GPUs(GPUCount);
    CheckVkResult(vkEnumeratePhysicalDevices(VkContext.Instance, &GPUCount, GPUs.GetData()));

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

bool VulkanRenderer::Initialize()
{
    DynamicArray<const char*> Extensions;
    u32 ExtensionsCount = 0;
    SDL_Vulkan_GetInstanceExtensions(Window->Get(), &ExtensionsCount, nullptr);
    Extensions.Resize(ExtensionsCount);
    SDL_Vulkan_GetInstanceExtensions(Window->Get(), &ExtensionsCount, Extensions.GetData());
    
    // Create Vulkan Instance
    {
        VkInstanceCreateInfo CreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
        };

        u32 PropertiesCount;
        DynamicArray<VkExtensionProperties> Properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &PropertiesCount, nullptr);
        Properties.Resize(PropertiesCount);
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
        CheckVkResult(vkCreateInstance(&CreateInfo, VkContext.Allocator, &VkContext.Instance));

#ifdef APP_USE_VULKAN_DEBUG_REPORT
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(VkContext.Instance, "vkCreateDebugReportCallbackEXT");
        EMBER_ASSERT(vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT DebugReportCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
            .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
            .pfnCallback = DebugReportFn,
            .pUserData = nullptr
        };
        CheckVkResult(vkCreateDebugReportCallbackEXT(VkContext.Instance, &DebugReportCreateInfo, VkContext.Allocator, &VkContext.DebugReport));
#endif
    }

    VkContext.PhysicalDevice = SelectPhysicalDevice();

    // Select Graphics Queue Family
    {
        u32 Count;
        vkGetPhysicalDeviceQueueFamilyProperties(VkContext.PhysicalDevice, &Count, nullptr);
        VkQueueFamilyProperties* Queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * Count);
        vkGetPhysicalDeviceQueueFamilyProperties(VkContext.PhysicalDevice, &Count, Queues);
        for(u32 i = 0; i < Count; ++i)
        {
            if(Queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                VkContext.QueueFamily = i;
                break;
            }
        }

        free(Queues);
        EMBER_ASSERT(VkContext.QueueFamily != (u32)-1);
    }

    // Create Logical Device
    {
        DynamicArray<const char*> DeviceExtensions;
        DeviceExtensions.Add("VK_KHR_swapchain");

        u32 PropertiesCount;
        DynamicArray<VkExtensionProperties> Properties;
        vkEnumerateDeviceExtensionProperties(VkContext.PhysicalDevice, nullptr, &PropertiesCount, nullptr);
        Properties.Resize(PropertiesCount);
        vkEnumerateDeviceExtensionProperties(VkContext.PhysicalDevice, nullptr, &PropertiesCount, Properties.GetData());
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
			if(IsExtensionAvailable(Properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
			{
				DeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
			}
#endif

        constexpr float QueuePriority[] = { 1.0f };
        VkDeviceQueueCreateInfo QueueInfo[1] = {};
        QueueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueInfo[0].queueFamilyIndex = VkContext.QueueFamily;
        QueueInfo[0].queueCount = 1;
        QueueInfo[0].pQueuePriorities = QueuePriority;

        VkDeviceCreateInfo CreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = ARRAY_SIZE(QueueInfo),
            .pQueueCreateInfos = QueueInfo,
            .enabledExtensionCount = (u32)DeviceExtensions.Size(),
            .ppEnabledExtensionNames = DeviceExtensions.GetData()
        };
        CheckVkResult(vkCreateDevice(VkContext.PhysicalDevice, &CreateInfo, VkContext.Allocator, &VkContext.Device));
        vkGetDeviceQueue(VkContext.Device, VkContext.QueueFamily, 0, &VkContext.Queue);
    }

    // Create Descriptor Pool
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
        CheckVkResult(vkCreateDescriptorPool(VkContext.Device, &PoolInfo, VkContext.Allocator, &VkContext.DescriptorPool));
    }

    {
        VkSurfaceKHR Surface;
        if(SDL_Vulkan_CreateSurface(Window->Get(), VkContext.Instance, &Surface) == 0)
        {
            EMBER_LOG(ELogCategory::Error, "Failed to create Vulkan Surface!");
            return false;
        }

        int Width, Height;
        SDL_GetWindowSize(Window->Get(), &Width, &Height);

        VkContext.Surface = Surface;

        VkBool32 Res;
        vkGetPhysicalDeviceSurfaceSupportKHR(VkContext.PhysicalDevice, VkContext.QueueFamily, VkContext.Surface, &Res);
        if(Res != VK_TRUE)
        {
            EMBER_LOG(Error, "[Vulkan]: No WSI Support on Physical Device 0");
            return false;
        }

        constexpr VkFormat RequestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        constexpr VkColorSpaceKHR RequestSurfaceColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkContext.SurfaceFormat = SelectSurfaceFormat(RequestSurfaceImageFormat, ARRAY_SIZE(RequestSurfaceImageFormat), RequestSurfaceColorSpace);

#ifdef APP_UNLIMITED_FRAME_RATE
        VkPresentModeKHR PresentModes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
        VkPresentModeKHR PresentModes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
        VkContext.PresentMode = SelectPresentMode(&PresentModes[0], ARRAY_SIZE(PresentModes));

        EMBER_ASSERT(MinImageCount >= 2);
        CreateOrResizeWindow(Width, Height);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& Io = ImGui::GetIO(); (void)Io;
        Io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        Io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForVulkan(Window->Get());
        ImGui_ImplVulkan_InitInfo InitInfo = {
            .Instance = VkContext.Instance,
            .PhysicalDevice = VkContext.PhysicalDevice,
            .Device = VkContext.Device,
            .QueueFamily = VkContext.QueueFamily,
            .Queue = VkContext.Queue,
            .DescriptorPool = VkContext.DescriptorPool,
            .RenderPass = VkContext.RenderPass,
            .MinImageCount = MinImageCount,
            .ImageCount = VkContext.ImageCount,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .PipelineCache = VkContext.PipelineCache,
            .Subpass = 0,
            .Allocator = VkContext.Allocator,
            .CheckVkResultFn = CheckVkResult
        };
        
        ImGui_ImplVulkan_Init(&InitInfo);
    }

    return true;
}

void VulkanRenderer::SetupVulkanWindow(VkSurfaceKHR Surface, int Width, int Height)
{

}

void VulkanRenderer::CreateOrResizeWindow(int Width, int Height)
{
    CreateSwapChain(Width, Height);
    CreateCommandBuffers();
}

u32 VulkanRenderer::GetMinImageCountFromPresentMode(VkPresentModeKHR PresentMode)
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

void VulkanRenderer::CreateSwapChain(int Width, int Height)
{
    // Create SwapChain
    VkSwapchainKHR OldSwapchain = VkContext.Swapchain;

    {
        VkContext.Swapchain = VK_NULL_HANDLE;
        CheckVkResult(vkDeviceWaitIdle(VkContext.Device));

        for(u32 i = 0; i < VkContext.ImageCount; ++i)
        {
            DestroyFrame(&VkContext.Frames[i]);
        }

        for(u32 i = 0; i < VkContext.SemaphoreCount; ++i)
        {
            DestroySemaphore(&VkContext.Semaphores[i]);
        }

        free(VkContext.Frames);
        free(VkContext.Semaphores);

        VkContext.Frames = nullptr;
        VkContext.Semaphores = nullptr;
        VkContext.ImageCount = 0;
        if(VkContext.RenderPass)
        {
            vkDestroyRenderPass(VkContext.Device, VkContext.RenderPass, VkContext.Allocator);
        }

        if(VkContext.Pipeline)
        {
            vkDestroyPipeline(VkContext.Device, VkContext.Pipeline, VkContext.Allocator);
        }

        if(MinImageCount == 0)
        {
            MinImageCount = GetMinImageCountFromPresentMode(VkContext.PresentMode);
        }
    
        VkSwapchainCreateInfoKHR Info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = VkContext.Surface,
            .minImageCount = MinImageCount,
            .imageFormat = VkContext.SurfaceFormat.format,
            .imageColorSpace = VkContext.SurfaceFormat.colorSpace,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VkContext.PresentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = OldSwapchain
        };

        VkSurfaceCapabilitiesKHR Capabilities;
        CheckVkResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkContext.PhysicalDevice, VkContext.Surface, &Capabilities));
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
            Info.imageExtent.width = VkContext.Width = Width;
            Info.imageExtent.height = VkContext.Height = Height;
        }
        else
        {
            Info.imageExtent.width = VkContext.Width = (int)Capabilities.currentExtent.width;
            Info.imageExtent.height = VkContext.Height = (int)Capabilities.currentExtent.height;
        }

        CheckVkResult(vkCreateSwapchainKHR(VkContext.Device, &Info, VkContext.Allocator, &VkContext.Swapchain));
        CheckVkResult(vkGetSwapchainImagesKHR(VkContext.Device, VkContext.Swapchain, &VkContext.ImageCount, nullptr));

        VkImage Backbuffers[16] = {};
        EMBER_ASSERT(VkContext.ImageCount >= MinImageCount);
        EMBER_ASSERT(VkContext.ImageCount < ARRAY_SIZE(Backbuffers));
        CheckVkResult(vkGetSwapchainImagesKHR(VkContext.Device, VkContext.Swapchain, &VkContext.ImageCount, Backbuffers));

        EMBER_ASSERT(VkContext.Frames == nullptr && VkContext.Semaphores == nullptr);
        VkContext.SemaphoreCount = (VkContext.ImageCount + 1);
        VkContext.Frames = EMBER_ALLOC(VkFrame, VkContext.ImageCount);
        VkContext.Semaphores = EMBER_ALLOC(VkSemaphores, VkContext.SemaphoreCount);
        EMBER_MEMZERO(VkContext.Frames, sizeof(VkContext.Frames[0]) * VkContext.ImageCount);
        EMBER_MEMZERO(VkContext.Semaphores, sizeof(VkContext.Semaphores[0]) * VkContext.SemaphoreCount);
        for(u32 i = 0; i < VkContext.ImageCount; ++i)
        {
            VkContext.Frames[i].Backbuffer = Backbuffers[i];
        }
    }

    if(OldSwapchain)
    {
        vkDestroySwapchainKHR(VkContext.Device, OldSwapchain, VkContext.Allocator);
    }

    // Create the RenderPass
    {
        if(VkContext.UseDynamicRendering == false)
        {
            VkAttachmentDescription Attachment = {
                .format = VkContext.SurfaceFormat.format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VkContext.ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
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
            CheckVkResult(vkCreateRenderPass(VkContext.Device, &Info, VkContext.Allocator, &VkContext.RenderPass));
        }
    }

    // Create Image Views
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
            .format = VkContext.SurfaceFormat.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A
            },
            .subresourceRange = ImageRange
        };

        for(u32 i = 0; i < VkContext.ImageCount; ++i)
        {
            VkFrame* Frame = &VkContext.Frames[i];
            Info.image = Frame->Backbuffer;
            CheckVkResult(vkCreateImageView(VkContext.Device, &Info, VkContext.Allocator, &Frame->BackbufferView));
        }
    }

    // Create Framebuffer
    {
        if(VkContext.UseDynamicRendering == false)
        {
            VkImageView Attachment[1];
            VkFramebufferCreateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = VkContext.RenderPass,
                .attachmentCount = 1,
                .pAttachments = Attachment,
                .width = (u32)VkContext.Width,
                .height = (u32)VkContext.Height,
                .layers = 1
            };

            for(u32 i = 0; i < VkContext.ImageCount; ++i)
            {
                VkFrame* Frame = &VkContext.Frames[i];
                Attachment[0] = Frame->BackbufferView;
                CheckVkResult(vkCreateFramebuffer(VkContext.Device, &Info, VkContext.Allocator, &Frame->Framebuffer));
            }
        }
    }
}

void VulkanRenderer::CreateCommandBuffers()
{
    EMBER_ASSERT(VkContext.PhysicalDevice != VK_NULL_HANDLE && VkContext.Device != VK_NULL_HANDLE);
    for(u32 i = 0; i < VkContext.ImageCount; ++i)
    {
        VkFrame* Frame = &VkContext.Frames[i];
        {
            VkCommandPoolCreateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = 0,
                .queueFamilyIndex = VkContext.QueueFamily
            };
            CheckVkResult(vkCreateCommandPool(VkContext.Device, &Info, VkContext.Allocator, &Frame->CommandPool));
        }
        {
            VkCommandBufferAllocateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = Frame->CommandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
            CheckVkResult(vkAllocateCommandBuffers(VkContext.Device, &Info, &Frame->CommandBuffer));
        }
        {
            VkFenceCreateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
            };
            CheckVkResult(vkCreateFence(VkContext.Device, &Info, VkContext.Allocator, &Frame->Fence));
        }
    }

    for(u32 i = 0; i < VkContext.SemaphoreCount; ++i)
    {
        VkSemaphores* Semaphores = &VkContext.Semaphores[i];
        {
            VkSemaphoreCreateInfo Info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
            };
            CheckVkResult(vkCreateSemaphore(VkContext.Device, &Info, VkContext.Allocator, &Semaphores->ImageAcquiredSemaphore));
            CheckVkResult(vkCreateSemaphore(VkContext.Device, &Info, VkContext.Allocator, &Semaphores->RenderCompleteSemaphore));
        }
    }
}

void VulkanRenderer::DestroyFrame(VkFrame* Frame) const
{
    vkDestroyFence(VkContext.Device, Frame->Fence, VkContext.Allocator);
    vkFreeCommandBuffers(VkContext.Device, Frame->CommandPool, 1, &Frame->CommandBuffer);
    vkDestroyCommandPool(VkContext.Device, Frame->CommandPool, VkContext.Allocator);
    Frame->Fence = VK_NULL_HANDLE;
    Frame->CommandBuffer = VK_NULL_HANDLE;
    Frame->CommandPool = VK_NULL_HANDLE;

    vkDestroyImageView(VkContext.Device, Frame->BackbufferView, VkContext.Allocator);
    vkDestroyFramebuffer(VkContext.Device, Frame->Framebuffer, VkContext.Allocator);
}

void VulkanRenderer::DestroySemaphore(VkSemaphores* Semaphores) const
{
    vkDestroySemaphore(VkContext.Device, Semaphores->ImageAcquiredSemaphore, VkContext.Allocator);
    vkDestroySemaphore(VkContext.Device, Semaphores->RenderCompleteSemaphore, VkContext.Allocator);
    Semaphores->ImageAcquiredSemaphore = Semaphores->RenderCompleteSemaphore = VK_NULL_HANDLE;
}

VkSurfaceFormatKHR VulkanRenderer::SelectSurfaceFormat(const VkFormat* RequestFormats, int RequestFormatsCount, VkColorSpaceKHR RequestColorSpace) const
{
    EMBER_ASSERT(RequestFormats != nullptr);
    EMBER_ASSERT(RequestFormatsCount > 0);

    u32 AvailableCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext.PhysicalDevice, VkContext.Surface, &AvailableCount, nullptr);
    DynamicArray<VkSurfaceFormatKHR> AvailableFormats(AvailableCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext.PhysicalDevice, VkContext.Surface, &AvailableCount, AvailableFormats.GetData());

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
        else
        {
            return AvailableFormats[0];
        }
    }
    else
    {
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
}

VkPresentModeKHR VulkanRenderer::SelectPresentMode(const VkPresentModeKHR* RequestModes, int RequestModesCount) const
{
    EMBER_ASSERT(RequestModes != nullptr);
    EMBER_ASSERT(RequestModesCount > 0);

    u32 AvailableCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(VkContext.PhysicalDevice, VkContext.Surface, &AvailableCount, nullptr);
    DynamicArray<VkPresentModeKHR> AvailableModes(AvailableCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(VkContext.PhysicalDevice, VkContext.Surface, &AvailableCount, AvailableModes.GetData());

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

void VulkanRenderer::Shutdown()
{
    CheckVkResult(vkDeviceWaitIdle(VkContext.Device));
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
    
    CheckVkResult(vkDeviceWaitIdle(VkContext.Device));

    for(u32 i = 0; i < VkContext.ImageCount; ++i)
    {
        DestroyFrame(&VkContext.Frames[i]);
    }

    for(u32 i = 0; i < VkContext.SemaphoreCount; ++i)
    {
        DestroySemaphore(&VkContext.Semaphores[i]);
    }

    free(VkContext.Frames);
    free(VkContext.Semaphores);

    VkContext.Frames = nullptr;
    VkContext.Semaphores = nullptr;

    vkDestroyPipeline(VkContext.Device, VkContext.Pipeline, VkContext.Allocator);
    vkDestroyRenderPass(VkContext.Device, VkContext.RenderPass, VkContext.Allocator);
    vkDestroySwapchainKHR(VkContext.Device, VkContext.Swapchain, VkContext.Allocator);
    vkDestroySurfaceKHR(VkContext.Instance, VkContext.Surface, VkContext.Allocator);
    
    vkDestroyDescriptorPool(VkContext.Device, VkContext.DescriptorPool, VkContext.Allocator);
#ifdef APP_USE_VULKAN_DEBUG_REPORT
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(VkContext.Instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(VkContext.Instance, VkContext.DebugReport, VkContext.Allocator);
#endif
    vkDestroyDevice(VkContext.Device, VkContext.Allocator);
    vkDestroyInstance(VkContext.Instance, VkContext.Allocator);
}

void VulkanRenderer::BeginFrame()
{
    RecreateSwapchainIfNecessary();
	
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    bool ShowDemoWindow = true;
    if(ShowDemoWindow)
    {
        ImGui::ShowDemoWindow(&ShowDemoWindow);
    }
}

void VulkanRenderer::EndFrame()
{
    ImGui::Render();
    ImDrawData* DrawData = ImGui::GetDrawData();
    const bool IsMinimized = (DrawData->DisplaySize.x <= 0.0f || DrawData->DisplaySize.y <= 0.0f);
    if(!IsMinimized)
    {
        FrameRender(DrawData);
        FramePresent();
    }
}

void VulkanRenderer::FrameRender(ImDrawData* DrawData)
{
    VkSemaphore ImageAcquiredSemaphore = VkContext.Semaphores[VkContext.SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore RenderCompleteSemaphore = VkContext.Semaphores[VkContext.SemaphoreIndex].RenderCompleteSemaphore;
    VkResult Error = vkAcquireNextImageKHR(VkContext.Device, VkContext.Swapchain, UINT64_MAX, ImageAcquiredSemaphore, VK_NULL_HANDLE, &VkContext.FrameIndex);
    if(Error == VK_ERROR_OUT_OF_DATE_KHR || Error == VK_SUBOPTIMAL_KHR)
    {
        SwapChainRebuild = true;
        return;
    }
    CheckVkResult(Error);

    VkFrame* Frame = &VkContext.Frames[VkContext.FrameIndex];
    {
        CheckVkResult(vkWaitForFences(VkContext.Device, 1, &Frame->Fence, VK_TRUE, UINT64_MAX));
        CheckVkResult(vkResetFences(VkContext.Device, 1, &Frame->Fence));
    }
    {
        CheckVkResult(vkResetCommandPool(VkContext.Device, Frame->CommandPool, 0));

        VkCommandBufferBeginInfo Info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        
        CheckVkResult(vkBeginCommandBuffer(Frame->CommandBuffer, &Info));
    }
    {
        VkRenderPassBeginInfo Info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = VkContext.RenderPass,
            .framebuffer = Frame->Framebuffer,
            .renderArea = {
                .extent = {
                    .width = (u32)VkContext.Width,
                    .height = (u32)VkContext.Height
                }
            },
            .clearValueCount = 1,
            .pClearValues = &VkContext.ClearValue
        };
        vkCmdBeginRenderPass(Frame->CommandBuffer, &Info, VK_SUBPASS_CONTENTS_INLINE);
    }

    ImGui_ImplVulkan_RenderDrawData(DrawData, Frame->CommandBuffer);

    vkCmdEndRenderPass(Frame->CommandBuffer);
    {
        VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo SubmitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &ImageAcquiredSemaphore,
            .pWaitDstStageMask = &WaitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &Frame->CommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &RenderCompleteSemaphore
        };

        CheckVkResult(vkEndCommandBuffer(Frame->CommandBuffer));
        CheckVkResult(vkQueueSubmit(VkContext.Queue, 1, &SubmitInfo, Frame->Fence));
    }
}

void VulkanRenderer::FramePresent()
{
    if(SwapChainRebuild)
    {
        return;
    }

    VkSemaphore RenderCompleteSemaphore = VkContext.Semaphores[VkContext.SemaphoreIndex].RenderCompleteSemaphore;

    VkPresentInfoKHR PresentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &RenderCompleteSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &VkContext.Swapchain,
        .pImageIndices = &VkContext.FrameIndex,
    };

    VkResult Error = vkQueuePresentKHR(VkContext.Queue, &PresentInfo);
    if(Error == VK_ERROR_OUT_OF_DATE_KHR || Error == VK_SUBOPTIMAL_KHR)
    {
        SwapChainRebuild = true;
        return;
    }
	
    CheckVkResult(Error);
    VkContext.SemaphoreIndex = (VkContext.SemaphoreIndex + 1) % VkContext.SemaphoreCount;
}

void VulkanRenderer::RecreateSwapchainIfNecessary()
{
    if(!SwapChainRebuild)
    {
        return;
    }
    
    int Width, Height;
    SDL_GetWindowSize(Window->Get(), &Width, &Height);
    if(Width > 0 && Height > 0)
    {
        ImGui_ImplVulkan_SetMinImageCount(MinImageCount);
        CreateOrResizeWindow(Width, Height);
        VkContext.FrameIndex = 0;
        SwapChainRebuild = false;
    }
}
