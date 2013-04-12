/*
 * Copyright (C) 2003, 2007 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
* This file was modified by Electronic Arts Inc Copyright � 2009-2011.
*/

#ifndef WindowFeatures_h
#define WindowFeatures_h

#include <wtf/FastAllocBase.h>
#include "PlatformString.h"
#include <wtf/HashMap.h>

namespace WebCore {

    struct WindowFeatures/*: public WTF::FastAllocBase*/ {
        WindowFeatures()
            : xSet(false)
            , ySet(false)
            , widthSet(false)
            , heightSet(false)
            , menuBarVisible(true)
            , statusBarVisible(true)
            , toolBarVisible(true)
            , locationBarVisible(true)
            , scrollbarsVisible(true)
            , resizable(true)
            , fullscreen(false)
            , dialog(false)
        {
        }

        WindowFeatures(const String& features);

        void setWindowFeature(const String& keyString, const String& valueString);

        static bool boolFeature(const HashMap<String, String>& features, const char* key, bool defaultValue = false);
        static float floatFeature(const HashMap<String, String>& features, const char* key, float min, float max, float defaultValue);

        float x;
        bool xSet;
        float y;
        bool ySet;
        float width;
        bool widthSet;
        float height;
        bool heightSet;

        bool menuBarVisible;
        bool statusBarVisible;
        bool toolBarVisible;
        bool locationBarVisible;
        bool scrollbarsVisible;
        bool resizable;

        bool fullscreen;
        bool dialog;

		// Note by Arpit Baldeva: This does not belong here but helps in trickling down the information in the least "invasive" way.
		// This is the URL specified in a JavaScript Window.open() call. This could be as a result of user click that runs JavaScript for example. We want this information to be 
		// passed outside and sticking it to this structure helps us avoid touching few different files as this structure reaches all the way down to WebChromeClient.cpp.
		String mURLToOpen;
    };

} // namespace WebCore

#endif // WindowFeatures_h