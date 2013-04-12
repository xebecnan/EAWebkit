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
// EAWebKitJavascriptDebugServer.h
// By David Siems
///////////////////////////////////////////////////////////////////////////////

#ifndef EAWEBKIT_EAWEBKITJAVASCRIPTDEBUGLISTENER_H
#define EAWEBKIT_EAWEBKITJAVASCRIPTDEBUGLISTENER_H

#include <EAWebKit/EAWebKitJavascriptValue.h>

namespace EA { namespace WebKit {
class EAWebKitJavascriptDebugListener;
class EAWebkitJavascriptCallFrame;
class EAWebkitJavascriptCallFrameWrapper;

/////////////////////////////////////////////////////////////////////////////////////////
// EAWebKitJavascriptDebugListener Setup Instructions
//
// Inherit from this class to create a custom debug listener.
// To begin receiving notifications, set the debug listener with a call to 
// EAWebKit::RegisterJavascriptDebugListener.
//
// To stop notification call EAWebKit::UnregisterJavascriptDebugListener.
//
// Notes:
// * sourceID's are not mapped 1:1 to files.  A file can have multiple sourceID's starting
// at different lines within the file.


// Helper class for EAWebKitJavascriptDebugListener
// This class has access to EAWebKitJavascriptDebugger
// and allows some of this functionality to be accessed
// externally.
class EAWEBKIT_API EAWebKitJavascriptDebuggerWrapper {
public:
    EAWebKitJavascriptDebuggerWrapper(void);
    virtual ~EAWebKitJavascriptDebuggerWrapper(void) {}

    // Setup / Teardown debug listening.
    virtual void Attach(EAWebKitJavascriptDebugListener *listener);
    virtual void Detach(void);

    // Helpers to translate between sourceID's and human readable names.
    virtual const char *IdToFile(int sourceId);
    virtual int FileToId(const char *filename, unsigned line);

    // JavascriptCallFrame Wrappers - You can use these to create watch expressions and get
    // information about the execution state.
    virtual bool EvaluateExpression(const char *expression, JavascriptValue *resultOut);
    virtual int GetCurrentLine(void);
    virtual const char *GetCurrentFile(void);

    virtual bool GetCallFrame(EAWebkitJavascriptCallFrameWrapper *wrapperOut);
    virtual void CleanupCallFrame(EAWebkitJavascriptCallFrameWrapper *wrapper);
    virtual const char16_t *FunctionName(EAWebkitJavascriptCallFrame *frame);
    virtual EASTLVectorJavaScriptValueWrapper *ScopeChain(EAWebkitJavascriptCallFrame *frame);
    virtual bool Caller(EAWebkitJavascriptCallFrame *frame, EAWebkitJavascriptCallFrameWrapper *wrapperOut);

    virtual void AddPage(void *page);

private:
    class EAWebKitJavascriptDebugger *mDebugger;
};

// EAWebkitJavascriptCallFrameWrapper
// Use this class to get access about the call stack / scope chain.
class EAWEBKIT_API EAWebkitJavascriptCallFrameWrapper {
public:
    EAWebkitJavascriptCallFrameWrapper(void) : mFrame(NULL), mDebugWrapper(NULL) {}
    virtual ~EAWebkitJavascriptCallFrameWrapper(void) {}

    virtual const char16_t *FunctionName(void) {
        if (mDebugWrapper) {
            return mDebugWrapper->FunctionName(mFrame);
        }

        return 0x0;
    }
    
    // Scope chain is actually a linked list, but it gets snapshotted into an array.
    // Entry 0 is the current scope, entry N is the bottom of the stack.
    virtual EASTLVectorJavaScriptValueWrapper *ScopeChain(void) {
        if (mDebugWrapper) {
            return mDebugWrapper->ScopeChain(mFrame);
        }

        return NULL;
    }

    // Gets the call frame of the function that called this one.
    // You should call EAWebKitJavascriptDebugListener::DestroyCallFrame when you're
    // finished with this to avoid leaking memory.
    virtual bool Caller(EAWebkitJavascriptCallFrameWrapper *wrapperOut) {
        if (mDebugWrapper) {
            return mDebugWrapper->Caller(mFrame, wrapperOut);
        }

        return false;
    }
    
private:
    friend class EAWebKitJavascriptDebuggerWrapper;
    EAWebkitJavascriptCallFrame *mFrame;
    EAWebKitJavascriptDebuggerWrapper *mDebugWrapper;
};

// EAWebKitJavascriptDebugListener
// Inherit from this and call EAWebKit::RegisterJavascriptDebugListener to use.
class EAWEBKIT_API EAWebKitJavascriptDebugListener {
public:
    virtual ~EAWebKitJavascriptDebugListener(void) {}

    ///////////////////////////////////////////////////////////////////////////////////
    // Interface Functions
    
    // Called when a script is loaded with a syntax error.
    virtual void OnParseFailure(const char *filename, int line, const char *message) {}
    
    // Called immediately when an exception is thrown during script execution.
    virtual void OnException(int sourceId, int line) {}

    // Called when an exception is thrown but will not be caught, this is called before the stack is unwound.
    virtual void OnUnhandledException(int sourceId, int line) {}

    // Called each time a new statement is executed.
    virtual void OnStatement(int sourceId, int line) {}

    // Called each time a function gets called in script.
    virtual void OnFunctionCall(int sourceId, int line) {}

    // Called each time a function returns in script.
    virtual void OnFunctionReturn(int sourceId, int line) {}

    // Called when the 'debugger' javascript keyword is encountered.
    virtual void OnJavascriptBreakpoint(int sourceId, int line) {}

    ///////////////////////////////////////////////////////////////////////////////////
    // Helper Functions - Use these to obtain additional debug information.

    const char *IdToFile(int sourceId) {
        if (mDebuggerWrapper) {
            return mDebuggerWrapper->IdToFile(sourceId);
        }

        return NULL;
    }

    int FileToId(const char *filename, unsigned line) {
        if (mDebuggerWrapper) {
            return mDebuggerWrapper->FileToId(filename, line);
        }

        return -1;
    }

    bool EvaluateExpression(const char *expression, JavascriptValue *resultOut) {
        if (mDebuggerWrapper) {
            return mDebuggerWrapper->EvaluateExpression(expression, resultOut);
        }

        return false;
    }

    int GetCurrentLine(void) {
        if (mDebuggerWrapper) {
            return mDebuggerWrapper->GetCurrentLine();
        }

        return -1;
    }

    const char *GetCurrentFile(void) {
        if (mDebuggerWrapper) {
            return mDebuggerWrapper->GetCurrentFile();
        }

        return NULL;
    }

    // You should call EAWebKitJavascriptDebugListener::CleanupCallFrame when you're
    // finished with this to avoid leaking memory.
    bool GetCallFrame(EAWebkitJavascriptCallFrameWrapper *wrapperOut) {
        if (mDebuggerWrapper) {
            return mDebuggerWrapper->GetCallFrame(wrapperOut);
        }

        return false;
    }

    void CleanupCallFrame(EAWebkitJavascriptCallFrameWrapper *wrapper) {
        if (mDebuggerWrapper) {
            mDebuggerWrapper->CleanupCallFrame(wrapper);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // Internal Functions
    void SetDebugWrapper(EAWebKitJavascriptDebuggerWrapper *wrapper);
    EAWebKitJavascriptDebuggerWrapper *GetDebugWrapper(void);
    void Attach(void);
    void Detach(void);
    void AddPage(void *page);

private:
    EAWebKitJavascriptDebuggerWrapper *mDebuggerWrapper;
};

}}

#endif
