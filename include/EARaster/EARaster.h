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
// EARaster.h
// By Paul Pedriana
///////////////////////////////////////////////////////////////////////////////


#ifndef EARASTER_EARASTER_H
#define EARASTER_EARASTER_H


#include <EABase/eabase.h>
#include <EARaster/EARasterColor.h>
#include <EARaster/EARasterConfig.h>
#include <EAWebKit/EAWebKitTextInterface.h>

#ifndef UNUSED_IRASTER_CALLS_ENABLED
    #define UNUSED_IRASTER_CALLS_ENABLED 0    // Set to 1 to activate unused Raster calls.  These are disabled to simplify IRaster.
#endif

namespace WKAL  // a.k.a. namespace WebCore
{
    class IntRect;
}


namespace EA
{
    namespace Raster
    {
        // Forward declarations
        class ISurface;
        class Color;


        // Typedefs
        typedef uint32_t NativeColor;     // Refers to a color used by a surface in the surface's PixelFormatType.


        // Orientation
        // These can be used as values or as flags.
        enum Orientation
        {
            kOLeft  = 0x01,
            kOUp    = 0x02,     // a.k.a. Top
            kORight = 0x04,
            kODown  = 0x08      // a.k.a. Bottom
        };


        // Note: WebKit's WKAL::Color type declares itself to be RGBA, but it is wrong; 
        // it is using ARGB, in particular the 32 bit 0xAARRGGBB type as we have below.
        enum PixelFormatType
        {
            kPixelFormatTypeInvalid,
            kPixelFormatTypeARGB,           // 32 bit 0xAARRGGBB, byte order in memory depends on endian-ness. For little endian it is B, G, R, A; for big endian it is A, R, G, B.
            kPixelFormatTypeRGBA,           // 32 bit 0xRRGGBBAA, byte order in memory depends on endian-ness. For little endian it is A, B, G, R; for big endian it is R, G, B, A. 
            kPixelFornatTypeXRGB,           // 32 bit 0xXXRRGGBB, byte order in memory depends on endian-ness. The X means the data is unused or can be considered to be always 0xff.
            kPixelFormatTypeRGBX,           // 32 bit 0xRRGGBBXX, byte order in memory depends on endian-ness. The X means the data is unused or can be considered to be always 0xff.
            kPixelFormatTypeRGB             // 24 bit 0xRRGGBB, byte order in memory is always R, G, B.
        };


        struct PixelFormat
        {
            PixelFormatType mPixelFormatType;   // e.g. kPixelFormatTypeARGB.
            uint8_t         mBytesPerPixel;     // Typically four, as in the case of ARGB.
            uint8_t         mSurfaceAlpha;      // Alpha that is applied to the entire surface and not just per pixel.
	        uint32_t        mRMask;
	        uint32_t        mGMask;
	        uint32_t        mBMask;
	        uint32_t        mAMask;
	        uint8_t         mRShift;
	        uint8_t         mGShift;
	        uint8_t         mBShift;
	        uint8_t         mAShift;
        };


        enum SurfaceFlags
        {
            kFlagRAMSurface     = 0x00,     // The pixel data resides in regular RAM.
            kFlagTextureSurface = 0x01,     // The pixel data resides in a hardware texture instead of RAM.
            kFlagOtherOwner     = 0x02,     // The Surface data is owned by somebody other than the Surface, and the Surface doesn't free it.
            kFlagDisableAlpha   = 0x04,     // Source alpha blending is used if the source has an alpha channel.
            kFlagIgnoreCompressRLE         = 0x08, // This image did not compress so ignore it for compression -CS Added 1/ 15/09
            kFlagIgnoreCompressYCOCGDXT5   = 0x10, // This image did not compress so ignore it for duplicate compression
            kFlagCompressedRLE             = 0x20, // Set when image was compressed using RLE
            kFlagCompressedYCOCGDXT5       = 0x40  // Set when image was compressed using RLE
        };

        
        // This provides some information on the general surface category that is being allocated
        enum SurfaceCategory
        {
            kSurfaceCategoryDefault,            // Default if no category is known
            kSurfaceCategoryMainView,           // The main view surface
            kSurfaceCategoryImage,              // Image or image buffer    
            kSurfaceCategoryImageCompression,   // This is a scratch surface used by the decompression to unpack to for the draw
            kSurfaceCategoryText,               // A text surface, probably a string of glyphs
            kSurfaceCategoryMovie,              // Movie
            kSurfaceCategoryZoom,               // Zoomed or shrunk surface - probably a scratch surface
            kSurfaceCategoryScratch,            // Temp surface
            kSurfaceCategorySelectDropDown,     // Select dropdown popup
            kSurfaceCategoryTooltip,            // Tooltip popup
            kSurfaceCategoryCanvas,             // For canvas surfaces (not yet actived in EAWebKit - canvas surfaces are set as image surfaces)
            kSurfaceCategoryExternal,           // Not inside EAWebKIt
        };


        struct BlitInfo
        {
            ISurface* mpSource;
            uint8_t* mpSPixels;     // Start of blit data.
            int      mnSWidth;      // Width of blit data.
            int      mnSHeight;     // Height of blit data.
            int      mnSSkip;       // Bytes from end of blit row to beginning of new blit row.

            ISurface* mpDest;
            uint8_t* mpDPixels;
            int      mnDWidth;
            int      mnDHeight;
            int      mnDSkip;
        };

        typedef void (*BlitFunctionType)(const BlitInfo& blitInfo);


        enum DrawFlags
        {
            kDrawFlagIdenticalFormats = 0x01    // The source and dest surfaces of a surface to surface operation are of identical format.
        };

        struct Point
        {
            int x;
            int y;

            Point() { } // By design, don't initialize members.
            Point(int px, int py) : x(px), y(py) { }

            void set(int px, int py) {x = px; y = py;}
        };


        // For now we use a x/y/w/h rect instead of a x0/y0/x1/y1 rect like often seen.
        // This is because WebKit's IntRect class works like that. The problem with 
        // the IntRect class is that it is tedious to use.
        struct Rect
        {
            int x;
            int y;
            int w;
            int h;

            Rect() { }  // By design, don't initialize members.

            Rect(int xNew, int yNew, int wNew, int hNew) 
                : x(xNew), y(yNew), w(wNew), h(hNew) { }

            int width()  const { return w; }
            int height() const { return h; }

            bool isPointInside(const Point& p) const { return (p.x >= x) && (p.x <= x + w) && (p.y >= y) && (p.y <= y + h); }

            void constrainRect(Rect& r) const
            {
                //if r is outside of this rect, set width or height to zero
                if(r.x > x+w || r.x + r.w < x){ r.w = 0;  return;}
                if(r.y > y+h || r.y + r.h < y){ r.h = 0;  return;}
                if(r.x < x){ r.w -= x - r.x; r.x = x;}
                if(r.x + r.w > x + w){r.w -= (r.x + r.w) - (x + w);}
                if(r.y < y){r.h -= y - r.y; r.y = y;}
                if(r.y + r.h > y + h){r.h -= (r.y + r.h) - (y + h);}
            }
        };

        EARASTER_API bool IntersectRect(const Rect& a, const Rect& b, Rect& result);
        EARASTER_API bool ClipForBlit(ISurface* pSource, const Rect* pRectSource, ISurface* pDest, const Rect* pRectDest, Rect& rectSourceResult, Rect& rectDestResult);
        EARASTER_API void IntRectToEARect(const WKAL::IntRect& in, EA::Raster::Rect& out);
        EARASTER_API void EARectToIntRect(const EA::Raster::Rect& in, WKAL::IntRect& out);

        struct Matrix2D
        {
            double m_m11;
            double m_m12;
            double m_m21;
            double m_m22;
            double m_dx;
            double m_dy;
        
            Matrix2D(double a, double b, double c, double d, double e, double f) :
             m_m11(a),m_m12(b),m_m21(c),m_m22(d),m_dx(e),m_dy(f){}
        };

        struct GlyphDrawInfo
        {
            int x1;     // Destination glyph box x left
            int x2;     // Destination glyph box x right
            int y1;     // Destination glyph box y top
            int y2;     // Destination glyph box y bottom

            float u0;   // TL texture coordinate [0,1]
            float v0;

            float u1;   // BR texture coordinate [0,1]
            float v1;
        };

        // Abstract ISurface class that is exportable.  
        class EARASTER_API ISurface
        {
        public:
            virtual ~ISurface(void) {}
    
			virtual bool Set(void *pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData, SurfaceCategory category) = 0;
            virtual bool Resize(int width, int height) = 0;
            
            virtual void SetClipRect(const Rect *pRect) = 0; // NULL means set it to the full image. (0,0,width,height)
			virtual const Rect &GetClipRect(void) = 0;

			virtual void SetUserData(void* p) = 0;
			virtual void *GetUserData(void) = 0;

			virtual void SetCategory(SurfaceCategory category) = 0;
			virtual SurfaceCategory GetCategory(void) = 0;

			virtual void SetPixelFormat(PixelFormatType pft) = 0;
			virtual const PixelFormat& GetPixelFormat(void) = 0;
            
			virtual void GetDimensions(int *widthOut, int *heightOut) = 0;
			virtual size_t GetSizeBytes(void) = 0;
			virtual size_t GetCompressedSizeBytes(void) = 0;

			// This is to support falling back on the software rasterizer from hardware if you have to.
			// This will be slow.
            virtual void Lock(void **dataOut, int *strideOut) = 0;
            virtual void Unlock(void) = 0;

            virtual bool IsAllocated(void) = 0;

            /*!!! DEPRECATED !!!*/
            virtual int GetWidth(void) {
                int width = 0;
                int height = 0;
                GetDimensions(&width, &height);
                return width;
            }
            virtual int GetHeight(void) {
                int width = 0;
                int height = 0;
                GetDimensions(&width, &height);
                return height;
            }
            /*!!! END DEPRECATED !!!*/
		};


        // Surface
        //
        // This class implements a single rectangular 2D pixel surface. 
        // Uses main memory to allocate the buffer
        //
        class EARASTER_API Surface : public ISurface
        {
        public:
            Surface();
            Surface(int width, int height, PixelFormatType pft);
            virtual ~Surface();

			// ISurface
			virtual bool Set(void* pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData, SurfaceCategory category);
			virtual bool Resize(int width, int height);

			virtual void SetClipRect(const Rect* pRect);
			virtual Rect &GetClipRect(void) { return mClipRect; }

			virtual void SetUserData(void* p) { mpUserData = p; } 
			virtual void *GetUserData() { return mpUserData; }

			virtual void SetCategory(SurfaceCategory category) { mCategory = category; }
			virtual SurfaceCategory GetCategory() { return mCategory; };

			virtual void SetPixelFormat(PixelFormatType pft);
			virtual PixelFormat& GetPixelFormat() { return mPixelFormat; }

			virtual void GetDimensions(int *widthOut, int *heightOut);
			virtual size_t GetSizeBytes(void);
			virtual size_t GetCompressedSizeBytes(void) { return (size_t)mCompressedSize; }

			virtual void Lock(void **dataOut, int *strideOut);
			virtual void Unlock(void);

			virtual bool IsAllocated() { return mpData != NULL; }

			// Surface Specific
            virtual bool Set(Surface* pSource);

            virtual int  AddRef(void);
            virtual int  Release(void);

            virtual bool FreeData(void);
            virtual int GetSurfaceFlags() const { return mSurfaceFlags; }
            virtual ISurface* GetBlitDest() const { return mpBlitDest; }
            virtual BlitFunctionType GetBlitFunction() const { return mpBlitFunction; }
            virtual int GetDrawFlags() const { return mDrawFlags; }

            virtual void SetSurfaceFlags(int flags) { mSurfaceFlags = flags; }
            virtual void SetBlitFunction(BlitFunctionType p) { mpBlitFunction = p; }
            virtual void SetDrawFlags(int flags) { mDrawFlags = flags; }
            virtual void SetBlitDest(ISurface* p) { mpBlitDest = p; }

            virtual void SetCompressedSize(int size) { mCompressedSize = size; }

        protected:
            virtual void InitMembers();


        private:
            PixelFormat         mPixelFormat;   // The format of the pixel data.
            int                 mSurfaceFlags;  // See enum SurfaceFlags.
            void*               mpData;         // The pixel data itself.
            int                 mWidth;         // In pixels.
            int                 mHeight;        // In pixels.
            int                 mStride;        // In bytes.
            int                 mLockCount;     // Used if the surface is somewhere other than conventional RAM.
            int                 mRefCount;      // Factory functions return a Surface with a mRefCount of 1.
            void*               mpUserData;     // Arbitrary data the user can associate with this surface. Used to store compressed images if image compression is active. Also used to store the view for scroll bars.
            int                 mCompressedSize; // Size of buffer if compression was used - CS 1/15/09 Added
            // Draw info.
            Rect                mClipRect;      // Drawing is restricted to within this rect.
            ISurface*            mpBlitDest;     // The last surface blitted to. Allows us to cache blit calculations.
            int                 mDrawFlags;     // See enum DrawFlags.
            BlitFunctionType    mpBlitFunction; // The blitting function currently used to blit to mpBlitDest.
            SurfaceCategory     mCategory;      // Surface main category like a main view surface or an image or text...
        };


        ///////////////////////////////////////////////////////////////////////
        // Primitive functions
        //
        // Functions with int return values use 0 for OK and -1 for error.
        //
        // Coordinates are end-inclusive. Thus a line drawn from x1 to x2 includes
        // a point at x2.
        //
        // Note the use of NativeColor vs. Color. NativeColor is a uint32_t
        // which is always in the pixel format of the surface that is using
        // it. This is as opposed to the Color type which is a generic 
        // ARGB color type which may need to be converted to a surface's
        // native type before written to the surface.
        ///////////////////////////////////////////////////////////////////////

		class IEARaster
		{
		public:
			virtual ~IEARaster(void) {}

			// Surface management
			virtual ISurface *CreateSurface(void) = 0;
			virtual ISurface *CreateSurface(int width, int height, PixelFormatType pft, SurfaceCategory category) = 0;
			virtual ISurface *CreateSurface(void* pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData, SurfaceCategory category) = 0;
			virtual void DestroySurface(ISurface* pSurface) = 0;

			// Color conversion
			virtual void ConvertColor(NativeColor colorIn, const PixelFormat &format, Color *colorOut) = 0;

            virtual int Clear(ISurface *pSurface, const Rect &rect, const Color &color) = 0;

			// Rectangle functions
			virtual int RectangleFilled(ISurface *pSurface, const Rect &rect, const Color &color) = 0;
			virtual int RectangleOutlined(ISurface* pSurface, const EA::Raster::Rect& rect, const Color &color) = 0;

			// Line functions
            virtual int Line(ISurface *pSurface, int x1, int y1, int x2, int y2, const Color &color) = 0;

			// Circle / Ellipse
			virtual int EllipseFilled(ISurface* pSurface, int x, int y, int rx, int ry, const Color &color) = 0;
			virtual int EllipseOutlined(ISurface* pSurface, int x, int y, int rx, int ry, const Color &color) = 0;

			// Polygon
			virtual int TriangleOriented(ISurface* pSurface, int x, int y, int size, Orientation o, const Color &color) = 0;
            virtual int PolygonFilled(ISurface* pSurface, const int *vx, const int *vy, int n, const Color &color) = 0;

			// Images
			virtual int DrawSurface(ISurface *pImage, const Rect &sourceRect, ISurface *pDest, const Rect &destRect, const Matrix2D &transform, float alpha) = 0;
			virtual int DrawSurfaceTiled(ISurface *pImage, const Rect &sourceRect, ISurface *pDest, const Rect &destRect, const Rect &clipRect, const Matrix2D &transform, float alpha) = 0;
			virtual int CompressImage(ISurface *pImage, bool hasAlpha, int sizeTotal, int *sizeOut) = 0;
			
            // Text
            virtual int DrawGlyphs(GlyphDrawInfo *glyphs, int glyphCount, EA::WebKit::ITextureInfo* source, ISurface *pDest, const Rect &rectSource, const Rect &rectDest, const Color &color, const Matrix2D &transform, float alpha, EA::WebKit::Effect textEffect) = 0;

            // Util
			virtual bool WriteSurfaceToFile(const char* pPath, ISurface* pSurface, bool bAlphaOnly) = 0;

            /*!!! DEPRECATED !!!*/
            virtual bool WritePPMFile(const char* pPath, ISurface* pSurface, bool bAlphaOnly) {
                return WriteSurfaceToFile(pPath, pSurface, bAlphaOnly);
            }
            /*!!! END DEPRECATED !!!*/
		};

		class EARasterConcrete: public IEARaster
		{
		public:
			virtual ~EARasterConcrete(void) {}

			// Surface management
			virtual ISurface *CreateSurface(void);
			virtual ISurface *CreateSurface(int width, int height, PixelFormatType pft, SurfaceCategory category);
			virtual ISurface *CreateSurface(void* pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData, SurfaceCategory category);
			virtual void DestroySurface(ISurface* pSurface);

			// Color conversion
			virtual void ConvertColor(NativeColor colorIn, const PixelFormat &format, Color *colorOut);

            virtual int Clear(ISurface *pSurface, const Rect &rect, const Color &color);

			// Rectangle functions
			virtual int RectangleFilled(ISurface *pSurface, const Rect &rect, const Color &color);
			virtual int RectangleOutlined(ISurface* pSurface, const EA::Raster::Rect& rect, const Color &c);

			// Line functions
			virtual int Line(ISurface *pSurface, int x1, int x2, int y1, int y2, const Color &c);

			// Circle / Ellipse
			virtual int EllipseFilled(ISurface* pSurface, int x, int y, int rx, int ry, const Color &color);
			virtual int EllipseOutlined(ISurface* pSurface, int x, int y, int rx, int ry, const Color &color);

			// Polygon
			virtual int TriangleOriented(ISurface* pSurface, int x, int y, int size, Orientation o, const Color &color);
			virtual int PolygonFilled(ISurface* pSurface, const int *vx, const int *vy, int n, const Color &color);

			// Images
			virtual int DrawSurface(ISurface *pImage, const Rect &sourceRect, ISurface *pDest, const Rect &destRect, const Matrix2D &transform, float alpha);
			virtual int DrawSurfaceTiled(ISurface *pImage, const Rect &sourceRect, ISurface *pDest, const Rect &destRect, const Rect &clipRect, const Matrix2D &transform, float alpha);
			virtual int CompressImage(ISurface *pImage, bool hasAlpha, int sizeTotal, int *sizeOut);

            // Text
            virtual int DrawGlyphs(GlyphDrawInfo *glyphs, int glyphCount, EA::WebKit::ITextureInfo* source, ISurface *pDest, const Rect &rectSource, const Rect &rectDest, const Color &color, const Matrix2D &transform, float alpha, EA::WebKit::Effect textEffect);

            // Util
			virtual bool WriteSurfaceToFile(const char* pPath, ISurface* pSurface, bool bAlphaOnly);

            virtual bool IntersectRect(const Rect& a, const Rect& b, Rect& result) { return EA::Raster::IntersectRect(a, b, result); }
            virtual bool ClipForBlit(ISurface* pSource, const Rect &rectSource, ISurface* pDest, const Rect &rectDest, Rect *rectSourceResult, Rect *rectDestResult) {
                return EA::Raster::ClipForBlit(pSource, &rectSource, pDest, &rectDest, *rectSourceResult, *rectDestResult);
            }

			private:
				ISurface* GetSurfaceToDraw(ISurface *pSurface, Rect *sourceRectInOut, const Matrix2D &transform, float alpha);
		};
    } // namespace Raster

} // namespace EA



#endif // Header include guard
