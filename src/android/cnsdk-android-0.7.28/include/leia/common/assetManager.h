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
#ifndef CNSDK_LEIA_COMMON_ASSET_MANAGER_H
#define CNSDK_LEIA_COMMON_ASSET_MANAGER_H

#include "leia/common/api.h"
#include "leia/common/types.fwd.h"

#include <cstdint>

BEGIN_CAPI_DECL

#pragma pack(push, 8)

struct leia_asset_manager_buffer {
    uint64_t size;
    char*    data;
};

#if defined(LEIA_OS_WINDOWS)
struct leia_asset_manager_resolve_context {
    /// Resolve assets relative to a module. moduleAddress is an address from a module (.dll, .exe).
    void* moduleAddress = nullptr;
};
#else
struct leia_asset_manager_resolve_context {
    char dummy;
};
#endif

/**
 * @brief Read an asset as a Buffer.
 *
 * @p assetPath is expected to be relative to the root of the asset directory.
 */
LEIA_NODISCARD
LEIA_COMMON_API
leia_asset_manager_buffer leia_asset_manager_read_data(leia_asset_manager*, const char* assetPath, struct leia_asset_manager_resolve_context* resolveContext);

LEIA_COMMON_API
void leia_asset_manager_buffer_release(leia_asset_manager_buffer*);

#pragma pack(pop)

END_CAPI_DECL

#endif // CNSDK_LEIA_COMMON_ASSET_MANAGER_H
