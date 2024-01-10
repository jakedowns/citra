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
package com.leia.sdk.memory.test

import android.os.Bundle
import android.os.PersistableBundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.leia.sdk.LeiaSDK
import com.leia.core.LogLevel
import com.leia.sdk.views.InputViewsAsset
import com.leia.sdk.views.InterlacedSurfaceView

private val TAG = SDKActivity::class.java.simpleName
class SDKActivity : AppCompatActivity() {
    lateinit var interlacedViewFullScreen: InterlacedSurfaceView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.i(TAG, "onCreate")

        setContentView(R.layout.activity_sdk)

        val initArgs = LeiaSDK.InitArgs()
        initArgs.platform.activity = this
        initArgs.enableFaceTracking = false
        initArgs.startFaceTracking = false
        initArgs.platform.logLevel = LogLevel.Trace
        initArgs.faceTrackingServerLogLevel = LogLevel.Trace
        val leiaSDK = LeiaSDK.createSDK(initArgs)

        interlacedViewFullScreen = findViewById(R.id.interlacedViewFullScreen)
        interlacedViewFullScreen.setViewAsset(InputViewsAsset.loadBitmapFromPathIntoSurface("image_0.jpg", this))
    }

    override fun onResume() {
        super.onResume()
        interlacedViewFullScreen.onResume()
    }

    override fun onPause() {
        super.onPause()
        interlacedViewFullScreen.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "onDestroy")
        LeiaSDK.shutdownSDK()
    }
}
