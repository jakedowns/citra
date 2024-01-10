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
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Log
import android.view.*
import android.widget.Button
import android.widget.ScrollView
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.view.isVisible
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager2.widget.ViewPager2
import com.google.android.exoplayer2.video.VideoListener
import com.leia.headtracking.HeadTrackingFrame
import com.leia.headtracking.HeadTrackingFrameListener
import com.leia.sdk.FaceTrackingRuntime
import com.leia.sdk.LeiaSDK
import com.leia.sdk.video.MonoVideoMLMethods
import com.leia.sdk.views.InputViewsAsset
import com.leia.sdk.views.InterlacedSurfaceView
import com.leia.sdk.views.InterlacedSurfaceViewConfig
import com.leia.sdk.views.InterlacedSurfaceViewPinchZoom
import com.leia.sdk.views.ScaleType
import java.io.IOException
import java.io.InputStream

private val TAG = MainActivity::class.java.simpleName
class MainActivity : AppCompatActivity(), ActivityCompat.OnRequestPermissionsResultCallback {
	private var moviePlayerAsset: InputViewsAsset? = null
	private var moviePlayerVideoListener: VideoListener? = null
	private lateinit var moviePlayer: MoviePlayer

	private lateinit var interlacedViewFullScreen: InterlacedSurfaceView
	private lateinit var interlacedViewFullScreenPinchZoom: InterlacedSurfaceViewPinchZoom
	private lateinit var interlacedView: InterlacedSurfaceView
	private lateinit var interlacedView3: InterlacedSurfaceView
	private lateinit var scrollableInterlacedView: InterlacedSurfaceView
	private lateinit var scrollView: ScrollView
	private lateinit var viewPager: ViewPager2

	private val PERMISSION_REQUEST_CODE_CAMERA = 1337
	var isSingleViewMode = false

	var show3D = false

	var imageIndex = 0

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)

		setContentView(R.layout.activity_main)

		moviePlayer = MoviePlayer(this)

		interlacedView = findViewById(R.id.interlacedThumb1)
		interlacedViewFullScreen = findViewById(R.id.interlacedViewFullScreen)
		interlacedViewFullScreenPinchZoom = InterlacedSurfaceViewPinchZoom(interlacedViewFullScreen)
		interlacedView3 = findViewById(R.id.interlacedThumb2)
		scrollableInterlacedView = findViewById(R.id.scrollableInterlacedView)
		scrollView = findViewById(R.id.scrollView)

		viewPager = findViewById(R.id.viewPager)
		viewPager.adapter = ImageViewAdapter(this, listOf("image_1.jpg", "image_0.jpg", "test_GM_2x1.png"))
		viewPager.offscreenPageLimit = 1

		// Moving and Resizing an InterlacedSurfaceView
		//
		// Since interlacing depends on the precise location on the screen of the image,
		// we need to keep the InterlacedSurfaceView up-to-date with its position and size.
		// Currently, the rendering of InterlacedSurfaceView and its presentation on the screen is asynchronous.
		// So animating position and size should be done with a few considerations:
		//   * Scaling animations are not supported. Avoid it at all costs.
		//   * Move animations are not supported *by default*.
		//
		// Prevent scaling animations on over-scroll.
		scrollView.overScrollMode = View.OVER_SCROLL_NEVER
		(viewPager.getChildAt(0) as? RecyclerView)?.overScrollMode = View.OVER_SCROLL_NEVER
		//
		// Currently, support of move animations is implemented by forcing a view to move over a predefined pixels grid, i.e. move it with a non-unit step.
		// The grid step depends on the display. For the 9V display, X-step is 3px, and Y-step is 9px.
		// This behavior is disabled by default because InterlacedSurfaceView can be rendered at any location on the screen.
		// There are two ways of approaching this grid alignment: manual and automatic.
		// The automatic approach is driven by ViewTreeObserver.OnPreDrawListener. In onPreDraw it does alignment and position updates. It covers most of the use cases.
		// The manual approach requires the user to call `InterlacedSurfaceView.updatePosition(alignX, alignY)` when the view is moved.
		// This approach is the only way to handle ScrollView and HorizontalScrollView.
		// The known use-cases where the automatic approach is enough - RecyclerView, ViewPager, and AppBarLayout.
		//
		// To activate the automatic position update either call InterlacedSurfaceView.setAutoAlign(alignX, alignY) or add autoAlignX or/and autoAlignY attributes to the XML.
		// If your view is expected to be moving horizontally, it should be aligned over the X-axis. And the same applies to vertical movement and the Y-axis.
		//
		// Align X because the view can move horizontally.
		scrollableInterlacedView.setManualPositionUpdate(true) // Not necessary but preferably to specify your intentions explicitly
		scrollView.setOnScrollChangeListener { _, _, _, _, _ ->
			scrollableInterlacedView.updatePosition(false, true)
		}
		//
		// Alignment for ViewPager children is enabled in image_item.xml via the autoAlignX attribute.

		// InterlacedSurfaceView supports transparency.
		//
		// `supportsTransparency=true` attribute must be specified on the view before use.
		interlacedView3.config.use {
			it.alpha = 0.5f
		}

		findViewById<Button>(R.id.picture_button).setOnClickListener { showTiledPicture("image_0.jpg", 2, 1) }
		findViewById<Button>(R.id.act_button).setOnClickListener { cycleImages() }
		findViewById<Button>(R.id.tests_button).setOnClickListener { showAll() }
		findViewById<Button>(R.id.scroll_button).setOnClickListener { toggleScroll() }
		findViewById<Button>(R.id.viewPager_button).setOnClickListener { showViewPager() }
		findViewById<Button>(R.id.cube_button).setOnClickListener { showCube() }
		findViewById<Button>(R.id.video_2d_button).setOnClickListener { show2DVideo() }
		findViewById<Button>(R.id.guiToggleButton).setOnClickListener {
			var currentFullSurfaceView: InterlacedSurfaceView? = null
			if (interlacedViewFullScreen.isVisible) {
				currentFullSurfaceView = interlacedViewFullScreen
			} else if (viewPager.isVisible) {
				currentFullSurfaceView =
					((viewPager.getChildAt(0) as? RecyclerView)
						?.findViewHolderForAdapterPosition(viewPager.currentItem) as? ImageViewAdapter.ImageViewHolder)
							?.mInterlacedView
			}

			currentFullSurfaceView?.let { view ->
				view.config.use { config ->
					config.isGuiVisible = !config.isGuiVisible
				}
			}
		}

		showTiledPicture("image_0.jpg", 2, 1)

		if (LeiaSDK.isFaceTrackingRuntimeSupported(FaceTrackingRuntime.InApp)) {
			// CNSDK with in-app face tracking must handle camera permission manually.
			val isCameraPermissionGranted = ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED
			if (!isCameraPermissionGranted) {
				ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.CAMERA),  PERMISSION_REQUEST_CODE_CAMERA)
			}
		}
	}

	// Once your application is ready to start rendering 3D content, you must first enable the display backlight and face-tracking.
	private fun set3DMode(show3D: Boolean) {
		this.show3D = show3D

		val enable3D = show3D && hasWindowFocus()
		LeiaSDK.getInstance()?.let {
			it.enableBacklight(enable3D)
			it.startFaceTracking(enable3D)
		}
	}

	override fun onWindowFocusChanged(hasFocus: Boolean) {
		super.onWindowFocusChanged(hasFocus)
		// Manually toggling mode depending on whether activity is in focus or not.
		// TODO: Switch to automatic mode toggling depending on whether there is an active InterlacedSurfaceView.
		set3DMode(show3D)
	}

	override fun onStart() {
		super.onStart()

		// FaceTrackingFrameListener can be used to listen to face tracking frames as soon as they arrive.
		// An alternative is the polling API - LeiaSDK.getPrimaryFace().
		if (faceTrackingListener == null) {
			faceTrackingListener = FaceTrackingListener()
			LeiaSDK.getInstance()?.setFaceTrackingFrameListener(faceTrackingListener!!)
		}

		// MainApp initializes CNSDK with face tracking in the paused state.
		// From now on, CNSDK will automatically pause/resume tracking when needed.
		LeiaSDK.getInstance()?.startFaceTracking(true)

		if (cnsdkDelegate == null) {
			cnsdkDelegate = CNSDKDelegate()
			(application as MainApp).addCnsdkDelegate(cnsdkDelegate!!)
		}
	}

	override fun onResume() {
		super.onResume()

		if (moviePlayerAsset != null) {
			Log.d(TAG, "moviePlayer.play()")
			moviePlayer.play()
		}
	}

	override fun onPause() {
		super.onPause()

		moviePlayer.saveCurrentPosition()
		moviePlayer.exoPlayer.stop()
	}

	override fun onStop () {
		super.onStop()

		moviePlayer.exoPlayer.stop()

		cnsdkDelegate?.let { (application as MainApp).removeCnsdkDelegate(it) }
		cnsdkDelegate = null

		LeiaSDK.getInstance()?.setFaceTrackingFrameListener(null)
		faceTrackingListener = null

		ml = null
	}

	override fun onDestroy() {
		super.onDestroy()

		moviePlayer.release()
	}

	override fun onTouchEvent(event: MotionEvent?): Boolean {
		return interlacedViewFullScreenPinchZoom.onTouchEvent(event)
	}

	override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String?>, grantResults: IntArray) {
		if (PERMISSION_REQUEST_CODE_CAMERA != requestCode) {
			super.onRequestPermissionsResult(requestCode, permissions, grantResults)
			return
		}

		if (grantResults.size == 1 &&
			grantResults[0] == PackageManager.PERMISSION_GRANTED) {
			LeiaSDK.getInstance()?.enableFaceTracking(true)
		}
	}

	private fun resetVisibility() {
		moviePlayerAsset = null
		moviePlayer.resetCurrentPosition()
		moviePlayer.exoPlayer.pause()
		moviePlayerVideoListener?.let {
			moviePlayer.exoPlayer.removeVideoListener(it)
			moviePlayerVideoListener = null
		}

		viewPager.isVisible = false
		scrollView.isVisible = false
		interlacedView.isVisible = false
		interlacedView3.isVisible = false
		interlacedViewFullScreen.isVisible = false

		interlacedView.releaseInputViewsAsset()
		interlacedView3.releaseInputViewsAsset()
		interlacedViewFullScreen.releaseInputViewsAsset()

		// default to stereo
		interlacedViewFullScreen.config.use {
			it.setNumTiles(2, 1)
			it.setSourceSize(InterlacedSurfaceViewConfig.LENGTH_UNSET, InterlacedSurfaceViewConfig.LENGTH_UNSET)
		}
	}

	private fun showTiledPicture(assetPath: String, tx: Int, ty: Int) {
		resetVisibility()

		set3DMode(true)

		// InterlacedSurfaceView accepts a view asset and interlaces it for Leia Display.
		// For SBS(Side by Side) bitmap, use either
		val asset = InputViewsAsset.loadBitmapFromPathIntoSurface(assetPath, this, null)
		// or provide a bitmap via asset.CreateSurfaceFromLoadedBitmap.
		//
		//
		// After InputViewsAsset is created assign it to InterlacedSurfaceView.
		// This will start the interlacing process and show the content in InterlacedSurfaceView.
		// To replace the image or video in the InterlacedSurfaceView, just create a new asset and assign it. This overwrites the previous asset.
		interlacedViewFullScreen.setViewAsset(asset)

		interlacedViewFullScreen.config.use {
			// We need to specify the tiling layout of the source media to interlace it correctly.
			// SBS Stereo, 2x1 is the default, and doesn't need to be specified if its the ONLY format in use.
			it.setNumTiles(tx, ty)

			// The convergence distance represents the distance at which the object appears exactly on the display in 3D space.
			// Objects in front of the convergence distance will appear to pop out of the display,
			// while objects beyond the convergence distance will appear to be behind the display.
			// Choosing the best value for convergence distance depends on your application’s content.
			//
			it.reconvergenceAmount = 0.0f
		}

		interlacedViewFullScreen.setSingleViewModeListener { isSingleViewMode ->
			// NOTE: this callback is invoked on the render thread
			Log.i(TAG, "InterlacedSurfaceView has entered single view mode: $isSingleViewMode")
		}

		interlacedViewFullScreen.isVisible = true
	}

	private fun cycleImages() {
		if (this.imageIndex==0) showTiledPicture("ACT_2x4.png", 2, 4)
		else if (this.imageIndex==1) showTiledPicture("ACT_ST_2x1.png", 2, 1)
		else if (this.imageIndex==2) showTiledPicture("test_RGB_3x4.png", 3, 4)
		else if (this.imageIndex==3) showTiledPicture("test_GM_2x1.png", 2, 1)
		else if (this.imageIndex==4) showTiledPicture("image_0.jpg", 2, 1)
		else if (this.imageIndex==5) showTiledPicture("image_1.jpg", 2, 1)
		else if (this.imageIndex==6) showTiledPicture("image_2x2.jpg", 2, 2)
		else if (this.imageIndex==7) showTiledPicture("FireIceDancer2_2x1.jpg", 2, 1)
		if (++this.imageIndex>7) this.imageIndex=0
	}

	private fun showAll() {
		resetVisibility()

		showVideo(true)

		// For pictures, sourceSize is determined automatically, there is no need to set manually.
		val birdsAsset = InputViewsAsset.loadBitmapFromPathIntoSurface("image_1.jpg", this, null)
		interlacedView.setViewAsset(birdsAsset)

		interlacedView3.setViewAsset(CubeAsset())

		interlacedView.isVisible = true
		interlacedView3.isVisible = true
	}

	private fun showCube() {
		resetVisibility()

		set3DMode(true)

		val cubeAsset = CubeAsset()
		interlacedViewFullScreen.setViewAsset(cubeAsset)
		interlacedViewFullScreen.isVisible = true
	}

	private fun toggleScroll() {
		if (!scrollView.isVisible) {
			val viewAsset = InputViewsAsset.loadBitmapFromPathIntoSurface("image_1.jpg", this, null)
			scrollableInterlacedView.setViewAsset(viewAsset)
		}
		scrollView.isVisible = !scrollView.isVisible
	}

	private fun showViewPager() {
		resetVisibility()

		set3DMode(true)

		viewPager.isVisible = true
	}

	private fun show2DVideo() {
		resetVisibility()

		showVideo(false)
	}

	private fun showVideo(show3D: Boolean) {
		set3DMode(show3D)

		// While video is loading, InterlacedSurfaceView is going to be black.
		// This can be changed by providing a bitmap to be displayed instead.
		var videoThumbnail: Bitmap? = null
		try {
			val istr: InputStream = resources.assets.open("image_0.jpg")
			videoThumbnail = BitmapFactory.decodeStream(istr)
		} catch (e: IOException) {
			e.printStackTrace()
		}

		moviePlayer.config(null)
		moviePlayer.set3DMode(show3D)
		// For SBS Videos use CreateEmptySurfaceForVideo.
		// After the surface is created, its corresponding surfaceTexture is returned in the SurfaceTextureReadyCallback, this can be used in Exoplayer to render SBS videos.
		moviePlayerAsset = InputViewsAsset.createSurfaceForVideo(videoThumbnail, true) { surfaceTexture ->
			moviePlayer.onSurfaceTextureReady(surfaceTexture)
		}
		val videoListener = object : VideoListener {
			override fun onVideoSizeChanged(width: Int, height: Int, unappliedRotationDegrees: Int, pixelWidthHeightRatio: Float) {
				super.onVideoSizeChanged(width, height, unappliedRotationDegrees, pixelWidthHeightRatio)

				// InterlacedSurfaceView uses sourceSize to adjust rendering when a non-default ScaleType is used (FILL).
				// sourceSize matches to the size of the asset displayed via InterlacedSurfaceView.
				// If it is not known initially (like in this case), set it later.
				interlacedViewFullScreen.config.use {
					it.setSourceSize(width, height)
				}
			}
		}
		moviePlayer.exoPlayer.addVideoListener(videoListener)
		moviePlayerVideoListener = videoListener
		interlacedViewFullScreen.config.use {
			if (show3D) {
				it.scaleType = ScaleType.FILL
				it.setNumTiles(2, 1)
			} else {
				it.scaleType = ScaleType.FIT_CENTER
				it.setNumTiles(1, 1)
			}
		}
		interlacedViewFullScreen.setViewAsset(moviePlayerAsset)
		interlacedViewFullScreen.isVisible = true
	}

	inner class CNSDKDelegate : LeiaSDK.Delegate {
		private val TAG = CNSDKDelegate::class.java.simpleName

		override fun didInitialize(leiaSDK: LeiaSDK) {
			Log.d(TAG, "didInitialize")
			assert(leiaSDK.isInitialized)

			setupML(leiaSDK.ml)

			// The display config is accessible only after CNSDK initialization.
			// API allows querying the current display config and changing it.
			val testConfig = false
			if (testConfig) {
				val config = leiaSDK.config
				config.cameraCenterX += 0.1f
				config.colorInversion = !config.colorInversion
				leiaSDK.config = config
			}
		}

		override fun onFaceTrackingFatalError(leiaSDK: LeiaSDK) {
			// no-op
		}

		override fun onFaceTrackingStarted(leiaSDK: LeiaSDK) {
			Log.i(TAG, "onFaceTrackingStarted")
		}

		override fun onFaceTrackingStopped(leiaSDK: LeiaSDK) {
			Log.i(TAG, "onFaceTrackingStopped")
		}
	}
	var cnsdkDelegate: CNSDKDelegate? = null

	inner class FaceTrackingListener : HeadTrackingFrameListener {
		private val TAG = FaceTrackingListener::class.java.simpleName
		var isProfilingEnabled = false
		override fun onFrame(frame: HeadTrackingFrame) {
			frame.use {
				val leiaSDK = LeiaSDK.getInstance()
				if (leiaSDK == null) {
					return
				}

				val primaryFace = leiaSDK.primaryFace
				if (primaryFace != null) {
					Log.d(TAG, String.format("primary face: %.2f %.2f %.2f", primaryFace.x, primaryFace.y, primaryFace.z))
				}

				Log.d(TAG, String.format("Face tracking frame: t=%d", frame.timestamp.ns))

				if (!isProfilingEnabled) {
					isProfilingEnabled = true
					try {
						leiaSDK.setFaceTrackingProfiling(true)
					} catch (e: java.lang.Exception) {
						Log.e(TAG, "Failed to query face tracking backend")
					}
				}
			}
		}
	}
	var faceTrackingListener: FaceTrackingListener? = null

	inner class MLComponent(leiaML: LeiaSDK.ML, mlImageButton: Button, mlVideoButton: Button) {
		private val TAG = MLComponent::class.java.simpleName

		private var leiaMonoVideoML: MonoVideoMLMethods? = null

		init {
			mlImageButton.setOnClickListener {
				imagePickerResultLauncher.launch("image/*")
			}

			leiaMonoVideoML = leiaML.CreateMonoVideoML(this@MainActivity)
			if (leiaMonoVideoML != null) {
				mlVideoButton.setOnClickListener {
					moviePlayer.config(leiaMonoVideoML)
					moviePlayer.set3DMode(true)
					val videoAsset = InputViewsAsset.createEmptySurfaceForVideo(moviePlayer)
					interlacedViewFullScreen.setViewAsset(videoAsset)
					interlacedView.isVisible = false
					interlacedView3.isVisible = false
				}
			} else {
				mlVideoButton.isVisible = false
			}
		}
		var imagePickerResultLauncher = this@MainActivity.registerForActivityResult(ActivityResultContracts.GetContent()) { selectedFile ->
			if (selectedFile != null) {
				try {
					val inputStream = contentResolver.openInputStream(selectedFile)
					val albedoBitmap = BitmapFactory.decodeStream(inputStream)
					if (albedoBitmap != null) {
						val multiviewImage = LeiaSDK.getInstance()?.ml?.Convert(albedoBitmap, 2)

						val newViewsAsset0 = InputViewsAsset.createSurfaceFromLoadedBitmap(multiviewImage, true)
						interlacedViewFullScreen.setViewAsset(newViewsAsset0)

						moviePlayer.exoPlayer.pause()

						interlacedView.isVisible = false
						interlacedView3.isVisible = false
					}
				} catch (e: Exception) {
					Log.e(TAG, e.toString())
				}
			}
		}
	}
	private var ml: MLComponent? = null

	private fun setupML(leiaML: LeiaSDK.ML?) {
		val mlTestButton = findViewById<View>(R.id.mlTestButton) as Button
		val mlVideoButton = findViewById<View>(R.id.mlVideoButton) as Button

		if (leiaML != null) {
			ml = MLComponent(leiaML, mlTestButton, mlVideoButton)
		} else {
			mlTestButton.isVisible = false
			mlVideoButton.isVisible = false
		}
	}
}

class ImageViewAdapter internal constructor(context: Context?, data: List<String>) :
	RecyclerView.Adapter<ImageViewAdapter.ImageViewHolder>() {

	private val mData: List<String>
	private val mInflater: LayoutInflater

	init {
		mInflater = LayoutInflater.from(context)
		mData = data
	}

	override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ImageViewHolder {
		val view = mInflater.inflate(R.layout.image_item, parent, false)
		return ImageViewHolder(view)
	}

	override fun onBindViewHolder(holder: ImageViewHolder, position: Int) {
		val asset = InputViewsAsset.loadBitmapFromPathIntoSurface(mData[position], holder.itemView.context, null)
		holder.mInterlacedView.setViewAsset(asset)
	}

	override fun onViewRecycled(holder: ImageViewHolder) {
		Log.d("ImageViewAdapter", "onViewRecycled")
		super.onViewRecycled(holder)
		holder.mInterlacedView.visibility = View.GONE
		holder.mInterlacedView.releaseInputViewsAsset()
	}

	override fun getItemCount(): Int {
		return mData.size
	}

	class ImageViewHolder internal constructor(itemView: View) :
		RecyclerView.ViewHolder(itemView) {
		var mInterlacedView: InterlacedSurfaceView

		init {
			mInterlacedView = itemView.findViewById(R.id.depth_image)
		}
	}
}
