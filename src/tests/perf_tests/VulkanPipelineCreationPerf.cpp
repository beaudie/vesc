//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanPipelineCreationPerf:
//   Performance benchmark for the Vulkan Pipeline creation.

#include "ANGLEPerfTest.h"

#include "src/libANGLE/BlobCache.h"
#include "src/libANGLE/Caps.h"
#include "src/libANGLE/renderer/FormatID_autogen.h"
#include "src/libANGLE/renderer/vulkan/GlslangWrapper.h"
#include "src/libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "src/libANGLE/renderer/vulkan/vk_utils.h"

#include "test_utils/third_party/vulkan_command_buffer_utils.h"

using namespace rx;

namespace
{

struct VulkanPipelineCreationTestParams
{
    std::string suffix;
    uint32_t frames;

    uint32_t numOfTransition;
    bool randomTransition;
    bool creationOnly;
    bool useCache;
};

enum TransitionOp : int32_t
{
    Trans_VertexInput   = 1 << 0,
    Trans_InputAssembly = 1 << 1,
    Trans_Tessellation  = 1 << 2,
    Trans_Viewport      = 1 << 3,
    Trans_Rasterization = 1 << 4,
    Trans_Multisample   = 1 << 5,
    Trans_DepthStencil  = 1 << 6,
    Trans_ColorBlend    = 1 << 7,

    Trans_NUM_OPs = 8,
    Trans_ALL_OPs = ~(1 << Trans_NUM_OPs),

    // we don't change ShaderModule, because ANGLE is not tracking
    // Shader related states in pipeline description
};

static const std::vector<int32_t> kFixedTransitions{
    Trans_ALL_OPs,
    Trans_VertexInput,
    Trans_Rasterization | Trans_DepthStencil,
    Trans_ColorBlend,
    Trans_Rasterization | Trans_DepthStencil,
    Trans_ColorBlend | Trans_Multisample,
    Trans_DepthStencil,
    Trans_VertexInput | Trans_DepthStencil,
};

class ContextVkEmulator : public vk::Context
{
  public:
    ContextVkEmulator() : vk::Context(nullptr), mDevice(VK_NULL_HANDLE) {}
    ~ContextVkEmulator() {}

    virtual void handleError(VkResult result,
                             const char *file,
                             const char *function,
                             unsigned int line)
    {
        std::cout << result << " " << file << " " << function << " " << line << std::endl;
    }

    void setDevice(VkDevice device) { mDevice = device; }
    virtual VkDevice getDevice() const { return mDevice; }

  private:
    VkDevice mDevice;
};

class VulkanPipelineCreationPerf
    : public ANGLEPerfTest,
      public ::testing::WithParamInterface<VulkanPipelineCreationTestParams>
{
  public:
    VulkanPipelineCreationPerf();
    ~VulkanPipelineCreationPerf();

    void SetUp() override;

    void step() override;

  private:
    void initPipelineCache();
    void initRenderPass();
    void initPipelineLayout();
    void initShaderModules();

    struct sample_info mInfo = {};
    std::string mSampleTitle;

    ContextVkEmulator mContext;

    vk::PipelineCache mCache;
    angle::SizedMRUCache<egl::BlobCache::Key,
                         std::pair<angle::MemoryBuffer, egl::BlobCache::CacheSource>>
        mBlobCache;

    vk::RenderPassDesc mRenderPassDesc;
    vk::RenderPass mRenderPass;
    vk::PipelineLayout mPipelineLayout;
    vk::ShaderModule mVertexShader;
    vk::ShaderModule mFragmentShader;

    vk::PipelineHelper *mGraphicsPipeline;
    GraphicsPipelineCache mPipelineCache;
    std::vector<vk::GraphicsPipelineDesc> mPipelineDescs;
};

VulkanPipelineCreationPerf::VulkanPipelineCreationPerf()
    : ANGLEPerfTest("VulkanPipelineCreationPerf", GetParam().suffix, GetParam().frames),
      mBlobCache(256)
{
    mInfo        = {};
    mSampleTitle = "Draw Textured Cube";
}

VulkanPipelineCreationPerf::~VulkanPipelineCreationPerf() {}

void VulkanPipelineCreationPerf::initPipelineCache()
{
    // TODO
    const std::pair<angle::MemoryBuffer, egl::BlobCache::CacheSource> *entry = nullptr;
    // bool result = mBlobCache.get(key, &entry);

    egl::BlobCache::Value initialData =
        egl::BlobCache::Value(entry->first.data(), entry->first.size());
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};

    pipelineCacheCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCreateInfo.flags           = 0;
    pipelineCacheCreateInfo.initialDataSize = initialData.size();
    pipelineCacheCreateInfo.pInitialData    = initialData.data();
    mCache.init(mContext.getDevice(), pipelineCacheCreateInfo);
}

void VulkanPipelineCreationPerf::initRenderPass()
{
    // hardcode format, D32_FLOAT, B8G8R8A8_UNORM
    mRenderPassDesc.setSamples(static_cast<GLuint>(NUM_SAMPLES));
    mRenderPassDesc.packColorAttachment(0, angle::FormatID::B8G8R8A8_UNORM);
    mRenderPassDesc.packDepthStencilAttachment(angle::FormatID::D32_FLOAT);

    VkAttachmentDescription attachments[2];
    attachments[0].format         = VK_FORMAT_B8G8R8A8_UNORM;
    attachments[0].samples        = NUM_SAMPLES;
    attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].flags          = 0;

    attachments[1].format         = VK_FORMAT_D32_SFLOAT;
    attachments[1].samples        = NUM_SAMPLES;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].flags          = 0;

    VkAttachmentReference color_reference = {};
    color_reference.attachment            = 0;
    color_reference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment            = 1;
    depth_reference.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass    = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags                   = 0;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = nullptr;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_reference;
    subpass.pResolveAttachments     = nullptr;
    subpass.pDepthStencilAttachment = &depth_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = nullptr;

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext                  = nullptr;
    rp_info.attachmentCount        = 2;
    rp_info.pAttachments           = attachments;
    rp_info.subpassCount           = 1;
    rp_info.pSubpasses             = &subpass;
    rp_info.dependencyCount        = 0;
    rp_info.pDependencies          = nullptr;

    mRenderPass.init(mContext.getDevice(), rp_info);
}

void VulkanPipelineCreationPerf::initPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext                  = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount         = 0;
    pipelineLayoutCreateInfo.pSetLayouts            = nullptr;

    mPipelineLayout.init(mContext.getDevice(), pipelineLayoutCreateInfo);
}

void VulkanPipelineCreationPerf::initShaderModules()
{
    gl::ShaderMap<std::string> glslShaders;
    glslShaders[gl::ShaderType::Vertex] =
        "#version 450\n"
        "void main() { gl_Position = vec4(1.0, 0.0, 0.0, 1.0);}";
    glslShaders[gl::ShaderType::Fragment] =
        "#version 450\n"
        "layout(location = 0) out vec4 outColor;void main() { outColor = vec4(1.0, 0.0, 0.0, "
        "1.0);}";

    gl::ShaderMap<std::vector<unsigned int>> spvShaders;

    gl::Version version{3, 0};
    gl::Extensions exts;
    gl::Caps cap = GenerateMinimumCaps(version, exts);

    GlslangWrapper::Initialize();
    angle::Result res =
        GlslangWrapper::GetShaderCode(&mContext, cap, false, glslShaders, &spvShaders);
    EXPECT_EQ(res, angle::Result::Continue);
    GlslangWrapper::Release();

    VkShaderModuleCreateInfo vertModuleCreateInfo;
    vertModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertModuleCreateInfo.pNext = nullptr;
    vertModuleCreateInfo.flags = 0;
    vertModuleCreateInfo.codeSize =
        spvShaders[gl::ShaderType::Vertex].size() * sizeof(unsigned int);
    vertModuleCreateInfo.pCode = spvShaders[gl::ShaderType::Vertex].data();
    mVertexShader.init(mContext.getDevice(), vertModuleCreateInfo);

    VkShaderModuleCreateInfo fragModuleCreateInfo;
    fragModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragModuleCreateInfo.pNext = nullptr;
    fragModuleCreateInfo.flags = 0;
    fragModuleCreateInfo.codeSize =
        spvShaders[gl::ShaderType::Fragment].size() * sizeof(unsigned int);
    fragModuleCreateInfo.pCode = spvShaders[gl::ShaderType::Fragment].data();
    mFragmentShader.init(mContext.getDevice(), vertModuleCreateInfo);
}

void VulkanPipelineCreationPerf::SetUp()
{
    init_global_layer_properties(mInfo);
    init_instance_extension_names(mInfo);
    init_device_extension_names(mInfo);
    init_instance(mInfo, mSampleTitle.c_str());
    init_enumerate_device(mInfo);
    init_window_size(mInfo, 500, 500);
    init_connection(mInfo);
    init_window(mInfo);
    init_swapchain_extension(mInfo);
    init_device(mInfo);

    mContext.setDevice(mInfo.device);

    initRenderPass();
    initPipelineLayout();
    initShaderModules();
    if (GetParam().useCache)
    {
        initPipelineCache();
    }

    vk::GraphicsPipelineDesc initialDesc;
    const vk::GraphicsPipelineDesc *descOut;
    initialDesc.setRenderPassDesc(mRenderPassDesc);
    angle::Result res =
        mPipelineCache.getPipeline(&mContext, mCache, mRenderPass, mPipelineLayout,
                                   gl::AttributesMask(), gl::ComponentTypeMask(), &mVertexShader,
                                   &mFragmentShader, initialDesc, &descOut, &mGraphicsPipeline);
    EXPECT_EQ(res, angle::Result::Continue);

    std::vector<int32_t> transitions;
    if (GetParam().randomTransition)
    {
        // TODO
    }
    else
    {
        transitions = kFixedTransitions;
    }

    vk::GraphicsPipelineDesc writingDesc;
    vk::GraphicsPipelineDesc transitionTmp;
    writingDesc.setRenderPassDesc(mRenderPassDesc);
    // for (const TransitionOp op : transitions)
    // {

    //     // auto func =
    // }
}

void VulkanPipelineCreationPerf::step() {}

}  // anonymous namespace

TEST_F(VulkanPipelineCreationPerf, Run)
{
    run();
}

VulkanPipelineCreationTestParams defaultParams()
{
    return VulkanPipelineCreationTestParams();
}

INSTANTIATE_TEST_SUITE_P(, VulkanPipelineCreationPerf, ::testing::Values(defaultParams()));
