#include <math.h>
#include <stdio.h>
#include "dy_types.h"
#include "globs.h"

#define COLOR_EQ(c1,c2)   ((c1.r == c2.r) && (c1.g == c2.g) && (c1.b == c2.b)) 


void message( const char *format, ... );


u32 QuickRand(u32 n)
 {
  Seed*=5;
  Seed++;
  Seed+=Seed>>13;
  return (Seed%n);
 }


u32 colorGammaDifference(u8colorVec *color0,u8colorVec *color1)
 {
  s32 diff,bluediff,greendiff,reddiff;
  s32 r0=Gamma[color0->r & 0xfe];
  s32 g0=Gamma[color0->g & 0xfe];
  s32 b0=Gamma[color0->b & 0xfe];
  s32 r1=Gamma[color1->r & 0xfe];
  s32 g1=Gamma[color1->g & 0xfe];
  s32 b1=Gamma[color1->b & 0xfe];
  
  bluediff=(b0-b1);
  bluediff*=bluediff;
  greendiff=(g0-g1);
  greendiff*=greendiff;
  reddiff=(r0-r1);
  reddiff*=reddiff;
  diff=greendiff*6+reddiff*3+bluediff;
  return diff;
 }


u32 colorDifference(u8colorVec *color0, u8colorVec *color1)
 {
  s32 diff,bluediff,greendiff,reddiff;
  s32 r0=color0->r & 0xfe;
  s32 g0=color0->g & 0xfe;
  s32 b0=color0->b & 0xfe;
  s32 r1=color1->r & 0xfe;
  s32 g1=color1->g & 0xfe;
  s32 b1=color1->b & 0xfe;
  
  bluediff = (b0-b1);
  bluediff *= bluediff;
  greendiff = (g0-g1);
  greendiff *= greendiff;
  reddiff = (r0-r1);
  reddiff *= reddiff;
  diff = greendiff*6+reddiff*3+bluediff;
  return diff;
 }


void findClosestPairInRange(u8 low,u8 high)
 {
  u32 bestDifference;
  u32 i,j;
  
  //set Color0 and Color1 globals
  bestDifference=colorGammaDifference(&Pal[low],&Pal[low+1]);
  Color0=low;
  Color1=low+1; 
  for (i=low;i<high;i++)
   {
    for (j=i+1;j<=high;j++)
     {
      if (colorGammaDifference(&Pal[i],&Pal[j])<bestDifference)
       {
        bestDifference=colorGammaDifference(&Pal[i],&Pal[j]);
        Color0=i;
        Color1=j;
       }
     } 
   }
 }


u8 findClosestGammaMatch(u8colorVec *colorPtr)
 {
  u32 closestIndex;
  u32 closestValue;
  u32 i;
  u32 hash=(((u32)(colorPtr->g)>>1)<<14)+(((u32)(colorPtr->r)>>1)<<7)+
            ((u32)(colorPtr->b)>>1);

  if ((Memory[hash]!=65535)&&(Rendering==1))
    return Memory[hash];

  closestIndex=BaseOffset;
  closestValue=colorGammaDifference(colorPtr,&Pal[BaseOffset]);
  for (i=BaseOffset;i<BaseOffset+TotalColors;i++)
   {
    if (colorGammaDifference(colorPtr,&Pal[i])<closestValue)
     {
      closestIndex=i;
      closestValue=colorGammaDifference(colorPtr,&Pal[i]);
     }
   }
  if (Rendering==1) 
    Memory[hash]=closestIndex; 
  return closestIndex; 
 }

u8 findClosestMatch(u8colorVec *colorPtr)
 {
  u32 closestIndex;
  u32 closestValue;
  u32 i;
  u32 hash=(((u32)(colorPtr->g)>>1)<<14)+(((u32)(colorPtr->r)>>1)<<7)+
            ((u32)(colorPtr->b)>>1);

  if ((Memory[hash]!=65535)&&(Rendering==1))
    return Memory[hash];

  closestIndex=BaseOffset;
  closestValue=colorDifference(colorPtr,&Pal[BaseOffset]);
  for (i=BaseOffset;i<BaseOffset+TotalColors;i++)
   {
    if (colorDifference(colorPtr,&Pal[i])<closestValue)
     {
      closestIndex=i;
      closestValue=colorDifference(colorPtr,&Pal[i]);
     }
   }
  if (Rendering==1) 
    Memory[hash]=closestIndex; 
  return closestIndex; 
 }

 
 
 void markColor(u8colorVec *colorPtr,u32 rangeNumber)
  {
   u32 hash=(((u32)(colorPtr->g)>>1)<<14)+(((u32)(colorPtr->r)>>1)<<7)+
            ((u32)(colorPtr->b)>>1);

   Memory[hash] |= 1<<rangeNumber;   
  }
  
 
 u32 colorInTable(u8colorVec *colorPtr,u32 low,u32 high)
  {
   u32 i;
  
   if (high<low)
    {
     message("Bad parameters to colorInTable: low=%ld,high=%ld\n",low,high);
     return 0;
    }
  
   if (low==high)
     return 0;
  
   for (i=low;i<high;i++)
     if ((colorPtr->r==Pal[i].r)&&(colorPtr->g==Pal[i].g)&&(colorPtr->b==Pal[i].b))
       return 1; 
   return 0;  
  }
 
 
 void addColorToRange(u8colorVec *colorPtr,u8 low,u8 high)
  {
   u32 i;
   u32 dist2;
   
   //first, is there an open slot?
   if (NewRange==0)
    {
     for (i=low;i<=high;i++)
      {
       if (PalSetFlags[i]==0)
        {
         PalSetFlags[i]=1;
         Pal[i]=*colorPtr;
         return;
        }
      }
    }  
   //it's full. find best pair
   if (NewRange==0)
    {
     findClosestPairInRange(low,high);
     MostRecentDist=colorGammaDifference(&Pal[Color0],&Pal[Color1]);
     NewRange=1;
    } 
   //find closest match between color I want to insert and palette
   dist2=colorGammaDifference(&Pal[findClosestGammaMatch(colorPtr)],colorPtr);
   //if two colors in palette are close, merge & insert
   if (MostRecentDist<dist2)
    {
     //new
     Pal[Color0].r=((u32)Pal[Color0].r+Pal[Color1].r)/2;
     Pal[Color0].g=((u32)Pal[Color0].g+Pal[Color1].g)/2;
     Pal[Color0].b=((u32)Pal[Color0].b+Pal[Color1].b)/2;
     //end of new
     Pal[Color1]=*colorPtr;
     NewRange=0;
    } 
  }
 

void scanForColors(void)
 {
  u32 x,y;
  u32 rangeNumber;
  u32 areaX,areaY;
  u32 yLine;
  
  Pointer=Data;
 
  //16 screen segments
  //message("Scanning image data.\n");  
  rangeNumber=0;
  for (areaY=0;areaY<4;areaY++)
   {
    for (areaX=0;areaX<4;areaX++)
     {
      for (y=areaY*(Height/4);y<areaY*(Height/4)+(Height/4);y++)
       {
        for (x=areaX*(Width/4);x<areaX*(Width/4)+(Width/4);x++)
         {
          u8colorVec color;
      
          color.b=Pointer[(x*3+y*PaddedWidth)];    // RST
          color.g=Pointer[(x*3+y*PaddedWidth)+1];  // RST
          color.r=Pointer[(x*3+y*PaddedWidth)+2];  // RST

          if (IgnoreColorOn) {
            if (COLOR_EQ(color, IgnoreColor))
               continue;
          }

          markColor(&color,rangeNumber);            
         }
       }
      rangeNumber++;
     }
   }
 }


void chooseColors(void)
 {
  u32 r,g,b;
  u32 hash;
  u8colorVec color;
  u32 rangeNumber;
  
  message("Choosing Base Palette\n");
  //first, full screen
  NewRange=0;
  for (g=0;g<128;g++)
   {
    u32 gHash=g<<14;
    if ((g&15)==0)
     {
      message("Choosing...%ld%% done.  ",g*100/127);
      fflush(stdout);
      message("\r");
     } 
    for (r=0;r<128;r++)
     {
      u32 rHash=gHash+(r<<7);
      for (b=0;b<128;b++)
       {
        hash=rHash+b;
        if (Memory[hash]!=0)
         {
          color.r=r+r;
          color.g=g+g;
          color.b=b+b;
          addColorToRange(&color,(u8)(ColorRange[16]),(u8)(BaseOffset+TotalColors-1));
         }    
       }
     }
   }   
  //now the 16 segments
  message("\n");
  for (rangeNumber=0;rangeNumber<16;rangeNumber++)
   {
    u16 bit=1<<rangeNumber;

    NewRange=0;
    message("Scanning region %ld",rangeNumber);
    fflush(stdout);
    message("\r");
    if (ColorRange[rangeNumber]<ColorRange[rangeNumber+1])
     { 
      for (g=0;g<128;g++)
       {
        u32 gHash=g<<14;
        for (r=0;r<128;r++)
         {
          u32 rHash=gHash+(r<<7);
          for (b=0;b<128;b++)
           {
            hash=rHash+b;
            if ((Memory[hash]&bit)!=0)
             {
              color.r=r+r;
              color.g=g+g;
              color.b=b+b;
              if ((color.b>BlackThreshold)||(color.r*3>BlackThreshold)||(color.g*6>BlackThreshold))
                if ((colorInTable(&color,BaseOffset,ColorRange[rangeNumber])==0)&&
                    (colorInTable(&color,ColorRange[16],BaseOffset+TotalColors)==0))
                  addColorToRange(&color,(u8)(ColorRange[rangeNumber]),(u8)(ColorRange[rangeNumber+1]-1));
             }
           } 
         }   
       }  
     }    
   }        
 }


u8 findShould(u8colorVec *color,u8 would,u32 x,u32 y)
 {
  //new
  u8 should;
  s32 index;
  s32 scaledR,scaledG,scaledB,i;
  s32 R,G,B;
  u32 red,green,blue;
  s32 rMove,bMove,gMove;
  
  red=color->r;
  green=color->g;
  blue=color->b;
    
  if (((Pal[would].r==red)&&(Pal[would].g==green)&&(Pal[would].b==blue))&&
      (ExactMatchPassThrough==1))
   {
    should=would;
    prevErrorRed=prevErrorGreen=prevErrorBlue=0;
   } 
  else
   {  
    if (Method==0)
      index=((x&1)*2+(y&1));
    if (Method==1)  
      index=QuickRand(4);
    if (Method==2)
     {
      rMove=(QuickRand(3)-1)*3;
      bMove=(QuickRand(3)-1)*6;
      gMove=(QuickRand(3)-1);
     }
    if (Method==3)
     {
      u8colorVec tempColor;
      s32 r,g,b;

      /* new code starts */
      if ((prevRed==red)&&(prevBlue==blue)&&(prevGreen==green))
         prevErrorRed=prevErrorGreen=prevErrorBlue=0;
   
      prevRed=red; prevBlue=blue; prevGreen=green; 
      /* new code ends */    

      r=red+prevErrorRed*75/100;
      g=green+prevErrorGreen*75/100;
      b=blue+prevErrorBlue*75/100;
       
      if (r>255) r=255;
      if (r<0) r=0;
      if (g>255) g=255;
      if (g<0) g=0;
      if (b>255) b=255;
      if (b<0) b=0;

      tempColor.r=r;
      tempColor.g=g;
      tempColor.b=b;
      
      should=findClosestMatch(&tempColor);
      
      if (should!=would)
        prevErrorRed=prevErrorGreen=prevErrorBlue=0;
      
      if ((abs(Pal[should].r-red)*3)+(abs(Pal[should].g-green)*6)+
          (abs(Pal[should].b-blue))>192)
       {
        should=would;
        prevErrorRed=prevErrorGreen=prevErrorBlue=0;
       }   
      
      prevErrorRed+=red-Pal[should].r;
      prevErrorGreen+=green-Pal[should].g;
      prevErrorBlue+=blue-Pal[should].b;

      #if 0
      if (((x*x-y*y)&63)==0)
        prevErrorRed=prevErrorGreen=prevErrorBlue=0;
      #else
      // 12/19/96 DaveM: get rid of triangle
      if (x==0)
        prevErrorRed=prevErrorGreen=prevErrorBlue=0;
      #endif

      return should;
      //end of new
     } 
		     	    
 
    for (i=MinDither;i<=MaxDither;i++)
     {
      scaledR=red*6;
      scaledG=green*6;
      scaledB=blue*6;
      
      if (Method<2)
       {
	    switch (index)
	     {
	      case 0:
	        scaledR=red*6+2*i;
	        scaledG=green*6-i;
		    break;
		  case 1:
		    scaledG=green*6+i;
		    scaledB=blue*6-6*i;
		    break;
		  case 2:
		    scaledB=blue*6+6*i;
		    scaledR=red*6-2*i;
		    break;    
		 }
       }
      if (Method==2)
       {
        scaledR=red*6+rMove*i;
        scaledG=green*6+gMove*i;
        scaledB=blue*6+bMove*i;
       }
	  R=scaledR/6;
	  G=scaledG/6;
	  B=scaledB/6;
	      
	  if (R>255) R=255;
	  if (R<0) R=0;
	  if (G>255) G=255;
	  if (G<0) G=0;
	  if (B>255) B=255;
	  if (B<0) B=0;          
	    
	  color->r=R;
	  color->g=G;
	  color->b=B;
	      
	  should=findClosestMatch(color);
	  if (should!=would)
	    break;
     }
   }
  return should;     	    
 }
 
//new 
void markColor2(u8colorVec *colorPtr,f32 change )
 {
  u32 hash=(((u32)(colorPtr->g)>>2)<<12)+(((u32)(colorPtr->r)>>2)<<6)+
            ((u32)(colorPtr->b)>>2);
  u32 mask=(62<<12)+(62<<6)+62;

  hash&=mask;  
  Memory2[hash]+=change;
 } 
 
 
void popularityScan(void)
 {
  u32 x,y;
 
  Pointer=Data;
 
  //message("PopularityScan\n"); 
  for (x=0;x<Width;x++)
   {
    for (y=0;y<Height;y++)
     {
      u8colorVec color, color2, color3;
      
      color.b=Pointer[(x*3+y*PaddedWidth)];   // RST
      color.g=Pointer[(x*3+y*PaddedWidth)+1]; // RST
      color.r=Pointer[(x*3+y*PaddedWidth)+2]; // RST

      if (IgnoreColorOn) {
         if (COLOR_EQ(color, IgnoreColor))
            continue;
      }

      if (zeroColorOn) {
         if (COLOR_EQ(color, zeroColor))
            continue;
      }

      markColor2(&color, 1.0);
      if (x+2<Width-1)
       {
        color2.b=Pointer[((x+2)*3+y*PaddedWidth)];
        color2.g=Pointer[((x+2)*3+y*PaddedWidth)+1];
        color2.r=Pointer[((x+2)*3+y*PaddedWidth)+2];
        if (colorDifference(&color,&color2)<512)
          markColor2(&color,10.0);
       }

      if (y+2<Height-1)
       {
        color3.b=Pointer[(x*3+(y+2)*PaddedWidth)];
        color3.g=Pointer[(x*3+(y+2)*PaddedWidth)+1];
        color3.r=Pointer[(x*3+(y+2)*PaddedWidth)+2];
        if (colorDifference(&color,&color3)<512)
          markColor2(&color,10.0);
       }
     }
   }
 }

void kill(s32 red,s32 green,s32 blue)
 {
  s32 r,g,b;
  u32 newHash;
 
  green>>=2;
  red>>=2;
  blue>>=2;
  newHash=(green<<12)+(red<<6)+blue;
  Memory2[newHash]=(f32)(0);      
  for (g=-1;g<2;g++)
    for (r=-3;r<4;r++)
      for (b=-8;b<9;b++)
       {
        if (red+r>63)
          continue;
        if (red+r<0)
          continue;
        if (green+g>63)
          continue;
        if (green+g<0)
          continue;
        if (blue+b>63)
          continue;
        if (blue+b<0)
          continue;      
        newHash=((green+g)<<12)+((red+r)<<6)+(blue+b);
        //Memory2[newHash]=(Memory2[newHash]+1)>>1;
        Memory2[newHash] *= (f32)(0.5);
       }
 }


void popularityChoose(void)
 {
  u32 hash;
  u32 i,g,r,b;
  
  message("\nPopularityChoose\n");
  for (i=BaseOffset;i<BaseOffset+TotalColors;i++)
   {
    f32 best=(f32)(0);
    u32 bestR,bestB,bestG;
    f32 measure;
    
    for (g=0;g<64;g++)
     {
      for (r=0;r<64;r++)
       {
        for (b=0;b<64;b++)
         {
          hash=(g<<12)+(r<<6)+b;
          measure=(f32)(0);
          if (Memory2[hash]!=0)
            measure=(f32)(Memory2[hash] +
            (Memory2[hash]*(Gamma[g*4]*6+Gamma[r*4]*3+Gamma[b*4]))/5000);
          if (measure>best)
           {
            best=measure;
            bestR=r;bestG=g;bestB=b;
           }   
         }
       }
     }
    Pal[i].r=bestR<<2;
    Pal[i].g=bestG<<2;
    Pal[i].b=bestB<<2;
    kill(Pal[i].r,Pal[i].g,Pal[i].b);
    if ((i&7)==0)
     {
      message("Choosing palette entry %ld",i);
      fflush(stdout);
      message("\r");
     } 
   }
   message("\n");
 }
//end of new  
