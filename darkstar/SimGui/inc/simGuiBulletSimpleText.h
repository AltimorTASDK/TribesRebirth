#ifndef _H_SIMGUIBULLETSIMPLETEXT
#define _H_SIMGUIBULLETSIMPLETEXT

namespace SimGui
{
class SimpleText;
class BulletSimpleText : public SimpleText
{
   private :
      typedef SimpleText Parent;

      enum TYPES
      {
         BULLET_TYPE_CIRCLE = 0,
         BULLET_TYPE_SQUARE,
         NUM_TYPES
      };

      Int32 iBulletSize, iBulletSpacing, iBulletType;

   public :
       BulletSimpleText();
      ~BulletSimpleText();

      Point2I getExtent();
      void    setBulletSize(int);
      void    setBulletSpacing(int);
      Int32   getBulletSize();
      Int32   getBulletSpacing();
      void    onRender(GFXSurface *, Point2I, const Box2I &);
      void    inspectRead (Inspect *);
      void    inspectWrite(Inspect *);

      DECLARE_PERSISTENT(BulletSimpleText);

      Persistent::Base::Error read (StreamIO &, int, int);
      Persistent::Base::Error write(StreamIO &, int, int);
};
};

#endif