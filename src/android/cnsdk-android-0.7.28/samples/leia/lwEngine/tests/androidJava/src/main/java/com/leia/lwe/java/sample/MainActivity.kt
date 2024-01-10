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
package com.leia.lwe.java.sample

import android.app.Activity
import android.opengl.EGL14
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

private class LWERenderer(val activity: Activity) : GLSurfaceView.Renderer, AutoCloseable {
	private val TAG = LWERenderer::class.java.simpleName

	var lweCore: LWE_Core? = null

	private var width = 0
	private var height = 0
	private var isSizeDirty = false

	private var enableBacklight = true

	fun onPause() {
		lweCore?.onPause()
	}

	fun onResume() {
		lweCore?.onResume()
	}

	fun setBacklight(enable: Boolean) {
		if (lweCore != null) {
			lweCore?.setBacklight(enable)
		} else {
			enableBacklight = enable
		}
	}

	override fun close() {
		lweCore?.close()
		lweCore = null
	}

	override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
		Log.d(TAG, "onSurfaceCreated")
	}

	override fun onSurfaceChanged(p0: GL10?, width: Int, height: Int) {
		Log.d(TAG, "onSurfaceChanged")
		this.width = width
		this.height = height
		this.isSizeDirty = true
}

	override fun onDrawFrame(p0: GL10?) {
		Log.d(TAG, "onDrawFrame")
		if (lweCore == null) { tryInit() }

		val lweCore = lweCore
		if (lweCore != null) {
			lweCore.tick(1.0 / 60.0)

			if (lweCore.isInitialized) {
				if (isSizeDirty) {
					isSizeDirty = false
					lweCore.onWindowSizeChanged(width, height)
				}
				lweCore.render()
			}
		}
	}

	private fun tryInit() {
		if (EGL14.eglGetCurrentContext() == EGL14.EGL_NO_CONTEXT) {
			return
		}

		lweCore = LWE_Core(activity)
		lweCore?.setBacklight(enableBacklight)
	}
}

private val TAG = MainActivity::class.java.simpleName
class MainActivity : AppCompatActivity() {
	private lateinit var lweRenderer: LWERenderer
	private lateinit var surfaceView: GLSurfaceView

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_main)

		lweRenderer = LWERenderer(this)

		surfaceView = findViewById(R.id.interlacedViewFullScreen)

		surfaceView.setOnTouchListener OnTouchListener@{ view, motionEvent -> Boolean
			surfaceView.queueEvent { lweRenderer.lweCore?.processGuiEvent(motionEvent) }
			return@OnTouchListener true
		}

		surfaceView.setEGLContextClientVersion(3)
		surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0)
		surfaceView.setPreserveEGLContextOnPause(true)
		surfaceView.setRenderer(lweRenderer)
	}

	override fun onDestroy() {
		super.onDestroy()
		lweRenderer.close()
	}

	override fun onStop () {
		super.onStop();
		surfaceView.queueEvent { lweRenderer.onPause() }
	}

	override fun onResume() {
		super.onResume()
		surfaceView.queueEvent { lweRenderer.onResume() }
	}

	override fun onPause() {
		surfaceView.queueEvent { lweRenderer.onPause() }
		super.onPause()
	}

	override fun onWindowFocusChanged(hasFocus: Boolean) {
		super.onWindowFocusChanged(hasFocus)
		surfaceView.queueEvent { lweRenderer.setBacklight(hasFocus) }
	}
}
