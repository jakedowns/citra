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
#ifndef CNSDK_LEIA_SDK_CORE_INTERLACER_D3D11_H
#define CNSDK_LEIA_SDK_CORE_INTERLACER_D3D11_H

#include "leia/sdk/core.interlacer.h"

#if !defined(LEIA_USE_DIRECTX)
#    error "CNSDK is not compiled with D3D11 support"
#endif

struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

BEGIN_CAPI_DECL

/*!
 * @defgroup interlacer_d3d11 D3D11 Interlacer
 * @ingroup interlacer
 *
 * @brief D3D11 interlacing on Leia devices.
 */

/*!
 * @addtogroup interlacer_d3d11
 * @{
 */

/**
 * @brief Initialize interlacer for usage with Direct3D 11.
 *
 * If your application uses Direct3D 11, use this method to initialize.
 *
 * @param[in] context Pointer to your ID3D11DeviceContext object.
 *
 * @relates leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_interlacer* leia_interlacer_d3d11_initialize(struct leia_core*                                core,
                                                         struct leia_interlacer_init_configuration const* config,
                                                         struct ID3D11DeviceContext*                      context);

/**
 * @brief Perform Direct3D 11 graphics rendering.
 *
 * Use this method to perform all graphics rendering - interlacing, sharpening, and GUI display.
 *
 * @param[in] width Width of the render-target.
 * @param[in] height Height of the render-target.
 * @param[in] blendedInterlace Whether to blend the graphics output onto the render-target.
 * @param[in] resultTexture Pointer to destination render-target view.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d11_do_post_process(
    struct leia_interlacer*, int32_t width, int32_t height, leia_bool blendedInterlace, struct ID3D11RenderTargetView* resultTexture);

/**
 * @brief Perform Direct3D 11 graphics rendering for picture content.
 *
 * Use this method to perform all graphics rendering - interlacing, sharpening, and GUI display. This method
 * is similar to DoPostProcess() but allows you to pass in the views to interlace and the destination render-target.
 *
 * @param[in] width Width of the render-target.
 * @param[in] height Height of the render-target.
 * @param[in] pictureShaderResourceView The shader resource view for the views texture.
 * @param[in] renderTargetView Pointer to destination render-target view.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d11_do_post_process_picture(struct leia_interlacer*,
                                                   int32_t                          width,
                                                   int32_t                          height,
                                                   struct ID3D11ShaderResourceView* pictureShaderResourceView,
                                                   struct ID3D11RenderTargetView*   renderTargetView);

/**
 * @brief Get Direct3D 11 depth-stencil view.
 *
 * Use this method if you need to retrieve the Direct3D 11 depth-stencil view for a specific view index.
 *
 * @param[in] view Index of the view.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
struct ID3D11DepthStencilView* leia_interlacer_d3d11_get_depth_stencil_view(struct leia_interlacer*, int32_t view);

/**
 * @brief Get Direct3D 11 render-target view.
 *
 * Use this method if you need to retrieve the Direct3D 11 render-target view for a specific view index.
 *
 * @param[in] view Index of the view.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
struct ID3D11RenderTargetView* leia_interlacer_d3d11_get_render_target_view(struct leia_interlacer*, int32_t view);

/**
 * @brief Set Direct3D 11 view texture atlas.
 *
 * If your application is managing views, this method allows you to specify your texture atlas
 * containing all views.
 *
 * @param[in] texture The shader resource view for the texture atlas containing all views.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d11_set_interlace_view_texture_atlas(struct leia_interlacer*, struct ID3D11ShaderResourceView* texture);

/**
 * @brief Set Direct3D 11 individual view texture.
 *
 * If your application is managing views, this method allows you to specify the texture
 * for a specific view index.
 *
 * @param[in] viewIndex The view index.
 * @param[in] renderTarget The individual texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d11_set_view_for_texture_array(struct leia_interlacer*, int32_t viewIndex, struct ID3D11RenderTargetView* renderTarget);

/**
 * @brief Set the Direct3D 11 view texture for stereo picture or video content.
 *
 * If you are using DoPostProcessPicture() or DoPostProcessVideo(), you can use this method to specify
 * the input views texture rather than passing it to the PostProcess methods directly.
 *
 * @param[in] view Shader resource view for the stereo views texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_d3d11_set_view_texture_id(struct leia_interlacer*, struct ID3D11ShaderResourceView* view);

/*!
 * @}
 */

END_CAPI_DECL

#endif // CNSDK_LEIA_SDK_CORE_INTERLACER_D3D11_H
