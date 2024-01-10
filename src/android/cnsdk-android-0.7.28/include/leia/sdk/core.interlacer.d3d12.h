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
#ifndef CNSDK_LEIA_SDK_CORE_INTERLACER_D3D12_H
#define CNSDK_LEIA_SDK_CORE_INTERLACER_D3D12_H

#include "leia/sdk/core.interlacer.h"

#if !defined(LEIA_USE_DIRECTX12)
#    error "CNSDK is not compiled with D3D12 support"
#endif

struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12Resource;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
typedef enum D3D12_RESOURCE_STATES;

BEGIN_CAPI_DECL

/*!
 * @defgroup interlacer_d3d12 D3D12 Interlacer
 * @ingroup interlacer
 *
 * @brief D3D12 interlacing on Leia devices.
 */

/*!
 * @addtogroup interlacer_d3d12
 * @{
 */

/**
 * @brief Initialize interlacer for usage with Direct3D 12.
 *
 * If your application uses Direct3D 12, use this method to initialize.
 *
 * @param[in] device Direct3D 12 device.
 * @param[in] commandQueue Command queue to use.
 *
 * @relates leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_interlacer* leia_interlacer_d3d12_initialize(struct leia_core*                                core,
                                                         struct leia_interlacer_init_configuration const* config,
                                                         struct ID3D12Device*                             device,
                                                         struct ID3D12CommandQueue*                       commandQueue);

/**
 * @brief Perform Direct3D 11 graphics rendering.
 *
 * Use this method to perform all graphics rendering - interlacing, sharpening, and GUI display.
 *
 * @param[in] width Width of the render-target.
 * @param[in] height Height of the render-target.
 * @param[in] blendedInterlace Whether to blend the graphics output onto the render-target.
 * @param[in] renderTarget Destination render-target for interlaced output.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d12_do_post_process(
    struct leia_interlacer*, int32_t width, int32_t height, leia_bool blendedInterlace, struct ID3D12Resource* renderTarget);

/**
 * @brief Perform Direct3D 12 graphics rendering for picture content.
 *
 * Use this method to perform all graphics rendering - interlacing, sharpening, and GUI display. This method
 * is similar to DoPostProcess() but allows you to pass in the views to interlace and the destination render-target.
 *
 * @param[in] width Width of the render-target.
 * @param[in] height Height of the render-target.
 * @param[in] pictureResource Views texture atlas.
 * @param[in] renderTarget Destination render-target for interlaced output.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d12_do_post_process_picture(
    struct leia_interlacer*, int32_t width, int32_t height, struct ID3D12Resource* pictureResource, struct ID3D12Resource* renderTarget);

/**
 * @brief Get Direct3D 12 depth-stencil resource.
 *
 * Use this method if you need to retrieve the Direct3D 12 depth-stencil resource for a specific view index.
 *
 * @param[in] view Index of the view.
 * @param[out] state State of resource.
 * @return Resource for specified view.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
struct ID3D12Resource* leia_interlacer_d3d12_get_depth_stencil_resource(struct leia_interlacer*, int32_t view, enum D3D12_RESOURCE_STATES* state);

/**
 * @brief Get Direct3D 12 depth-stencil view.
 *
 * Use this method if you need to retrieve the Direct3D 12 depth-stencil view for a specific view index.
 *
 * @param[in] view Index of the view.
 * @param[out] handle Descriptor for the specified view.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d12_get_depth_stencil_view(struct leia_interlacer*, int32_t view, struct CD3DX12_CPU_DESCRIPTOR_HANDLE* handle);

/**
 * @brief Get Direct3D 12 render-target resource.
 *
 * Use this method if you need to retrieve the Direct3D 12 render-target resource for a specific view index.
 *
 * @param[in] view Index of the view.
 * @param[out] state State of resource.
 * @return Resource for specified view.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
struct ID3D12Resource* leia_interlacer_d3d12_get_render_target_resource(struct leia_interlacer*, int32_t view, enum D3D12_RESOURCE_STATES* state);

/**
 * @brief Get Direct3D 12 render-target view.
 *
 * Use this method if you need to retrieve the Direct3D 12 render-target resource for a specific view index.
 *
 * @param[in] view Index of the view.
 * @param[out] handle Descriptor for the specified view.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d12_get_render_target_view(struct leia_interlacer*, int32_t view, struct CD3DX12_CPU_DESCRIPTOR_HANDLE* handle);

/**
 * @brief Set Direct3D 12 view texture atlas.
 *
 * If your application is managing views, this method allows you to specify your texture atlas
 * containing all views.
 *
 * @param[in] texture The resource containing all views.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d12_set_interlace_view_texture_atlas(struct leia_interlacer*, struct ID3D12Resource* texture);

/**
 * @brief Set the Direct3D 12 view texture for stereo picture or video content.
 *
 * If you are using DoPostProcessPicture() or DoPostProcessVideo(), you can use this method to specify
 * the input views texture rather than passing it to the PostProcess methods directly.
 *
 * @param[in] view Resource for the stereo views texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d12_set_view_texture_id(struct leia_interlacer*, struct ID3D12Resource* view);

/*!
 * @}
 */

END_CAPI_DECL

#endif // CNSDK_LEIA_SDK_CORE_INTERLACER_D3D12_H
