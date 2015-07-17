//---------------------------------------------------------------------------
//   ts_partInstance.h
//---------------------------------------------------------------------------

#ifndef _TS_PARTINST_H_
#define _TS_PARTINST_H_

#include <ts_shapeInst.h>

namespace TS
{
//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//------------------------------------------------------------------

class DLLAPI PartInstance : public ShapeInstance
{
    static NodeInstance * addNode(ShapeInstance *, PartInstance *,
                                  NodeInstance * thisNode, NodeInstance * breakNode,
                                  NodeInstance * parentNode,
                                  Vector<PartInstance*> & partList,
                                  bool separateObjects, bool addSiblings,
                                  ResourceManager &);

    static NodeInstance * addNode(ShapeInstance *, PartInstance *,
                                  NodeInstance * thisNode, float probBreak,
                                  NodeInstance * parentNode,
                                  Vector<PartInstance*> & partList,
                                  bool addSiblings,
                                  ResourceManager &);
                                  
    static void generateDetails(PartInstance * part, NodeInstance * copyFrom,
                                NodeInstance * parentCopyTo, NodeInstance * attachTo,
                                bool topOfTree);

    static void makeParts(ShapeInstance * shape, NodeInstance * fromNode,
                          Vector<PartInstance *> & partList,
                          ResourceManager &);

    static int addObjects(ShapeInstance * shape, NodeInstance * fromNode,
                          NodeInstance * toNode, bool deactivate = true);

    static void breakShape(ShapeInstance * shape, NodeInstance * thisNode,
                           Vector<PartInstance*> & partList, bool breakSiblings,
                           ResourceManager &);
public:

    PartInstance( ShapeInstance *, NodeInstance *, ShapeObjectInstance *, ResourceManager &);
    PartInstance( ShapeInstance *, ResourceManager &);

    TSShapeInstance::NodeInstance * addDetail( ShapeInstance * shape, NodeInstance * thisNode,
                                               ResourceManager &);

    ~PartInstance();

    void init(ShapeInstance *, NodeInstance *, ShapeObjectInstance *);

    Shape::Node boundsNode;
    Vector<TMat3F *> fTransforms; // replace fTransformList in ShapeInstance because
                                  // fTransforms grows and may move in memory
   
    TSMaterialList fMaterialList;

    // how many objects have been added to this part?
    int objectCount;

    class BoundsObject : public TS::ObjectInstance
    {
        // only functionality of this object is
        // to get and set the bounding box

        public:
        Box3F bbox;
        bool boxSet;
        void getBox(ShapeInstance *, Box3F &);
        void setBox(ShapeInstance *, const Box3F &);
        BoundsObject()
        {
           boxSet = false; 
           bbox.fMin.set( 1E20f, 1E20f, 1E20f);
           bbox.fMax.set(-1E20f,-1E20f,-1E20f);
        }
    } * boundsObject;

    static void breakShape(ShapeInstance *,
                           NodeInstance * startNode, NodeInstance * breakNode,
                           Vector<PartInstance*> & partlist, bool separateObjects,
                           ResourceManager &);

    static void breakShape(ShapeInstance *,
                           NodeInstance * startNode, float probBreak,
                           Vector<PartInstance*> & partlist,
                           ResourceManager &);

    static void breakShape(ShapeInstance * shape, NodeInstance * startNode,
                           Vector<PartInstance*> & partList,
                           ResourceManager &);

    static void pullOff(ShapeInstance * shape, NodeInstance * breakNode,
                        Vector<PartInstance*> & partList, bool separateObjects,
                        ResourceManager &);

    // this is here for convenience only -- same function as above pullOff method
    // except that it doesn't create any partInstances
    static void pullOff(NodeInstance * breakNode, bool offSiblings = false);
    
    // its counterpart is used to reattach pieces that have been removed
    static void putOn(NodeInstance * fixNode, bool onSiblings = false);

    void extendBox(NodeInstance *, Box3F & box);
    void extendBox(NodeInstance *);
    void setBox();
    void centerBox(TMat3F & transform); // center shape at origin

};

//------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

//------------------------------------------------------------------
}; // namespace TS

//
typedef TS::PartInstance TSPartInstance;

#endif


