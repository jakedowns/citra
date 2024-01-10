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
#include <jni.h>

#include "leia/lwEngine/LWE_Core.h"

struct JNILWE_Core {
    leia::lwe::LWE_Core CoreEngine;

    JNILWE_Core(jobject activity) : CoreEngine(LEIA_GRAPHICS_API_OPENGL)
    {
        leia::lwe::LWE_CoreInitArgs initArgs;
        initArgs.coreConfig.SetPlatformLogLevel(kLeiaLogLevelTrace);
        initArgs.coreConfig.SetPlatformAndroidHandle(LEIA_CORE_ANDROID_HANDLE_ACTIVITY, activity);
        initArgs.appGLContext = eglGetCurrentContext();

        initArgs.window.title                  = "Leia OpenGL Sample";
        initArgs.window.x                      = 0;
        initArgs.window.y                      = 0;
        initArgs.window.width                  = 2560;
        initArgs.window.height                 = 1600;
        initArgs.window.fullscreen             = true;
        initArgs.window.preferSecondaryMonitor = true;
        initArgs.window.proc                   = nullptr;

        CoreEngine.Initialize(initArgs);
    }

    ~JNILWE_Core()
    {
        CoreEngine.Shutdown();
    }
};

extern "C" {

JNIEXPORT jlong JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_init(JNIEnv* jniEnv, jobject, jobject activity)
{
    return (jlong) new JNILWE_Core(activity);
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_close(JNIEnv* jniEnv, jobject, jlong cxxPtr)
{
    delete (JNILWE_Core*)cxxPtr;
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_onWindowSizeChanged(JNIEnv* jniEnv, jobject, jlong cxxPtr, jint width, jint height)
{
    ((JNILWE_Core*)cxxPtr)->CoreEngine.OnWindowSizeChanged(width, height);
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_onPause(JNIEnv* jniEnv, jobject, jlong cxxPtr)
{
    ((JNILWE_Core*)cxxPtr)->CoreEngine.OnPause();
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_onResume(JNIEnv* jniEnv, jobject, jlong cxxPtr)
{
    ((JNILWE_Core*)cxxPtr)->CoreEngine.OnResume();
}

JNIEXPORT jboolean JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_isInitialized(JNIEnv* jniEnv, jobject, jlong cxxPtr)
{
    return ((JNILWE_Core*)cxxPtr)->CoreEngine.IsInitialized();
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_tick(JNIEnv* jniEnv, jobject, jlong cxxPtr, jdouble deltaTime)
{
    ((JNILWE_Core*)cxxPtr)->CoreEngine.Tick(deltaTime);
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_render(JNIEnv* jniEnv, jobject, jlong cxxPtr)
{
    ((JNILWE_Core*)cxxPtr)->CoreEngine.Render(nullptr);
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_processGuiEvent(JNIEnv* jniEnv, jobject, jlong cxxPtr, jobject motionEvent)
{
    ((JNILWE_Core*)cxxPtr)->CoreEngine.ProcessGuiMotionInput(jniEnv, motionEvent);
}

JNIEXPORT void JNICALL Java_com_leia_lwe_java_sample_LWE_1Core_setBacklight(JNIEnv* jniEnv, jobject, jlong cxxPtr, jboolean enable)
{
    ((JNILWE_Core*)cxxPtr)->CoreEngine.SetBacklight(enable);
}

} // extern "C"
