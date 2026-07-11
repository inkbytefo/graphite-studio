#pragma once

#include <algorithm>
#include <cmath>

namespace core {

// Porter-Duff "Over" alpha compositing operator.
// Blends source (src) onto destination (dst) in-place.
// All values are in [0.0, 1.0] range.
inline void AlphaCompositeOver(
    float srcR, float srcG, float srcB, float srcA,
    float& dstR, float& dstG, float& dstB, float& dstA)
{
    float outA = srcA + dstA * (1.0f - srcA);
    if (outA > 0.0f) {
        dstR = (srcR * srcA + dstR * dstA * (1.0f - srcA)) / outA;
        dstG = (srcG * srcA + dstG * dstA * (1.0f - srcA)) / outA;
        dstB = (srcB * srcA + dstB * dstA * (1.0f - srcA)) / outA;
    }
    dstA = outA;
}

// Soft eraser: reduces destination alpha by stroke intensity.
// Returns the new alpha value.
inline float SoftErase(float prevAlpha, float strokeAlpha) {
    return prevAlpha * (1.0f - strokeAlpha);
}

// Clamp a float to [0, 255] and convert to unsigned char.
inline unsigned char FloatToByte(float v) {
    return static_cast<unsigned char>(std::clamp(v * 255.0f, 0.0f, 255.0f));
}

// Convert unsigned char to float [0, 1].
inline float ByteToFloat(unsigned char v) {
    return v / 255.0f;
}

} // namespace core
