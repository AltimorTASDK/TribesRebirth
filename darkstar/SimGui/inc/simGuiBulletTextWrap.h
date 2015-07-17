#ifndef _H_SIMGUIBULLETTEXTWRAP
#define _H_SIMGUIBULLETTEXTWRAP

namespace SimGui
{
class TextWrap;
class BulletTextWrap : public TextWrap
{
   private :
      typedef TextWrap Parent;

      enum TYPES
      {
         BULLET_TYPE_CIRCLE = 0,
         BULLET_TYPE_SQUARE,
         NUM_TYPES
      };

      Int32 iBulletSize, iBulletSpacing, iBulletType;

   public :
       BulletTextWrap();
      ~BulletTextWrap();

      Point2I getExtent();
      void    setBulletSize(int);
      void    setBulletSpacing(int);
      Int32   getBulletSize();
      Int32   getBulletSpacing();
      void    onRender(GFXSurface *, Point2I, const Box2I &);
      void    inspectRead (Inspect *);
      void    inspectWrite(Inspect *);

      DECLARE_PERSISTENT(BulletTextWrap);

      Persistent::Base::Error read (StreamIO &, int, int);
      Persistent::Base::Error write(StreamIO &, int, int);
};
};

#endif