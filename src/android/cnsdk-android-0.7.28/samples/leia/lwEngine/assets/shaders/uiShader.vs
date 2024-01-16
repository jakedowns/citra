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

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTexCoord;

//float SCR_WIDTH = 800 / 5;
//float SCR_HEIGHT = 600 / 5;
//float widthZ = 1.0/SCR_WIDTH;
//float heightZ = 1.0/SCR_HEIGHT;

const float widthZ  = 0.00625f;
const float heightZ = 0.008333f;

uniform mat4 projection;

uniform vec4 uiPos;
uniform vec4 uiSrc;
uniform vec2 uiBaseSrc;


out vec2 TexCoord;

float getNorm(float currentValue, float minValue, float maxValue)
{
    return (currentValue - minValue)/(maxValue - minValue);
}

void main()
{
    int idx = int(aCol.x);
    vec4 locPos =   vec4(aPos.xz, 0.0, 1.0);

    locPos.x *= widthZ * uiPos.z;
    locPos.y *= heightZ * uiPos.w;
        

    float srcWZ = 1.0/uiBaseSrc.x;
    float srcHZ = 1.0/uiBaseSrc.y;

    if(idx == 3)
    {
        locPos.x = uiPos.x * widthZ; 
        locPos.y = uiPos.y * heightZ;

              TexCoord = vec2(0,0);

        TexCoord.x = uiSrc.x * srcWZ;
        TexCoord.y = uiSrc.y * srcHZ;

    }
    else if(idx == 2)
    {
        locPos.x = uiPos.x * widthZ; 
        locPos.y = (uiPos.y + uiPos.w) * heightZ;
  

        TexCoord = vec2(0,1);

        TexCoord.x = uiSrc.x * srcWZ;
        TexCoord.y = (uiSrc.y + uiSrc.w )* srcHZ;
    }
    else if(idx == 1)
    {
        locPos.x = (uiPos.x + uiPos.z) * widthZ; 
        locPos.y = (uiPos.y + uiPos.w) * heightZ;

         TexCoord = vec2(1,1);

        TexCoord.x = (uiSrc.x + uiSrc.z) * srcWZ;
        TexCoord.y = (uiSrc.y + uiSrc.w ) * srcHZ;

    }
    else if(idx == 0)
    {
        locPos.x = (uiPos.x + uiPos.z) * widthZ;  
        locPos.y = uiPos.y * heightZ;


                TexCoord = vec2(1,0);

        TexCoord.x = (uiSrc.x + uiSrc.z) * srcWZ;
        TexCoord.y = uiSrc.y * srcHZ;
    }

    locPos.x =  -1.f + locPos.x;
    locPos.y =  1.f - locPos.y;

    TexCoord.y = 1.f - TexCoord.y;
    

    gl_Position = locPos;

}