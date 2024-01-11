package com.leia

import android.app.Activity
import android.view.MotionEvent

class LWE_Core(activity: Activity) {
    private val cxxPtr: Long = init(activity)

    private external fun init(activity: Activity): Long

    fun close() {
        close(cxxPtr)
    }

    private external fun close(cxxPtr: Long)

    fun onWindowSizeChanged(width: Int, height: Int) {
        onWindowSizeChanged(cxxPtr, width, height)
    }

    private external fun onWindowSizeChanged(cxxPtr: Long, width: Int, height: Int)

    fun drawFrame() {
        drawFrame(cxxPtr)
    }

    private external fun drawFrame(cxxPtr: Long)

    fun onPause() {
        onPause(cxxPtr)
    }

    private external fun onPause(cxxPtr: Long)

    fun onResume() {
        onResume(cxxPtr)
    }

    private external fun onResume(cxxPtr: Long)

    fun isInitialized(): Boolean {
        return isInitialized(cxxPtr)
    }

    private external fun isInitialized(cxxPtr: Long): Boolean

    fun tick(deltaTime: Double) {
        tick(cxxPtr, deltaTime)
    }

    private external fun tick(cxxPtr: Long, deltaTime: Double)

    fun render() {
        render(cxxPtr)
    }

    private external fun render(cxxPtr: Long)

    fun processGuiEvent(motionEvent: MotionEvent) {
        processGuiEvent(cxxPtr, motionEvent)
    }

    private external fun processGuiEvent(cxxPtr: Long, motionEvent: MotionEvent)

    fun setBacklight(enable: Boolean) {
        setBacklight(cxxPtr, enable)
    }

    private external fun setBacklight(cxxPtr: Long, enable: Boolean)

    companion object {
        init {
            System.loadLibrary("lweJni")
        }
    }
}
