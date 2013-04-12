/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

/*
* This file was modified by Electronic Arts Inc Copyright © 2011
*/

#include "config.h"
#include "CSSFontFaceSource.h"

#include "CachedFont.h"
#include "CSSFontFace.h"
#include "CSSFontSelector.h"
#include "DocLoader.h"
#include "FontCache.h"
#include "FontDescription.h"
#include "GlyphPageTreeNode.h"
#include "SimpleFontData.h"
#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/internal/EAWebKitAssert.h>

#if ENABLE(SVG_FONTS)
#include "FontCustomPlatformData.h"
#include "HTMLNames.h"
#include "SVGFontData.h"
#include "SVGFontElement.h"
#include "SVGURIReference.h"
#endif

namespace WebCore {

CSSFontFaceSource::CSSFontFaceSource(const String& str, CachedFont* font)
: m_string(str)
, m_font(font)
, m_face(0)
{
    if (m_font)
        m_font->addClient(this);
}

CSSFontFaceSource::~CSSFontFaceSource()
{
    if (m_font)
        m_font->removeClient(this);
    pruneTable();
}

void CSSFontFaceSource::pruneTable()
{
    if (m_fontDataTable.isEmpty())
        return;
    HashMap<unsigned, SimpleFontData*>::iterator end = m_fontDataTable.end();
    for (HashMap<unsigned, SimpleFontData*>::iterator it = m_fontDataTable.begin(); it != end; ++it)
        GlyphPageTreeNode::pruneTreeCustomFontData(it->second);
    deleteAllValues(m_fontDataTable);
    m_fontDataTable.clear();
}

bool CSSFontFaceSource::isLoaded() const
{
    if (m_font)
        return m_font->isLoaded();
    return true;
}

bool CSSFontFaceSource::isValid() const
{
    if (m_font)
        return !m_font->errorOccurred();
    return true;
}

void CSSFontFaceSource::fontLoaded(CachedFont*)
{
    pruneTable();
    if (m_face)
        m_face->fontLoaded(this);
}

SimpleFontData* CSSFontFaceSource::getFontData(const FontDescription& fontDescription, bool syntheticBold, bool syntheticItalic, CSSFontSelector* fontSelector)
{
    // If the font hasn't loaded or an error occurred, then we've got nothing.
    if (!isValid())
        return 0;

#if ENABLE(SVG_FONTS)
    if (!m_font && !m_svgFontFaceElement) {
#else
    if (!m_font) {
#endif
        FontPlatformData* data = FontCache::getCachedFontPlatformData(fontDescription, m_string);
        SimpleFontData* fontData = FontCache::getCachedFontData(data);

        // We're local. Just return a SimpleFontData from the normal cache.
        return fontData;
    }

    // See if we have a mapping in our FontData cache.
    unsigned hashKey = fontDescription.computedPixelSize() << 2 | (syntheticBold ? 2 : 0) | (syntheticItalic ? 1 : 0);
    
    SimpleFontData* cachedData = m_fontDataTable.get(hashKey);
    if(cachedData)
        return cachedData;

    OwnPtr<SimpleFontData> fontData;

    // If we are still loading, then we let the system pick a font.
    if (isLoaded()) {
        if (m_font) {
#if ENABLE(SVG_FONTS)
            if (m_font->isSVGFont()) {
                // For SVG fonts parse the external SVG document, and extract the <font> element.
                if (!m_font->ensureSVGFontData())
                    return 0;

                if (!m_externalSVGFontElement)
                    m_externalSVGFontElement = m_font->getSVGFontById(SVGURIReference::getTarget(m_string));

                if (!m_externalSVGFontElement)
                    return 0;

                SVGFontFaceElement* fontFaceElement = 0;

                // Select first <font-face> child
                for (Node* fontChild = m_externalSVGFontElement->firstChild(); fontChild; fontChild = fontChild->nextSibling()) {
                    if (fontChild->hasTagName(SVGNames::font_faceTag)) {
                        fontFaceElement = static_cast<SVGFontFaceElement*>(fontChild);
                        break;
                    }
                }

                if (fontFaceElement) {
                    if (!m_svgFontFaceElement) {
                        // We're created using a CSS @font-face rule, that means we're not associated with a SVGFontFaceElement.
                        // Use the imported <font-face> tag as referencing font-face element for these cases.
                        m_svgFontFaceElement = fontFaceElement;
                    }

                    SVGFontData* svgFontData = new SVGFontData(fontFaceElement);
                    fontData.set(new SimpleFontData(m_font->platformDataFromCustomData(fontDescription.computedPixelSize(), syntheticBold, syntheticItalic, fontDescription.renderingMode()), true, false, svgFontData));
                }
            } else
#endif
            {
                // Create new FontPlatformData from our CGFontRef, point size and ATSFontRef.
                if (!m_font->ensureCustomFontData()) {

                        //String name(fontDescription.family().family().string());
                        String name(m_string.string());
                        EAW_ASSERT_FORMATTED(false,"Font: \"%ls\" could not open. Please verify that the font format is supported by EAText.\n",name.charactersWithNullTermination());
                        return 0;
                }

#ifdef ADD_FONTFACE_FONTS_TO_FONT_SERVER
                FontPlatformData fontPD = m_font->platformDataFromCustomData(fontDescription);                
#else
                FontPlatformData fontPD = m_font->platformDataFromCustomData(fontDescription.computedPixelSize(), syntheticBold, syntheticItalic, fontDescription.renderingMode());                

                // 1/5/11 CSidhall - Added font style settings to support smoothing, size, and effect. 
                // Set the size directly and smoothing.  Normally this is passed through the text style but custom fonts are created differently.
                if( (fontPD.mpFont) && (fontPD.mpFont->GetSize() == 0.0f) ) {
                        
                    const float fFontSize = fontDescription.computedSize();
 
                    // Set text smoothing. We only want to set it if the font has not init (size == 0f)
                    EA::WebKit::Smooth smooth = WKAL::GetFontSmoothingStyle(fFontSize, syntheticBold, syntheticItalic);
                    fontPD.mpFont->SetSmoothing(smooth);

                    // Some magic here to adjust the size.  See BCFontPlatformDataEA.cpp with Set TextStyle::mfSize for more info.
                    const float adjustedFontSize = WKAL::GetFontAdjustedSize(fFontSize);
                    fontPD.mpFont->SetTransform(adjustedFontSize);           
               
                    // Set effect
                    if(fontDescription.getTextEffectType() != EA::WebKit::kEffectNone) {
                        EA::WebKit::IFontServer* const pFontServer = EA::WebKit::GetFontServer();
                        fontPD.mpFont->SetEffect(fontDescription.getTextEffectData(), pFontServer);          
                    }
                }
                
                fontData.set(new SimpleFontData(fontPD, true, false));
#endif
            }
        } else {
#if ENABLE(SVG_FONTS)
            // In-Document SVG Fonts
            if (m_svgFontFaceElement) {
                SVGFontData* svgFontData = new SVGFontData(m_svgFontFaceElement.get());
                fontData.set(new SimpleFontData(FontPlatformData(fontDescription.computedPixelSize(), syntheticBold, syntheticItalic), true, false, svgFontData));
            }
#endif
        }
    } else {
        // Kick off the load now.
        m_font->beginLoadIfNeeded(fontSelector->docLoader());
        // FIXME: m_string is a URL so it makes no sense to pass it as a family name.
//        FontPlatformData* tempData = FontCache::getCachedFontPlatformData(fontDescription, m_string);
        FontPlatformData* tempData = FontCache::getCachedFontPlatformData(fontDescription, fontDescription.family().family().string());

        if (!tempData)
            tempData = FontCache::getLastResortFallbackFont(fontDescription);
        fontData.set(new SimpleFontData(*tempData, true, true));
    }

    m_fontDataTable.set(hashKey, fontData.get());
    return fontData.release();
}

}

