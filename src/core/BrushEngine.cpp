#include "core/BrushEngine.h"
#include "core/BlendMath.h"
#include <cmath>
#include <algorithm>

namespace core {

void BrushEngine::BeginStroke(Layer* target, const BrushParams& params) {
    if (!target || target->locked || !target->visible) return;
    m_Target = target;
    m_Params = params;
    m_IsActive = true;
}

void BrushEngine::ContinueStroke(ImVec2 currentPos) {
    if (!m_IsActive || !m_Target) return;

    // On first call after BeginStroke, m_LastPos is set to currentPos
    if (m_LastPos.x == 0.0f && m_LastPos.y == 0.0f) {
        m_LastPos = currentPos;
    }

    PaintSegment(m_LastPos, currentPos);
    m_LastPos = currentPos;
}

void BrushEngine::EndStroke() {
    m_IsActive = false;
    m_Target = nullptr;
    m_LastPos = {0, 0};
}

void BrushEngine::PaintSegment(ImVec2 from, ImVec2 to) {
    int w = m_Target->width;
    int h = m_Target->height;
    auto& pixels = m_Target->GetCpuPixels();

    float radius = m_Params.size * 0.5f;

    // Interpolate steps along the line to prevent dotted gaps during fast mouse drag
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Step size is 10% of brush diameter, at least 1 pixel
    float stepSize = std::max(1.0f, m_Params.size * 0.10f);
    int numSteps = (distance == 0.0f) ? 1 : static_cast<int>(std::ceil(distance / stepSize));

    // Track modified bounding box for uploading to GPU
    int minX = w, maxX = 0, minY = h, maxY = 0;

    for (int step = 0; step <= numSteps; ++step) {
        float t = (numSteps == 0) ? 0.0f : (static_cast<float>(step) / numSteps);
        float cx = from.x + dx * t;
        float cy = from.y + dy * t;

        // Bounding box of the brush circle at this step
        int x0 = std::max(0, static_cast<int>(std::floor(cx - radius)));
        int x1 = std::min(w - 1, static_cast<int>(std::ceil(cx + radius)));
        int y0 = std::max(0, static_cast<int>(std::floor(cy - radius)));
        int y1 = std::min(h - 1, static_cast<int>(std::ceil(cy + radius)));

        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                float px = static_cast<float>(x) + 0.5f;
                float py = static_cast<float>(y) + 0.5f;
                float dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));

                if (dist <= radius) {
                    // Calculate hardness falloff density
                    float density = 1.0f;
                    if (m_Params.hardness < 1.0f) {
                        float innerRadius = radius * m_Params.hardness;
                        if (dist > innerRadius) {
                            density = (radius - dist) / (radius - innerRadius);
                        }
                    }
                    float strokeAlpha = density * m_Params.opacity;

                    int idx = (y * w + x) * 4;

                    if (m_Params.isEraser) {
                        // Soft eraser: reduce alpha using centralized BlendMath
                        float prevAlpha = ByteToFloat(pixels[idx + 3]);
                        pixels[idx + 3] = FloatToByte(SoftErase(prevAlpha, strokeAlpha));
                    } else {
                        // Standard alpha compositing using centralized BlendMath
                        float dstR = ByteToFloat(pixels[idx]);
                        float dstG = ByteToFloat(pixels[idx + 1]);
                        float dstB = ByteToFloat(pixels[idx + 2]);
                        float dstA = ByteToFloat(pixels[idx + 3]);

                        float srcA = m_Params.color.w * strokeAlpha;

                        AlphaCompositeOver(
                            m_Params.color.x, m_Params.color.y, m_Params.color.z, srcA,
                            dstR, dstG, dstB, dstA
                        );

                        pixels[idx]     = FloatToByte(dstR);
                        pixels[idx + 1] = FloatToByte(dstG);
                        pixels[idx + 2] = FloatToByte(dstB);
                        pixels[idx + 3] = FloatToByte(dstA);
                    }

                    // Expand modified bounding box
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }
    }

    // Upload only the modified sub-rectangle to the GPU texture
    if (maxX >= minX && maxY >= minY) {
        m_Target->UploadSubRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
    }
}

} // namespace core
