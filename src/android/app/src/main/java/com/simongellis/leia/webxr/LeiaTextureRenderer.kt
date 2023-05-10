package com.simongellis.leia.webxr

import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext.GL_TEXTURE_EXTERNAL_OES
import android.opengl.GLES20.*
import android.util.Log
import android.util.Size
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer
import java.nio.ShortBuffer

class LeiaTextureRenderer {
    private val TAG = "LeiaTextureRenderer"

    private val textureHolders = mutableListOf<TextureHolder>()
    private var size = Size(640, 480)
    private var textureSize = Size(640, 480)
    private var overUnder = false
    private var swapImages = false

    private var program = -1
    private var posLocation = -1
    private var texCoordLocation = -1
    private var mvLocation = -1
    private var texLocation = -1
    private var overUnderLocation = -1
    private var swapImagesLocation = -1

    fun setOverUnder(value: Boolean) {
        overUnder = value
    }

    fun getOverUnder(): Boolean {
        return overUnder
    }

    fun setSwapImages(value: Boolean) {
        swapImages = value
    }

    fun getSwapImages(): Boolean {
        return swapImages
    }

    fun addTexture(texture: SurfaceTexture, transform: FloatArray) {
        Log.i(TAG, "addingTexture")
        textureHolders.add(TextureHolder(texture, transform))
    }

    fun onSurfaceCreated() {
        Log.i(TAG, "onSurfaceCreated")
        val textureIds = IntArray(textureHolders.size)
        glGenTextures(textureIds.size, textureIds, 0)
        textureHolders.forEachIndexed { index, textureHolder ->
            val textureId = textureIds[index]
            textureHolder.updateTextureId(textureId)
        }

        program = glCreateProgram()
        val vertexShader = makeShader(GL_VERTEX_SHADER, VERTEX_SHADER)
        glAttachShader(program, vertexShader)
        val fragmentShader = makeShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER)
        glAttachShader(program, fragmentShader)
        glLinkProgram(program)

        posLocation = glGetAttribLocation(program, "a_Pos")
        texCoordLocation = glGetAttribLocation(program, "a_TexCoord")
        mvLocation = glGetUniformLocation(program, "u_MV")
        texLocation = glGetUniformLocation(program, "u_Texture")
        overUnderLocation = glGetUniformLocation(program, "u_OverUnder")
        swapImagesLocation = glGetUniformLocation(program, "u_SwapImages")

        Log.i(TAG, "swapImagesLocation: $swapImagesLocation")
    }

    fun onSurfaceChanged(width: Int, height: Int) {
        Log.i(TAG, "onSurfaceChanged")
        size = Size(width, height)
    }

    fun onDrawFrame() {
        //Log.i(TAG, "onDrawFrame")
        glViewport(0, 0, size.width, size.height)
        logError("glViewport")
        glUseProgram(program)
        logError("glUseProgram")
        for (holder in textureHolders) {
            renderTexture(holder)
        }
    }

    private fun makeShader(type: Int, source: String): Int {
        val shader = glCreateShader(type)
        glShaderSource(shader, source)
        glCompileShader(shader)
        return shader
    }

    private fun renderTexture(holder: TextureHolder) {
        holder.tryUpdateTexImage()
        val textureId = holder.textureId
        val mv = holder.transform

        glActiveTexture(GL_TEXTURE0)
        logError("glActiveTexture")
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId)
        logError("glBindTexture")

        // Set texture parameters to clamp to border and specify black border color
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)


        glUniform1i(texLocation, 0)
        logError("bind tex location")
        glUniformMatrix4fv(mvLocation, 1, false, mv, 0)
        logError("bind mv location")
        glUniform1i(overUnderLocation, (if (overUnder) 1 else 0))
        logError("bind overUnder location")
        glUniform1i(swapImagesLocation, (if (swapImages) 1 else 0))
        logError("bind swapImages location")

        glVertexAttribPointer(
                posLocation,
                VERTEX_SIZE,
                GL_FLOAT,
                false,
                VERTEX_STRIDE,
                SQUARE_POS_VERTICES
        )
        logError("bind pos location")
        glVertexAttribPointer(
                texCoordLocation,
                VERTEX_SIZE,
                GL_FLOAT,
                false,
                VERTEX_STRIDE,
                SQUARE_TEX_VERTICES
        )
        logError("bind tex coord location")
        glEnableVertexAttribArray(posLocation)
        logError("enable pos vertex attrib array")
        glEnableVertexAttribArray(texCoordLocation)
        logError("enable tex coord vertex attrib array")
        glDrawElements(GL_TRIANGLES, SQUARE_INDICES_SIZE, GL_UNSIGNED_SHORT, SQUARE_INDICES)
        logError("draw shit")
    }

    private fun logError(message: String) {
        return;
        var error = glGetError()
        while (error != 0) {
            Log.i(TAG, "${error.toString(16)}: $message")
            if (error == GL_INVALID_FRAMEBUFFER_OPERATION) {
                val status = glCheckFramebufferStatus(GL_FRAMEBUFFER)
                Log.i(TAG, "\tstatus: ${status.toString(16)}")
            }
            error = glGetError()
        }
    }

    private class TextureHolder(private val texture: SurfaceTexture, val transform: FloatArray) {
        var textureId = -1
        private var stale = true
        init {
            texture.setOnFrameAvailableListener { stale = true }
        }

        fun updateTextureId(textureId: Int) {
            this.textureId = textureId
            texture.attachToGLContext(textureId)
        }

        fun tryUpdateTexImage() {
            if (stale) {
                stale = false
                texture.updateTexImage()
            }
        }
    }

    companion object {
        const val VERTEX_SHADER = """
            attribute vec4 a_Pos;
            attribute vec2 a_TexCoord;
            uniform mat4 u_MV;
            varying vec2 v_TexCoord;
            void main() {
                gl_Position = u_MV * a_Pos;
                // leia renders upside down, so flip Y values
                v_TexCoord = vec2(a_TexCoord.x, 1.0f - a_TexCoord.y);
            }
        """

        const val FRAGMENT_SHADER = """
            #extension GL_OES_EGL_image_external : require
            precision mediump float;
            varying vec2 v_TexCoord;
            uniform samplerExternalOES u_Texture;
            uniform int u_OverUnder;
            uniform int u_SwapImages;
            void main() {
                vec2 modifiedTexCoord = vec2(v_TexCoord.x, v_TexCoord.y);
                
                //modifiedTexCoord.y -= 0.05;
                if (u_OverUnder == 1) {
                    modifiedTexCoord.x = modifiedTexCoord.x * 2.0; // our output texture is 2x wide for interlacing so, we need to double the x coordinate
                    if (u_SwapImages == 1){
                        if(v_TexCoord.x > 0.5){
                            modifiedTexCoord.x -= 1.0;
                            // RIGHT EYE OUTPUT (from bottom of input)
                            
                            modifiedTexCoord.y *= 0.5;
                            modifiedTexCoord.y += 0.05;
                        }else{
                            // LEFT EYE OUTPUT (from top of input)
                            modifiedTexCoord.y = modifiedTexCoord.y * 0.5;
                            modifiedTexCoord.y += 0.5;
                        }
                    }else{
                        if(v_TexCoord.x > 0.5){
                            modifiedTexCoord.x -= 1.0;
                            // RIGHT EYE OUTPUT (from bottom of input)
                            modifiedTexCoord.y = modifiedTexCoord.y * 0.5;
                            modifiedTexCoord.y += 0.5;
                        }else{
                            // LEFT EYE OUTPUT (from top of input)
                            modifiedTexCoord.y *= 0.5;
                            modifiedTexCoord.y += 0.05;
                        }
                    }
                }                
                
                gl_FragColor = texture2D(u_Texture, modifiedTexCoord);
                if (gl_FragColor.a < 0.1) {
                    discard;
                }
            }
        """

        const val VERTEX_SIZE = 2
        const val VERTEX_STRIDE = 0

        val SQUARE_POS_VERTICES = floatBufferOf(
                -1f, +1f,
                -1f, -1f,
                +1f, -1f,
                +1f, +1f
        )
        val SQUARE_TEX_VERTICES = floatBufferOf(
                0f, 0f,
                0f, 1f,
                1f, 1f,
                1f, 0f
        )
        val SQUARE_INDICES = shortBufferOf(0, 1, 2, 0, 2, 3)
        val SQUARE_INDICES_SIZE = 6

        private fun floatBufferOf(vararg elements: Float): FloatBuffer {
            val buffer = ByteBuffer.allocateDirect(elements.size * 4)
                    .order(ByteOrder.nativeOrder())
                    .asFloatBuffer()
            buffer.put(elements)
            buffer.position(0)
            return buffer
        }

        private fun shortBufferOf(vararg elements: Short): ShortBuffer {
            val buffer = ByteBuffer.allocateDirect(elements.size * 4)
                    .order(ByteOrder.nativeOrder())
                    .asShortBuffer()
            buffer.put(elements)
            buffer.position(0)
            return buffer
        }

    }
}