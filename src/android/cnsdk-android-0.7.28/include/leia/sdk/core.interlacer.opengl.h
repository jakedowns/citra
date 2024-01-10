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
#ifndef CNSDK_LEIA_SDK_CORE_INTERLACER_OPENGL_H
#define CNSDK_LEIA_SDK_CORE_INTERLACER_OPENGL_H

#include "leia/sdk/core.interlacer.h"

#if !defined(LEIA_USE_OPENGL)
#    error "CNSDK is not compiled with OpenGL support"
#endif

BEGIN_CAPI_DECL

/*!
 * @defgroup interlacer_opengl OpenGL Interlacer
 * @ingroup interlacer
 *
 * @brief OpenGL interlacing on Leia devices.
 */

/*!
 * @addtogroup interlacer_opengl
 * @{
 */

/**
 * @brief Initialize interlacer for usage with OpenGL.
 *
 * If your application uses OpenGL or OpenGL-ES, use this method to initialize.
 *
 * @param[in] context OpenGL context.
 *
 * @relates leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
struct leia_interlacer* leia_interlacer_opengl_initialize(struct leia_core* core, struct leia_interlacer_init_configuration const* config, void* context);

/**
 * @brief Perform OpenGL graphics rendering.
 *
 * Use this method to perform all graphics rendering - interlacing, sharpening, and GUI display.
 *
 * @param[in] width Width of the render-target.
 * @param[in] height Height of the render-target.
 * @param[in] blendedInterlace Whether to blend the graphics output onto the render-target.
 * @param[in] outputRenderTarget Name of OpenGL render-target to use, or zero for the frame-buffer.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_do_post_process(struct leia_interlacer*, int32_t width, int32_t height, leia_bool blendedInterlace, uint32_t outputRenderTarget);

/**
 * @brief Perform OpenGL graphics rendering for texture content.
 *
 * Use this method to perform all graphics rendering - interlacing, sharpening, and GUI display. This method
 * is similar to leia_interlacer_opengl_do_post_process() but allows you to pass in the views to interlace.
 *
 * @param[in] width Width of the render-target.
 * @param[in] height Height of the render-target.
 * @param[in] views Name of OpenGL views texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_do_post_process_picture(struct leia_interlacer*, int32_t width, int32_t height, uint32_t views);

/**
 * @brief Perform OpenGL graphics rendering for video content.
 *
 * Use this method to perform all graphics rendering - interlacing, sharpening, and GUI display. This method
 * is similar to leia_interlacer_opengl_do_post_process() but allows you to pass in the views to interlace.
 *
 * @param[in] width Width of the render-target.
 * @param[in] height Height of the render-target.
 * @param[in] views Name of OpenGL views texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_do_post_process_video(struct leia_interlacer*, int32_t width, int32_t height, uint32_t views);

/**
 * @brief Get the OpenGL render-target name for a specified view.
 *
 * To get the name of a render-target for a specified view, use this method. If the CNSDK is managing views,
 * call this method to obtain the native OpenGL render-target name to render into.
 *
 * @param[in] view Index of the view.
 * @return OpenGL render-target name for the specified view.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
uint32_t leia_interlacer_opengl_get_render_target_for_view(struct leia_interlacer*, int32_t view);

/**
 * @brief Get the OpenGL render-target name and viewport information for a specified view.
 *
 * To get the name of a render-target and viewport information for a specified view, use this method. If the CNSDK is managing views,
 * call this method to obtain the native OpenGL render-target name to render into.
 *
 * @param[in] view Index of the view.
 * @param[out] framebuffer Name of the OpenGL framebuffer for the specified view.
 * @param[out] viewportX Viewport horizontal offset.
 * @param[out] viewportY Viewport vertical offset.
 * @param[out] viewportWidth Viewport width.
 * @param[out] viewportHeight Viewport height.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_get_render_target_for_view_ex(
    struct leia_interlacer*, int32_t view, uint32_t* framebuffer, int32_t* viewportX, int32_t* viewportY, int32_t* viewportWidth, int32_t* viewportHeight);

/**
 * @brief Set the OpenGL views atlas OpenGL name.
 *
 * If you are managing the views atlas in your application, use this method to specify the name of the
 * texture to use for views.
 *
 * @param[in] texture Name of views OpenGL texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_set_interlace_view_texture_atlas(struct leia_interlacer*, uint32_t texture);

/**
 * @brief Set the OpenGL output texture.
 *
 * To render to a render-target other than the default backbuffer, use this method to specify it's name.
 *
 * @param[in] renderTarget Name of views OpenGL render-target.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_set_output_render_target(struct leia_interlacer*, uint32_t renderTarget);

/**
 * @brief Set the OpenGL view texture for individual texture use.
 *
 * If you aren't using an atlas and have your views as individual textures, use this method to
 * set the texture for the specified view.
 *
 * @param[in] viewIndex Index of the view.
 * @param[in] glTextureId Name of OpenGL texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_set_view_for_texture_array(struct leia_interlacer*, int32_t viewIndex, uint32_t glTextureId);

/**
 * @brief Set the OpenGL view texture for stereo texture content.
 *
 * If you are using leia_interlacer_opengl_do_post_process_picture() or leia_interlacer_opengl_do_post_process_video(), you can use this method to specify
 * the input views texture rather than passing it to the PostProcess methods directly.
 *
 * @param[in] newId Index Name of the OpenGL stereo views texture.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_opengl_set_view_texture_id(struct leia_interlacer*, int32_t newId);

/*!
 * @}
 */

END_CAPI_DECL

#endif // CNSDK_LEIA_SDK_CORE_INTERLACER_OPENGL_H
