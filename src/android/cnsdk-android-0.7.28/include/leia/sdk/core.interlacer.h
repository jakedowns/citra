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
#ifndef CNSDK_LEIA_SDK_CORE_INTERLACER_H
#define CNSDK_LEIA_SDK_CORE_INTERLACER_H

#include "leia/sdk/api.h"
#include "leia/sdk/core.types.h"
#include "leia/common/types.h"
#include "leia/common/slice.h"
#include "leia/common/jniTypes.h"

BEGIN_CAPI_DECL

#pragma pack(push, 8)

/*!
 * @defgroup interlacer Interlacer
 * @ingroup core
 *
 * @brief Interlacing on Leia devices.
 */

/*!
 * @addtogroup interlacer
 * @{
 */

#ifdef LEIA_DOXYGEN

/**
 * @brief The main entity used for performing interlacing on Leia devices.
 *
 * Use one of the graphics API-specific init functions to create it:
 *   - leia_interlacer_opengl_initialize(struct leia_core*, struct leia_interlacer_init_configuration const*, void*),
 *   - leia_interlacer_d3d11_initialize(struct leia_core*, struct leia_interlacer_init_configuration const*, struct ID3D11DeviceContext*),
 *   - leia_interlacer_d3d12_initialize(struct leia_core*, struct leia_interlacer_init_configuration const*, struct ID3D12Device*, struct ID3D12CommandQueue*),
 *   - leia_interlacer_vulkan_initialize(struct leia_core*, struct leia_interlacer_init_configuration const*, VkDevice, VkPhysicalDevice, VkFormatInt,
 * VkFormatInt, VkFormatInt, int32_t).
 */
struct leia_interlacer {};

/**
 * @brief leia_interlacer's initial configuration.
 */
struct leia_interlacer_init_configuration {};

struct leia_interlacer_gui_configuration {};

#endif

LEIA_NODISCARD
LEIASDK_API
struct leia_interlacer_init_configuration* leia_interlacer_init_configuration_alloc();

/// Whether we are using a protected OpenGL context.
LEIASDK_API
void leia_interlacer_init_configuration_set_is_protected(struct leia_interlacer_init_configuration*, leia_bool);

/// If we should use atlas by default.
LEIASDK_API
void leia_interlacer_init_configuration_set_use_atlas_for_views(struct leia_interlacer_init_configuration*, leia_bool);

LEIASDK_API
void leia_interlacer_init_configuration_free(struct leia_interlacer_init_configuration*);

/**
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_shutdown(struct leia_core*, struct leia_interlacer*);

/**
 * @brief Enable or disable sharpening.
 *
 * Use this method to enable or disable sharpening. When enabled, the interlaced image result
 * will have reduced 3d artifacts.
 *
 * @see leia_interlacer_is_sharpening_enabled
 *
 * @param enable Whether to enable or disable sharpening.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_enable_sharpening(struct leia_interlacer*, leia_bool enable);

/**
 * @brief Gets whether sharpening is enabled or not.
 *
 * @see leia_interlacer_enable_sharpening
 *
 * @return Whether sharpening is enabled.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_interlacer_is_sharpening_enabled(struct leia_interlacer*);

/**
 * @brief Set a debug mode for interlacing.
 *
 * Use this method to override default interlacing behavior and render in a debug mode specified
 * by the user.
 *
 * @see leia_interlacer_get_shader_debug_mode
 *
 * @param[in] mode The debug mode to apply.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_shader_debug_mode(struct leia_interlacer*, enum leia_shader_debug_mode mode);

/**
 * @brief Get the shader debug mode.
 *
 * Use this method to retrieve the shader debug mode set via leia_interlacer_set_shader_debug_mode().
 *
 * @see leia_interlacer_set_shader_debug_mode
 *
 * @return The current shader debug mode amount.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
enum leia_shader_debug_mode leia_interlacer_get_shader_debug_mode(struct leia_interlacer*);

/**
 * @brief Set the layout of views in a atlas.
 *
 * If you are using a atlas for views, the views are laid-out in a certain format. By
 * default a stereo 2x1 left/right format is used. Use this method to override to any view
 * layout.
 *
 * @see leia_interlacer_get_num_tiles
 *
 * @param[in] tx Number of views horizontally.
 * @param[in] ty Number of views vertically.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_num_tiles(struct leia_interlacer*, int32_t tx, int32_t ty);

/**
 * @brief Get the layout of views in the atlas.
 *
 * When rendering multiple views to a single atlas, they are laid-out in a certain way. By
 * default we use a stereo 2x1 layout. Use this method to retrieve the set layout.
 *
 * @see leia_interlacer_set_num_tiles
 *
 * @param[out] tx Number of horizontal views.
 * @param[out] ty Number of vertical views.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_get_num_tiles(struct leia_interlacer*, int32_t* tx, int32_t* ty);

/**
 * @brief Sets the baseline scaling.
 *
 * Use this method to set the baseline scaling value. The baseline scaling value determines the horizontal
 * view camera spacing and hence the strength of the 3D effect in the interlaced result.
 *
 * @see leia_interlacer_get_baseline_scaling
 *
 * @param[in] baseline The baseline scaling to use.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_baseline_scaling(struct leia_interlacer*, float baseline);

/**
 * @brief Get the baseline scaling.
 *
 * Use this method to retrieve the baseline scaling value set via leia_interlacer_set_baseline_scaling().
 * The baseline scaling value determines the horizontal view camera spacing and hence the
 * strength of the 3D effect in the interlaced result.
 *
 * @see leia_interlacer_set_baseline_scaling
 *
 * @return The current baseline scaling.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
float leia_interlacer_get_baseline_scaling(struct leia_interlacer*);

/**
 * @brief Get the convergence distance.
 *
 * Use this method to retrieve the convergence distance value set via leia_interlacer_set_convergence_distance().
 * The convergence distance value determines the distance at which objects appear directly on the
 * display. Objects closer than the convergence distance appear to float in front of the display and
 * objects farther than the convergence distance appear to be inside the display.
 *
 * @see leia_interlacer_set_convergence_distance
 *
 * @return The current convergence distance.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
float leia_interlacer_get_convergence_distance(struct leia_interlacer*);

/**
 * @brief Set the convergence distance.
 *
 * Use this method to set the convergence distance. The convergence distance value determines the distance
 * at which objects appear directly on the display. Objects closer than the convergence distance appear to
 * float in front of the display and objects farther than the convergence distance appear to be inside the display.
 *
 * @see GetConvergenceDistance
 *
 * @param[in] distance The convergence distance.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_convergence_distance(struct leia_interlacer*, float distance);

/**
 * @brief Retrieve information related to a view.
 *
 * Use this method to obtain general information about a view when the view will be renderered with an orthographic projection.
 *
 * @param[in] viewIndex The view index, starting at zero and increasing from left-to-right from the main camera's perspective.
 * @param[in] cameraPos Main camera position.
 * @param[in] cameraDir Main camera direction.
 * @param[in] cameraUp Main camera up vector.
 * @param[in] width Width of orthograpic projection.
 * @param[in] height Height of orthograpic projection.
 * @param[in] nearPlane Frustum near plane distance.
 * @param[in] farPlane Frustum far plane distance.
 * @param[out] viewPos Position of the view [Optional].
 * @param[out] viewProjectionMatrix Projection matrix of the view [Optional].
 * @param[out] viewShearX Horizontal shear value for the view [Optional].
 * @param[out] viewShearY Vertical shear value for the view [Optional].
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_get_converged_orthographic_view_info(struct leia_interlacer*,
                                                          int32_t viewIndex,
                                                          LEIA_CONST_FLOAT_SLICE(3) cameraPos,
                                                          LEIA_CONST_FLOAT_SLICE(3) cameraDir,
                                                          LEIA_CONST_FLOAT_SLICE(3) cameraUp,
                                                          float width,
                                                          float height,
                                                          float nearPlane,
                                                          float farPlane,
                                                          LEIA_FLOAT_SLICE(3) viewPos,
                                                          LEIA_FLOAT_SLICE(16) viewProjectionMatrix,
                                                          float* viewShearX,
                                                          float* viewShearY);

/**
 * @brief Retrieve information related to a view.
 *
 * Use this method to obtain general information about a view when the view will be renderered with a perspective projection.
 *
 * @param[in] viewIndex The view index, starting at zero and increasing from left-to-right from the main camera's perspective.
 * @param[in] cameraPos Main camera position.
 * @param[in] cameraDir Main camera direction.
 * @param[in] cameraUp Main camera up vector.
 * @param[in] fieldOfView Horizontal field of view of the projection (in radians).
 * @param[in] aspectRatio Ratio of the horizontal-to-vertical projection plane size.
 * @param[in] nearPlane Frustum near plane distance.
 * @param[in] farPlane Frustum far plane distance.
 * @param[out] viewPos Position of the view [Optional].
 * @param[out] viewProjectionMatrix Projection matrix of the view [Optional].
 * @param[out] viewFieldOfView Horizontal field of view of the view [Optional].
 * @param[out] viewShearX Horizontal shear value for the view [Optional].
 * @param[out] viewShearY Vertical shear value for the view [Optional].
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_get_converged_perspective_view_info(struct leia_interlacer*,
                                                         int32_t viewIndex,
                                                         LEIA_CONST_FLOAT_SLICE(3) cameraPos,
                                                         LEIA_CONST_FLOAT_SLICE(3) cameraDir,
                                                         LEIA_CONST_FLOAT_SLICE(3) cameraUp,
                                                         float fieldOfView,
                                                         float aspectRatio,
                                                         float nearPlane,
                                                         float farPlane,
                                                         LEIA_FLOAT_SLICE(3) viewPos,
                                                         LEIA_FLOAT_SLICE(16) viewProjectionMatrix,
                                                         float* viewFieldOfView,
                                                         float* viewShearX,
                                                         float* viewShearY);

/**
 * @brief Set the fit mode.
 *
 * Use this method to set the fit mode. The fit mode allows fitting interlaced content at a different
 * aspect-ratio than the render-target.
 *
 * @see leia_interlacer_get_fit_mode
 *
 * @param[in] mode The desired fit mode.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_fit_mode(struct leia_interlacer*, enum leia_fit_mode mode);

/**
 * @brief Get the fit mode.
 *
 * Use this method to retrieve the fit mode value set via leia_interlacer_set_fit_mode.
 * The fit mode allows fitting interlaced content at a different aspect-ratio than the render-target.
 *
 * @see leia_interlacer_set_fit_mode
 *
 * @return The current fit mode.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
enum leia_fit_mode leia_interlacer_get_fit_mode(struct leia_interlacer*);

/**
 * @brief Get the rect matrix.
 *
 * Use this method to retrieve the matrix set via leia_interlacer_set_fit_mode.
 *
 * @see leia_interlacer_set_fit_mode
 *
 * @param[out] transformMatrix The rect matrix is returned here
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_get_rect_matrix(struct leia_interlacer*, LEIA_FLOAT_SLICE(16) transformMatrix);

/**
 * @brief Set the local matrix.
 *
 * Use this method to set the local matrix e.g. for zoom & pan gesture handling
 *
 * @param[in] transformMatrix The new local matrix
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_local_matrix(struct leia_interlacer*, LEIA_CONST_FLOAT_SLICE(16) transformMatrix);

/**
 * @brief Set the custom texture matrix.
 *
 * Use this method to scale media which SurfaceView thinks has non-standard dimensions
 *
 * @param[in] textureTransform The new custom texture matrix
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_custom_texture_matrix(struct leia_interlacer*, LEIA_CONST_FLOAT_SLICE(16) textureTransform);

LEIASDK_API
void leia_interlacer_set_enable_user_matrix(struct leia_interlacer* interlacer, leia_bool enabled);
LEIASDK_API
void leia_interlacer_get_user_matrix(struct leia_interlacer* interlacer, LEIA_FLOAT_SLICE(16) transformMatrix);
LEIASDK_API
void leia_interlacer_set_user_matrix(struct leia_interlacer* interlacer, LEIA_CONST_FLOAT_SLICE(16) transformMatrix);

/**
 * @brief Sets the alpha blending value.
 *
 * When calling DoPostProcess, you have the option of blending the interlaced result onto
 * the render-target. This method sets the blend amount, where zero is invisible and one is
 * fully opaque.
 *
 * @param[in] alpha The alpha-blending value.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_alpha(struct leia_interlacer*, float alpha);

/**
 * @brief Set whether to flip the views vertically.
 *
 * If your views are inverted vertically, use this method to flip them during interlacing.
 *
 * @param[in] flip Whether to flip the views.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_flip_input_uv_vertical(struct leia_interlacer*, leia_bool flip);

/**
 * @brief Set single-view interlacing mode.
 *
 * Use this method to disable interlacing and render a single view instead.
 *
 * @param[in] setSingleView Whether to enable single-view mode.
 * @param[in] singleViewIndex Index of the view to be rendering if single-view mode is enabled.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_single_view_mode(struct leia_interlacer*, leia_bool setSingleView, int32_t singleViewIndex);

struct leia_interlacer_single_view_mode_listener {
    void* userData;
    void (*onSingleViewModeChange)(void* userData, leia_bool singleViewMode);
};

/**
 * @brief Set single-view mode callback.
 *
 * Single-view mode is automatically enabled when no face is detected for a certain amount of
 * time and disabled when a new face is detected. To receive a callback when this occurs, use
 * this method to specify the callback structure.
 *
 * @param[in] listener The structure that will get a method callback when single-view mode changes.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_single_view_mode_listener(struct leia_interlacer*, struct leia_interlacer_single_view_mode_listener const* listener);

/**
 * @brief Set dimensions of the views.
 *
 * Use this method to specify the size and layout of the views texture. This is necessary when you a
 * fit mode other than FILL.
 *
 * @see leia_interlacer_set_fit_mode
 *
 * @param[in] sourceWidth Width of the views.
 * @param[in] sourceHeight Height of the views.
 * @param[in] isHorizontalViews If the views are laid-out horizontally.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_source_views_size(struct leia_interlacer*, int32_t sourceWidth, int32_t sourceHeight, leia_bool isHorizontalViews);

/**
 * @brief Set the viewport position
 *
 * Use this method to set the viewport origin in the render-target where the interlacing result
 * will be rendered.
 *
 * @param[in] newXPos Origin pixel x-coordinate
 * @param[in] newYPos Origin pixel y-coordinate
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_viewport_position(struct leia_interlacer*, int32_t newXPos, int32_t newYPos);

/**
 * @brief Enable or disable reconvergence.
 *
 * Use this method to enable or disable reconvergence. When enabled, the input views are shifted
 * depending on the value set via SetReconvergence().
 *
 * @see leia_interlacer_is_reconvergence_enabled, leia_interlacer_get_reconvergence, SetReconvergence, leia_interlacer_set_reconvergence_zoom
 *
 * @param enable Whether to enable or disable reconvergence.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_enable_reconvergence(struct leia_interlacer*, leia_bool enable);

/**
 * @brief Gets whether we are using reconvergence.
 *
 * @see leia_interlacer_get_reconvergence, leia_interlacer_enable_reconvergence, SetReconvergence, leia_interlacer_set_reconvergence_zoom
 *
 * @return Whether reconvergence is enabled.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_interlacer_is_reconvergence_enabled(struct leia_interlacer*);

/**
 * @brief Get the reconvergence amount.
 *
 * Use this method to retrieve the reconvergence amount set via SetReconvergence().
 *
 * @see leia_interlacer_is_reconvergence_enabled, leia_interlacer_enable_reconvergence, SetReconvergence, leia_interlacer_set_reconvergence_zoom
 *
 * @return The current reconvergence amount.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
float leia_interlacer_get_reconvergence(struct leia_interlacer*);

/**
 * @brief Set the reconvergence amount.
 *
 * Using reconvergence allows shifting the views to account for disparity.
 *
 * @see leia_interlacer_get_reconvergence, leia_interlacer_is_reconvergence_enabled, leia_interlacer_enable_reconvergence,
 * leia_interlacer_set_reconvergence_zoom
 *
 * @param[in] value Amount of reconvergence.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_reconvergence(struct leia_interlacer*, float value);

/**
 * @brief Set what directions reconvergence is calculated for.
 *
 * Use this method to specify whether you wish to apply reconvergence horizontally or horizontally
 * and vertically.
 *
 * @see leia_interlacer_get_reconvergence, leia_interlacer_is_reconvergence_enabled, leia_interlacer_enable_reconvergence, SetReconvergence
 *
 * @param[in] zoomX Whether to apply reconvergence in the horizontal direction.
 * @param[in] zoomXY Whether to apply reconvergence in the horizontal and vertical direction.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_reconvergence_zoom(struct leia_interlacer*, leia_bool zoomX, leia_bool zoomXY);

/**
 * @brief Get the number of views.
 *
 * Use this method to retrieve the number of views. This always returns 2.
 *
 * @return The current number of views.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
int32_t leia_interlacer_get_num_views(struct leia_interlacer*);

/**
 * @brief Set whether the views are a single atlas vs individual textures.
 *
 * You can specify whether your views are all packed into a atlas or are in indidual
 * textures with this method.
 *
 * @see leia_interlacer_get_use_atlas_for_views
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_use_atlas_for_views(struct leia_interlacer*, leia_bool);

/**
 * @brief Gets whether we are using a single atlas or individual views.
 *
 * Use this method to determine if views are individual textures or are all combined into
 * a single atlas.
 *
 * @see leia_interlacer_set_use_atlas_for_views
 *
 * @return Whether an atlas is used for views.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_interlacer_get_use_atlas_for_views(struct leia_interlacer*);

/**
 * @brief Gets a view offset.
 *
 * Use this method to get the view position for the specified view index. The returned value is
 * the relative offset where the view is located.
 *
 * @see leia_interlacer_get_use_atlas_for_views
 *
 * @param[in] view View index.
 * @param[out] offset Offset of view camera.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_get_view_offset(struct leia_interlacer*, int32_t view, LEIA_FLOAT_SLICE(3) offset);

/**
 * @brief Initializes and enables use of a debug GUI.
 *
 * Use this method to enable rendering a debug GUI on top of the interlaced result. This can be useful
 * for testing and debugging. The GUI is interactive and allows the user to view and modify many options.
 *
 * @see leia_interlacer_set_gui_visibility, leia_interlacer_is_gui_visible
 *
 * @param[in] config Arguments for the debug menu, see leia_interlacer_debug_menu_configuration.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_initialize_gui(struct leia_interlacer*, struct leia_interlacer_debug_menu_configuration const* config);

/**
 * @brief Set whether the GUI is visible.
 *
 * Use this method to make the GUI visible or not.
 *
 * @see leia_interlacer_is_gui_visible, leia_interlacer_initialize_gui
 *
 * @param[in] isVisible Whether to show or hide GUI.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_gui_visibility(struct leia_interlacer*, leia_bool isVisible);

/**
 * @brief Gets whether we the GUI is visible.
 *
 * @see leia_interlacer_set_gui_visibility, leia_interlacer_initialize_gui
 *
 * @return If the GUI is initialized and visible.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
leia_bool leia_interlacer_is_gui_visible(struct leia_interlacer*);

#if defined(LEIA_OS_WINDOWS)

/**
 * @brief Process user-input for the GUI.
 *
 * When the GUI is enabled, we need to process user input (keyboard, mouse, touch) to drive
 * the interface. Call this method when you receive window messages in your WNDPROC callback.
 *
 * @param[in] surface The hWnd value from the WNDPROC.
 * @param[in] msg The uMsg value from the WNDPROC.
 * @param[in] wParam The uMsg value from the WNDPROC.
 * @param[in] lParam The uMsg value from the WNDPROC.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
struct leia_interlacer_gui_input_state leia_interlacer_process_gui_input(struct leia_interlacer*, void* surface, uint32_t msg, uint64_t wparam, int64_t lparam);

#elif defined(LEIA_OS_ANDROID)

struct AInputEvent;

/**
 * @brief Process user-input for the GUI.
 *
 * When the GUI is enabled, we need to process user input (keyboard, mouse, touch) to drive
 * the interface. Call this method when your Android app receives an input event.
 *
 * @param[in] inputEvent The received AInputEvent.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
struct leia_interlacer_gui_input_state leia_interlacer_process_gui_input(struct leia_interlacer*, struct AInputEvent const* inputEvent);

/**
 * @brief Process user-input for the GUI.
 *
 * When the GUI is enabled, we need to process user input (keyboard, mouse, touch) to drive
 * the interface. Call this method when your Android app receives a motion input event.
 *
 * @param[in] jni The JNI environment structure for the application.
 * @param[in] motionInput The received motion input.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
struct leia_interlacer_gui_input_state leia_interlacer_process_gui_motion_input(struct leia_interlacer*, JNIEnv* jni, jobject motionInput);

#endif

/**
 * @brief Shutdown the GUI
 *
 * Use this method when you are finished with the GUI.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_shutdown_gui(struct leia_interlacer*);

/**
 * @brief Sets the calibration pattern.
 *
 * This method allows setting calibration pattern. This can be used to calibrate Leia devices but also to
 * visually inspect and verify if a display is calibrated correctly.
 *
 * @param[in] pattern The calibration pattern to set.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_calibration_pattern(struct leia_interlacer*, enum leia_calibration_pattern pattern);

/**
 * @brief Set the gamma value during interlacing.
 *
 * @param[in] rGamma Red component of gamma.
 * @param[in] gGamma Green component of gamma.
 * @param[in] bGamma Blue component of gamma.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_gamma(struct leia_interlacer*, float rGamma, float gGamma, float bGamma);

/**
 * @brief Set the aspect ratio offset.
 *
 * Use this method to set the aspect ratio offset value. Setting a non-zero aspect-ratio offset
 * will change the aspect-ratio of the interlacedresult, as long as the current FitMode is set to Fill.
 *
 * @see leia_interlacer_get_aspect_ratio_offset
 *
 * @param[in] offset The apsect-ratio offset to use.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_aspect_ratio_offset(struct leia_interlacer*, float offset);

/**
 * @brief Get the aspect ratio offset.
 *
 * Use this method to retrieve the aspect ratio offset value set via leia_interlacer_set_aspect_ratio_offset.
 * Setting a non-zero aspect-ratio offset will change the aspect-ratio of the interlaced
 * result, as long as the current FitMode is set to Fill.
 *
 * @see leia_interlacer_set_aspect_ratio_offset
 *
 * @return The current aspect ratio offset.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
float leia_interlacer_get_aspect_ratio_offset(struct leia_interlacer*);

/**
 * @brief Set whether to invert screen texture coordinates vertically.
 *
 * If the interlaced image is going to be inverted on presentation, use this method to flip the interlacing grid pattern.
 *
 * @param[in] invert Whether to invert the interlacing grid pattern.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_invert_screen_texture_coordinates(struct leia_interlacer*, leia_bool invert);

/**
 * @brief Set the interlacing mode.
 *
 * Use this method to set the interlacing mode, which can be stereo, view peeling, or
 * view sliding.
 *
 * @see leia_interlacer_get_interlace_mode
 *
 * @param[in] interlaceMode Desired interlace mode.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_interlacer_set_interlace_mode(struct leia_interlacer*, enum leia_interlace_mode interlaceMode);

/**
 * @brief Get the interlace mode.
 *
 * Use this method to retrieve the interlace mode which was set via leia_interlacer_set_interlace_mode.
 *
 * @see leia_interlacer_set_interlace_mode
 *
 * @return The current interlace mode.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
enum leia_interlace_mode leia_interlacer_get_interlace_mode(struct leia_interlacer*);

/**
 * @brief Get the graphics API.
 *
 * Use this method to retrieve the graphics API which is determined by which Initialize{OpenGL|D3D11|D3D12|Vulan} method
 * was used.
 *
 * @return The current graphics API.
 *
 * @public @memberof leia_interlacer
 */
LEIA_NODISCARD
LEIASDK_API
enum leia_graphics_api leia_interlacer_get_graphics_api(struct leia_interlacer*);

/**
 * @brief A helper function to compute comfort zone range.
 * @param[in] mode View mode
 * @param[in] baselineScaling Baseline scaling value from Leia SDK
 * @param[in] convergencePlaneDistance Convergence plane distance.
 * @param[in] nearPlane Frustum near plane.
 * @param[in] farPlane Frustum far plane.
 * @param[out] viewComfortZoneNear Near comfort zone. For best 3d results, keep geometry beyond this distance.
 * @param[out] viewComfortZoneFar Far comfort zone. For best 3d results, keep geometry before this distance.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_get_view_comfort_zone(enum leia_view_info_mode mode,
                                float                    baselineScaling,
                                float                    convergencePlaneDistance,
                                float                    nearPlane,
                                float                    farPlane,
                                float*                   viewComfortZoneNear,
                                float*                   viewComfortZoneFar);

/**
 * @brief A helper function to compute per-view values.
 * @param[in] mode View mode
 * @param[in] viewOffset View offset from Leia Interlacer
 * @param[in] cameraPos Camera position.
 * @param[in] cameraDir Camera forward direction (unit vector).
 * @param[in] cameraUp Camera up direction (unit vector).
 * @param[in] perspectiveFieldOfView The field of view for perspective projections.
 * @param[in] perspectiveAspectRatio Aspect ratio for perspective projections.
 * @param[in] orthoWidth Width of orthographic projection.
 * @param[in] orthoHeight Height of orthographic projection.
 * @param[in] nearPlane Frustum near plane.
 * @param[in] farPlane Frustum far plane.
 * @param[in] convergencePlaneDistance Convergence plane distance.
 * @param[in] convergencePlaneHeight Convergence plane height.
 * @param[out] viewPos Position of camera for specified view.
 * @param[out] viewProjectionMatrix Projection matrix for specified view.
 * @param[out] viewFieldOfView Field-of-view for specified view.
 * @param[out] viewShearX X shear factor.
 * @param[out] viewShearY X shear factor.
 *
 * @public @memberof leia_interlacer
 */
LEIASDK_API
void leia_get_view_info(enum leia_view_info_mode mode,
                        LEIA_CONST_FLOAT_SLICE(3) viewOffset,
                        LEIA_CONST_FLOAT_SLICE(3) cameraPos,
                        LEIA_CONST_FLOAT_SLICE(3) cameraDir,
                        LEIA_CONST_FLOAT_SLICE(3) cameraUp,
                        float perspectiveFieldOfView,
                        float perspectiveAspectRatio,
                        float orthoWidth,
                        float orthoHeight,
                        float nearPlane,
                        float farPlane,
                        float convergencePlaneDistance,
                        float convergencePlaneHeight,
                        LEIA_FLOAT_SLICE(3) viewPos,
                        LEIA_FLOAT_SLICE(16) viewProjectionMatrix,
                        float* viewFieldOfView,
                        float* viewShearX,
                        float* viewShearY);

/*!
 * @}
 */

#pragma pack(pop)

END_CAPI_DECL

#endif // CNSDK_LEIA_SDK_CORE_INTERLACER_H
