/*
 * Copyright (c) 2023 Leia Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <malloc.h>
#include <exception>

#include <android/sensor.h>
#include <android/log.h>

#include "android_native_app_glue.h"

#include "leia/lwEngine/LWE_Types.h"
#include "leia/lwEngine/LWE_Core.h"
#include "leia/common/assert.h"

#include <imgui.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

void SetUpAltStack();
void SetUpSigActionHandler();
void backtraceToLogcat();
void dump_stack();

#ifdef LEIA_USE_VULKAN
#    include <vulkan/vulkan.hpp>
#    include <vulkan/vulkan_android.h>
#endif

/**
 * Our saved state data.
 */
struct saved_state {
    float   angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager*    sensorManager;
    const ASensor*     accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int                animating;
    EGLDisplay         display;
    EGLSurface         surface;
    EGLContext         context;
    int32_t            width;
    int32_t            height;
    struct saved_state state;
};

// Select OpenGL or Vulkan with this variable.
leia::sdk::GraphicsAPI graphicsAPI = LEIA_GRAPHICS_API_OPENGL;

leia::lwe::LWE_Core CoreEngine(graphicsAPI);

/**
 * Custom app menu gui
 */
void render_custom_gui()
{
    if (ImGui::CollapsingHeader("lweTest"))
    {
        if (ImGui::Button("Toggle Animation"))
            CoreEngine.ToggleAnimation();
        // if (ImGui::Button("Toggle Use LeiaSDK")) CoreEngine.ToggleUseLeiaSDK();
        ImGui::Spacing();
        if (ImGui::Button("Cubes"))
            CoreEngine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_NONE);
        ImGui::SameLine();
        if (ImGui::Button("Stereo Image"))
            CoreEngine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_BEER_GLASS);
        ImGui::SameLine();
        if (ImGui::Button("Test Pattern"))
            CoreEngine.SetShaderDebugMode(LEIA_SHADER_DEBUG_MODE_SHOW_CALIBRATION_IMAGE);
        ImGui::Spacing();
        ImGui::Text("Move camera");
        ImGui::SameLine();
        if (ImGui::ArrowButton("moveLeft", ImGuiDir_Left))
        {
            CoreEngine.MoveLeft(100.0f);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("moveRight", ImGuiDir_Right))
        {
            CoreEngine.MoveRight(100.0f);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("moveForward", ImGuiDir_Up))
        {
            CoreEngine.MoveForward(100.0f);
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("moveBackward", ImGuiDir_Down))
        {
            CoreEngine.MoveBackward(100.0f);
        }
    }
}

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine)
{
    if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
#ifdef LEIA_USE_VULKAN
        engine->display     = nullptr;
        engine->context     = nullptr;
        engine->surface     = nullptr;
        engine->width       = ANativeWindow_getWidth(engine->app->window);
        engine->height      = ANativeWindow_getHeight(engine->app->window);
        engine->state.angle = 0;
#endif
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
        /*
         * Here specify the attributes of the desired configuration.
         * Below, we select an EGLConfig with at least 8 bits per color
         * component compatible with on-screen windows
         */
        const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
        EGLint       w, h, format;
        EGLint       numConfigs;
        EGLConfig    config;
        EGLSurface   surface;
        EGLContext   context;

        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        eglInitialize(display, 0, 0);

        /* Here, the application chooses the configuration it desires. In this
         * sample, we have a very simplified selection process, where we pick
         * the first EGLConfig that matches our criteria */
        eglChooseConfig(display, attribs, &config, 1, &numConfigs);

        /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
         * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
         * As soon as we picked a EGLConfig, we can safely reconfigure the
         * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
        eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

        ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

        // Specify OpenGL ES 3.0.
        const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};

        surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
        context = eglCreateContext(display, config, NULL, contextAttribs);

        if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
        {
            LOGW("Unable to eglMakeCurrent");
            return -1;
        }

        eglQuerySurface(display, surface, EGL_WIDTH, &w);
        eglQuerySurface(display, surface, EGL_HEIGHT, &h);

        engine->display     = display;
        engine->context     = context;
        engine->surface     = surface;
        engine->width       = w;
        engine->height      = h;
        engine->state.angle = 0;

        // Initialize GL state.
        glEnable(GL_CULL_FACE);
        // glShadeModel(GL_SMOOTH);
        glDisable(GL_DEPTH_TEST);
    }
    else
    {
        LNK_ASSERT(false);
    }

    // Initialize LWE engine.
    {
        leia::lwe::LWE_CoreInitArgs initArgs;
        initArgs.coreConfig.SetPlatformAndroidHandle(LEIA_CORE_ANDROID_HANDLE_ACTIVITY, engine->app->activity->clazz);
        initArgs.coreConfig.SetPlatformAndroidJavaVM(engine->app->activity->vm);
        initArgs.coreConfig.SetPlatformLogLevel(kLeiaLogLevelTrace);

        initArgs.appGLContext = engine->context;

        initArgs.window.title                  = "Leia OpenGL Sample";
        initArgs.window.x                      = 0;
        initArgs.window.y                      = 0;
        initArgs.window.width                  = 2560;
        initArgs.window.height                 = 1600;
        initArgs.window.fullscreen             = true;
        initArgs.window.preferSecondaryMonitor = true;
        initArgs.window.proc                   = nullptr;
        initArgs.window.androidWindow          = engine->app->window;

        initArgs.customGui = render_custom_gui;

        CoreEngine.Initialize(initArgs);
    }

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine)
{

    // Skip if no display on OpenGL.
    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
        if (engine->display == NULL)
            return;

    CoreEngine.Tick(1.0 / 60.0);

    if (CoreEngine.IsInitialized())
    {
        CoreEngine.OnWindowSizeChanged(engine->width, engine->height);
        CoreEngine.Render(nullptr);
    }

    if (graphicsAPI == LEIA_GRAPHICS_API_OPENGL)
    {
        EGLBoolean ret = eglSwapBuffers(engine->display, engine->surface);
        LNK_ASSERT(ret == EGL_TRUE);
    }
    else if (graphicsAPI == LEIA_GRAPHICS_API_VULKAN)
    {
        // Vulkan present done in LWE
    }
    else
    {
        LNK_ASSERT(false);
    }
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine)
{
    if (engine->display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display   = EGL_NO_DISPLAY;
    engine->context   = EGL_NO_CONTEXT;
    engine->surface   = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    bool wantCaptureMouse = false;
    if (CoreEngine.IsUsingLeiaSDK())
    {
        leia_interlacer_gui_input_state inputState = CoreEngine.ProcessGuiInput(event);
        if (inputState.wantCaptureInput)
        {
            return 1;
        }
        wantCaptureMouse = inputState.wantCaptureMouse;
    }

    struct engine* engine = (struct engine*)app->userData;
    if (!wantCaptureMouse && AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);

        // Toggle GUI when user touches the screen.
        int32_t action = AMotionEvent_getAction(event);
        int     flags  = action & AMOTION_EVENT_ACTION_MASK;
        if (flags == AMOTION_EVENT_ACTION_UP)
        {
            CoreEngine.ToggleGui();
        }
#if 0
        else if (touchCount == 3)
        {
            ANativeActivity_finish(app->activity);
        }
#endif

        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd)
    {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL)
            {

                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL)
            {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
                // We'd like to get 60 events per second (in microseconds).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L / 60) * 1000);
            }
            engine->animating = 1;
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL)
            {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state)
{
    struct engine engine;

    memset(&engine, 0, sizeof(engine));
    state->userData     = &engine;
    state->onAppCmd     = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app          = state;

    // Prepare to monitor accelerometer
    engine.sensorManager       = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue    = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL)
    {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    engine.animating = 1;

    SetUpAltStack();
    SetUpSigActionHandler();

    // loop waiting for stuff to do.

    while (1)
    {
        // Read all pending events.
        int                         ident;
        int                         events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
        {

            // Process this event.
            if (source != NULL)
            {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER)
            {
                if (engine.accelerometerSensor != NULL)
                {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0)
                    {
                        /*LOGI("accelerometer: x=%f y=%f z=%f",
                            event.acceleration.x, event.acceleration.y,
                            event.acceleration.z);*/
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0)
            {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating)
        {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1)
            {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}

// 3 methods of backtracing are supported:
// - LIBUNWIND_WITH_REGISTERS_METHOD
// - UNWIND_BACKTRACE_WITH_REGISTERS_METHOD
// - UNWIND_BACKTRACE_WITH_SKIPPING_METHOD
// LIBUNWIND_WITH_REGISTERS_METHOD works more reliably on 32-bit ARM,
// but it is more difficult to build.

// LIBUNWIND_WITH_REGISTERS_METHOD can only be used on 32-bit ARM.
// Android NDK r16b contains "libunwind.a"
// for "armeabi" and "armeabi-v7a" ABIs.
// We can use this library, but we need matching headers,
// namely "libunwind.h" and "__libunwind_config.h".
// For NDK r16b, the headers can be fetched here:
// https://android.googlesource.com/platform/external/libunwind_llvm/+/ndk-r16/include/
// #if __arm__
// #define LIBUNWIND_WITH_REGISTERS_METHOD 1
// #include "libunwind.h"
// #endif

// UNWIND_BACKTRACE_WITH_REGISTERS_METHOD can only be used on 32-bit ARM.
#if __arm__
#    define UNWIND_BACKTRACE_WITH_REGISTERS_METHOD 1
#endif

// UNWIND_BACKTRACE_WITH_SKIPPING_METHOD be used on any platform.
// It usually does not work on devices with 32-bit ARM CPUs.
// Usually works on devices with 64-bit ARM CPUs in 32-bit mode though.
#define UNWIND_BACKTRACE_WITH_SKIPPING_METHOD 1

#define HIDE_EXPORTS 1
#include <unwind.h>

#define ENABLE_DEMANGLING 1
#if __cplusplus
extern "C"
#endif
    char*
    __cxa_demangle(const char* mangled_name, char* output_buffer, size_t* length, int* status);

#include <dlfcn.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const size_t address_count_max = 30;

struct BacktraceState {
    // On ARM32 architecture this context is needed
    // for getting backtrace of the before-crash stack,
    // not of the signal handler stack.
    const ucontext_t* signal_ucontext;

    // On non-ARM32 architectures signal handler stack
    // seems to be "connected" to the before-crash stack,
    // so we only need to skip several initial addresses.
    size_t address_skip_count;

    size_t    address_count;
    uintptr_t addresses[address_count_max];
};
typedef struct BacktraceState BacktraceState;

void BacktraceState_Init(BacktraceState* state, const ucontext_t* ucontext)
{
    LNK_ASSERT(state);
    LNK_ASSERT(ucontext);
    memset(state, 0, sizeof(BacktraceState));
    state->signal_ucontext    = ucontext;
    state->address_skip_count = 3;
}

bool BacktraceState_AddAddress(BacktraceState* state, uintptr_t ip)
{
    LNK_ASSERT(state);

    // No more space in the storage. Fail.
    if (state->address_count >= address_count_max)
        return false;

#if __thumb__
    // Reset the Thumb bit, if it is set.
    const uintptr_t thumb_bit = 1;
    ip &= ~thumb_bit;
#endif

    if (state->address_count > 0)
    {
        // Ignore null addresses.
        // They sometimes happen when using _Unwind_Backtrace()
        // with the compiler optimizations,
        // when the Link Register is overwritten by the inner
        // stack frames, like PreCrash() functions in this example.
        if (ip == 0)
            return true;

        // Ignore duplicate addresses.
        // They sometimes happen when using _Unwind_Backtrace()
        // with the compiler optimizations,
        // because we both add the second address from the Link Register
        // in ProcessRegisters() and receive the same address
        // in UnwindBacktraceCallback().
        if (ip == state->addresses[state->address_count - 1])
            return true;
    }

    // Finally add the address to the storage.
    state->addresses[state->address_count++] = ip;
    return true;
}

#define _my_log(x) __android_log_print(ANDROID_LOG_INFO, "com.Android2", x)
#define _my_log2(x, y, z, w) __android_log_print(ANDROID_LOG_INFO, "com.Android2", x, y, z, w)
#define _my_log3(x, y) __android_log_print(ANDROID_LOG_INFO, "com.Android2", x, y)

#if LIBUNWIND_WITH_REGISTERS_METHOD

void LibunwindWithRegisters(BacktraceState* state)
{
    LNK_ASSERT(state);

    // Initialize unw_context and unw_cursor.
    unw_context_t unw_context = {};
    unw_getcontext(&unw_context);
    unw_cursor_t unw_cursor = {};
    unw_init_local(&unw_cursor, &unw_context);

    // Get more contexts.
    const ucontext_t* signal_ucontext = state->signal_ucontext;
    LNK_ASSERT(signal_ucontext);
    const struct sigcontext* signal_mcontext = &(signal_ucontext->uc_mcontext);
    LNK_ASSERT(signal_mcontext);

    // Set registers.
    unw_set_reg(&unw_cursor, UNW_ARM_R0, signal_mcontext->arm_r0);
    unw_set_reg(&unw_cursor, UNW_ARM_R1, signal_mcontext->arm_r1);
    unw_set_reg(&unw_cursor, UNW_ARM_R2, signal_mcontext->arm_r2);
    unw_set_reg(&unw_cursor, UNW_ARM_R3, signal_mcontext->arm_r3);
    unw_set_reg(&unw_cursor, UNW_ARM_R4, signal_mcontext->arm_r4);
    unw_set_reg(&unw_cursor, UNW_ARM_R5, signal_mcontext->arm_r5);
    unw_set_reg(&unw_cursor, UNW_ARM_R6, signal_mcontext->arm_r6);
    unw_set_reg(&unw_cursor, UNW_ARM_R7, signal_mcontext->arm_r7);
    unw_set_reg(&unw_cursor, UNW_ARM_R8, signal_mcontext->arm_r8);
    unw_set_reg(&unw_cursor, UNW_ARM_R9, signal_mcontext->arm_r9);
    unw_set_reg(&unw_cursor, UNW_ARM_R10, signal_mcontext->arm_r10);
    unw_set_reg(&unw_cursor, UNW_ARM_R11, signal_mcontext->arm_fp);
    unw_set_reg(&unw_cursor, UNW_ARM_R12, signal_mcontext->arm_ip);
    unw_set_reg(&unw_cursor, UNW_ARM_R13, signal_mcontext->arm_sp);
    unw_set_reg(&unw_cursor, UNW_ARM_R14, signal_mcontext->arm_lr);
    unw_set_reg(&unw_cursor, UNW_ARM_R15, signal_mcontext->arm_pc);

    unw_set_reg(&unw_cursor, UNW_REG_IP, signal_mcontext->arm_pc);
    unw_set_reg(&unw_cursor, UNW_REG_SP, signal_mcontext->arm_sp);

    // unw_step() does not return the first IP,
    // the address of the instruction which caused the crash.
    // Thus let's add this address manually.
    BacktraceState_AddAddress(state, signal_mcontext->arm_pc);

    _my_log3("unw_is_signal_frame(): %i\n", unw_is_signal_frame(&unw_cursor));

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&unw_cursor) > 0)
    {
        unw_word_t ip = 0;
        unw_get_reg(&unw_cursor, UNW_REG_IP, &ip);

        bool ok = BacktraceState_AddAddress(state, ip);
        if (!ok)
            break;

        _my_log3("unw_is_signal_frame(): %i\n", unw_is_signal_frame(&unw_cursor));
    }
}

#endif // #if LIBUNWIND_WITH_REGISTERS_METHOD

#if UNWIND_BACKTRACE_WITH_REGISTERS_METHOD

void ProcessRegisters(struct _Unwind_Context* unwind_context, BacktraceState* state)
{
    LNK_ASSERT(unwind_context);
    LNK_ASSERT(state);

    const ucontext_t* signal_ucontext = state->signal_ucontext;
    LNK_ASSERT(signal_ucontext);

    const struct sigcontext* signal_mcontext = &(signal_ucontext->uc_mcontext);
    LNK_ASSERT(signal_mcontext);

    _Unwind_SetGR(unwind_context, REG_R0, signal_mcontext->arm_r0);
    _Unwind_SetGR(unwind_context, REG_R1, signal_mcontext->arm_r1);
    _Unwind_SetGR(unwind_context, REG_R2, signal_mcontext->arm_r2);
    _Unwind_SetGR(unwind_context, REG_R3, signal_mcontext->arm_r3);
    _Unwind_SetGR(unwind_context, REG_R4, signal_mcontext->arm_r4);
    _Unwind_SetGR(unwind_context, REG_R5, signal_mcontext->arm_r5);
    _Unwind_SetGR(unwind_context, REG_R6, signal_mcontext->arm_r6);
    _Unwind_SetGR(unwind_context, REG_R7, signal_mcontext->arm_r7);
    _Unwind_SetGR(unwind_context, REG_R8, signal_mcontext->arm_r8);
    _Unwind_SetGR(unwind_context, REG_R9, signal_mcontext->arm_r9);
    _Unwind_SetGR(unwind_context, REG_R10, signal_mcontext->arm_r10);
    _Unwind_SetGR(unwind_context, REG_R11, signal_mcontext->arm_fp);
    _Unwind_SetGR(unwind_context, REG_R12, signal_mcontext->arm_ip);
    _Unwind_SetGR(unwind_context, REG_R13, signal_mcontext->arm_sp);
    _Unwind_SetGR(unwind_context, REG_R14, signal_mcontext->arm_lr);
    _Unwind_SetGR(unwind_context, REG_R15, signal_mcontext->arm_pc);

    // Program Counter register aka Instruction Pointer will contain
    // the address of the instruction where the crash happened.
    // UnwindBacktraceCallback() will not supply us with it.
    BacktraceState_AddAddress(state, signal_mcontext->arm_pc);
}

_Unwind_Reason_Code UnwindBacktraceWithRegistersCallback(struct _Unwind_Context* unwind_context, void* state_voidp)
{
    LNK_ASSERT(unwind_context);
    LNK_ASSERT(state_voidp);

    BacktraceState* state = (BacktraceState*)state_voidp;
    LNK_ASSERT(state);

    // On the first UnwindBacktraceCallback() call,
    // set registers to _Unwind_Context and BacktraceState.
    if (state->address_count == 0)
    {
        ProcessRegisters(unwind_context, state);
        return _URC_NO_REASON;
    }

    uintptr_t ip = _Unwind_GetIP(unwind_context);
    bool      ok = BacktraceState_AddAddress(state, ip);
    if (!ok)
        return _URC_END_OF_STACK;

    return _URC_NO_REASON;
}

void UnwindBacktraceWithRegisters(BacktraceState* state)
{
    LNK_ASSERT(state);
    _Unwind_Backtrace(UnwindBacktraceWithRegistersCallback, state);
}

#endif // #if UNWIND_BACKTRACE_WITH_REGISTERS_METHOD

#if UNWIND_BACKTRACE_WITH_SKIPPING_METHOD

_Unwind_Reason_Code UnwindBacktraceWithSkippingCallback(struct _Unwind_Context* unwind_context, void* state_voidp)
{
    LNK_ASSERT(unwind_context);
    LNK_ASSERT(state_voidp);

    BacktraceState* state = (BacktraceState*)state_voidp;
    LNK_ASSERT(state);

    // Skip some initial addresses, because they belong
    // to the signal handler frame.
    if (state->address_skip_count > 0)
    {
        state->address_skip_count--;
        return _URC_NO_REASON;
    }

    uintptr_t ip = _Unwind_GetIP(unwind_context);
    bool      ok = BacktraceState_AddAddress(state, ip);
    if (!ok)
        return _URC_END_OF_STACK;

    return _URC_NO_REASON;
}

void UnwindBacktraceWithSkipping(BacktraceState* state)
{
    LNK_ASSERT(state);
    _Unwind_Backtrace(UnwindBacktraceWithSkippingCallback, state);
}

#endif // #if UNWIND_BACKTRACE_WITH_SKIPPING_METHOD

void PrintBacktrace(BacktraceState* state)
{
    LNK_ASSERT(state);

    size_t frame_count = state->address_count;
    for (size_t frame_index = 0; frame_index < frame_count; ++frame_index)
    {

        void* address = (void*)(state->addresses[frame_index]);
        LNK_ASSERT(address);

        const char* symbol_name = "";

        Dl_info info = {};
        if (dladdr(address, &info) && info.dli_sname)
        {
            symbol_name = info.dli_sname;
        }

        // Relative address matches the address which "nm" and "objdump"
        // utilities give you, if you compiled position-independent code
        // (-fPIC, -pie).
        // Android requires position-independent code since Android 5.0.
        unsigned long relative_address = (char*)address - (char*)info.dli_fbase;

        char* demangled = NULL;

#if ENABLE_DEMANGLING
        int status = 0;
        demangled  = __cxa_demangle(symbol_name, NULL, NULL, &status);
        if (demangled)
            symbol_name = demangled;
#endif

        LNK_ASSERT(symbol_name);
        _my_log2("  #%02zu:  0x%lx  %s\n", frame_index, relative_address, symbol_name);

        free(demangled);
    }
}

// void printStackTrace(unsigned int max_frames = 63)
//{
//   void* addrlist[max_frames + 1];
//
//   // retrieve current stack addresses
//   u32 addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
//
//   if (addrlen == 0)
//   {
//       printf(stderr, "  <empty, possibly corrupt>\n");
//       return;
//   }
//
//   char** symbollist = backtrace_symbols(addrlist, addrlen);
//
//   for (u32 i = 3; i < addrlen; i++)
//       printf(stderr, "%s\n", symbollist[i]) :
// }

void SigActionHandler(int sig, siginfo_t* info, void* ucontext)
{
    const ucontext_t* signal_ucontext = (const ucontext_t*)ucontext;
    LNK_ASSERT(signal_ucontext);

    // dump_stack();
    // exit(sig);

#if LIBUNWIND_WITH_REGISTERS_METHOD
    {
        _my_log("Backtrace captured using LIBUNWIND_WITH_REGISTERS_METHOD:\n");
        BacktraceState backtrace_state;
        BacktraceState_Init(&backtrace_state, signal_ucontext);
        LibunwindWithRegisters(&backtrace_state);
        PrintBacktrace(&backtrace_state);
    }
#endif

#if UNWIND_BACKTRACE_WITH_REGISTERS_METHOD
    {
        _my_log("Backtrace captured using UNWIND_BACKTRACE_WITH_REGISTERS_METHOD:\n");
        BacktraceState backtrace_state;
        BacktraceState_Init(&backtrace_state, signal_ucontext);
        UnwindBacktraceWithRegisters(&backtrace_state);
        PrintBacktrace(&backtrace_state);
    }
#endif

#if UNWIND_BACKTRACE_WITH_SKIPPING_METHOD
    {
        _my_log("Backtrace captured using UNWIND_BACKTRACE_WITH_SKIPPING_METHOD:\n");
        BacktraceState backtrace_state;
        BacktraceState_Init(&backtrace_state, signal_ucontext);
        UnwindBacktraceWithSkipping(&backtrace_state);
        PrintBacktrace(&backtrace_state);
    }
#endif

    exit(sig);
}

void SetUpAltStack()
{
    // Set up an alternate signal handler stack.
    stack_t stack  = {};
    stack.ss_size  = 0;
    stack.ss_flags = 0;
    stack.ss_size  = SIGSTKSZ;
    stack.ss_sp    = malloc(stack.ss_size);
    LNK_ASSERT(stack.ss_sp);

    sigaltstack(&stack, NULL);
}

void SetUpSigActionHandler()
{
    // Set up signal handler.
    struct sigaction action = {};
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_sigaction = SigActionHandler;
    action.sa_flags     = SA_RESTART | SA_SIGINFO | SA_ONSTACK;

    sigemptyset(&action.sa_mask);

    sigaction(SIGABRT, &action, NULL);
    sigaction(SIGSEGV, &action, NULL);
    sigaction(SIGBUS, &action, NULL);
    sigaction(SIGILL, &action, NULL);
    sigaction(SIGFPE, &action, NULL);
    sigaction(SIGPIPE, &action, NULL);
}

#include <iostream>
#include <iomanip>

#include <unwind.h>
#include <dlfcn.h>

namespace {

struct BacktraceStateNS {
    void** current;
    void** end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
{
    BacktraceStateNS* state = static_cast<BacktraceStateNS*>(arg);
    uintptr_t         pc    = _Unwind_GetIP(context);
    if (pc)
    {
        if (state->current == state->end)
        {
            return _URC_END_OF_STACK;
        }
        else
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

} // namespace

size_t captureBacktrace(void** buffer, size_t max)
{
    BacktraceStateNS state = {buffer, buffer + max};
    _Unwind_Backtrace(unwindCallback, &state);

    return state.current - buffer;
}

void dumpBacktrace(std::ostream& os, void** buffer, size_t count)
{
    for (size_t idx = 0; idx < count; ++idx)
    {
        const void* addr   = buffer[idx];
        const char* symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname)
        {
            symbol = info.dli_sname;
        }

        os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
    }
}

#include <sstream>

void backtraceToLogcat()
{
    const size_t       max = 30;
    void*              buffer[max];
    std::ostringstream oss;

    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));

    __android_log_print(ANDROID_LOG_INFO, "com.Android2", "%s", oss.str().c_str());
}

struct android_backtrace_state {
    void** current;
    void** end;
};

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context, void* arg)
{
    android_backtrace_state* state = (android_backtrace_state*)arg;
    uintptr_t                pc    = _Unwind_GetIP(context);
    if (pc)
    {
        if (state->current == state->end)
        {
            return _URC_END_OF_STACK;
        }
        else
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

#include <cxxabi.h>

void dump_stack(void)
{
    _my_log("android stack dump");

    const int max = 100;
    void*     buffer[max];

    android_backtrace_state state;
    state.current = buffer;
    state.end     = buffer + max;

    _Unwind_Backtrace(android_unwind_callback, &state);

    int count = (int)(state.current - buffer);

    for (int idx = 0; idx < count; idx++)
    {
        const void* addr   = buffer[idx];
        const char* symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname)
        {
            symbol = info.dli_sname;
        }
        int   status    = 0;
        char* demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status);

        _my_log2("%03d: 0x%p %s", idx, addr, (NULL != demangled && 0 == status) ? demangled : symbol);

        if (NULL != demangled)
            free(demangled);
    }

    _my_log("android stack dump done");
}
