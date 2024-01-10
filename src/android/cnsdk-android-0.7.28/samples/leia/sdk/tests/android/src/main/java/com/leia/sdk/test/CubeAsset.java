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
package com.leia.sdk.test;

import android.opengl.GLES30;
import android.opengl.Matrix;

import com.leia.core.Vector3;
import com.leia.sdk.graphics.ConvergedPerspectiveViewInfo;
import com.leia.sdk.graphics.Interlacer;
import com.leia.sdk.graphics.RenderTarget;
import com.leia.sdk.views.InputGLBinding;
import com.leia.sdk.views.InputViewsAsset;
import com.leia.sdk.views.InterlacedRenderer;
import com.leia.sdk.views.InterlacedSurfaceViewConfigAccessor;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.ShortBuffer;

/** An example of custom GL-based InputViewsAsset that can be used with InterlacedSurfaceView. */
class CubeAsset extends InputViewsAsset {
    public CubeAsset() {
        super(new Impl());
    }

    static class GLBinding extends InputGLBinding {
        private final CubeRenderer cubeRenderer = new CubeRenderer();
        private final boolean useAtlasForViews = false;
        private Vector3 cameraPos = new Vector3(0,0,0);
        private Vector3 cameraDir = new Vector3(0,1,0);
        private Vector3 cameraUp = new Vector3(0,0,1);
        private float fov = 90.0f * 3.14159f / 180.0f;
        private float nearz = 1.0f;
        private float farz = 10000.0f;
        private boolean isDirty = true;

        GLBinding(CubeAsset.Impl asset) {
            super(asset);
        }

        @Override
        protected void update(InterlacedRenderer renderer, boolean isProtected) {
            super.update(renderer, isProtected);

            if (!mIsValid) {
                return;
            }

            CubeAsset.Impl asset = updateAsset(CubeAsset.Impl.class);
            if (asset == null) {
                return;
            }

            if (!cubeRenderer.update()) {
                close();
                return;
            }

            if (isDirty) {
                isDirty = false;
                try (InterlacedSurfaceViewConfigAccessor accessor = renderer.getConfig()) {
                    accessor.setNumTiles(2, 1);
                    accessor.setBaselineScaling(20.0f);
                    accessor.setConvergenceDistance(cubeRenderer.geometryDist);
                    accessor.setUseAtlasForViews(useAtlasForViews);
                }
            }
        }

        @Override
        protected void reset() {
            cubeRenderer.reset(isValidGLContext());

            super.reset();
        }

        @Override
        protected void render(Interlacer interlacer, int viewportWidth, int viewportHeight) {
            if (!mIsValid) {
                return;
            }

            // Remember viewport values to restore them later.
            IntBuffer initialViewport = IntBuffer.allocate(4);
            GLES30.glGetIntegerv(GLES30.GL_VIEWPORT, initialViewport);

            // Render the cube into each of the views
            for (int viewIndex = 0; viewIndex < 2; viewIndex++) {
                RenderTarget renderTarget = interlacer.getRenderTargetForView(viewIndex);

                GLES30.glBindFramebuffer(GLES30.GL_FRAMEBUFFER, renderTarget.framebuffer);

                if (!useAtlasForViews || viewIndex == 0) {
                    GLES30.glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
                    GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT | GLES30.GL_DEPTH_BUFFER_BIT);
                }

                float aspectRatio = (float)renderTarget.viewportWidth / (float)renderTarget.viewportHeight;

                ConvergedPerspectiveViewInfo convergedPerspectiveViewInfo = interlacer.getConvergedPerspectiveViewInfo(viewIndex, cameraPos, cameraDir, cameraUp, fov, aspectRatio, nearz, farz);

                // Get the camera position for the current view.
                Vector3 viewCameraPos = convergedPerspectiveViewInfo.position;

                // Get the camera target for the current view.
                float viewTargetPosX = viewCameraPos.x + cameraDir.x;
                float viewTargetPosY = viewCameraPos.y + cameraDir.y;
                float viewTargetPosZ = viewCameraPos.z + cameraDir.z;

                // Get camera transform.
                float[] cameraTransform = new float[16];
                Matrix.setLookAtM(cameraTransform, 0, viewCameraPos.x, viewCameraPos.y, viewCameraPos.z, viewTargetPosX, viewTargetPosY, viewTargetPosZ, cameraUp.x, cameraUp.y, cameraUp.z);

                float[] viewProjectionMatrix = new float[16];
                Matrix.multiplyMM(viewProjectionMatrix, 0, convergedPerspectiveViewInfo.projectionMatrix, 0, cameraTransform, 0);

                GLES30.glViewport(renderTarget.viewportX, renderTarget.viewportY, renderTarget.viewportWidth, renderTarget.viewportHeight);

                cubeRenderer.render(viewProjectionMatrix);
            }

            // Restore previous viewport.
            GLES30.glViewport(initialViewport.get(0), initialViewport.get(1), initialViewport.get(2), initialViewport.get(3));

            // Interlace the views
            interlacer.doPostProcess(viewportWidth, viewportHeight);
        }
    }

    static class Impl extends InputViewsAsset.Impl {
        Impl() {
            super(null);
        }

        @Override
        protected InputGLBinding createGLBinding() {
            return new GLBinding(this);
        }
    }

    /** The sole purpose of this class is to render a rotating cube. */
    static class CubeRenderer {
        float geometryDist = 500.0f;

        private long startTime = 0;
        private float elapsedTime = 0;
        private float[] geometryTransform = new float[16];

        private int program = 0;
        private int[] vao = new int[1];
        private int[] indexBuffer = new int[1];
        private int[] vertexPositionsBuffer = new int[1];
        private int[] vertexColorsBuffer = new int[1];

        boolean update() {
            if (!init()) {
                return false;
            }

            {
                long curTime = System.currentTimeMillis();
                if (startTime == 0.0) {
                    startTime = curTime;
                }
                elapsedTime = (float)(curTime - startTime) / 1000.0f;
            }

            {
                float[] rotateMatrix = new float[16];
                Matrix.setRotateEulerM(rotateMatrix, 0, 20.0f * elapsedTime, 0.0f * elapsedTime, 40.0f * elapsedTime);

                float[] translateMatrix = new float[16];
                Matrix.setIdentityM(translateMatrix, 0);
                translateMatrix[13] = geometryDist;

                Matrix.multiplyMM(geometryTransform, 0, translateMatrix, 0, rotateMatrix, 0);
            }

            return true;
        }

        void render(float[] viewProjectionMatrix) {
            GLES30.glEnable(GLES30.GL_DEPTH_TEST);
            GLES30.glEnable(GLES30.GL_CULL_FACE);

            float[] mvp = new float[16];
            Matrix.multiplyMM(mvp, 0, viewProjectionMatrix, 0, geometryTransform, 0);

            GLES30.glUseProgram(program);
            int tranformHandle = GLES30.glGetUniformLocation(program, "transform");
            GLES30.glUniformMatrix4fv(tranformHandle, 1, false, FloatBuffer.wrap(mvp));
            GLES30.glBindVertexArray(vao[0]);
            GLES30.glEnableVertexAttribArray(0);
            GLES30.glEnableVertexAttribArray(1);
            int triangles = 6 * 2;
            GLES30.glDrawElements(GLES30.GL_TRIANGLES, triangles * 3, GLES30.GL_UNSIGNED_SHORT, 0);
        }

        void reset(boolean isValidGlContext) {
            if (isValidGlContext) {
                if (program != 0) {
                    GLES30.glDeleteProgram(program);
                    program = 0;
                }

                if (vao[0] != 0) {
                    GLES30.glDeleteBuffers(1, IntBuffer.wrap(vao));
                    vao[0] = 0;
                }
                if (indexBuffer[0] != 0) {
                    GLES30.glDeleteBuffers(1, IntBuffer.wrap(indexBuffer));
                    indexBuffer[0] = 0;
                }
                if (vertexPositionsBuffer[0] != 0) {
                    GLES30.glDeleteBuffers(1, IntBuffer.wrap(vertexPositionsBuffer));
                    vertexPositionsBuffer[0] = 0;
                }
                if (vertexColorsBuffer[0] != 0) {
                    GLES30.glDeleteBuffers(1, IntBuffer.wrap(vertexColorsBuffer));
                    vertexColorsBuffer[0] = 0;
                }
            }
        }

        private final String vertexShaderCode =
                "#version 310 es\n" +
                        "in vec3 inPos;\n" +
                        "in vec3 inColor;\n" +
                        "out vec3 color;\n" +
                        "uniform mat4 transform;\n" +
                        "void main() {\n" +
                        "  gl_Position = transform * vec4(inPos, 1.0);\n" +
                        "  color = inColor;\n" +
                        "}";

        private final String fragmentShaderCode =
                "#version 310 es\n" +
                        "precision highp float;\n" +
                        "in vec3 color;\n" +
                        "out vec4 frag_color;\n" +
                        "void main() {\n" +
                        "    frag_color = vec4(color, 1.0);\n" +
                        "}";

        private boolean init() {
            if (program != 0) {
                return true;
            }

            float cubeWidth  = 200.0f;
            float cubeHeight = 200.0f;
            float cubeDepth  = 200.0f;

            float l = -cubeWidth / 2.0f;
            float r = l + cubeWidth;
            float b = -cubeHeight / 2.0f;
            float t = b + cubeHeight;
            float n = -cubeDepth / 2.0f;
            float f = n + cubeDepth;

            float[][] cubeVerts =
                    {
                            {l, n, b}, // Left Near Bottom
                            {l, f, b}, // Left Far Bottom
                            {r, f, b}, // Right Far Bottom
                            {r, n, b}, // Right Near Bottom
                            {l, n, t}, // Left Near Top
                            {l, f, t}, // Left Far Top
                            {r, f, t}, // Right Far Top
                            {r, n, t}  // Right Near Top
                    };

            int[][] faces =
                    {
                            {0,1,2,3}, // bottom
                            {1,0,4,5}, // left
                            {0,3,7,4}, // front
                            {3,2,6,7}, // right
                            {2,1,5,6}, // back
                            {4,7,6,5}  // top
                    };

            float[][] faceColors =
                    {
                            {1,0,0},
                            {0,1,0},
                            {0,0,1},
                            {1,1,0},
                            {0,1,1},
                            {1,0,1}
                    };

            FloatBuffer verts = FloatBuffer.allocate(6*12);
            FloatBuffer colors = FloatBuffer.allocate(6*12);
            ShortBuffer indices= ShortBuffer.allocate(6*6);
            for (int i = 0; i < 6; i++)
            {
                int i0 = faces[i][0];
                int i1 = faces[i][1];
                int i2 = faces[i][2];
                int i3 = faces[i][3];

                // Add indices.
                int startIndex = (int)verts.position()/3;
                indices.put((short)(startIndex + 0));
                indices.put((short)(startIndex + 1));
                indices.put((short)(startIndex + 2));
                indices.put((short)(startIndex + 0));
                indices.put((short)(startIndex + 2));
                indices.put((short)(startIndex + 3));

                verts.put(cubeVerts[i0][0]);
                verts.put(cubeVerts[i0][1]);
                verts.put(cubeVerts[i0][2]);

                verts.put(cubeVerts[i1][0]);
                verts.put(cubeVerts[i1][1]);
                verts.put(cubeVerts[i1][2]);

                verts.put(cubeVerts[i2][0]);
                verts.put(cubeVerts[i2][1]);
                verts.put(cubeVerts[i2][2]);

                verts.put(cubeVerts[i3][0]);
                verts.put(cubeVerts[i3][1]);
                verts.put(cubeVerts[i3][2]);

                colors.put(faceColors[i][0]);
                colors.put(faceColors[i][1]);
                colors.put(faceColors[i][2]);

                colors.put(faceColors[i][0]);
                colors.put(faceColors[i][1]);
                colors.put(faceColors[i][2]);

                colors.put(faceColors[i][0]);
                colors.put(faceColors[i][1]);
                colors.put(faceColors[i][2]);

                colors.put(faceColors[i][0]);
                colors.put(faceColors[i][1]);
                colors.put(faceColors[i][2]);
            }

            indices.position(0);
            verts.position(0);
            colors.position(0);

            int vertexPositionsAttributeIndex  = 0;
            int vertexColorsAttributeIndex     = 1;

            GLES30.glGenVertexArrays(1, IntBuffer.wrap(vao));
            GLES30.glBindVertexArray(vao[0]);

            // Create index buffer.
            GLES30.glGenBuffers(1, IntBuffer.wrap(indexBuffer));
            GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, indexBuffer[0]);
            GLES30.glBufferData(GLES30.GL_ELEMENT_ARRAY_BUFFER, indices.capacity() * 2, indices, GLES30.GL_STATIC_DRAW);

            // Create vertex positions buffer.
            GLES30.glGenBuffers(1, IntBuffer.wrap(vertexPositionsBuffer));
            GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vertexPositionsBuffer[0]);
            GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, verts.capacity() * 4, verts, GLES30.GL_STATIC_DRAW);
            GLES30.glVertexAttribPointer(vertexPositionsAttributeIndex, 3, GLES30.GL_FLOAT, false, 0, 0);
            GLES30.glEnableVertexAttribArray(vertexPositionsAttributeIndex);

            // Create vertex colors buffer.
            GLES30.glGenBuffers(1, IntBuffer.wrap(vertexColorsBuffer));
            GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vertexColorsBuffer[0]);
            GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, colors.capacity() * 4, colors, GLES30.GL_STATIC_DRAW);
            GLES30.glVertexAttribPointer(vertexColorsAttributeIndex, 3, GLES30.GL_FLOAT, false, 0, 0);
            GLES30.glEnableVertexAttribArray(vertexColorsAttributeIndex);

            // Unbind buffers
            GLES30.glBindVertexArray(0);
            GLES30.glBindBuffer(GLES30.GL_ELEMENT_ARRAY_BUFFER, 0);
            GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, 0);

            // Create vertex shader.
            int vertexShader = GLES30.glCreateShader(GLES30.GL_VERTEX_SHADER);
            GLES30.glShaderSource(vertexShader, vertexShaderCode);
            GLES30.glCompileShader(vertexShader);

            // Verify vertex shader compile succeeded.
            {
                int[] success = new int[1];
                GLES30.glGetShaderiv(vertexShader, GLES30.GL_COMPILE_STATUS, IntBuffer.wrap(success));
                if (success[0] == 0)
                {
                    String infoLog = GLES30.glGetShaderInfoLog(vertexShader);
                    return false;
                }
            }

            // Create fragment shader.
            int fragmentShader = GLES30.glCreateShader(GLES30.GL_FRAGMENT_SHADER);
            GLES30.glShaderSource(fragmentShader, fragmentShaderCode);
            GLES30.glCompileShader(fragmentShader);

            // Verify fragment shader compile succeeded.
            {
                int[] success = new int[1];
                GLES30.glGetShaderiv(vertexShader, GLES30.GL_COMPILE_STATUS, IntBuffer.wrap(success));
                if (success[0] == 0)
                {
                    String infoLog = GLES30.glGetShaderInfoLog(vertexShader);
                    return false;
                }
            }

            // Create program.
            program = GLES30.glCreateProgram();
            GLES30.glAttachShader(program, vertexShader);
            GLES30.glAttachShader(program, fragmentShader);
            GLES30.glLinkProgram(program);

            // Verify program link succeeded.
            {
                int[] success = new int[1];
                GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, IntBuffer.wrap(success));
                if (success[0] == 0)
                {
                    String infoLog = GLES30.glGetProgramInfoLog(program);
                    return false;
                }
            }

            // Delete non-needed shaders since the program has them already.
            GLES30.glDeleteShader(vertexShader);
            GLES30.glDeleteShader(fragmentShader);

            return true;
        }
    }
}
