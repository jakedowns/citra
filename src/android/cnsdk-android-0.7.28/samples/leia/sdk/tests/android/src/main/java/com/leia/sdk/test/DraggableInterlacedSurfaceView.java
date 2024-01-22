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

import static java.lang.Math.abs;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.leia.sdk.views.InterlacedSurfaceView;

/**
 * For testing the flicker issue.
 * To use, just replace InterlacedSurfaceView in activity layout (activity_main.xml) with DraggableInterlacedSurfaceView.
 */
public class DraggableInterlacedSurfaceView extends InterlacedSurfaceView {
    public DraggableInterlacedSurfaceView(Context context) throws Exception {
        super(context);
    }

    public DraggableInterlacedSurfaceView(Context context, AttributeSet attrs) throws Exception {
        super(context, attrs);
    }

    private float previousX = 0.f;
    private float previousY = 0.f;

    private int roundNearest(int number, int multiple) {
        int result = abs(number) + multiple/2;
        result -= result % multiple;
        result *= number > 0 ? 1 : -1;
        return result;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        boolean isEventHandled = super.onTouchEvent(event);
        if (!isEventHandled) {
            float x = event.getRawX();
            float y = event.getRawY();
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    previousX = x;
                    previousY = y;
                    return true;
                case MotionEvent.ACTION_MOVE:
                    int n = 1; //getGridSize();
                    float dx = (float) roundNearest((int) (x - previousX), n);
                    float dy = (float) roundNearest((int) (y - previousY), n);
                    if (abs(dx) >= n) setX(getX() + dx);
                    if (abs(dy) >= n) setY(getY() + dy);
                    previousX = x;
                    previousY = y;
                    return true;
            }
        }
        return false;
    }
}
