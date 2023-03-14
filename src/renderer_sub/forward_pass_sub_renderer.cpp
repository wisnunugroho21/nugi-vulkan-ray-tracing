#include "forward_pass_sub_renderer.hpp"

#include <assert.h>
#include <array>

namespace nugiEngine {
  EngineForwardPassSubRenderer::EngineForwardPassSubRenderer(EngineDevice &device, int imageCount, int width, int height) 
    : device{device}, width{width}, height{height}
  {
    this->createAlbedoResources(imageCount);
    this->createNormalResources(imageCount);
    this->createDepthResources(imageCount);
    this->createRenderPass(imageCount);
  }

  std::vector<VkDescriptorImageInfo> EngineForwardPassSubRenderer::getNormalInfoResources() {
    std::vector<VkDescriptorImageInfo> descInfos{};
    for (auto &&normalInfo : this->normalResources) {
      descInfos.emplace_back(normalInfo->getDescriptorInfo(VK_IMAGE_LAYOUT_GENERAL));
    }

    return descInfos;
  }

  std::vector<VkDescriptorImageInfo> EngineForwardPassSubRenderer::getAlbedoInfoResources() {
    std::vector<VkDescriptorImageInfo> descInfos{};
    for (auto &&albedoInfo : this->albedoResources) {
      descInfos.emplace_back(albedoInfo->getDescriptorInfo(VK_IMAGE_LAYOUT_GENERAL));
    }

    return descInfos;
  }

  void EngineForwardPassSubRenderer::createNormalResources(int imageCount) {
    this->normalResources.clear();

    for (int i = 0; i < imageCount; i++) {
      auto normalResource = std::make_shared<EngineImage>(
        this->device, this->width, this->height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
      );

      this->normalResources.push_back(normalResource);
    }
  }

  void EngineForwardPassSubRenderer::createAlbedoResources(int imageCount) {
    this->albedoResources.clear();

    for (int i = 0; i < imageCount; i++) {
      auto colorImage = std::make_shared<EngineImage>(
        this->device, this->width, this->height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT
      );

      this->albedoResources.push_back(colorImage);
    }
  }

  void EngineForwardPassSubRenderer::createDepthResources(int imageCount) {
    VkFormat depthFormat = this->findDepthFormat();

    this->depthImages.clear();

    for (int i = 0; i < imageCount; i++) {
      auto depthImage = std::make_shared<EngineImage>(
        this->device, this->width, this->height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
      );

      this->depthImages.push_back(depthImage);
    }
  }

  void EngineForwardPassSubRenderer::createRenderPass(int imageCount) {
    VkAttachmentDescription albedoAttachment{};
    albedoAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    albedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    albedoAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    albedoAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    albedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    albedoAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference albedoAttachmentRef = {};
    albedoAttachmentRef.attachment = 0;
    albedoAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription normalAttachment{};
    normalAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference normalAttachmentRef = {};
    normalAttachmentRef.attachment = 1;
    normalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentReference> colorAttachmentRefs = { albedoAttachmentRef, normalAttachmentRef };

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = this->findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments = colorAttachmentRefs.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency colorDependency{};
    colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    colorDependency.srcAccessMask = 0;
    colorDependency.dstSubpass = 0;
    colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depthDependency{};
    depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.srcAccessMask = 0;
    depthDependency.dstSubpass = 0;
    depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency outDependency{};
    outDependency.srcSubpass = 0;
    outDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    outDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    outDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    outDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    outDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    EngineRenderPass::Builder renderPassBuilder = EngineRenderPass::Builder(this->device, this->width, this->height)
			.addAttachments(albedoAttachment)
      .addAttachments(normalAttachment)
			.addAttachments(depthAttachment)
			.addSubpass(subpass)
			.addDependency(colorDependency)
			.addDependency(depthDependency);

    for (int i = 0; i < imageCount; i++) {
			renderPassBuilder.addViewImages({
        this->albedoResources[i]->getImageView(),
        this->normalResources[i]->getImageView(),
        this->depthImages[i]->getImageView(),
      });
    }

		this->renderPass = renderPassBuilder.build();
  }

  VkFormat EngineForwardPassSubRenderer::findDepthFormat() {
    return this->device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  void EngineForwardPassSubRenderer::beginRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer, int currentImageIndex) {
		VkRenderPassBeginInfo renderBeginInfo{};
		renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderBeginInfo.renderPass = this->getRenderPass()->getRenderPass();
		renderBeginInfo.framebuffer = this->getRenderPass()->getFramebuffers(currentImageIndex);

		renderBeginInfo.renderArea.offset = { 0, 0 };
		renderBeginInfo.renderArea.extent = { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height) };

		std::array<VkClearValue, 3> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[2].depthStencil = { 1.0f, 0 };

		renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer->getCommandBuffer(), &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(this->width);
		viewport.height = static_cast<float>(this->height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{{0, 0}, { static_cast<uint32_t>(this->width), static_cast<uint32_t>(this->height) }};
		vkCmdSetViewport(commandBuffer->getCommandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer->getCommandBuffer(), 0, 1, &scissor);
	}

	void EngineForwardPassSubRenderer::endRenderPass(std::shared_ptr<EngineCommandBuffer> commandBuffer) {
		vkCmdEndRenderPass(commandBuffer->getCommandBuffer());
	}

  void EngineForwardPassSubRenderer::transferFrame(std::shared_ptr<EngineCommandBuffer> commandBuffer, uint32_t imageIndex) {
    this->albedoResources[imageIndex]->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, 
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
      commandBuffer);

    this->normalResources[imageIndex]->transitionImageLayout(VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, 
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 
      VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
      commandBuffer);
  }
} // namespace nugiEngine
