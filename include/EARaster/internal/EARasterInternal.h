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
// EARasterUtils.h
// By Yee Cheng Chin
///////////////////////////////////////////////////////////////////////////////

#ifndef EARASTER_EARASTERINTERNAL_H
#define EARASTER_EARASTERINTERNAL_H

#if EAWEBKIT_THROW_BUILD_ERROR
#error This file should be included only in a dll build
#endif

namespace EA { namespace Raster {
	// Surface management
	EARASTER_API ISurface*   CreateSurface();
	EARASTER_API ISurface*   CreateSurface(int width, int height, PixelFormatType pft,SurfaceCategory category);
	EARASTER_API ISurface*   CreateSurface(void* pData, int width, int height, int stride, PixelFormatType pft, bool bCopyData, bool bTakeOwnership,SurfaceCategory category);
	EARASTER_API void        DestroySurface(ISurface* pSurface);

	// Color conversion
	EARASTER_API void ConvertColor(NativeColor c, const PixelFormat& pf, Color *colorOut);

	// Pixel functions
	EARASTER_API void  GetPixel                (ISurface* pSurface, int x, int y, Color& color);
	EARASTER_API int   SetPixelSolidColor      (ISurface* pSurface, int x, int y, const Color& color);
	EARASTER_API int   SetPixelSolidColorNoClip(ISurface* pSurface, int x, int y, const Color& color);
	EARASTER_API int   SetPixelColor           (ISurface* pSurface, int x, int y, const Color& color);
	EARASTER_API int   SetPixelColorNoClip     (ISurface* pSurface, int x, int y, const Color& color);
	EARASTER_API int   SetPixelRGBA            (ISurface* pSurface, int x, int y, int r, int g, int b, int a);
	EARASTER_API int   SetPixelRGBANoClip      (ISurface* pSurface, int x, int y, int r, int g, int b, int a);

	// Rectangle functions
	EARASTER_API int   FillRectSolidColor      (ISurface* pSurface, const Rect* pRect, const Color& color);
	EARASTER_API int   FillRectColor           (ISurface* pSurface, const Rect* pRect, const Color& color);
	EARASTER_API int   RectangleColor          (ISurface* pSurface, int x1, int y1, int x2, int y2, const Color& color);
	EARASTER_API int   RectangleColor          (ISurface* pSurface, const EA::Raster::Rect& rect, const Color& c);
	EARASTER_API int   RectangleRGBA           (ISurface* pSurface, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

	//EARASTER_API int   BoxColor                (ISurface* pSurface, int x1, int y1, int x2, int y2, const Color& color);
	//EARASTER_API int   BoxRGBA                 (ISurface* pSurface, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

	// Line functions
	EARASTER_API int   HLineSolidColor(ISurface* pSurface, int x1, int x2, int  y, const Color& color);
	EARASTER_API int   HLineColor     (ISurface* pSurface, int x1, int x2, int  y, const Color& color);
	EARASTER_API int   VLineSolidColor(ISurface* pSurface, int  x, int y1, int y2, const Color& color);
	EARASTER_API int   VLineColor     (ISurface* pSurface, int  x, int y1, int y2, const Color& color);
	EARASTER_API int   LineColor      (ISurface* pSurface, int x1, int y1, int x2, int y2, const Color& color);
	EARASTER_API int   LineRGBA       (ISurface* pSurface, int x1, int y1, int x2, int y2, int r, int g, int b, int a);
	EARASTER_API int   AALineRGBA     (ISurface* pSurface, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

	// Circle / Ellipse
	EARASTER_API int   CircleColor       (ISurface* pSurface, int x, int y, int radius, const Color& color);
	EARASTER_API int   CircleRGBA        (ISurface* pSurface, int x, int y, int radius, int r, int g, int b, int a);
	EARASTER_API int   EllipseColor      (ISurface* pSurface, int x, int y, int rx, int ry, const Color& color);
	EARASTER_API int   EllipseRGBA       (ISurface* pSurface, int x, int y, int rx, int ry, int r, int g, int b, int a);
	EARASTER_API int   AAEllipseColor    (ISurface* pSurface, int xc, int yc, int rx, int ry, const Color& color);
	EARASTER_API int   FilledEllipseColor(ISurface* pSurface, int x, int y, int rx, int ry, const Color& color);
	EARASTER_API int   FilledEllipseRGBA (ISurface* pSurface, int x, int y, int rx, int ry, int r, int g, int b, int a);


	// Polygon
	EARASTER_API int   SimpleTriangle      (ISurface* pSurface, int  x, int  y, int size, Orientation o, const Color& color);
	EARASTER_API int   PolygonColor        (ISurface* pSurface, const int* vx, const int* vy, int n, const Color& color);
	EARASTER_API int   PolygonRGBA         (ISurface* pSurface, const int* vx, const int* vy, int n, int r, int g, int b, int a);
	EARASTER_API int   AAPolygonColor      (ISurface* pSurface, const int* vx, const int* vy, int n, const Color& color);
	EARASTER_API int   AAPolygonRGBA       (ISurface* pSurface, const int* vx, const int* vy, int n, int r, int g, int b, int a);
	EARASTER_API int   FilledPolygonColor  (ISurface* pSurface, const int* vx, const int* vy, int n, const Color& color);
	EARASTER_API int   FilledPolygonColorMT(ISurface* pSurface, const int* vx, const int* vy, int n, const Color& color, int** polyInts, int* polyAllocated);
	EARASTER_API int   FilledPolygonRGBAMT (ISurface* pSurface, const int* vx, const int* vy, int n, int r, int g, int b, int a, int** polyInts, int* polyAllocated);
	EARASTER_API int   FilledPolygonRGBA   (ISurface* pSurface, const int* vx, const int* vy, int n, int r, int g, int b, int a);


#if UNUSED_IRASTER_CALLS_ENABLED 
	// Line
	EARASTER_API int   HLineSolidRGBA (ISurface* pSurface, int x1, int x2, int  y, int r, int g, int b, int a);
	EARASTER_API int   HLineRGBA      (ISurface* pSurface, int x1, int x2, int  y, int r, int g, int b, int a);
	EARASTER_API int   VLineSolidRGBA (ISurface* pSurface, int  x, int y1, int y2, int r, int g, int b, int a);
	EARASTER_API int   VLineRGBA      (ISurface* pSurface, int  x, int y1, int y2, int r, int g, int b, int a);
	EARASTER_API int   AALineColor    (ISurface* pSurface, int x1, int y1, int x2, int y2, const Color& color, bool bDrawEndpoint);
	EARASTER_API int   AALineColor    (ISurface* pSurface, int x1, int y1, int x2, int y2, const Color& color);

	// Circle / Ellipse
	EARASTER_API int   ArcColor          (ISurface* pSurface, int x, int y, int r, int start, int end, const Color& color);
	EARASTER_API int   ArcRGBA           (ISurface* pSurface, int x, int y, int radius, int start, int end, int r, int g, int b, int a);
	EARASTER_API int   AACircleColor     (ISurface* pSurface, int x, int y, int r, const Color& color);
	EARASTER_API int   AACircleRGBA      (ISurface* pSurface, int x, int y, int radius, int r, int g, int b, int a);
	EARASTER_API int   FilledCircleRGBA  (ISurface* pSurface, int x, int y, int radius, int r, int g, int b, int a);
	EARASTER_API int   FilledCircleColor (ISurface* pSurface, int x, int y, int r, const Color& color);
	EARASTER_API int   AAEllipseRGBA     (ISurface* pSurface, int x, int y, int rx, int ry, int r, int g, int b, int a);
	EARASTER_API int   PieColor          (ISurface* pSurface, int x, int y, int radius, int start, int end, const Color& color);
	EARASTER_API int   PieRGBA           (ISurface* pSurface, int x, int y, int radius,  int start, int end, int r, int g, int b, int a);
	EARASTER_API int   FilledPieColor    (ISurface* pSurface, int x, int y, int radius, int start, int end, const Color& color);
	EARASTER_API int   FilledPieRGBA     (ISurface* pSurface, int x, int y, int radius, int start, int end, int r, int g, int b, int a);
	// Polygon
	EARASTER_API int   TrigonColor         (ISurface* pSurface, int x1, int y1, int x2, int y2, int x3, int y3, const Color& color);
	EARASTER_API int   TrigonRGBA          (ISurface* pSurface, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a);
	EARASTER_API int   AATrigonColor       (ISurface* pSurface, int x1, int y1, int x2, int y2, int x3, int y3, const Color& color);
	EARASTER_API int   AATrigonRGBA        (ISurface* pSurface, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a);
	EARASTER_API int   FilledTrigonColor   (ISurface* pSurface, int x1, int y1, int x2, int y2, int x3, int y3, const Color& color);
	EARASTER_API int   FilledTrigonRGBA    (ISurface* pSurface, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a);
	EARASTER_API int   TexturedPolygon     (ISurface* pSurface, const int* vx, const int* vy, int n, ISurface* pTexture,int texture_dx,int texture_dy);
	EARASTER_API int   TexturedPolygonMT   (ISurface* pSurface, const int* vx, const int* vy, int n, ISurface* pTexture, int texture_dx, int texture_dy, int** polyInts, int* polyAllocated);
#endif // UNUSED_IRASTER_CALLS_ENABLED

	///////////////////////////////////////////////////////////////////////
	// Resampling
	///////////////////////////////////////////////////////////////////////

	// Zoom
	// Zooms a 32bit or 8bit 'src' surface to newly created 'dst' surface.
	// 'zoomx' and 'zoomy' are scaling factors for width and height. If 'smooth' is 1
	// then the destination 32bit surface is anti-aliased. If the surface is not 8bit
	// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.
	// zoomX/zoomY can be less than 1.0 for shrinking.
	EARASTER_API ISurface* ZoomSurface(ISurface* pSurface, double zoomx, double zoomy, bool bSmooth);

	// Returns the size of the target surface for a zoomSurface() call
	EARASTER_API void ZoomSurfaceSize(int width, int height, double zoomx, double zoomy, int* dstwidth, int* dstheight);

	// Shrinks a 32bit or 8bit surface to a newly created surface.
	// 'factorX' and 'factorY' are the shrinking ratios (i.e. 2 = 1/2 the size,
	// 3 = 1/3 the size, etc.) The destination surface is antialiased by averaging
	// the source box RGBA or Y information. If the surface is not 8bit
	// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.
	EARASTER_API ISurface* ShrinkSurface(ISurface* pSurface, int factorX, int factorY);

	// Returns a rotated surface by 90, 180, or 270 degrees.
	EARASTER_API ISurface* RotateSurface90Degrees(ISurface* pSurface, int nClockwiseTurns);

	// Apply a transform to surface .
	EARASTER_API ISurface* TransformSurface(ISurface* pSurface, Rect& scrRect, const Matrix2D& matrix);


	// Creates a surface that is the same as pSource but with surfaceAlpha multiplied into pSource.
	EARASTER_API ISurface* CreateTransparentSurface(ISurface* pSource, int surfaceAlpha);


	///////////////////////////////////////////////////////////////////////
	// Blit functions
	///////////////////////////////////////////////////////////////////////

	// Generates rectSourceResult and rectDestResult from source and dest
	// surfaces and unclipped rectangles.
	EARASTER_API bool ClipForBlit(ISurface* pSource, const Rect* pRectSource, ISurface* pDest, const Rect* pRectDest, Rect& rectSourceResult, Rect& rectDestResult);

	// Does a 1:1 blit from pRectSource in pSource to pRectDest in pDest. 
	// Handles the case whereby pRectSource and pRectDest may refer to out of bounds of 
	// pSource and pDest, respectively.
	// If pDestClipRect is non-NULL, the output is further clipped to pDestClipRect.
	// pDestClipRect is not quite the same as pRectDest, as it's sometimes 
	// useful to blit a source rect to a dest rect but have it clip to another rect.
	// Returns 0 if OK or a negative error code.
	EARASTER_API int Blit(Surface* pSource, const Rect* pRectSource, ISurface* pDest, const Rect* pRectDest, const Rect* pDestClipRect = NULL);

	// Does a 1:1 blit from pSource to pDest with the assumption that pRectSource and 
	// pRectDest are already clipped to pSource and pDest, respectively.
	// Returns 0 if OK or a negative error code.
	EARASTER_API int BlitNoClip(Surface* pSource, const Rect* pRectSource, ISurface* pDest, const Rect* pRectDest);

	//////////////////////////////////////////////////////////////////////////
	/// Blit a repeating pattern.
	/// The offsetX/Y position is the location within pRectDest that 
	/// the source origin will be. The blit is clipped to within pRectDest.
	/// If pRectSource is NULL then the entire Source is used, else the part
	/// of pSource that is tiled into pDest is the defined by pRectSource.
	EARASTER_API int BlitTiled(Surface* pSource, const Rect* pRectSource, ISurface* pDest, const Rect* pRectDest, int offsetX, int offsetY);

	//////////////////////////////////////////////////////////////////////////
	/// This function stretches an image over a rectangular region
	/// by repeating (tiling) the center portion of the image.
	/// This is useful for drawing arbitrarily sized GUI buttons and
	/// scrollbar parts.
	///
	/// The image is divided like so:
	/// +-----------+-----------+-----------+
	/// | TL Corner |   Top     | TR Corner |
	/// |           |   Edge    |           |
	/// +-----------+-----------+-----------+
	/// | Left      |   Center  | Right     |
	/// | Edge      |           | Edge      |
	/// +-----------+-----------+-----------+
	/// | BL Corner |   Bottom  | BR Corner |
	/// |           |   Edge    |           |
	/// +-----------+-----------+-----------+
	/// The center portions will be tiled to cover the destination
	/// rectangle, where the edge portions will only be blitted
	/// once.
	///
	/// Parameters:
	///   pDest - the target drawing context.
	///   pRectDest - the destination rectangle to fill
	///   pImage - the source image to render.
	///   pRectSource - the source rectangle which has nine parts.
	///   pRectSourceCenter - Defines where the dividing lines between center
	///       and edges are (For example, mLeft controls the position
	///       of the dividing line between left edge and center.)
	EARASTER_API int BlitEdgeTiled(Surface* pSource, const Rect* pRectSource, ISurface* pDest, const Rect* pRectDest, const Rect* pRectSourceCenter);

	// Sets up the blit function needed to blit pSource to pDest.
	// Normally you don't need to call this function, as the Surface class and Blit 
	// functions will do it automatically.
	EARASTER_API bool SetupBlitFunction(Surface* pSource, ISurface* pDest);



	///////////////////////////////////////////////////////////////////////
	// Utility functions
	///////////////////////////////////////////////////////////////////////

	// A PPM file is a simple bitmap format which many picture viewers can read.
	EARASTER_API bool WritePPMFile(const char* pPath, ISurface* pSurface, bool bAlphaOnly);
}}

#endif
