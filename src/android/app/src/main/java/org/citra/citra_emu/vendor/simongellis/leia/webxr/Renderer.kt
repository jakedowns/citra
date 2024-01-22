package org.citra.citra_emu.vendor.simongellis.leia.webxr

interface Renderer {
    fun onSurfaceCreated()
    fun onSurfaceChanged(width: Int, height: Int)
    fun onDrawFrame()
}