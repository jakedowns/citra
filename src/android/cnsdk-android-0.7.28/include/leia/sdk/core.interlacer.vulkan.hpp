/*
 * Copyright 2023 (c) Leia Inc.  All rights reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Leia Inc. and its suppliers, if any.  The
 * intellectual and technical concepts contained herein are
 * proprietary to Leia Inc. and its suppliers and may be covered
 * by U.S. and Foreign Patents, patents in process, and are
 * protected by trade secret or copyright law.  Dissemination of
 * this information or reproduction of this materials strictly
 * forbidden unless prior written permission is obtained from
 * Leia Inc.
 */
#ifndef CNSDK_LEIA_CORE_INTERLACER_VULKAN_HPP
#define CNSDK_LEIA_CORE_INTERLACER_VULKAN_HPP

#include "leia/sdk/core.interlacer.hpp"
#include "leia/sdk/core.interlacer.vulkan.h"

/*!
 * @addtogroup interlacer_vulkan
 * @{
 */

namespace leia {
namespace sdk {

/**
 * @brief C++ wrapper for Vulkan leia_interlacer
 */
class InterlacerVulkan : public Interlacer {
public:
    InterlacerVulkan(Core&                              core,
                     InterlacerInitConfiguration const& config,
                     VkDevice                           device,
                     VkPhysicalDevice                   physicalDevice,
                     VkFormatInt                        textureFormat,
                     VkFormatInt                        renderTargetFormat,
                     VkFormatInt                        depthStencilFormat,
                     int32_t                            maxInFlightFrameCount)
        : Interlacer(
              core.GetHandle(),
              leia_interlacer_vulkan_initialize(
                  core.GetHandle(), config.GetHandle(), device, physicalDevice, textureFormat, renderTargetFormat, depthStencilFormat, maxInFlightFrameCount),
              true)
    {
    }
    InterlacerVulkan(InterlacerVulkan const&) = delete;
    InterlacerVulkan& operator=(InterlacerVulkan const&) = delete;
    InterlacerVulkan(InterlacerVulkan&&) = delete;
    InterlacerVulkan& operator=(InterlacerVulkan&&) = delete;

    void DoPostProcess(int32_t       width,
                       int32_t       height,
                       bool          blendedInterlace,
                       VkFramebuffer frameBuffer,
                       VkImage       colorImage,
                       VkImage       depthImage,
                       VkSemaphore   imageAvailableSemaphore,
                       VkSemaphore   renderFinishedSemaphore,
                       int32_t       currentFrame)
    {
        leia_interlacer_vulkan_do_post_process(
            _interlacer, width, height, blendedInterlace, frameBuffer, colorImage, depthImage, imageAvailableSemaphore, renderFinishedSemaphore, currentFrame);
    }

    void DoPostProcessPicture(int32_t       width,
                              int32_t       height,
                              VkImage       pictureResource,
                              VkFramebuffer frameBuffer,
                              VkImage       colorImage,
                              VkImage       depthImage,
                              VkSemaphore   imageAvailableSemaphore,
                              VkSemaphore   renderFinishedSemaphore,
                              int32_t       currentFrame)
    {
        leia_interlacer_vulkan_do_post_process_picture(
            _interlacer, width, height, pictureResource, frameBuffer, colorImage, depthImage, imageAvailableSemaphore, renderFinishedSemaphore, currentFrame);
    }

    VkImage GetDepthStencilImage(int32_t view, VkImageLayoutInt* layout = nullptr)
    {
        return leia_interlacer_vulkan_get_depth_stencil_image(_interlacer, view, layout);
    }

    VkFramebuffer GetFramebuffer(int32_t view)
    {
        return leia_interlacer_vulkan_get_framebuffer(_interlacer, view);
    }

    VkImage GetRenderTargetImage(int32_t view, VkImageLayoutInt* layout = nullptr)
    {
        return leia_interlacer_vulkan_get_render_target_image(_interlacer, view, layout);
    }

    LEIA_NODISCARD
    int32_t GetRenderTargetImageHeight()
    {
        return leia_interlacer_vulkan_get_render_target_image_height(_interlacer);
    }

    LEIA_NODISCARD
    int32_t GetRenderTargetImageWidth()
    {
        return leia_interlacer_vulkan_get_render_target_image_width(_interlacer);
    }

    void GetRenderTargetView(int32_t view, VkImageView* handle)
    {
        return leia_interlacer_vulkan_get_render_target_view(_interlacer, view, handle);
    }

    void SetInterlaceViewTextureAtlas(VkImage texture, VkImageView textureView)
    {
        leia_interlacer_vulkan_set_interlace_view_texture_atlas(_interlacer, texture, textureView);
    }

    void SetViewForTextureArray(int32_t viewIndex, VkImageView imageView)
    {
        leia_interlacer_vulkan_set_view_for_texture_array(_interlacer, viewIndex, imageView);
    }

    void SetViewTextureId(VkImage view)
    {
        leia_interlacer_vulkan_set_view_texture_id(_interlacer, view);
    }

    InterlacerVulkan(leia_interlacer* nonOwningInterlacer) : Interlacer(nullptr, nonOwningInterlacer, false)
    {
    }
};

LEIA_NODISCARD
inline InterlacerVulkan* AsVulkan(Interlacer* interlacer)
{
    if (interlacer && interlacer->GetGraphicsAPI() == LEIA_GRAPHICS_API_VULKAN)
    {
        return static_cast<InterlacerVulkan*>(interlacer);
    }
    return nullptr;
}

} // namespace sdk
} // namespace leia

/*!
 * @}
 */

#endif // CNSDK_LEIA_CORE_INTERLACER_VULKAN_HPP