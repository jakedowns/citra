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
#ifndef CNSDK_LEIA_CORE_INTERLACER_OPENGL_HPP
#define CNSDK_LEIA_CORE_INTERLACER_OPENGL_HPP

#include "leia/sdk/core.interlacer.hpp"
#include "leia/sdk/core.interlacer.opengl.h"

/*!
 * @addtogroup interlacer_opengl
 * @{
 */

namespace leia {
namespace sdk {

/**
 * @brief C++ wrapper for OpenGL leia_interlacer
 */
class InterlacerOpenGL : public Interlacer {
public:
    InterlacerOpenGL(Core& core, InterlacerInitConfiguration const& config, void* context)
        : Interlacer(core.GetHandle(), leia_interlacer_opengl_initialize(core.GetHandle(), config.GetHandle(), context), true)
    {
    }
    InterlacerOpenGL(InterlacerOpenGL const&) = delete;
    InterlacerOpenGL& operator=(InterlacerOpenGL const&) = delete;
    InterlacerOpenGL(InterlacerOpenGL&&) = delete;
    InterlacerOpenGL& operator=(InterlacerOpenGL&&) = delete;

    void DoPostProcess(int32_t width, int32_t height, bool blendedInterlace, uint32_t outputRenderTarget)
    {
        leia_interlacer_opengl_do_post_process(_interlacer, width, height, blendedInterlace, outputRenderTarget);
    }
    void DoPostProcessPicture(int32_t width, int32_t height, uint32_t views)
    {
        leia_interlacer_opengl_do_post_process_picture(_interlacer, width, height, views);
    }
    void DoPostProcessVideo(int32_t width, int32_t height, uint32_t views)
    {
        leia_interlacer_opengl_do_post_process_video(_interlacer, width, height, views);
    }
    LEIA_NODISCARD
    uint32_t GetRenderTargetForView(int32_t view)
    {
        return leia_interlacer_opengl_get_render_target_for_view(_interlacer, view);
    }
    void GetRenderTargetForView(int32_t view, uint32_t* framebuffer, int32_t* viewportX, int32_t* viewportY, int32_t* viewportWidth, int32_t* viewportHeight)
    {
        leia_interlacer_opengl_get_render_target_for_view_ex(_interlacer, view, framebuffer, viewportX, viewportY, viewportWidth, viewportHeight);
    }
    void SetInterlaceViewTextureAtlas(uint32_t texture)
    {
        leia_interlacer_opengl_set_interlace_view_texture_atlas(_interlacer, texture);
    }
    void SetOutputRenderTarget(uint32_t renderTarget)
    {
        leia_interlacer_opengl_set_output_render_target(_interlacer, renderTarget);
    }
    void SetViewForTextureArray(int32_t viewIndex, uint32_t glTextureId)
    {
        leia_interlacer_opengl_set_view_for_texture_array(_interlacer, viewIndex, glTextureId);
    }
    void SetViewTextureId(int32_t newId)
    {
        leia_interlacer_opengl_set_view_texture_id(_interlacer, newId);
    }

    InterlacerOpenGL(leia_interlacer* nonOwningInterlacer) : Interlacer(nullptr, nonOwningInterlacer, false)
    {
    }
};

LEIA_NODISCARD
inline InterlacerOpenGL* AsOpenGL(Interlacer* interlacer)
{
    if (interlacer && interlacer->GetGraphicsAPI() == LEIA_GRAPHICS_API_OPENGL)
    {
        return static_cast<InterlacerOpenGL*>(interlacer);
    }
    return nullptr;
}

// using IThreadedInterlacer = InterlacerOpenGL;
// using leia_interlacer  = InterlacerOpenGL;

} // namespace sdk
} // namespace leia

/*!
 * @}
 */

#endif // CNSDK_LEIA_CORE_INTERLACER_OPENGL_HPP
