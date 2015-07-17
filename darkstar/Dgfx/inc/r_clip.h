//================================================================
//   
// $Workfile:   r_clip.h  $
// $Version$
// $Revision:   1.1  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _R_CLIP_H_
#define _R_CLIP_H_

#include <ml.h>
#include <d_defs.h>

struct GFXRasterList;

extern Bool rectClip(const Point2I *in_pt, const RectI *in_clip );
extern Bool rectClip(GFXRasterList *in_rastList, RectI *in_clipRect, RectI *io_lastRect );
extern Bool rectClip(Point2I *io_pt1, Point2I *io_pt2, const RectI *in_clip );
extern Bool rectClip(RectI *io_rect, const RectI *in_clip );
extern Bool rectClip(RectI *io_src, RectI *io_dest, const RectI *in_clip);
extern Bool rectClip(RectI *io_src, const RectI *in_srcClip, RectI *io_dest, const RectI *in_destClip);
extern Bool rectClipScaled(RectI *io_src, RectI *io_dest, const RectI *in_clip, Int32 *xAdj, Int32 *out_yAdj, GFXFlipFlag in_flip);
extern Bool rectClipScaled(RectI *io_src, const RectI *in_srcClip, RectI *io_dest, const RectI *in_dstClip, Int32 *out_xAdj, Int32 *out_yAdj);
extern void rectUnion(const RectI *in_a, const RectI *in_b, RectI *out);
extern BOOL validateRect(const RectI *in_rect, const RectI *in_clipRect);

#endif
