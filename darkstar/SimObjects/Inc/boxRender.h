//
// boxRender.h
//

#ifndef _BOXRENDER_H_
#define _BOXRENDER_H_

class BoxRenderImage : public SimRenderImage
{
   public:
      TMat3F transform;
	   Box3F box;
   	BoxRenderImage() { itype = Normal; }
      void render(TSRenderContext &rc);
};

#endif