package org.citra.citra_emu

import android.app.Application
import android.content.Context
import android.graphics.SurfaceTexture
import android.util.AttributeSet
import android.util.Log
import android.view.Surface
import android.widget.Toast
import com.leia.core.LogLevel
import com.leia.sdk.LeiaSDK
import com.leia.sdk.views.InputViewsAsset
import com.leia.sdk.views.InterlacedSurfaceView
import org.citra.citra_emu.vendor.simongellis.leia.webxr.LeiaTextureRenderer
import org.citra.citra_emu.vendor.simongellis.leia.webxr.RendererImpl

class LeiaSurfaceView(context: Context, attrs: AttributeSet) : InterlacedSurfaceView(context, attrs) {
    private val textureRenderer = LeiaTextureRenderer()
    private val asset = InputViewsAsset(RendererImpl(textureRenderer))
        
    // constructor(context: Context?) : this(context, null)
    // constructor(context: Context?, attrs: AttributeSet?) : super(context, attrs)

    fun init() {
        Log.i("LeiaHelper3D: LeiaSurfaceView","init")
        setViewAsset(asset)
    }

    fun setSurfaceListener(surfaceListener: LeiaHelper3D.SurfaceListener) {
        val surfaceAsset = InputViewsAsset.createEmptySurfaceForVideo {
                surfaceTexture: SurfaceTexture? ->
            Log.d("SurfaceListener", "createEmptySurfaceForVideo -> calling onSurfaceChanged")
            surfaceListener.onSurfaceChanged(Surface(surfaceTexture))
        }
        setViewAsset(surfaceAsset)
    }

    fun addTexture(texture: SurfaceTexture, transform: FloatArray) {
        Log.d("SurfaceListener", "addTexture")
        textureRenderer.addTexture(texture, transform)
    }
}

class LeiaHelper3D {

    fun interface SurfaceListener {
        fun onSurfaceChanged(surface: Surface)
    }
    
    companion object {
        fun init(application: Application) {
            try {
                val initArgs = LeiaSDK.InitArgs().apply {
                    platform.app = application
                    platform.logLevel = LogLevel.Trace
                }
                val leiaSDK = LeiaSDK.createSDK(initArgs)
                leiaSDK.startFaceTracking(false)
            } catch (e: Exception) {
                Log.e("MainApp", "Failed to initialize LeiaSDK: ${e.message}")
                Toast.makeText(application, "Failed to initialize LeiaSDK", Toast.LENGTH_SHORT).show()
            }
        }

        fun update3dMode(surfaceView: InterlacedSurfaceView, enable3dMode: Boolean, hasFocus: Boolean) {
            surfaceView.config.use { config ->
                config.setNumTiles(if (enable3dMode) 2 else 1, 1)
            }

            val leiaSDK = LeiaSDK.getInstance()
            leiaSDK?.let {
                it.startFaceTracking(enable3dMode && hasFocus)
                it.enableBacklight(enable3dMode && hasFocus)
            }
        }
    }
}
