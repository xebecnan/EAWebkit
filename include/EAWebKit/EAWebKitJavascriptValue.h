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
// EAWebKitJavascriptValue.h
//
// Created by Chris Stott, David Siems
///////////////////////////////////////////////////////////////////////////////

#ifndef EAWEBKIT_EAWEBKITJAVASCRIPTVALUE_H
#define EAWEBKIT_EAWEBKITJAVASCRIPTVALUE_H

#include <EAWebKit/EAWebkitSTLWrapper.h>

namespace EA
{
    namespace WebKit
    {
		//////////////////////////////////////////////////////////////////////////
		// Types - Order is important here, NonPOD types should be grouped and
        // should come after POD types.
		enum JavascriptValueType
		{
            // POD Types
            JavascriptValueType_Undefined,
			JavascriptValueType_Number,
            JavascriptValueType_Boolean,

            // Non-POD Types
			JavascriptValueType_String,
            JavascriptValueType_Array,
            JavascriptValueType_Object,
		};

        //////////////////////////////////////////////////////////////////////////
        //
		class JavascriptValue {
		public:
            //////////////////////////////////////////////////////////////////////////
            // Constructing - May only happen within the DLL
            JavascriptValue(void);

            //////////////////////////////////////////////////////////////////////////
            // Copying - Care must be taken for Non-POD types because they are
            // reference counted. You shouldn't copy these values from outside
            // the DLL.
            JavascriptValue(const JavascriptValue &v);
            JavascriptValue &operator=(const JavascriptValue &v);
            virtual ~JavascriptValue(void);

			//////////////////////////////////////////////////////////////////////////
			// Setters
			void SetUndefined(void) { 
                Release();
                mType = JavascriptValueType_Undefined;
                mNumberValue = 0.0;
			}

			void SetNumberValue(double value) {
                Release();
				mType = JavascriptValueType_Number;
				mNumberValue = value;
			}

            void SetBooleanValue(bool value) {
                Release();
                mType = JavascriptValueType_Boolean;
                mBooleanValue = value;
            }

            // It's okay to pass NULL here.
            void SetStringValue(const char16_t *chars) {
                SetStringInternal(chars);
            }

            void SetStringValue(const char16_t *chars, int count) {
                SetStringInternal(chars, count);
            }

            // It's okay to pass NULL here.
            void SetArrayValue(JavascriptValue *pValues, int size) {
                SetArrayInternal(pValues, size);
            }

            void SetNewObjectValue(void) {
                SetNewObjectInternal();
            }

			//////////////////////////////////////////////////////////////////////////
			// Getters
            JavascriptValueType GetType(void) const { 
                return mType;
            }

			double GetNumberValue(void) const {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_Number);
            #endif
                return mNumberValue;
			}

            bool GetBooleanValue(void) const {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_Boolean);
            #endif
                return mBooleanValue;
            }

            // To extract the string you will need to use the EAWebkit::GetCharacters() API.
			EASTLFixedString16Wrapper& GetStringValue(void) {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_String);
            #endif
                return mNonPODValue.mValue->mStringValue;
			}

            const EASTLFixedString16Wrapper& GetStringValue(void) const {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_String);
            #endif
                return mNonPODValue.mValue->mStringValue;
            }

            // To extract the array you will need to use the EAWebkit::() API
            EASTLVectorJavaScriptValueWrapper& GetArrayValue(void) {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_Array);
            #endif
                return mNonPODValue.mValue->mArrayValue;
            }

            const EASTLVectorJavaScriptValueWrapper& GetArrayValue(void) const {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_Array);
            #endif
                return mNonPODValue.mValue->mArrayValue;
            }

            // To extract the hashmap you will need to use the EAWebkit::Get() API
            EASTLJavascriptValueHashMapWrapper& GetHashMapValue(void) {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_Object);
            #endif
                return mNonPODValue.mValue->mHashMapValue;
            }

            const EASTLJavascriptValueHashMapWrapper& GetHashMapValue(void) const {
            #if defined(EA_DEBUG)
                AssertType(JavascriptValueType_Object);
            #endif
                return mNonPODValue.mValue->mHashMapValue;
            }

            //////////////////////////////////////////////////////////////////////////
            // Helpers
            virtual const char16_t *GetStringCharacters(void);
            virtual void GetArrayValues(JavascriptValue **ppValuesOut, int *pSizeOut);
            virtual JavascriptValue *GetHashMapValue(const char16_t *key, bool createIfMissing = false);

            //////////////////////////////////////////////////////////////////////////
            // Deprecated
            void SetStringType(void) {
                if (mType != JavascriptValueType_String) {
                    SetStringValue(NULL);
                }
            }

            void SetArrayType(void) {
                if (mType != JavascriptValueType_Array) {
                    SetArrayValue(NULL, 0);
                }
            }

            void SetObjectType(void) {
                if (mType != JavascriptValueType_Object) {
                    SetNewObjectValue();
                }
            }
            //
            //////////////////////////////////////////////////////////////////////////

		private:
            // These methods must be virtual so they can be called from outside the DLL.
            virtual void ShallowCopy(const JavascriptValue &v);
            virtual void Release(void);

            virtual void SetStringInternal(const char16_t *chars);
            virtual void SetStringInternal(const char16_t *chars, int count);
            virtual void SetArrayInternal(JavascriptValue *pValues, int size);
            virtual void SetNewObjectInternal(void);

        #if defined(EA_DEBUG)
            virtual void AssertType(JavascriptValueType type) const;
        #endif
            
            // Only used internally.
            void EnsureNonPODStorage(void);
            void FreeNonPODStorage(void);
            bool IsNonPOD(void);
            void ReleaseNonPODStorage(void);

            struct NonPODValue {
                EASTLFixedString16Wrapper mStringValue;
                EASTLVectorJavaScriptValueWrapper mArrayValue;
                EASTLJavascriptValueHashMapWrapper mHashMapValue;
            };
            
            struct NonPODValueRef {
                NonPODValue *mValue;
                bool mShouldDelete;
            };

            // Care should be taken to keep this class as small as possible.
            JavascriptValueType	mType;          
            union
            {
                double mNumberValue;
                bool mBooleanValue;
                NonPODValueRef mNonPODValue;
            };
		};
    } // namespace WebKit
} // namespace EA

#endif // EAWEBKIT_EAWEBKITJAVASCRIPTVALUE_H
