/*
Copyright (C) 2008-2010 Electronic Arts, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

///////////////////////////////////////////////////////////////////////////////
// BCFontEA.cpp
// By Paul Pedriana
///////////////////////////////////////////////////////////////////////////////


#include "config.h"
#include "Font.h"
#include "GraphicsContext.h"
#include "SimpleFontData.h"
#include "IntSize.h"
#include <EASTL/fixed_string.h>
#include <EARaster/EARaster.h>
#include "EARaster/internal/EARasterUtils.h"
#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebKitTextInterface.h>  
#include <EAWebKit/internal/EAWebKitAssert.h>

namespace WKAL {

typedef Vector<EA::Raster::GlyphDrawInfo, 128> GlyphDrawInfoArray;


#if defined(EA_DEBUG) && defined(AUTHOR_PPEDRIANA_DISABLED)
    // This is slow, but it's debug-only.
    char16_t ConvertGlyphToChar(EA::Text::Font* pFont, EA::Text::GlyphId glyphId)
    {
        EA::Text::GlyphId glyphIdResult;

        for(char16_t c = 0x20; c < 0x7f; ++c)
        {
            pFont->GetGlyphIds(&c, 1, &glyphIdResult, false);

            if(glyphIdResult == glyphId)
                return c;
        } 

        return '?';
    }

    void TraceGlyphBufferToString(const SimpleFontData* pSimpleFontData, const GlyphBuffer& glyphBuffer, int glyphIndexBegin, int glyphCount, const FloatPoint& point)
    {
        eastl::fixed_string<char8_t, 128> sDebug;
        sDebug.sprintf("[%d,%d] ", (int)point.x(), (int)point.y());

        GlyphBufferGlyph* pGlyphArray = const_cast<GlyphBufferGlyph*>(glyphBuffer.glyphs(glyphIndexBegin));

        for(int i = glyphIndexBegin; i < glyphIndexBegin + glyphCount; ++i, ++pGlyphArray)
            sDebug += (char8_t)ConvertGlyphToChar(pSimpleFontData->m_font.mpFont, *pGlyphArray);

        sDebug += "\n";

       OWB_PRINTF("%s", sDebug.c_str());
    }
#endif

// We need to draw the glyphBuffer glyphs/advances with the pSimpleFontData font onto the 
// pGraphicsContext pSurface. We draw glyphCount glyphs starting at the glyphIndexBegin.
void Font::drawGlyphs(GraphicsContext* pGraphicsContext, const SimpleFontData* pSimpleFontData, const GlyphBuffer& glyphBuffer,
                        int glyphIndexBegin, int glyphCount, const FloatPoint& point) const
{
    // 11/09/09 CSidhall Added notify start of process to user
	NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeDrawGlyph, EA::WebKit::kVProcessStatusStarted);
	
    #if defined(EA_DEBUG) && defined(AUTHOR_PPEDRIANA_DISABLED) && defined(EA_PLATFORM_WINDOWS)
        TraceGlyphBufferToString(pSimpleFontData, glyphBuffer, glyphIndexBegin, glyphCount, point);
    #endif

    GlyphBufferGlyph*               glyphs      = const_cast<GlyphBufferGlyph*>(glyphBuffer.glyphs(glyphIndexBegin));
    float                           offset      = 0;   
    int                             x_offset    = 0;
    EA::WebKit::IFont*            pFont       = pSimpleFontData->m_font.mpFont;	
    EA::WebKit::IGlyphCache*      pGlyphCache = EA::WebKit::GetGlyphCache(); 

    EAW_ASSERT_MSG(pGlyphCache, "GlyphCache is not set");
	if(!pGlyphCache)
	{
		NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeDrawGlyph, EA::WebKit::kVProcessStatusEnded);
		return;
	}
    
    EA::WebKit::ITextureInfo*     pTI         = pGlyphCache->GetTextureInfo(0); // Right now we hard-code usage of what is the only texture.
	EAW_ASSERT_MSG(pTI, "GlyphCache is not initialized with enough number of textures");
	if(!pTI)
	{
		NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeDrawGlyph, EA::WebKit::kVProcessStatusEnded);
		return;
	}

    EA::WebKit::IGlyphTextureInfo gti;
    EA::WebKit::GlyphMetrics      glyphMetrics;
    GlyphDrawInfoArray              gdiArray((size_t)(unsigned)glyphCount);

    // Walk through the list of glyphs and build up render info for each one.
    for (int i = 0; i < glyphCount; i++)
    {
        EA::WebKit::GlyphId g = glyphs[i];

        if(!pFont->GetGlyphMetrics(g, glyphMetrics))
        {
            EAW_ASSERT_MSG(false, "Font::drawGlyphs: invalid glyph/Font combo.");
            pFont->GetGlyphIds(L"?", 1, &g, true);
            pFont->GetGlyphMetrics(g, glyphMetrics);
        }

        if(!pGlyphCache->GetGlyphTextureInfo(pFont, g, gti))
        {
            // 8/11/10 CSidhall - Moved this out of the draw because was directly using the **p from the passed EAText package.
            // It has been moved to DrawGlyphBitmap() in the EAWebKitTextWrapper.h/cpp.
            pFont->DrawGlyphBitmap(pGlyphCache, g, gti);    
        }

        // Apply kerning.
        // Note by Paul Pedriana: Can we really apply kerning here at the render stage without it looking 
        // wrong? It seems to me this cannot work unless kerning is also taken into account at the 
        // layout stage. To do: See if in fact kerning is taken into account at the layout stage.
        EA::WebKit::Kerning kerning;

        if((i > 0) && pFont->GetKerning(glyphs[i - 1], g, kerning, 0))
            offset += kerning.mfKernX;

        // The values we calculate here are relative to the current pen position at the 
        // baseline position of [xoffset, 0], where +X is rightward and +Y is upward.
        gdiArray[i].x1 = (int)(offset + glyphMetrics.mfHBearingX);
        gdiArray[i].x2 = (int)(offset + glyphMetrics.mfHBearingX + glyphMetrics.mfSizeX);
        gdiArray[i].y1 = (int)(glyphMetrics.mfHBearingY);
        gdiArray[i].y2 = (int)(glyphMetrics.mfHBearingY - glyphMetrics.mfSizeY);
        gdiArray[i].u0 = gti.mX1;
        gdiArray[i].v0 = gti.mY1;
        gdiArray[i].u1 = gti.mX2;
        gdiArray[i].v1 = gti.mY2;

        // advanceAt should return a value that is usually equivalent to glyphMetrics.mfHAdvanceX, at least 
        // for most simple Western text. A case where it would be different would be Arabic combining glyphs,
        // and custom kerning, though kerning adjustments are handled above.
        offset += glyphBuffer.advanceAt(glyphIndexBegin + i);
    
		// Setting a negative gdiArray[0].x1 to 0 causes the font to overlap a bit if the first characters has 
		// negative horizonal bearing for small fonts. We need to take care of this if we want fonts to not overlap so we add 
		// to the offset in case we set this value to 0. 
		if( (i==0) && (glyphMetrics.mfHBearingX < 0.0f))
            offset -=glyphMetrics.mfHBearingX;

        // Free wrapper
        if(gti.mpTextureInfo)
        {
            gti.mpTextureInfo->DestroyWrapper();
            gti.mpTextureInfo = 0;
        }
    }

    // Find the X and Y minima and maxima of the glyph boxes.
    // To do: We can improve the efficiency of this code in a couple of ways, including moving the 
    // min/max tests up into the glyphMetrics code above.
    int xMin = gdiArray[0].x1;
    int xMax = gdiArray[0].x2;
    int yMin = gdiArray[0].y2;    // y2 is min because y goes upward.
    int yMax = gdiArray[0].y1;

    for (int i = 0; i < glyphCount; ++i) {
        const EA::Raster::GlyphDrawInfo& gdi = gdiArray[i];

        // We assume tha x1 is always <= x2.
        if (gdi.x1 < xMin)
            xMin = gdi.x1;

        if (gdi.x2 > xMax)
            xMax = gdi.x2;

        // if (gdi.y1 < yMin)  // We normally don't need this check, because y2 will usually (or always?) be less than y1.
        //     yMin = gdi.y1;

        if (gdi.y2 < yMin)
            yMin = gdi.y2;

        if (gdi.y1 > yMax)
            yMax = gdi.y1;

        // if (gdi.y2 > yMax)  // We normally don't need this check, because y1 will usually (or always?) be greater than y2.
        //     yMax = gdi.y2;
    }

    const uint16_t destWidth  = (abs(xMin) + xMax);  // abs(xMin) because it's possible the left of the first glyph is to the left of the origin. Note by Paul Pedriana: Shouldn't this instead be (xMax - min(xMin, 0))?
    const uint16_t destHeight = (yMax - yMin);

    // Transform to source rect space.
    for (int i = 0; i < glyphCount; ++i) {
        gdiArray[i].y1 = (destHeight + yMin) - gdiArray[i].y1;
        gdiArray[i].y2 = (destHeight + yMin) - gdiArray[i].y2;
    }

    if(destWidth && destHeight)  // (If we are drawing just space chars (which often happens), then destWidth and/or destHeight will be zero)
    {
        // Question by Paul Pedriana: What is the following code doing? I copied this from the 
        // WebKit Freetype equivalent of this function assuming it must be useful. It seems to 
        // me that it is chopping off any glyph pixels to the left of zero. This is probably 
        // going to usually be OK, because it is abnormal for the first char of just about any
        // written language to be a combining char (and thus drawn to the left of the pen position).

        if (gdiArray[0].x1 < 0)
        {			
            x_offset = gdiArray[0].x1;
            gdiArray[0].x1 = 0;
        }

        EA::Raster::ISurface* const pSurface = pGraphicsContext->platformContext();
        const Color penColor = pGraphicsContext->fillColor();

        EA::Raster::Rect rectSrc;
        EA::Raster::Rect rectDest;

        rectSrc.w  = destWidth;
        rectSrc.h  = destHeight;
        rectSrc.x  = 0;
        rectSrc.y  = 0;
        rectDest.x = (int)point.x() + x_offset + pGraphicsContext->origin().width();
        rectDest.y = (int)point.y() - yMax + pGraphicsContext->origin().height();
        pSurface->GetDimensions(&rectDest.w, &rectDest.h);

        EA::Raster::Matrix2D textTransform(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0);
        if (pGraphicsContext->hasTransform()) {
            pGraphicsContext->transform(rectSrc, &rectDest, &textTransform);
        }

        EA::Raster::IEARaster* pRaster = EA::WebKit::GetEARasterInstance();
        pRaster->DrawGlyphs(gdiArray.data(), gdiArray.size(), pTI, pSurface, rectSrc, rectDest, penColor, textTransform, pGraphicsContext->transparencyLayer(), fontDescription().getTextEffectType());
    }

    pTI->DestroyWrapper();

	NOTIFY_PROCESS_STATUS(EA::WebKit::kVProcessTypeDrawGlyph, EA::WebKit::kVProcessStatusEnded);
}


void Font::drawComplexText(GraphicsContext* /*pGraphicsContext*/, const TextRun& /*run*/, const FloatPoint& /*point*/, int /*glyphIndexBegin*/, int /*to*/) const
{
    // To do: implement this. EAText supports Arabic and Hebrew.
}


float Font::floatWidthForComplexText(const TextRun& /*run*/) const
{
    // To do: implement this. EAText supports Arabic and Hebrew.
    return 0.f;
}


int Font::offsetForPositionForComplexText(const TextRun& /*run*/, int /*x*/, bool /*includePartialGlyphs*/) const
{
    // To do: implement this. EAText supports Arabic and Hebrew.
    return 0;
}


FloatRect Font::selectionRectForComplexText(const TextRun& run, const IntPoint& point, int h, int glyphIndexBegin, int to) const
{
    // To do: implement this. EAText supports Arabic and Hebrew.
    return FloatRect();
}


}  // namespace 
