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
struct VSInput
{
    float3 Pos : POSITION;
    float3 Col : COLOR;
    float2 Tex : TEXCOORD;
};

struct PSInput
{
    float4               Pos : SV_POSITION;
    float4               Col : COLOR;
    noperspective float2 Tex : TEXCOORD;
};

cbuffer LeiaSharpenShaderConstantBufferData : register(b0)
{
    float4x4 worldViewProj;
};

PSInput VSMain(VSInput input)
{
    PSInput output = (PSInput)0;

    output.Pos = mul(worldViewProj, float4(input.Pos, 1.0f));
    output.Col = float4(input.Col, 1.0f);
    output.Tex = input.Tex;

    return output;
}

float4 PSMain(PSInput input) : SV_Target0
{
    return input.Col;
}
