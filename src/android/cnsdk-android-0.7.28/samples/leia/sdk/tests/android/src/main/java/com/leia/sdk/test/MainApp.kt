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
package com.leia.sdk.test

import android.Manifest
import android.app.Application
import android.content.pm.PackageManager
import android.os.Handler
import android.os.Looper
import android.system.Os
import android.util.Log
import androidx.core.app.ActivityCompat
import com.leia.core.LogLevel
import com.leia.sdk.FaceTrackingRuntime
import com.leia.sdk.LeiaSDK

private val TAG = MainApp::class.java.simpleName
class MainApp : Application(), LeiaSDK.Delegate {
    private var isCnsdkInitialized = false

    private val cnsdkDelegateMutex = Object()
    private var subDelegates = mutableSetOf<LeiaSDK.Delegate>()

    private var mainThreadHandler: Handler? = null

    override fun onCreate() {
        super.onCreate()

        mainThreadHandler = Handler(Looper.getMainLooper())

        // CNSDK with in-app face tracking must handle camera permission manually.
        //
        // If the permission is not granted, we postpone face tracking initialization until it's given.
        val isCameraPermissionGranted = ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED

        val enableFaceTracking = isCameraPermissionGranted || !LeiaSDK.isFaceTrackingRuntimeSupported(FaceTrackingRuntime.InApp)

        try {
            val initArgs = LeiaSDK.InitArgs()
            // It's important to provide either initArgs.platform.app or initArgs.platform.activity for CNSDK.
            // If none is available at the time CNSDK must be initialized, provide initArgs.platform.context.
            initArgs.platform.app = this
            initArgs.delegate = this
            initArgs.enableFaceTracking = enableFaceTracking
            // face tracking started in an activity
            initArgs.startFaceTracking = false
            initArgs.platform.logLevel = LogLevel.Trace
            initArgs.faceTrackingServerLogLevel = LogLevel.Trace
            val leiaSDK = LeiaSDK.createSDK(initArgs)
        } catch (e: Exception) {
            Log.e(TAG, String.format("Failed to initialize LeiaSDK: %s", e.toString()))
            e.printStackTrace()
            throw e;
        }
    }

    override fun didInitialize(leiaSDK: LeiaSDK) {
        synchronized(cnsdkDelegateMutex) {
            isCnsdkInitialized = true
            subDelegates.forEach { it.didInitialize(leiaSDK) }
        }
    }

    override fun onFaceTrackingFatalError(leiaSDK: LeiaSDK) {
        synchronized(cnsdkDelegateMutex) {
            subDelegates.forEach { it.onFaceTrackingFatalError(leiaSDK) }
        }

        // When face tracking encounters a fatal error, it's no longer can be used.
        // The best we can do is to disable it, and possibly attempt to enable it later.
        mainThreadHandler?.post { leiaSDK.enableFaceTracking(false) }
    }

    override fun onFaceTrackingStarted(leiaSDK: LeiaSDK) {
        synchronized(cnsdkDelegateMutex) {
            subDelegates.forEach { it.onFaceTrackingStarted(leiaSDK) }
        }
    }

    override fun onFaceTrackingStopped(leiaSDK: LeiaSDK) {
        synchronized(cnsdkDelegateMutex) {
            subDelegates.forEach { it.onFaceTrackingStopped(leiaSDK) }
        }
    }

    fun addCnsdkDelegate(delegate: LeiaSDK.Delegate) {
        synchronized(cnsdkDelegateMutex) {
            subDelegates.add(delegate)
            if (isCnsdkInitialized) {
                val instance = LeiaSDK.getInstance()
                if (instance != null) {
                    delegate.didInitialize(instance)
                }
            }
        }
    }
    fun removeCnsdkDelegate(delegate: LeiaSDK.Delegate) {
        synchronized(cnsdkDelegateMutex) {
            subDelegates.remove(delegate)
        }
    }
}