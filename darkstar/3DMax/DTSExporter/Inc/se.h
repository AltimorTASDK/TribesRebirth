#ifndef SCENEENUM_H_
#define SCENEENUM_H_

#include <filstrm.h>

#include "ShapeBuilder.h"

//--------------------------------------------------------------

#define OBTYPE_MESH 0

//--------------------------------------------------------------

class SceneEnumProc : public ITreeEnumProc 
{
	FILE *					fp;
	ShapeBuilder			sb;
	VectorPtr<INode *>	entries;
	VectorPtr<INode *>	always_nodes;
	INode *					bounds_node;
	bool						no_transform_collapse;
   
   // If the parameter is not NULL in the call to getNode(), then the VICON
   //    node replication will happen.  The first two parameters are 
   //    set up at the top of the call tree, the last two are configured 
   //    case-by-case down the call tree.  
   class ViconContext 
   {
      public:
         ViconContext()
         {
            replicatingDetNumber = 0;
            // isDetailMeshNode = false;
            nodeHasMesh = true;
         }
         Vector<int>       detailNumbers; 
         int               replicatingDetNumber;   // also a flag for if we're 
         // bool              isDetailMeshNode;       //    replicating.  
         bool              nodeHasMesh;            // 
   };
   bool isAlways( INode *pNode );
   bool isDummy( INode *pNode );
   bool isDummyAlways( INode *pNode );

	void		getSequence( INode *pNode );
	void		getTransition( INode *pNode );
   void		getNode( INode *pNode, INode *pRootTo, char *linkName, 
							int priority, Bool IsDetail, Bool IsShape, 
							int detailNum, Matrix3 *preScale = 0, ViconContext * vc = NULL );
                     
	bool		getNodeTransformKeys( INode *pNode, INode *pRootTo, 
					ShapeBuilder::Node * pSBNode, int priority, Matrix3 *preScale = 0 );

	void		getNodeMorphKeys( INode *pNode, Object *ob, Interval &range, 
					int nodeId, Bool IsShape, INode *rootTo, 
					int nameId );

	void		getNodeTransform( INode *pNode, INode *pRootTo, int time, 
					MyTransform *xform, Matrix3 *preScale = 0 );
	void		setNodeDefaultTransform( INode *pNode, INode *pRootTo,
					ShapeBuilder::Node * pSBNode, Matrix3 *preScale = 0 );
	void		makeNodeKeyframe( int time, int startTime, int endTime, 
					INode *pNode, INode *pRootTo, MyTransform *xform, 
					float *pos, Matrix3 *preScale = 0 );

	Bool		addMorphDefault( INode *pNode, INode *pRootTo, Object *ob, MeshBuilder * pMB );
	Bool		addMorphKeyframe( int time, int startTime, int endTime, INode *pNode, 
				Object *obj, MeshBuilder * pMB, INode *rootTo, int *last_vis, 
				int changes );

	void		getControllerTimes( Control * pControl, TS::IndexList * pTimes );
	void		getTransformTimes( INode *pNode, TS::IndexList * pTimes );

public:
					SceneEnumProc(IScene *scene);
					~SceneEnumProc();

   static bool isBounds( INode *pNode );
   static bool isVICON( INode *pNode );
   
	int		Empty() 
						{ return 0 == entries.size(); }
	int		callback( INode *node );
	void		getNodes( FileWStream & ost );
};

//--------------------------------------------------------------
#endif
