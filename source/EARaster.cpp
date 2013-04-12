/*
Copyright (C) 2009-2011 Electronic Arts, Inc.  All rights reserved.

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
// EARaster.cpp
// By Paul Pedriana
///////////////////////////////////////////////////////////////////////////////


#include <eastl/fixed_vector.h>
#include <EAWebkit/EAWebkitAllocator.h>
#include <EARaster/EARaster.h>
#include <EARaster/internal/EARasterInternal.h>
#include "EARaster/internal/EARasterUtils.h"
#include <EARaster/EARasterColor.h>
#include "Color.h"
#include "IntRect.h"
#include <EAWebkit/internal/EAWebKitAssert.h>
#include "BAL/WKAL/Concretizations/Graphics/EA/BCImageCompressionEA.h"
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>


namespace EA {

	namespace WebKit {
		EAWEBKIT_API EA::Raster::IEARaster* GetEARasterInstance();   
	}

	namespace Raster {

		// Utility function
		EARASTER_API bool IntersectRect(const Rect& a, const Rect& b, Rect& result)
		{
			// Horizontal
			int aMin = a.x;
			int aMax = aMin + a.w;
			int bMin = b.x;
			int bMax = bMin + b.w;

			if(bMin > aMin)
				aMin = bMin;
			result.x = aMin;
			if(bMax < aMax)
				aMax = bMax;
			result.w = (((aMax - aMin) > 0) ? (aMax - aMin) : 0);

			// Vertical
			aMin = a.y;
			aMax = aMin + a.h;
			bMin = b.y;
			bMax = bMin + b.h;

			if(bMin > aMin)
				aMin = bMin;
			result.y = aMin;
			if(bMax < aMax)
				aMax = bMax;
			result.h = (((aMax - aMin) > 0) ? (aMax - aMin) : 0);

			return (result.w && result.h);
		}

		///////////////////////////////////////////////////////////////////////
		// Surface
		///////////////////////////////////////////////////////////////////////


		Surface::Surface()
		{
			InitMembers();
			SetPixelFormat(kPixelFormatTypeARGB);
		}


		Surface::Surface(int width, int height, PixelFormatType pft)
		{
			InitMembers();
			SetPixelFormat(pft);
			Resize(width, height);
		}


		Surface::~Surface()
		{
			FreeData();
		}


		int Surface::AddRef()
		{
			// This is not thread-safe.
			return ++mRefCount;
		}


		int Surface::Release()
		{
			// This is not thread-safe.
			if(mRefCount > 1)
				return --mRefCount;

			DestroySurface(this);
			return 0;
		}


		void Surface::InitMembers()
		{
			// mPixelFormat  // Should probably init this.
			mSurfaceFlags  = 0;
			mpData         = NULL;
			mWidth         = 0;
			mHeight        = 0;
			mStride        = 0;
			mLockCount     = 0;
			mRefCount      = 0;
			mpUserData     = NULL;
			mCompressedSize = 0;

			// Draw info.
			mClipRect.x    = 0;
			mClipRect.y    = 0;
			mClipRect.w    = 0; //INT_MAX;  - Testing setting to 0 instead as an infinite size seems dangerous as we rely on the cliprect to keep things within a surface.
			mClipRect.h    = 0; //INT_MAX;
			mpBlitDest     = 0;
			mDrawFlags     = 0;
			mpBlitFunction = NULL;
			mCategory      = kSurfaceCategoryDefault; 
		}


		void Surface::SetPixelFormat(PixelFormatType pft)
		{
			mPixelFormat.mPixelFormatType = pft;
			mPixelFormat.mSurfaceAlpha    = 255;

			if (pft == kPixelFormatTypeRGB)
				mPixelFormat.mBytesPerPixel = 3;
			else
				mPixelFormat.mBytesPerPixel = 4;

			switch (pft)
			{
			case kPixelFormatTypeInvalid:
				break;

			case kPixelFormatTypeARGB:
				mPixelFormat.mRMask  = 0x00ff0000;
				mPixelFormat.mGMask  = 0x0000ff00;
				mPixelFormat.mBMask  = 0x000000ff;
				mPixelFormat.mAMask  = 0xff000000;
				mPixelFormat.mRShift = 16;
				mPixelFormat.mGShift = 8;
				mPixelFormat.mBShift = 0;
				mPixelFormat.mAShift = 24;
				break;

			case kPixelFormatTypeRGBA:
				mPixelFormat.mRMask  = 0xff000000;
				mPixelFormat.mGMask  = 0x00ff0000;
				mPixelFormat.mBMask  = 0x0000ff00;
				mPixelFormat.mAMask  = 0x000000ff;
				mPixelFormat.mRShift = 24;
				mPixelFormat.mGShift = 16;
				mPixelFormat.mBShift = 8;
				mPixelFormat.mAShift = 0;
				break;

			case kPixelFornatTypeXRGB:
				mPixelFormat.mRMask  = 0x00ff0000;
				mPixelFormat.mGMask  = 0x0000ff00;
				mPixelFormat.mBMask  = 0x000000ff;
				mPixelFormat.mAMask  = 0x00000000;
				mPixelFormat.mRShift = 16;
				mPixelFormat.mGShift = 8;
				mPixelFormat.mBShift = 0;
				mPixelFormat.mAShift = 24;
				break;

			case kPixelFormatTypeRGBX:
				mPixelFormat.mRMask  = 0xff000000;
				mPixelFormat.mGMask  = 0x00ff0000;
				mPixelFormat.mBMask  = 0x0000ff00;
				mPixelFormat.mAMask  = 0x00000000;
				mPixelFormat.mRShift = 24;
				mPixelFormat.mGShift = 16;
				mPixelFormat.mBShift = 8;
				mPixelFormat.mAShift = 0;
				break;

			case kPixelFormatTypeRGB:
				mPixelFormat.mRMask  = 0xff000000;  // I think this is wrong, or at least endian-dependent.
				mPixelFormat.mGMask  = 0x00ff0000;
				mPixelFormat.mBMask  = 0x0000ff00;
				mPixelFormat.mAMask  = 0x00000000;
				mPixelFormat.mRShift = 24;
				mPixelFormat.mGShift = 16;
				mPixelFormat.mBShift = 8;
				mPixelFormat.mAShift = 0;
				break;
			}
		}

		bool Surface::Set(void *pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData, SurfaceCategory category)
		{
			FreeData();

			mpBlitDest     = 0;
			mDrawFlags     = 0;
			mpBlitFunction = NULL;

			if(pData)
			{
				SetPixelFormat(pft);
				SetCategory(category);

				if(bCopyData)
				{
					if(Resize(width, height))
					{
						void *pDest = NULL;
						int destStride = 0;

						Lock(&pDest, &destStride);
						if (pDest == NULL) {
							return false;
						}

						if(destStride == stride)
						{
							memcpy(pDest, pData, height * stride);
						}
						else
						{
							// Case where the source and dest buffers have different strides so line copy instead                   
							char* pDestRow = reinterpret_cast<char*>(pDest);     
							char* pSourceRow = reinterpret_cast<char*>(pData);     
							int lineSize = width * GetPixelFormat().mBytesPerPixel;

							for(int y = 0; y < height; ++y)
							{
								memcpy(pDestRow, pSourceRow, lineSize);          

								pDestRow += destStride;   
								pSourceRow +=stride;    
							}
						}

						Unlock();
					}
					else
					{
						return false;
					}
				}
				else
				{
					mpData  = pData;
					mWidth  = width;
					mHeight = height;
					mStride = stride;

					mSurfaceFlags |= kFlagOtherOwner;
					SetClipRect(NULL);  
				}
			}

			return true;
		}


		bool Surface::Set(Surface* pSourceImage)
		{
			EAW_ASSERT(pSourceImage->GetPixelFormat().mPixelFormatType == this->GetPixelFormat().mPixelFormatType);

			int sourceWidth = 0;
			int sourceHeight = 0;
			pSourceImage->GetDimensions(&sourceWidth, &sourceHeight);

			void *pSource = NULL;
			int sourceStride = 0;
			pSourceImage->Lock(&pSource, &sourceStride);

			if (pSource == NULL) {
				return false;
			}

			bool result = Set(pSource, sourceWidth, sourceHeight, sourceStride, pSourceImage->GetPixelFormat().mPixelFormatType, true, pSourceImage->GetCategory());

			pSourceImage->Unlock();

			return result;
		}


		bool Surface::Resize(int width, int height)
		{
			FreeData();

			const size_t kNewMemorySize = width * height * mPixelFormat.mBytesPerPixel;
			mpData = EAWEBKIT_NEW("Surface Resize") char[kNewMemorySize];

			if(mpData) 
			{
#ifdef EA_DEBUG
				// Commented out because we want the same behavior in debug as release.
                // memset(mpData, 0, kNewMemorySize);
#endif

				mWidth  = width;
				mHeight = height;
				mStride = width * mPixelFormat.mBytesPerPixel;

				SetClipRect(NULL);
				return true;
			}

			return false;
		}


		bool Surface::FreeData()
		{
			bool returnFlag = false;

			// Remove the compressed data buffer which is stored in the user data
			if( ((mSurfaceFlags & (EA::Raster::kFlagCompressedRLE | EA::Raster::kFlagCompressedYCOCGDXT5 )) != 0 ) &&
				(mpUserData) && (mCompressedSize) )
			{
				EAWEBKIT_DELETE[] ((char*)mpUserData);
				mpUserData = NULL;
				mCompressedSize = 0;
				mSurfaceFlags &= ~(EA::Raster::kFlagCompressedRLE | EA::Raster::kFlagCompressedYCOCGDXT5);
				returnFlag = true;
			}

			if((mSurfaceFlags & kFlagOtherOwner) == 0)  // If we own the pointer...
			{
				EAWEBKIT_DELETE[] ((char*)mpData);
				mpData = NULL;

				// We only want to clear this stuff if there is no other owner for the other owner might still need this info.
				mSurfaceFlags = 0; 
				returnFlag = true;
			}

			return returnFlag;
		}

		// Can be used for texture lock notification
		void Surface::Lock(void **pDataOut, int *strideOut)
		{
			*pDataOut = mpData;
			*strideOut = mWidth * mPixelFormat.mBytesPerPixel;
		}

		void Surface::Unlock()
		{
			// Do nothing.
		}

		void Surface::GetDimensions(int *widthOut, int *heightOut) 
		{
			*widthOut = mWidth;
			*heightOut = mHeight;
		}

		size_t Surface::GetSizeBytes(void) 
		{
			return mWidth * mHeight * mPixelFormat.mBytesPerPixel;
		}

		void Surface::SetClipRect(const Rect* pRect)
		{
			Rect fullRect(0, 0, mWidth, mHeight);

			if(pRect)
				EA::Raster::IntersectRect(*pRect, fullRect, mClipRect);
			else
				mClipRect = fullRect;
		}

		ISurface* CreateSurface(void)
		{
			// This makes a call out to the raster's create surface function so that all
			// surfaces are created through the external interface.
			return EA::WebKit::GetEARasterInstance()->CreateSurface();    
		}

		ISurface* CreateSurface(int width, int height, PixelFormatType pft,SurfaceCategory category)
		{
			ISurface* pNewSurface = CreateSurface();

			if(pNewSurface)
			{
				// Note that pNewSurface is already AddRef'd.
				pNewSurface->SetPixelFormat(pft);
				pNewSurface->SetCategory(category);
				if(!pNewSurface->Resize(width, height))
				{
					DestroySurface(pNewSurface);
					pNewSurface = NULL;
				}
			}

			return pNewSurface;
		}

		ISurface* CreateSurface(void* pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData,SurfaceCategory category)
		{
			ISurface* pNewSurface = CreateSurface();

			if(pNewSurface)
			{
				// Note that pNewSurface is already AddRef'd.
				if(!pNewSurface->Set(pData, width, height, stride, pft, bCopyData, category))
				{
					DestroySurface(pNewSurface);
					pNewSurface = NULL;
				}
			}

			return pNewSurface;
		}


		void DestroySurface(ISurface* pSurface)
		{
			EAWEBKIT_DELETE pSurface;
		}


		///////////////////////////////////////////////////////////////////////
		// Utility functions
		///////////////////////////////////////////////////////////////////////

		// To do: Convert this to a .tga writer instead of .ppm, as .tga is more commonly
		//        supported yet is still a fairly simple format.
		EARASTER_API bool WritePPMFile(const char* pPath, ISurface* pSurface, bool bAlphaOnly)
		{
			FILE* const fp = fopen(pPath, "w");

			if(fp)
			{
				const bool bARGB = (pSurface->GetPixelFormat().mPixelFormatType == EA::Raster::kPixelFormatTypeARGB);

				int width = 0;
				int height = 0;
				pSurface->GetDimensions(&width, &height);
				fprintf(fp, "P3\n");
				fprintf(fp, "# %s\n", pPath);
				fprintf(fp, "%d %d\n", width, height);
				fprintf(fp, "%d\n", 255);

				for(int y = 0; y < height; y++)
				{
					for(int x = 0; x < width; x++)
					{
						EA::Raster::Color color; EA::Raster::GetPixel(pSurface, x, y, color);
						const uint32_t    c = color.rgb();
						unsigned          a, r, g, b;

						if(bAlphaOnly)
						{
							if(bARGB)
								a = (unsigned)((c >> 24) & 0xff);  // ARGB
							else
								a = (unsigned)(c & 0xff);          // RGBA

							fprintf(fp, "%03u %03u %03u \t", a, a, a);
						}
						else
						{
							if(bARGB)
							{
								r = (unsigned)((c >> 16) & 0xff); // ARGB
								g = (unsigned)((c >>  8) & 0xff);
								b = (unsigned)((c >>  0) & 0xff);
							}
							else
							{
								r = (unsigned)((c >> 24) & 0xff); // RGBA
								g = (unsigned)((c >> 16) & 0xff);
								b = (unsigned)((c >>  8) & 0xff);
							}

							fprintf(fp, "%03u %03u %03u \t", r, g, b);
						}
					}

					fprintf(fp, "\n");
				}

				fprintf(fp, "\n");
				fclose(fp);

				return true;
			}

			return false;
		}


		void IntRectToEARect(const WKAL::IntRect& in, EA::Raster::Rect& out)
		{
			out = EA::Raster::Rect( in.x(), in.y(), in.width(), in.height() );
		}

		void EARectToIntRect(const EA::Raster::Rect& in, WKAL::IntRect& out)
		{
			out = WebCore::IntRect( in.x, in.y, in.w, in.h );
		}

	} // namespace Raster

} // namespace EA

namespace EA
{
	namespace Raster
	{
		// Surface management
		ISurface* EARasterConcrete::CreateSurface() 
		{
			Surface* pSurface = EAWEBKIT_NEW("Surface") Surface();//WTF::fastNew<Surface>();

			if(pSurface)
				pSurface->AddRef();

			return pSurface;	 
		}

		ISurface* EARasterConcrete::CreateSurface(int width, int height, PixelFormatType pft,SurfaceCategory category)
		{
			return EA::Raster::CreateSurface(width, height, pft, category);
		}

		ISurface* EARasterConcrete::CreateSurface(void* pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData, SurfaceCategory category)
		{
			return EA::Raster::CreateSurface(pData, width, height, stride, pft, bCopyData, category);
		}

		void EARasterConcrete::DestroySurface(ISurface* pSurface)
		{
			EA::Raster::DestroySurface(pSurface);
		}

		// Color conversion
		void EARasterConcrete::ConvertColor(NativeColor c, const PixelFormat& pf, Color *colorOut)
		{
			EA::Raster::ConvertColor(c, pf, colorOut);
		}

        int EARasterConcrete::Clear(ISurface *pSurface, const Rect &rect, const Color &color) {
            return EA::Raster::FillRectSolidColor(pSurface, &rect, color);
        }

		// Rectangle functions
		int EARasterConcrete::RectangleFilled(ISurface *pSurface, const Rect &rect, const Color &color) 
		{
			if (color.alpha() == 0xff)
			{
				return EA::Raster::FillRectSolidColor(pSurface, &rect, color);
			}
			else
			{
				return EA::Raster::FillRectColor(pSurface, &rect, color);
			}
		}

		int EARasterConcrete::RectangleOutlined(ISurface* pSurface, const EA::Raster::Rect& rect, const Color& color)
		{
			return EA::Raster::RectangleColor(pSurface, rect, color);
		}

		// Line functions
		int EARasterConcrete::Line(ISurface *pSurface, int x1, int y1, int x2, int y2, const Color &color)
		{
			return EA::Raster::LineColor(pSurface, x1, y1, x2, y2, color);
		}

		// Circle / Ellipse
		int EARasterConcrete::EllipseFilled(ISurface* pSurface, int x, int y, int rx, int ry, const Color &color)
		{
			return EA::Raster::FilledEllipseColor(pSurface, x, y, rx, ry, color);
		}

		int EARasterConcrete::EllipseOutlined(ISurface* pSurface, int x, int y, int rx, int ry, const Color &color)
		{
			return EA::Raster::EllipseColor(pSurface,  x,  y,  rx,  ry,  color);
		}

		// Polygon
		int EARasterConcrete::TriangleOriented(ISurface* pSurface, int x, int y, int size, Orientation o, const Color &color)
		{
			return EA::Raster::SimpleTriangle(pSurface,   x,   y,  size, o, color);
		}

		int EARasterConcrete::PolygonFilled(ISurface* pSurface, const int *vx, const int *vy, int n, const Color &color)
		{
			return EA::Raster::FilledPolygonColorMT(pSurface, vx, vy, n, color, NULL, NULL);
		}

		// Images
		int EARasterConcrete::DrawSurface(ISurface *pImage, const Rect &sourceRect, ISurface *pDest, const Rect &destRect, const Matrix2D &transform, float alpha)
		{
			Rect transformedSourceRect = sourceRect;
			ISurface *pSurfaceToDraw = GetSurfaceToDraw(pImage, &transformedSourceRect, transform, alpha);
			if (pSurfaceToDraw)
			{
				EA::Raster::Blit(static_cast<Surface*>(pSurfaceToDraw), &transformedSourceRect, pDest, &destRect);

				if (pSurfaceToDraw != pImage)
				{
					DestroySurface(pSurfaceToDraw);
				}
			}

			return 0;
		}

		int EARasterConcrete::DrawSurfaceTiled(ISurface *pImage, const Rect &sourceRect, ISurface *pDest, const Rect &destRect, const Rect &clipRect, const Matrix2D &transform, float alpha) 
		{
			Rect transformedSourceRect = sourceRect;
			ISurface *pSurfaceToDraw = GetSurfaceToDraw(pImage, &transformedSourceRect, transform, alpha);
			if (pSurfaceToDraw)
			{
                int xStart = destRect.x;
                int xStop = destRect.x + destRect.width();
                int yStart = destRect.y;
                int yStop = destRect.y + destRect.height();

                int width = transformedSourceRect.width();
                int height = transformedSourceRect.height();
				for (int x = xStart; x <= xStop; x += width)
				{
					for (int y = yStart; y <= yStop; y += height)
					{
                        EA::Raster::Rect tileRect(x, y, width, height);
						EA::Raster::Blit(static_cast<Surface*>(pSurfaceToDraw), &transformedSourceRect, pDest, &tileRect, &clipRect);
					}
				}

				if (pSurfaceToDraw != pImage)
				{
					DestroySurface(pSurfaceToDraw);
				}
			}

			return 0;
		}

		ISurface *EARasterConcrete::GetSurfaceToDraw(ISurface *pSurface, Rect *sourceRectInOut, const Matrix2D &transform, float alpha)
		{
			EA::Raster::ISurface* pAlphadSurface = 0;
			EA::Raster::ISurface *pTransformSurface = 0;   
			EA::Raster::ISurface* pDecompressedImage = 0;

#if EAWEBKIT_USE_RLE_COMPRESSION || EAWEBKIT_USE_YCOCGDXT5_COMPRESSION            
			pDecompressedImage = WebCore::BCImageCompressionEA::UnpackCompressedImage(static_cast<EA::Raster::Surface*>(pSurface));         
			if(pDecompressedImage != NULL)
				pSurface = pDecompressedImage;
#endif

			// Alpha surface
			if (alpha < 1.0f)
			{
				// Note by Paul Pedriana: Why is it applying transparency via a new image when instead it could 
				// just do the blit with surface transparency added in? Is that not possible with SDL?
				pAlphadSurface = EA::Raster::CreateTransparentSurface(pSurface, static_cast<int>(alpha * 255) & 0xff);
				if (pAlphadSurface)
					pSurface = pAlphadSurface;

#if EAWEBKIT_USE_RLE_COMPRESSION || EAWEBKIT_USE_YCOCGDXT5_COMPRESSION            
				// Reduce memory right away when possible
				if(pDecompressedImage) 
				{
					DestroySurface(pDecompressedImage);    
					pDecompressedImage = 0;
				}
#endif
			}

			// Transform surface
			if(!(transform.m_m11 == 1.0f && transform.m_m12 == 0.0f && transform.m_m21 == 0.0f && transform.m_m22 == 1.0f && transform.m_dx == 0.0f && transform.m_dy == 0.0f))
			{
				pTransformSurface = EA::Raster::TransformSurface(pSurface, *sourceRectInOut, transform);

				if (pTransformSurface) {
					pSurface = pTransformSurface;
					// Set the rect to the entire size of the transformed image, the TransformSurface function should
					// return a 'cut-out' of the original surface.
					sourceRectInOut->x = 0;
					sourceRectInOut->y = 0;
					pTransformSurface->GetDimensions(&sourceRectInOut->w, &sourceRectInOut->h);
				}   

				if (pAlphadSurface)
				{
					DestroySurface(pAlphadSurface);
					pAlphadSurface = NULL;
				}

#if EAWEBKIT_USE_RLE_COMPRESSION || EAWEBKIT_USE_YCOCGDXT5_COMPRESSION            
				if (pDecompressedImage)
				{
					DestroySurface(pDecompressedImage);
					pDecompressedImage = NULL;
				}
#endif
			}

			return pSurface;
		}

		int EARasterConcrete::CompressImage(ISurface *pImage, bool hasAlpha, int sizeTotal, int *sizeOut)
		{
#if EAWEBKIT_USE_RLE_COMPRESSION || EAWEBKIT_USE_YCOCGDXT5_COMPRESSION             
			Surface *pSurface = static_cast<Surface*>(pImage);

			static const int MIN_DECODED_SIZE_FOR_COMPRESSION = 1024;    // 1K

			// Only load once the image has fully been decoded or we have problems...
			if(sizeTotal > MIN_DECODED_SIZE_FOR_COMPRESSION) {
				int bufferSize = WebCore::BCImageCompressionEA::PackAsCompressedImage(pSurface, hasAlpha, true);
				if(bufferSize > 0) {
					pSurface->SetCompressedSize(bufferSize);
					*sizeOut = bufferSize;
				}
			}
#endif

			return 0;
		}

        int EARasterConcrete::DrawGlyphs(GlyphDrawInfo *glyphs, int glyphCount, EA::WebKit::ITextureInfo* source, ISurface *pDest, const Rect &rectSource, const Rect &rectDest, const Color &color, const Matrix2D &transform, float alpha, EA::WebKit::Effect textEffect) {
            if (glyphCount <= 0) {
                return 0;
            }

            // Write the glyphs into a linear buffer.
            // To consider: 
            //See if it's possible to often get away with using stack space for this by
            //              using Vector<uint32_t, N> in order to avoid a memory allocation. A typical
            //              width is ~400 pixels and a typical height ~16 pixels, which is 25600 bytes.
            //              So unless the string is small there won't be any benefit to this.

            //Answer(Arpit Baldeva): In practice, I am seeing lot of smaller allocations.
            //We allocate 2k on the stack in following. So if (destWidth*destHeight)<=500,
            //we save an allocation. This could be significant as it is used for each text draw. Currently, we have
            //weird full screen draw issues, so this is pretty helpful.
            //In case (destWidth*destHeight) > 500, following vector would automatically allocate from the heap.
            eastl::fixed_vector<uint32_t, 500, true, EA::WebKit::EASTLAllocator> glyphRGBABuffer;
            glyphRGBABuffer.reserve(rectSource.w * rectSource.h);

            //WTF::Vector<uint32_t, 500> glyphRGBABuffer(rectSource.w * rectSource.h);		
            uint32_t  penC    = color.rgb();
            uint32_t  penA    = (penC >> 24);
            uint32_t  penRGB  = (penC & 0x00ffffff);

            memset(glyphRGBABuffer.data(), 0, glyphRGBABuffer.capacity() * sizeof(uint32_t));

            const int textureSize = (int)source->GetSize();
            for (int i = 0; i < glyphCount; ++i)
            {
                const EA::Raster::GlyphDrawInfo& gdi         = glyphs[i];
                const int       yOffset     = gdi.y1;
                // Note by Arpit Baldeva: Old index calculation below caused a memory overrun(or I should say under-run on http://get.adobe.com/flashplayer/).
                // Basically, yOffset * destWidth) + gdi.x1 could end up being negative if the yOffset is 0 and gdi.x1 is negative. Since I do want
                // to make sure that index is positive, I just force it to be at least 0. There is not enough understanding about this code as of now
                // and Paul said that he would take a look at this along with other Font problems he is currently looking at.				
                const int bufferIndex = (yOffset * rectSource.w) + gdi.x1;
                EAW_ASSERT_FORMATTED(bufferIndex>=0, "Buffer Index is negative. This would corrupt memory. yOffset:%d,destWidth:%u,gdi.x1:%d",yOffset,rectSource.w,gdi.x1);
                uint32_t*            pDestColor  = glyphRGBABuffer.data() + (bufferIndex >= 0 ? bufferIndex : 0);//Old Index calculation - (yOffset * destWidth) + gdi.x1;

                const int            glyphWidth  = (gdi.x2 - gdi.x1);
                const int            glyphHeight = (gdi.y2 - gdi.y1);			

                const int tx = (int)(gdi.u0 * textureSize);
                const int ty = (int)(gdi.v0 * textureSize);

                // We need to check what bit format was are in here.
                if(source->GetFormat() == EA::WebKit::kBFARGB)     
                {
                    const uint32_t* pGlyphAlpha = (uint32_t*) ((source->GetData()) + (ty * source->GetStride()) + (tx << 2));
                    const uint32_t stride = source->GetStride() >> 2; // >> 2 for 32 bits

                    // Normal 32 bit render using the pen color
                    if(textEffect == EA::WebKit::kEffectNone)        
                    {
                        for (int y = 0; y < glyphHeight; ++y)
                        {
                            for (int x = 0; x < glyphWidth; ++x)
                            {
                                //+ 1/5/10 CSidhall - When building the text string texture map, kerning can make certain letters overlap and stomp over the alpha of previous letters so 
                                // we need to preserve texels that were already set. Should be able to OR here because we have a clean buffer and penRGB is the same.
                                //+ 2/23/10 YChin - We have switched to using premultiplied colors, so we need to multiply the alpha onto the final color so the Blt
                                //uint32_t destAlpha = EA::Raster::DivideBy255Rounded(penA * pGlyphAlpha[x]) | (pDestColor[x] >> 24);
                                //uint32_t destRGB = EA::Raster::MultiplyColorAlpha(penRGB, destAlpha);
                                //+ 10/21/10 CSidhall - Adapted to support 32bit textures
                                uint32_t surfaceAlpha = pDestColor[x] >> 24; 
                                uint32_t glyphColor = pGlyphAlpha[x]; 

                            #if defined(EA_PLATFORM_PS3)
                                // For some reason, we end up with RGBA here                    
                                uint32_t glyphAlpha = glyphColor&0xff; 
                            #else
                                // ARGB here
                                uint32_t glyphAlpha = glyphColor >> 24; 
                            #endif   

                                uint32_t destAlpha = EA::Raster::DivideBy255Rounded(penA * glyphAlpha) | surfaceAlpha;
                                uint32_t destRGB = EA::Raster::MultiplyColorAlpha(penRGB, destAlpha);
                                
                                pDestColor[x] = (destAlpha << 24 ) | destRGB;		
                            }

                            pDestColor  += rectSource.w;
                            pGlyphAlpha += stride;   
                        }
                    }
                    else
                    {
                        // Using the 32bit colors instead of the pen. For multicolored effects. 
                        for (int y = 0; y < glyphHeight; ++y)
                        {
                            for (int x = 0; x < glyphWidth; ++x)
                            {
                                //+ 1/5/10 CSidhall - When building the text string texture map, kerning can make certain letters overlap and stomp over the alpha of previous letters so 
                                // we need to preserve texels that were already set. Should be able to OR here because we have a clean buffer and penRGB is the same.
                                //+ 2/23/10 YChin - We have switched to using premultiplied colors, so we need to multiply the alpha onto the final color so the Blt
                                //+ 10/19/10 CSidhall - Adapted to using 32 bit colors 
                                uint32_t surfaceAlpha = pDestColor[x] >> 24; 
                                uint32_t glyphColor = pGlyphAlpha[x]; 

                            #if defined(EA_PLATFORM_PS3)
                                // For some reason, we end up with RGBA here                    
                                uint32_t glyphAlpha = glyphColor&0xff; 
                                glyphColor >>=8;
                            #else
                                uint32_t glyphAlpha = glyphColor >> 24; 
                                glyphColor &=0x00ffffff;
                            #endif   
                                // Note: This seems to work pretty well for the current usage.
                                // It basically just gives the the RGB weight to the one that has the 
                                // strongest alpha.  Another option could be to consider src and dest color blending here. 
                                if(glyphAlpha > surfaceAlpha)     
                                {                        
                                    uint32_t destRGB = EA::Raster::MultiplyColorAlpha(glyphColor, glyphAlpha);
                                    pDestColor[x] = (glyphAlpha << 24 ) | destRGB;		
                                }
                            }
                            pDestColor  += rectSource.w;
                            pGlyphAlpha += stride;        
                        }
                    }
                }
                else // our default: kBFGrayscale     
                {
                    // This is for the more compact 8bit format. Just passes down the alpha and the pen color provides the RGB
                    const uint8_t* pGlyphAlpha = (source->GetData()) + (ty * source->GetStride()) + tx;
                    const uint32_t stride = source->GetStride();

                    for (int y = 0; y < glyphHeight; ++y)
                    {
                        for (int x = 0; x < glyphWidth; ++x)
                        {
                            //+ 1/5/10 CSidhall - When building the text string texture map, kerning can make certain letters overlap and stomp over the alpha of previous letters so 
                            // we need to preserve texels that were already set. Should be able to OR here because we have a clean buffer and penRGB is the same.
                            //+ 2/23/10 YChin - We have switched to using premultiplied colors, so we need to multiply the alpha onto the final color so the Blt
                            // Old code:
                            //pDestColor[x] = ((penA * pGlyphAlpha[x] / 255) << 24) | penRGB;
                            // New code:
                            uint32_t destAlpha = EA::Raster::DivideBy255Rounded(penA * pGlyphAlpha[x]) | (pDestColor[x] >> 24);
                            uint32_t destRGB = EA::Raster::MultiplyColorAlpha(penRGB, destAlpha);
                            pDestColor[x] = (destAlpha << 24 ) | destRGB;					
                        }

                        pDestColor  += rectSource.w;
                        pGlyphAlpha += stride;
                    }
                }     
            }

            // It would probably be faster if we kept around a surface for multiple usage instead of 
            // continually recreating this one.
            EA::Raster::ISurface* pGlyphSurface = CreateSurface((void*)glyphRGBABuffer.data(), rectSource.w, rectSource.h, rectSource.w * 4, EA::Raster::kPixelFormatTypeARGB, false, EA::Raster::kSurfaceCategoryText);

            DrawSurface(pGlyphSurface, rectSource, pDest, rectDest, transform, alpha);

            if(pGlyphSurface)
                DestroySurface(pGlyphSurface);
 
            return 0;
        }

		// A PPM file is a simple bitmap format which many picture viewers can read.
		bool EARasterConcrete::WriteSurfaceToFile(const char* pPath, ISurface* pSurface, bool bAlphaOnly)
		{
			return EA::Raster::WritePPMFile(pPath, pSurface, bAlphaOnly);
		}
	}
}

