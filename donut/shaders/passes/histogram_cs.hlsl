#pragma pack_matrix(row_major)

#include <donut/shaders/tonemapping_cb.h>

#if SOURCE_ARRAY
Texture2DArray t_Source : register(t0);
#else
Texture2D t_Source : register(t0);
#endif
RWBuffer<uint> u_Histogram : register(u0);

cbuffer c_ToneMapping : register(b0)
{
    ToneMappingConstants g_ToneMapping;
};

float Luminance(float3 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

#define GROUP_X 16
#define GROUP_Y 16
#define FIXED_POINT_FRAC_BITS 6
#define FIXED_POINT_FRAC_MULTIPLIER (1 << FIXED_POINT_FRAC_BITS)

groupshared uint s_Histogram[HISTOGRAM_BINS];

[numthreads(GROUP_X, GROUP_Y, 1)]
void main(uint linearIdx : SV_GroupIndex, uint2 globalIdx : SV_DispatchThreadID)
{
    uint2 pixelPos = globalIdx.xy + g_ToneMapping.viewOrigin.xy;
    bool valid = all(globalIdx.xy < g_ToneMapping.viewSize.xy);

    if (linearIdx < HISTOGRAM_BINS)
    {
        s_Histogram[linearIdx] = 0;
    }
    
    GroupMemoryBarrierWithGroupSync();

    if (valid)
    {
#if SOURCE_ARRAY
        float3 color = t_Source[uint3(pixelPos, g_ToneMapping.sourceSlice)].rgb;
#else
        float3 color = t_Source[pixelPos].rgb;
#endif

        float luminance = Luminance(color);
        float biasedLogLuminance = log2(luminance) * g_ToneMapping.logLuminanceScale + g_ToneMapping.logLuminanceBias;
        float histogramBin = saturate(biasedLogLuminance) * (HISTOGRAM_BINS - 1);
        
        uint leftBin = uint(floor(histogramBin));
        uint rightBin = leftBin + 1;

        uint rightWeight = uint(frac(histogramBin) * FIXED_POINT_FRAC_MULTIPLIER);
        uint leftWeight = FIXED_POINT_FRAC_MULTIPLIER - rightWeight;

        if(leftWeight != 0 && leftBin < HISTOGRAM_BINS) 
            InterlockedAdd(s_Histogram[leftBin], leftWeight);
        if(rightWeight != 0 && rightBin < HISTOGRAM_BINS) 
            InterlockedAdd(s_Histogram[rightBin], rightWeight);
    }

    GroupMemoryBarrierWithGroupSync();

    if (linearIdx < HISTOGRAM_BINS)
    {
        uint localBinValue = s_Histogram[linearIdx];
        if (localBinValue != 0)
            InterlockedAdd(u_Histogram[linearIdx], localBinValue);
    }
}
