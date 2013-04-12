/*
Copyright (C) 2008-2011 Electronic Arts, Inc.  All rights reserved.

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
// EAWebKitJavascriptValue.cpp
//
// Created by David Siems
///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <EAWebKit/EAWebKitJavascriptValue.h>
#include <EAWebkit/internal/EAWebKitAssert.h>
#include <EAWebkit/internal/EAWebKitNewDelete.h>
#include <EAWebkit/internal/EAWebkitEASTLHelpers.h>

namespace EA { namespace WebKit {
    JavascriptValue::JavascriptValue(void) 
    : mType(JavascriptValueType_Undefined)
    , mNumberValue(0.0) {
        // Do nothing.
    }

    void JavascriptValue::ShallowCopy(const JavascriptValue &v) {
        if (IsNonPOD()) {
            ReleaseNonPODStorage();
        }

        memcpy(this, &v, sizeof(*this));

        if (IsNonPOD()) {
            mNonPODValue.mShouldDelete = false;
        }
    }

    void JavascriptValue::Release(void) {
        if (IsNonPOD()) {
            ReleaseNonPODStorage();
        }
    }

    JavascriptValue::JavascriptValue(const JavascriptValue &v)
    : mType(JavascriptValueType_Undefined)
    , mNumberValue(0.0) {
        ShallowCopy(v);
    }

    JavascriptValue &JavascriptValue::operator=(const JavascriptValue &v) {
        ShallowCopy(v);
        return *this;
    }

    JavascriptValue::~JavascriptValue(void) {
        Release();
    }

    void JavascriptValue::SetStringInternal(const char16_t *chars) {
        EnsureNonPODStorage();
        mType = JavascriptValueType_String;
    
        if (chars) {
            GetFixedString(mNonPODValue.mValue->mStringValue)->assign(chars);
        }
    }

    void JavascriptValue::SetStringInternal(const char16_t *chars, int size) {
        EnsureNonPODStorage();
        mType = JavascriptValueType_String;

        if (chars) {
            GetFixedString(mNonPODValue.mValue->mStringValue)->assign(chars, size);
        }
    }

    void JavascriptValue::SetArrayInternal(JavascriptValue *pValues, int size) {
        EnsureNonPODStorage();
        mType = JavascriptValueType_Array;

        if (pValues) {
            GetVectorJavaScriptValue(mNonPODValue.mValue->mArrayValue)->assign(pValues, pValues + size);
        }
    }

    void JavascriptValue::SetNewObjectInternal(void) {
        EnsureNonPODStorage();
        mType = JavascriptValueType_Object;
    }

    const char16_t *JavascriptValue::GetStringCharacters(void) {
        EAW_ASSERT(mType == JavascriptValueType_String);
        return GetFixedString(mNonPODValue.mValue->mStringValue)->c_str();
    }

    void JavascriptValue::GetArrayValues(JavascriptValue **ppValuesOut, int *pSizeOut) {
        EAW_ASSERT(mType == JavascriptValueType_Array);

        VectorJavaScriptValue *valueVector = GetVectorJavaScriptValue(mNonPODValue.mValue->mArrayValue);
        *pSizeOut = valueVector->size();
        *ppValuesOut = valueVector->data();
    }

    JavascriptValue *JavascriptValue::GetHashMapValue(const char16_t *key, bool createIfMissing) {
        EAW_ASSERT(mType == JavascriptValueType_Object);
        HashMapJavaScriptValue* hashMap = GetHashMap(mNonPODValue.mValue->mHashMapValue);

        if (!createIfMissing) {
            HashMapJavaScriptValue::iterator iValue = hashMap->find(key);
            if (iValue == hashMap->end()) {
                return NULL;
            }

            return &iValue->second;
        }
        else {
            return &(*hashMap)[key];
        }
    }

#if defined(EA_DEBUG)
    void JavascriptValue::AssertType(JavascriptValueType type) const {
        EAW_ASSERT(mType == type);
    }
#endif

    void JavascriptValue::EnsureNonPODStorage(void) {
        if (!IsNonPOD()) {
            mNonPODValue.mValue = EAWEBKIT_NEW("JavascriptValueStorage") NonPODValue();
            mNonPODValue.mShouldDelete = true;     
        }
    }

    void JavascriptValue::FreeNonPODStorage(void) {
        EAW_ASSERT(IsNonPOD());
        EAW_ASSERT(mNonPODValue.mShouldDelete == true);
        EAWEBKIT_DELETE mNonPODValue.mValue;
        mNonPODValue.mValue = NULL;
        mNonPODValue.mShouldDelete = false;
    }

    bool JavascriptValue::IsNonPOD(void) {
        return mType >= JavascriptValueType_String;
    }

    void JavascriptValue::ReleaseNonPODStorage(void) {
        EAW_ASSERT(IsNonPOD());

        if (!mNonPODValue.mShouldDelete) {
            mNonPODValue.mValue = NULL;
        }
        else {
            FreeNonPODStorage();
        }

        mType = JavascriptValueType_Undefined;
    }
}}
