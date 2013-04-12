/*
Copyright (C) 2011 Electronic Arts, Inc.  All rights reserved.

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
// EAWebKitLog.h
//
// By David Siems
///////////////////////////////////////////////////////////////////////////////


#ifndef EAWEBKIT_EAWEBKITLOG_H
#define EAWEBKIT_EAWEBKITLOG_H


///////////////////////////////////////////////////////////////////////
// This file isolates the EAWebkit log functionality from the EAWebkit.h. The reason for it is to avoid the dependency of core Webkit code on
// EAWebkit.h. The EAWebkit log functionality is something that should be internal to the library and not exposed to the main app code.
///////////////////////////////////////////////////////////////////////

#include <EAWebKit/EAWebKitConfig.h>
#if EAWEBKIT_THROW_BUILD_ERROR
#error This file should be included only in a dll build
#endif

///////////////////////////////////////////////////////////////////////
// Logging Helpers
///////////////////////////////////////////////////////////////////////

// This function will call ViewNotification::DebugLog.
extern "C" void EAWebkitLog(int channel, const char *format, ...);

#define EAW_MULTILINE_MACRO_BEGIN do {
#define EAW_MULTILINE_MACRO_END } while (0)

#if EAWEBKIT_TRACE_ENABLED
#define EAW_TRACE(channel, message, ...) EAW_MULTILINE_MACRO_BEGIN \
    EAWebkitLog((int)channel, message, ##__VA_ARGS__);  \
EAW_MULTILINE_MACRO_END
#else
#define EAW_TRACE(channel, message, ...) EAW_MULTILINE_MACRO_BEGIN \
    (void)(sizeof(channel));    \
    (void)(sizeof(message));    \
EAW_MULTILINE_MACRO_END
#endif

#define EAW_TRACE_UNFILTERED(message, ...) EAW_TRACE(0, message, ##__VA_ARGS__)

#endif
