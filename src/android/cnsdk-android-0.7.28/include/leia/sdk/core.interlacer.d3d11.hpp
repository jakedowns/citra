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
#ifndef CNSDK_LEIA_CORE_INTERLACER_D3D11_HPP
#define CNSDK_LEIA_CORE_INTERLACER_D3D11_HPP

#include "leia/sdk/core.interlacer.hpp"
#include "leia/sdk/core.interlacer.d3d11.h"

/*!
 * @addtogroup interlacer_d3d11
 * @{
 */

namespace leia {
namespace sdk {

/**
 * @brief C++ wrapper for D3D11 leia_interlacer
 */
class InterlacerD3D11 : public Interlacer {
public:
    InterlacerD3D11(Core& core, InterlacerInitConfiguration const& config, ID3D11DeviceContext* context)
        : Interlacer(core.GetHandle(), leia_interlacer_d3d11_initialize(core.GetHandle(), config.GetHandle(), context), true)
    {
    }
    InterlacerD3D11(InterlacerD3D11 const&) = delete;
    InterlacerD3D11& operator=(InterlacerD3D11 const&) = delete;
    InterlacerD3D11(InterlacerD3D11&&) = delete;
    InterlacerD3D11& operator=(InterlacerD3D11&&) = delete;

    void DoPostProcess(int width, int height, bool blendedInterlace, ID3D11RenderTargetView* resultTexture)
    {
        leia_interlacer_d3d11_do_post_process(_interlacer, width, height, blendedInterlace, resultTexture);
    }
    void DoPostProcessPicture(int width, int height, ID3D11ShaderResourceView* pictureShaderResourceView, ID3D11RenderTargetView* renderTargetView)
    {
        leia_interlacer_d3d11_do_post_process_picture(_interlacer, width, height, pictureShaderResourceView, renderTargetView);
    }
    LEIA_NODISCARD
    ID3D11DepthStencilView* GetDepthStencilView(int view)
    {
        return leia_interlacer_d3d11_get_depth_stencil_view(_interlacer, view);
    }
    LEIA_NODISCARD
    ID3D11RenderTargetView* GetRenderTargetView(int view)
    {
        return leia_interlacer_d3d11_get_render_target_view(_interlacer, view);
    }
    void SetInterlaceViewTextureAtlas(ID3D11ShaderResourceView* texture)
    {
        leia_interlacer_d3d11_set_interlace_view_texture_atlas(_interlacer, texture);
    }
    void SetViewForTextureArray(int viewIndex, ID3D11RenderTargetView* renderTarget)
    {
        leia_interlacer_d3d11_set_view_for_texture_array(_interlacer, viewIndex, renderTarget);
    }
    void SetViewTextureId(ID3D11ShaderResourceView* view)
    {
        leia_interlacer_d3d11_set_view_texture_id(_interlacer, view);
    }

    InterlacerD3D11(leia_interlacer* nonOwningInterlacer) : Interlacer(nullptr, nonOwningInterlacer, false)
    {
    }
};

LEIA_NODISCARD
inline InterlacerD3D11* AsD3D11(Interlacer* interlacer)
{
    if (interlacer && interlacer->GetGraphicsAPI() == LEIA_GRAPHICS_API_D3D11)
    {
        return static_cast<InterlacerD3D11*>(interlacer);
    }
    return nullptr;
}

} // namespace sdk
} // namespace leia

/*!
 * @}
 */

#endif // CNSDK_LEIA_CORE_INTERLACER_D3D11_HPP
