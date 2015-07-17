10.??.96

        Math:
        -----

        Removed fixed point trig and matrices.

        Added quaternions.

        Added some new methods to matrix classes.

        Added interpolation methods to rotations, points.

        Base:
        -----

        Added interpolation methods to color types.
        
        Added binary search algorithm for sorted vectors.

        Modified vector template to allow creating a vector from a read-only
        buffer.

7.24.96
        ALL:
        ---------------------------------------------------------------------

        Note: MSVC version now built with version 4.2.
        
        Calling Conventions
        -------------------

        all library functions now have explicitly specified calling 
        conventions, as defined by COREAPI.  This is defined as __fastcall
        for the release version and __cdecl for the debug version.
        Constructors and destructors are still built according to the default 
        calling convention, which is also __fastcall for release and __cdecl
        for debug.  Except with MSVC++, which always uses __thiscall for
        constructors and destructors.

        What does this mean?  
        
        1) you must (possibly) make changes to your code: if you override
           any virtual functions in classes derived from library classes,
           you must include COREAPI in their declarations.

        2) you may now use the default calling convention of your choice.
           unless you are unfortunate enough to be using borland.

        3) debug builds, with the __cdecl calling convention, should permit
           improved opportunities for examining the call stack.

        Old Math Types
        --------------

        Several old math types have been eliminated.  Actually, they were
        just typedefs of other existing types, which are still there.  You
        will need to change your code if you use any of these types:

        Angle       -->   AngleX        Rect        -->   RectI     
        Euler       -->   EulerX        LineSeg2    -->   LineSeg2I
        Point2      -->   Point2I       LineSeg3    -->   LineSeg3I
        Point2f     -->   Point2X       LineSeg     -->   LineSeg3 
        Point3      -->   Point3I       Circle      -->   CircleI  
        Point3f     -->   Point3X       Sphere      -->   SphereI  
        Point4      -->   Point4I       PtList2     -->   PtList2I 
        Vector3     -->   Point3        PtList3     -->   PtList3I 
        RMat3       -->   RMat3X        Poly2       -->   Poly2I   
        TMat3       -->   TMat3X        Poly3       -->   Poly3I   
        Box2        -->   Box2I         Plane       -->   PlaneI   
        Box3        -->   Box3I     
     
        Structure Alignment
        -------------------

        pragmas have been added to ensure correct alignment of structures
        regardless of the default alignment.  this means you may set the
        alignment as you like in your makefile and the libraries should
        still work.  watch out if you are using borland and using these
        pragmas yourself, as borland does not support nested packing pragmas.

7.17.96

        BASE:
        ---------------------------------------------------------------------

        changed to work with Symantec
        added Vector template class

        ML:
        ---------------------------------------------------------------------

        added missing include <m_dist.h> to m_sphere.h
        added method to circle/sphere class to adjust to enclose a point
        fixed bug in 1 axis matrix setup
        added new matrix functions: pitch, yaw, roll, normalize
        added Symantec compatibility
        
        FILEIO:
        ---------------------------------------------------------------------
        
        changed to work with Symantec
        changed rlestream to work with dword alignment
        
        GFX:
        ---------------------------------------------------------------------
        
        added 16-bit polygon support - flat, gouraud, textured, shaded txt, gouraud txt,
        added 16-bit rectangle and clear support
        poly pipeline now does not modify input polygon in any way
        removed all old unused poly code (raster list based)
        removed gfx_set_sub_shift_enable()
        split 16-bit color table generation in the palette into 3 seperate functions
        fixed floating-point stack overflow bug
        
        TS:
        ---------------------------------------------------------------------
        
        changed world viewport from RectI to RectF
        new file format
        added versioning and new versions for some classes: 
                TSPartList,TSShape,TSPart,TSMesh,TSBspPart,
        added encoded normal lighting stuff; currently disabled
        fixed GFX flags for flat shaded polys
        added prelim animated mesh; this will not stay here
        changed TSPartList to adjust bounding sphere when parts are added
        light struct rearranged to improve cache performance
        some changes to PointArray class
        
        Added optional bool load flag to TSMaterialList::load()*
                Locking a matierial list with no palette and RGB entries now issues 
                a warning and continues (used to fatal), the color index is set to 0. 
        
        
        TERRAIN:
        ---------------------------------------------------------------------
        	Converted to use new RectF camera world viewport
        	Added new functionality to allow loading material and bitmaps
        		into blocks directly from bitmaps ptrs.

1.17.96
        General
        ---------------------------------------------------------------------
        
        Added DDrawLib directory with direct draw libs.

        3Space
        ---------------------------------------------------------------------

        A number of bug fixes and internal implementation changes.

        Added a class to keep some simple metrics (i.e. number of polys drawn)

        TSMaterialList static load method now locks the material list.
        To make this work, this method now takes an optional
        GFXPalette * arg.  If the render interface is not locked, a palette
        should be supplied.  TSShape::load() is changed the same way, so
        it also now takes an optional GFXPalette*.

        Added isLocked method to TSMaterial.

        Changed several camera items to floating point, mostly internal.

        Modified interface to GFX for textured polys.  w is now passed
        as part of the vertex, which is now a Point4I, rather than as
        part of the texture coordinate structure.

        Added static load() method to TSMaterialList.

        Added TSPointArray class.

        Changed name of TSSearchPath() to TSFindFile()

        Added class TSSearchPath to facilitate temporarily overriding
        search paths in static load methods.

        Things that will require changes to your code include:

                - Focal Length for perspective is now RealF

                - Ortho Depth for ortho cameras is now RealF

                - projectPerspective() methods have been eliminated

                - project() routines now return projected points in
                  Point4I structures.  This is an Int32 Point3 plus
                  a RealF w.
                
        
