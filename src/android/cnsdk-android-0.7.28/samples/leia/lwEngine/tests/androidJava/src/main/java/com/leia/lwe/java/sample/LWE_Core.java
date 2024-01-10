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
package com.leia.lwe.java.sample;

import android.app.Activity;
import android.view.MotionEvent;

public class LWE_Core {
    static {
        System.loadLibrary("lweJni");
    }

    private long cxxPtr;

    public LWE_Core(Activity activity) {
        cxxPtr = init(activity);
    }
    private native long init(Activity activity);

    public void close() {
        close(cxxPtr);
    }
    private native void close(long cxxPtr);

    public void onWindowSizeChanged(int width, int height) {
        onWindowSizeChanged(cxxPtr, width, height);
    }
    private native void onWindowSizeChanged(long cxxPtr, int width, int height);

    public void drawFrame() {
        drawFrame(cxxPtr);
    }
    private native void drawFrame(long cxxPtr);

    public void onPause() {
        onPause(cxxPtr);
    }
    private native void onPause(long cxxPtr);

    public void onResume() {
        onResume(cxxPtr);
    }
    private native void onResume(long cxxPtr);

    public boolean isInitialized() {
        return isInitialized(cxxPtr);
    }
    private native boolean isInitialized(long cxxPtr);

    public void tick(double deltaTime) {
        tick(cxxPtr, deltaTime);
    }
    private native void tick(long cxxPtr, double deltaTime);

    public void render() {
        render(cxxPtr);
    }
    private native void render(long cxxPtr);

    public void processGuiEvent(MotionEvent motionEvent) {
        processGuiEvent(cxxPtr, motionEvent);
    }
    private native void processGuiEvent(long cxxPtr, MotionEvent motionEvent);

    public void setBacklight(boolean enable) {
        setBacklight(cxxPtr, enable);
    }
    private native void setBacklight(long cxxPtr, boolean enable);
}
