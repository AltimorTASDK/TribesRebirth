//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _RN_MANAG_H_
#define _RN_MANAG_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

enum renderFlag
{
	renderInterpS = 	   0x0001,
	renderInterpUV = 	   0x0002,
	renderInterpW = 	   0x0004,
	renderInterpH =      0x0008,
	renderConstantS =	   0x0010,
	renderConstantA =	   0x0020,
	renderTransparent =  0x0040,
	renderWrapped = 	   0x0080,
};


void __cdecl badRendererCalled();
int  __cdecl badDeltaCalcCalled(void*);

class rendererManager
{
   bool m_ignoreBadRenderers;

public:
	typedef void (__cdecl *renderSpanFunc)();
	typedef int (__cdecl *deltaCalcFunc)(void *);
private:
	int flags;
	renderSpanFunc spanRenderTable[256];
	deltaCalcFunc deltaCalcTriTable[16];
	deltaCalcFunc deltaCalcPolyTable[16];
	void installFunction(int flags, renderSpanFunc rsf) { spanRenderTable[flags] = rsf; }
	void installTriDeltaFunction(int flags, deltaCalcFunc fn) {deltaCalcTriTable[flags] = fn; }
	void installPolyDeltaFunction(int flags, deltaCalcFunc fn) {deltaCalcPolyTable[flags] = fn; }
public:
	rendererManager();

   void setIgnoreBadRenderers(const bool in_ignore) { m_ignoreBadRenderers = in_ignore; }
   
	inline bool isTransparent() {return ((flags & (renderConstantA | renderTransparent)) != 0); }
	inline void setRenderFlag(renderFlag f) { flags |= f; }
	inline void clearRenderFlag(renderFlag f) { flags &= ~f; }
   inline bool needsShadeMap() { return flags & (renderConstantA | renderInterpS | renderInterpH | renderConstantS); }

	inline renderSpanFunc getRenderer()
   {
      AssertFatal(m_ignoreBadRenderers == true ||
                  spanRenderTable[flags] != badRendererCalled,
                  avar("Error, no renderer installed for those flags: %x", flags));
      return spanRenderTable[flags];
   }
	inline int calcDelta(void *calcStruct, Bool isTriangle)
	{
		if(isTriangle)
		{
			AssertFatal(deltaCalcTriTable[flags & 0xF] != badDeltaCalcCalled,
                     avar("No tri delta function for current rendering ops: %x", (flags&0xf)));
			return deltaCalcTriTable[flags & 0xF](calcStruct);
		}
		else
		{
			AssertFatal(deltaCalcPolyTable[flags & 0xF] != badDeltaCalcCalled,
                     avar("No poly delta function for current rendering ops: %x", (flags&0xf)));
			return deltaCalcPolyTable[flags & 0xF](calcStruct);
		}
	}
};

extern rendererManager g_renderTable;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_RN_MANAG_H_
