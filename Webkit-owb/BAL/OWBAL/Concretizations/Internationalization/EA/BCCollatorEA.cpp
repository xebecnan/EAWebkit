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

/////////////////////////////////////////////////////////////////////////////
// CollatorEA.cpp
// Written by Paul Pedriana
/////////////////////////////////////////////////////////////////////////////


#include "config.h"
#include "Collator.h"
#include "Assertions.h"

#if defined(_WIN32) && !defined(_XBOX)
    #include <windows.h>
#endif


namespace WTF {


std::auto_ptr<Collator> Collator::userDefault()
{
    return std::auto_ptr<Collator>(new Collator(0));
}


Collator::Result Collator::collate(const UChar* lhs, size_t lhsLength, const UChar* rhs, size_t rhsLength) const
{
    // To do: Switch this to use EAText::EACollator once we make a new version of it which allows for length arguments and works on console.

    int result; // enum Result { Equal = 0, Greater = 1, Less = -1 };

    #if defined(_WIN32) && !defined(_XBOX)
        // Until we completely finish the platform-independent code, we use the Windows CompareString API under Windows.
        // Note that we should ideally be using the locale that we are running in as opposed to the default user locale.
        result = CompareStringW(LOCALE_USER_DEFAULT, 0, lhs, (int)lhsLength, rhs, (int)rhsLength) - 2;
    #else
        // This is a minimum version which is incorrect for some locales.
        UChar c1 = 0, c2 = 0;
        const UChar* const lhsEnd = lhs + lhsLength;
        const UChar* const rhsEnd = rhs + rhsLength;

        while((lhs < lhsEnd) && (rhs < rhsEnd) && (c1 = *lhs++) == (c2 = *rhs++))
        {
            if(lhs == lhsEnd)
                return Equal;
        }

        if(lhs == lhsEnd)
            result = 1;
        else if(rhs == rhsEnd)
            result = -1;
        else
            result = (int)(c1 > c2 ? 1 : -1);

    #endif

    return (Collator::Result)result;
	// 04/20/2011 - abaldeva: Fixed the return value bug. There are two things wrong here.
	// 1. m_lowerFirst is uninitialized. We were fortunate so far that the garbage value was making it set to true(even in our debug build, memory is filled with 0xcd which ends up being
	// true.
	// 2. m_lowerFirst is an option that ICU uses to sort lower case before upper case (so 'a' sorts before 'A'). It seems like it was misinterpreted here into something that should reverse
	// the sorting order. This code is only called from the JavaScript (localeCompare) function and the ECMAScript specification does not specify choosing a custom sort order.
	// We conform (or intend to) the Unicode sorting. 
	// return m_lowerFirst ? (Collator::Result)result : (Collator::Result)-result;
}



} // namespace WTF
