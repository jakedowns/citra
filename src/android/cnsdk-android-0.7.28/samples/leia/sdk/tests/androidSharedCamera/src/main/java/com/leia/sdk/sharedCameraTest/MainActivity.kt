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
package com.leia.sdk.sharedCameraTest

import android.annotation.SuppressLint
import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.ImageFormat
import android.graphics.SurfaceTexture
import android.graphics.Matrix
import android.hardware.camera2.CameraCaptureSession
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraDevice
import android.hardware.camera2.CameraManager
import android.hardware.camera2.CaptureRequest
import android.hardware.camera2.params.OutputConfiguration
import android.hardware.camera2.params.SessionConfiguration
import android.media.ImageReader
import android.widget.Switch
import android.os.AsyncTask
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import android.os.SystemClock
import android.util.Log
import android.util.Range
import android.view.Surface
import android.view.TextureView
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.view.isVisible
import com.leia.core.LogLevel
import com.leia.core.SharedCameraSink
import com.leia.sdk.FaceTrackingRuntime
import com.leia.sdk.LeiaSDK
import com.leia.sdk.views.InputViewsAsset
import com.leia.sdk.views.InterlacedSurfaceView
import java.util.concurrent.Executor

private val TAG = MainActivity::class.java.simpleName
class MainActivity : AppCompatActivity(), ActivityCompat.OnRequestPermissionsResultCallback {
	private var cameraSample: CameraSample? = null
	private val PERMISSION_REQUEST_CODE_CAMERA = 1337

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)

		if (LeiaSDK.isFaceTrackingInService()) {
			val msg = "Cannot setup shared camera sample: LeiaSDK compiled with in-service face tracking"
			Log.e(TAG, msg)
			Toast.makeText(this, msg, Toast.LENGTH_LONG).show()
			finish()
			return
		}

		setContentView(R.layout.activity_main)

		supportActionBar?.hide()

		val newViewsAsset = InputViewsAsset()
		newViewsAsset.LoadBitmapFromPathIntoSurface("image_0.jpg", this, null)

		interlacedView = findViewById(R.id.interlacedViewFullScreen)
		interlacedView.setViewAsset(newViewsAsset)
		interlacedView.config.use { config -> config.isGuiVisible = !config.isGuiVisible }

		toggleViewsButton = findViewById(R.id.toggleViewsButton)
		toggleViewsButton.setOnClickListener {
			toggleViews()
		}
		toggleViews()

		val textureView = findViewById<TextureView>(R.id.textureView)
		textureViewListener = TextureViewListener(textureView)
		textureView.surfaceTextureListener = textureViewListener
		if (textureView.isAvailable) {
			textureView.surfaceTexture?.let {
				textureView.surfaceTextureListener?.onSurfaceTextureAvailable(it, textureView.width, textureView.height)
			}
		}

		val trackingSwitch = findViewById<Switch>(R.id.trackingSwitch)
		trackingSwitch?.setOnCheckedChangeListener { _, isChecked ->
			trackingSwitch.text = if (isChecked) { "Stop Tracking" } else { "Start Tracking" }
			if (isChecked) {
				if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
					ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.CAMERA), PERMISSION_REQUEST_CODE_CAMERA)
				} else {
					startTracking()
				}
			} else {
				stopTracking()
			}
		}
		trackingSwitch?.isChecked = false;

		try {
			val initArgs = LeiaSDK.InitArgs()
			// Set this to true to let LeiaSDK know that it will get camera frames from the app and not to initialize camera by itself.
			initArgs.useSharedCameraSink = true
			initArgs.platform.logLevel = LogLevel.Trace
			initArgs.platform.activity = this
			initArgs.delegate = CNSDKDelegate()

			val leiaSDK = LeiaSDK.createSDK(initArgs)
			leiaSDK.faceTrackingRuntime = FaceTrackingRuntime.InApp
		} catch (e: Exception) {
			Log.e(TAG, String.format("Failed to initialize LeiaSDK: %s", e.toString()))
			e.printStackTrace()

			Toast.makeText(this, "Failed to initialize LeiaSDK", Toast.LENGTH_LONG).show()
			finish()
		}
	}

	@Volatile
	private var isTrackingStartRequested = false
	private val trackingStatusMutex = Object()
	private fun tryStartTracking() {
		LeiaSDK.getInstance()?.let { leiaSDK ->
			synchronized(trackingStatusMutex) {
				if (cameraSample != null || !isTrackingStartRequested) {
					return
				}

				// sharedCameraSink is used to send camera frames to the LeiaSDK for face-tracking
				val sharedCameraSink = leiaSDK.sharedCameraSink

				val surface = textureViewListener.surface
				if (!leiaSDK.isInitialized) {
					Log.e(TAG, "LeiaSDK is not yet initialized")
				} else if (surface == null) {
					Log.e(TAG, "Surface is not ready for use")
				} else if (sharedCameraSink == null) {
					Log.e(TAG, "LeiaSDK initialized without shared camera sink support")
				} else {
					leiaSDK.enableFaceTracking(true)
					cameraSample = CameraSample(this, sharedCameraSink, surface)
				}
			}
		}
	}

	private var hasFocus = false
	override fun onWindowFocusChanged(hasFocus: Boolean) {
		super.onWindowFocusChanged(hasFocus)
		this.hasFocus = hasFocus
		updateBacklight()
	}

	private fun updateBacklight() {
		LeiaSDK.getInstance()?.enableBacklight(hasFocus && isTrackingStarted && viewMode.requiresBacklight())
	}

	override fun onDestroy() {
		super.onDestroy()
		LeiaSDK.shutdownSDK()
	}

	private fun startTracking() {
		toggleViewsButton.visibility = View.VISIBLE

		synchronized(trackingStatusMutex) {
			isTrackingStartRequested = true
			tryStartTracking()
		}
	}

	private fun stopTracking() {
		toggleViewsButton.visibility = View.INVISIBLE

		synchronized(trackingStatusMutex) {
			isTrackingStartRequested = false

			cameraSample?.close()
			cameraSample = null
		}

		LeiaSDK.getInstance()?.enableFaceTracking(false)
	}

	private var isTrackingStarted = false
	private inner class CNSDKDelegate : LeiaSDK.Delegate {
		override fun didInitialize(leiaSDK: LeiaSDK) {
			tryStartTracking()
		}

		override fun onFaceTrackingFatalError(leiaSDK: LeiaSDK) {
			isTrackingStarted = false
			updateBacklight()
		}

		override fun onFaceTrackingStarted(leiaSDK: LeiaSDK) {
			isTrackingStarted = true
			updateBacklight()
		}

		override fun onFaceTrackingStopped(leiaSDK: LeiaSDK) {
			isTrackingStarted = false
			updateBacklight()
		}
	}

	override fun onRequestPermissionsResult(
		requestCode: Int, permissions: Array<String?>,
		grantResults: IntArray
	) {
		if (PERMISSION_REQUEST_CODE_CAMERA != requestCode) {
			super.onRequestPermissionsResult(requestCode,
				permissions,
				grantResults);
			return;
		}

		if (grantResults.size == 1 &&
			grantResults[0] == PackageManager.PERMISSION_GRANTED) {
			runOnUiThread { startTracking() }
		}
	}

	enum class ViewMode {
		Camera, Image;

		fun next(): ViewMode {
			val modes = values()
			var index = modes.indexOf(this)
			index = (index + 1) % modes.size
			return modes[index]
		}

		fun requiresBacklight(): Boolean {
			return this == Image
		}
	}

	private lateinit var interlacedView: InterlacedSurfaceView
	private lateinit var toggleViewsButton: Button
	private var viewMode = ViewMode.Camera
	private fun toggleViews() {
		viewMode = viewMode.next()

		toggleViewsButton.text = "Show ${viewMode.next()}"
		if (viewMode == ViewMode.Camera) {
			interlacedView.isVisible = false
		} else if (viewMode == ViewMode.Image) {
			interlacedView.isVisible = true
		}
		updateBacklight()
	}

	private inner class TextureViewListener(val textureView: TextureView) : TextureView.SurfaceTextureListener {
		var surface: Surface? = null

		private val targetSurfaceWidth = 1280
		private val targetSurfaceHeight = 720

		override fun onSurfaceTextureAvailable(p0: SurfaceTexture, p1: Int, p2: Int) {
			p0.setDefaultBufferSize(targetSurfaceWidth, targetSurfaceHeight)
			resizeTextureView(p1, p2)
			surface = Surface(p0)

			tryStartTracking()
		}

		override fun onSurfaceTextureDestroyed(p0: SurfaceTexture): Boolean {
			surface = null
			return true
		}

		override fun onSurfaceTextureSizeChanged(p0: SurfaceTexture, p1: Int, p2: Int) {
			// TextureView might reset the buffer size back to match the screen.
			// But we want it to match with target stream size at all times.
			p0.setDefaultBufferSize(targetSurfaceWidth, targetSurfaceHeight)
		}

		override fun onSurfaceTextureUpdated(p0: SurfaceTexture) {
		}

		private fun resizeTextureView(textureWidth: Int, textureHeight: Int) {
			val rotation = windowManager.defaultDisplay.rotation
			val newHeight = textureHeight
			var newWidth = textureHeight * targetSurfaceHeight / targetSurfaceWidth
			if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
				newWidth = textureHeight * targetSurfaceWidth / targetSurfaceHeight
			}
			val layoutParams = ViewGroup.LayoutParams(newWidth, newHeight)
			textureView.setLayoutParams(layoutParams)
			configureTransform(newWidth, newHeight)
		}

		private fun configureTransform(width: Int, height: Int) {
			val mDisplayOrientation = windowManager.defaultDisplay.rotation * 90
			val matrix = Matrix()
			if (mDisplayOrientation % 180 == 90) {
				// Rotate the camera preview when the screen is landscape.
				matrix.setPolyToPoly(
					floatArrayOf(
						0f, 0f,  // top left
						width.toFloat(), 0f,  // top right
						0f, height.toFloat(),  // bottom left
						width.toFloat(), height.toFloat()
					), 0,
					if (mDisplayOrientation == 90) floatArrayOf(
						0f, height.toFloat(),  // top left
						0f, 0f,  // top right
						width.toFloat(), height.toFloat(),  // bottom left
						width.toFloat(), 0f
					) else floatArrayOf(
						width.toFloat(), 0f,  // top left
						width.toFloat(), height.toFloat(),  // top right
						0f, 0f,  // bottom left
						0f, height.toFloat()
					), 0,
					4
				)
			} else if (mDisplayOrientation == 180) {
				matrix.postRotate(180f, width * 0.5f, height * 0.5f)
			}
			textureView.setTransform(matrix)
		}
	}
	private lateinit var textureViewListener: TextureViewListener
}

class CameraSample(context: Context, private val sharedCameraSink: SharedCameraSink, private val surface: Surface) {
	private val cameraManager = context.getSystemService(CameraManager::class.java)!!
	private lateinit var captureSession: CameraCaptureSession
	var imageReader: ImageReader
	val width: Int = 640
	val height: Int = 480
	val fps: Int = 90

	private val handler: Handler?
	private val handlerThread: HandlerThread?

	private var isSystemTimestamp = false

	init {
		handlerThread = HandlerThread("CameraThread")
		handlerThread.start()
		handler = Handler(handlerThread.looper)

		// The camera frames to sent to SDK must be in this format
		imageReader = ImageReader.newInstance(width, height, ImageFormat.YUV_420_888, 2)
		imageReader.setOnImageAvailableListener({ reader -> onFrameAvailable(reader!!) }, handler)

		// The camera intrinsic must be updated to match LP2 Camera
		sharedCameraSink.updateIntrinsics(
			width,
			height,
			width * (1644.687180124067f / 3264),
			height * (1222.303816782698f / 2448),
			1767.318265546948f * 0.1961f,
			1771.178379069267f * 0.1961f,
			true)

		val surfaces = listOf(surface, imageReader.surface)
		createCameraSession(findCameraId(), surfaces) {
			startPreview(it, surfaces)
		}
	}

	fun close() {
		captureSession.device.close()
		captureSession.close()
	}

	private fun findCameraId(): String {
		return cameraManager.cameraIdList.first {
			cameraManager
				.getCameraCharacteristics(it)
				.get(CameraCharacteristics.LENS_FACING) == CameraCharacteristics.LENS_FACING_FRONT
		}
	}

	private fun onFrameAvailable(reader: ImageReader) {
		reader.acquireLatestImage().use {
			if (it != null) {
				if (isSystemTimestamp) {
					val timestampNow = SystemClock.elapsedRealtimeNanos()
					val elapsedTime = timestampNow - it.timestamp
					Log.d(TAG, String.format("Camera delay: %.2fms", elapsedTime.toDouble() * 1e-6))
				}

				// Send the camera frame
				sharedCameraSink.onImage(it, 0, isSystemTimestamp)
			}
		}
	}

	private fun startPreview(captureSession: CameraCaptureSession, surfaces: List<Surface>) {
		this.captureSession = captureSession
		val captureRequest = captureSession.device.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW).apply {
			surfaces.forEach { addTarget(it) }
			set(CaptureRequest.CONTROL_AE_TARGET_FPS_RANGE, Range(fps, fps))
		}.build()
		captureSession.setRepeatingRequest(captureRequest, null, handler)
	}

	@SuppressLint("MissingPermission")
	private fun createCameraSession(
		cameraId: String,
		logicalTargets: List<Surface>,
		executor: Executor = AsyncTask.SERIAL_EXECUTOR,
		callback: (CameraCaptureSession) -> Unit) {

		val timestampSource = cameraManager
			.getCameraCharacteristics(cameraId)
			.get(CameraCharacteristics.SENSOR_INFO_TIMESTAMP_SOURCE)
		isSystemTimestamp = timestampSource == CameraCharacteristics.SENSOR_INFO_TIMESTAMP_SOURCE_REALTIME

		val configs = ArrayList<OutputConfiguration>()
		configs += logicalTargets.map { OutputConfiguration(it) }

		val sessionConfiguration = SessionConfiguration(
			SessionConfiguration.SESSION_REGULAR,
			configs,
			executor,
			object : CameraCaptureSession.StateCallback() {
				override fun onConfigured(session: CameraCaptureSession) { callback(session) }
				override fun onConfigureFailed(session: CameraCaptureSession) = session.device.close()
			}
		)

		cameraManager.openCamera(
			cameraId,
			object : CameraDevice.StateCallback() {
				override fun onOpened(device: CameraDevice) {
					device.createCaptureSession(sessionConfiguration)
				}
				override fun onError(device: CameraDevice, error: Int) {
					onDisconnected(device)
				}
				override fun onDisconnected(device: CameraDevice) {
					device.close()
				}
			},
			null
		)
	}
}
