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

import android.content.Context
import android.graphics.SurfaceTexture
import android.view.Surface
import com.google.android.exoplayer2.MediaItem
import com.google.android.exoplayer2.SimpleExoPlayer
import com.google.android.exoplayer2.video.VideoListener
import com.leia.sdk.graphics.SurfaceTextureReadyCallback
import com.leia.sdk.video.MonoVideoMLCallback
import com.leia.sdk.video.MonoVideoMLMethods

class MoviePlayer : SurfaceTextureReadyCallback, MonoVideoMLCallback {
    private var mContext : Context? = null;

    var exoPlayer: SimpleExoPlayer
    private var leiaMonoVideoML: MonoVideoMLMethods? = null
    private var currentSurface: Surface? = null
    private var seekToPosition: Long = 0

    // TODO: use a separate class for ML video rendering
    private var depthTexture: SurfaceTexture? = null
    private var srcWidth = 0
    private var srcHeight = 0

    private var show3D = false

    constructor(context : Context) {
        mContext = context
        exoPlayer = SimpleExoPlayer.Builder(context).build()
    }

    fun config(ml: MonoVideoMLMethods?){
        leiaMonoVideoML = ml
        leiaMonoVideoML?.setCallback(this)
    }

    fun set3DMode(show3D: Boolean) {
        this.show3D = show3D
    }

    fun release(){
        exoPlayer.release()
        leiaMonoVideoML?.close()
    }

    override fun onSurfaceTextureReady (surfaceTexture : SurfaceTexture ) {
        if(leiaMonoVideoML != null)
            leiaMonoVideoML?.onSurfaceTextureReady(surfaceTexture)
        else
            onConfigAndPlay(surfaceTexture, surfaceTexture, 0, 0)
    }

    override fun onConfigAndPlay(surfaceTexture: SurfaceTexture, depthTexture: SurfaceTexture,
                                 srcWidth: Int, srcHeight: Int) {
        currentSurface = Surface(surfaceTexture)
        this.depthTexture = depthTexture
        this.srcWidth = srcWidth
        this.srcHeight = srcHeight

        play()
    }

    fun play() {
        if (currentSurface != null) {
            val videoURL: String

            if (leiaMonoVideoML != null) {
                exoPlayer.addVideoListener(
                    object : VideoListener {
                        override fun onVideoSizeChanged(
                            width: Int,
                            height: Int,
                            unappliedRotationDegrees: Int,
                            pixelWidthHeightRatio: Float
                        ) {
                            // Buffer size matches Lumepad resolution without losing aspect ratio.
                            var newWidth =
                                Math.sqrt((width / height * srcWidth * srcHeight).toDouble())
                                    .toFloat()
                            var newHeight: Float = srcWidth * srcHeight / newWidth
                            // Supersample the input texture to increase sharpness
                            newWidth *= 1.5.toFloat()
                            newHeight *= 1.5.toFloat()
                            depthTexture?.setDefaultBufferSize(newWidth.toInt(), newHeight.toInt())
                        }
                    })
                videoURL = "https://drive.google.com/u/1/uc?id=1IY46UO7Qv3yKVIF9zFkGXRsuyuLJVApM&export=download"
            } else {
                if (show3D) {
                    videoURL = "https://drive.google.com/u/1/uc?id=1bI2NeJGkl61WV2N1pRwO6KM5zNLjJcw4&export=download"
                } else {
                    videoURL = "https://drive.google.com/u/1/uc?id=1tbRaxGrGpu8_UlNHI3ZtIf9o11RSgUsI&export=download"
                }
            }

            exoPlayer.setVideoSurface(currentSurface)

            val mediaItem = MediaItem.fromUri(videoURL)
            exoPlayer.setMediaItem(mediaItem)
            exoPlayer.seekTo(seekToPosition)
            exoPlayer.prepare()
            exoPlayer.playWhenReady = true
        }
    }

    fun saveCurrentPosition() {
        seekToPosition = exoPlayer.currentPosition
    }
    fun resetCurrentPosition() {
        seekToPosition = 0
    }
}