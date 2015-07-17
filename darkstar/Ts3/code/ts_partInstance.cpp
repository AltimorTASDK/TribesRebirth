//---------------------------------------------------------------------------
// PartInstance
//---------------------------------------------------------------------------

#include <ts_partInstance.h>

Random breakRand;

// constructions methods
// 1 object, or
// break into parts:  supply node to break shape at [static method], or
//                    break into all objects [static method], or
//                    pull part off at a given node [static method]
namespace TS
{

PartInstance::PartInstance( ShapeInstance * shape, NodeInstance * node, ShapeObjectInstance * object,
                            ResourceManager & rm)
{
   frShape = shape->frShape;
   fpShape = (Shape const *)frShape;
   frMaterialList = 0;

   // use the shapeInst's material list if it has one, ow use shapes:
   if (&shape->getMaterialList()==fpShape->getMaterialList())
      // just use shapes, no need to copy actual list
      fpMaterialList = &shape->getMaterialList();
   else
   {
      fMaterialList = shape->getMaterialList();
      fpMaterialList = &fMaterialList;
   }
   const_cast<MaterialList*>(fpMaterialList)->load( rm, 1 );

   objectCount = 0;

   init(shape,node,object);
}

PartInstance::PartInstance( ShapeInstance * shape, ResourceManager & rm)
{
   frShape = shape->frShape;
   fpShape = (Shape const *)frShape;
   frMaterialList = 0;

   // use the shapeInst's material list if it has one, ow use shapes:
   if (&shape->getMaterialList()==fpShape->getMaterialList())
      // just use shapes, no need to copy actual list
      fpMaterialList = &shape->getMaterialList();
   else
   {
      fMaterialList = shape->getMaterialList();
      fpMaterialList = &fMaterialList;
   }
   const_cast<MaterialList*>(fpMaterialList)->load( rm, 1 );

   objectCount = 0;

   init(shape,NULL,NULL);
}

//---------------------------------------------------------------------------

// node need be non-null only if creating object
void PartInstance::init( ShapeInstance * shape, NodeInstance * node, ShapeObjectInstance * object)
{
   // default to first detail:
   fDetailLevel = 0;
   fDrawFlatPerspective = false;
   fDrawFlatPerspectiveDetail = false;
   perspectiveScale = 0.0f;   
   detailScale = 1.0f;
   fAlwaysAlpha = false;
   fAlwaysNode = NULL;
   fUseAnimation = false;
   fUseDetails = false;

   // note:: fTransforms replaces fTransformList from ts_shapeinstance
   // because the list of transforms has an uncertain size at this point,
   // so we want a list of pointers rather than a list of TMats (because
   // nodes need the pointer to the transform on construction, and vectors
   // can move in memory as the size changes).

   // build bounds node and object
   fTransforms.increment();
   fTransforms.last() = new TMat3F;
   fNodeInstanceList.increment();
   NodeInstance * boundsNodeInst = 
      fNodeInstanceList.last() = new NodeInstance( boundsNode, fTransforms.last());
   (*fTransforms.last()).identity();
   boundsObject = new BoundsObject;
   boundsNodeInst->AddDependentObject(boundsObject);
   // box on bounds object needs to be set still

   // if node && objectNumber>0, add the object
   if (node && object)
   {
      // need a new node
      fTransforms.increment();
      fTransforms.last() = new TMat3F;
      fNodeInstanceList.increment();
      NodeInstance * newNode =
         fNodeInstanceList.last() = new NodeInstance( node->fNode, fTransforms.last() );
      newNode->fpParentTransform = 0;
      newNode->fVisible = node->fVisible;         

      // hook us up
      boundsNodeInst->fpChild = newNode;

      // now make the new object
      ShapeObjectInstance * newObject = 
         new ShapeInstance::ShapeObjectInstance(object->fObject,fpShape);
      newObject->fFrameIndex = object->fFrameIndex;
      newObject->fMatIndex = object->fMatIndex;
      newObject->fVisible = object->fVisible;

      // now add the object to the new node
      newNode->AddDependentObject(newObject);

      // since this is the only object in the shape
      // use bounds on this object for bounding box, transfering
      // all transforms to fRootDeltaTransform
      Box3F box;
      newObject->getBox(this,box);
      Point3F offset = (box.fMin+box.fMax)*0.5f;
      box.fMin -= offset;
      box.fMax -= offset;
      newNode->fpTransform->identity();
      newNode->fpTransform->p -= offset;
      m_mul(*node->fpTransform,shape->fRootDeltaTransform,&fRootDeltaTransform);
      fRootDeltaTransform.p += offset;

      boundsObject->setBox(this,box);

      objectCount++;
   }
   else
      // root transform same as in original shape
      fRootDeltaTransform = shape->fRootDeltaTransform;

   if ( shape->fMaterialRemap)
   {
      fMaterialRemap = new int[ fpShape->fnDefaultMaterials ];
      for ( int i = 0; i < fpShape->fnDefaultMaterials; i++ )
         fMaterialRemap[i] = shape->fMaterialRemap[i];
   }
   else
      fMaterialRemap = 0;
}

PartInstance::~PartInstance()
{
   for (int i = 0; i < fTransforms.size(); i++)
      delete fTransforms[i];
}

// ----------------------------------------------------------------------------
// Static method to break shape into pieces.  All sub-trees below breakNode become their
// own partInstance.  Everything up to breakNode (including siblings of breakNode) are
// incorporated into another partInstance.  If separateObjects is true, then the objects
// off breakNode become their own partInstance, o.w. they are incorporated into the upper
// part of the shape tree.
// ----------------------------------------------------------------------------
void PartInstance::breakShape( ShapeInstance * shape, 
                               NodeInstance * startNode, NodeInstance * breakNode, 
                               Vector<PartInstance*> & partList, bool separateObjects,
                               ResourceManager & rm )
{
   partList.clear();

   // make a new part
   partList.increment();
   PartInstance * firstPart =
      partList.last() = new PartInstance(shape,rm);

   NodeInstance * firstPartNode  = firstPart->fNodeInstanceList.last();

   firstPartNode->fpChild = 
      PartInstance::addNode( shape, firstPart,
                             startNode, breakNode, 
                             NULL,
                             partList, separateObjects, false, rm );

   for (int i=0; i<partList.size(); i++)
   {
      PartInstance * part = partList[i];
      
      if (part->objectCount==0)
      {
         delete part;
         partList[i] = partList.last();
         partList.decrement();
         i--;
         continue;
      }

      AssertFatal(part->boundsObject->boxSet,"PartInstance::breakShape: bbox not set!");

      part->centerBox(part->fRootDeltaTransform);
   }
}

// ----------------------------------------------------------------------------
// Static method to break shape into pieces.  Break at each node with passed
// probability.  Based on above breakShape.
// ----------------------------------------------------------------------------
void PartInstance::breakShape( ShapeInstance * shape, 
                               NodeInstance * startNode, float probBreak,
                               Vector<PartInstance*> & partList,
                               ResourceManager & rm )
{
   partList.clear();

   // make a new part
   partList.increment();
   PartInstance * firstPart =
      partList.last() = new PartInstance(shape,rm);

   NodeInstance * firstPartNode  = firstPart->fNodeInstanceList.last();

   firstPartNode->fpChild = 
      PartInstance::addNode( shape, firstPart,
                             startNode, probBreak,
                             NULL,
                             partList, false, rm );

   for (int i=0; i<partList.size(); i++)
   {
      PartInstance * part = partList[i];
      
      if (part->objectCount==0)
      {
         delete part;
         partList[i] = partList.last();
         partList.decrement();
         i--;
         continue;
      }

      AssertFatal(part->boundsObject->boxSet,"PartInstance::breakShape: bbox not set!");

      part->centerBox(part->fRootDeltaTransform);
   }
}

// ----------------------------------------------------------------------------
// Static method to break shape into pieces.  Each object becomes it's own part.
// ----------------------------------------------------------------------------

void PartInstance::breakShape( ShapeInstance * shape, NodeInstance * startNode,
                               Vector<PartInstance*> & partList,
                               ResourceManager & rm)
{
   PartInstance::breakShape(shape,startNode,partList,false,rm);
}   

void PartInstance::breakShape( ShapeInstance * shape, NodeInstance * thisNode,
                               Vector<PartInstance*> & partList, bool breakSiblings,
                               ResourceManager & rm)
{
   PartInstance::makeParts(shape,thisNode,partList,rm);
   if (thisNode->fpChild)
      PartInstance::breakShape(shape,thisNode->fpChild,partList,true,rm);
   if (breakSiblings && thisNode->fpNext)
      PartInstance::breakShape(shape,thisNode->fpNext,partList,true,rm);
}

//---------------------------------------------------------------------------------
// Static method to pull pieces off a shape.  If separateObjects is true, all sub-trees
// below breakNode become their own partInstance as do the objects off breakNode.  If
// separateObjects is false, breakNode, it's children, and the objects off breakNode
// all become one object.
//---------------------------------------------------------------------------------

void PartInstance::pullOff( ShapeInstance * shape, NodeInstance * breakNode, 
                            Vector<PartInstance*> & partList, bool separateObjects,
                            ResourceManager & rm )
{
   partList.clear();

   //
   partList.increment();
   PartInstance * part = partList.last() = new PartInstance(shape,rm);
   NodeInstance * partBaseNode = part->fNodeInstanceList.last();

   //
   if (separateObjects)
   {
      PartInstance::makeParts(shape, breakNode, partList, rm);
      if (breakNode->fpChild)
         partBaseNode->fpChild =
            PartInstance::addNode( shape,
                                   part, 
                                   breakNode->fpChild,
                                   0,
                                   NULL,
                                   partList,
                                   false,
                                   true,
                                   rm);
   }
   else
      partBaseNode->fpChild =
         PartInstance::addNode( shape,
                                part, 
                                breakNode,
                                0,
                                NULL,
                                partList,
                                false,
                                false,
                                rm);

   for (int i=0; i<partList.size(); i++)
   {
      part = partList[i];
      
      if (part->objectCount==0)
      {
         delete part;
         partList[i] = partList.last();
         partList.decrement();
         i--;
         continue;
      }

      AssertFatal(part->boundsObject->boxSet,"PartInstance::pullOff: bbox not set!");

      part->centerBox(part->fRootDeltaTransform);
   }
}

void PartInstance::pullOff( NodeInstance * offNode, bool offSiblings )
{
   for (int i = 0; i < offNode->fObjectList.size(); i++)
   {
      ShapeObjectInstance * object = 
         dynamic_cast<ShapeObjectInstance*>(offNode->fObjectList[i]);
      if (object)
         object->fActive = object->fVisible = false;
   }
   
   if (offNode->fpChild)
      PartInstance::pullOff(offNode->fpChild,true);
   if (offNode->fpNext && offSiblings)
      PartInstance::pullOff(offNode->fpNext,true);   
}   

// the complement of the short version of pullOff, but assumes the shape is complete
// and hasn't been cut up by partinstance code (except above version of pullOff).
void PartInstance::putOn( NodeInstance * onNode, bool onSiblings )
{
   for (int i = 0; i < onNode->fObjectList.size(); i++)
   {
      ShapeObjectInstance * object = 
         dynamic_cast<ShapeObjectInstance*>(onNode->fObjectList[i]);
      if (object)
         object->fActive = true;
   }

   if (onNode->fpChild)
      PartInstance::putOn(onNode->fpChild,true);
   if (onNode->fpNext && onSiblings)
      PartInstance::putOn(onNode->fpNext,true);         
}   

//---------------------------------------------------------------------------------
// add a detail level to shape
// move thisNode on down from shape to a new detail level on us
//---------------------------------------------------------------------------------

TSShapeInstance::NodeInstance *
PartInstance::addDetail( ShapeInstance * shape, NodeInstance * thisNode, ResourceManager & rm)
{
   fTransforms.increment();
   fTransforms.last() = new TMat3F;
   *fTransforms.last() = *thisNode->fpTransform;

   // this'll be the node for our new detail
   fNodeInstanceList.increment();
   fNodeInstanceList.last() = new NodeInstance(thisNode->fNode,fTransforms.last());

   NodeInstance * startNode;

   if (fNodeInstanceList.size()!=2) // bounds node + the one we just added
   {
      // not first detail level, find last one and make it point to our new detail
      startNode = fNodeInstanceList[1];
      while (startNode->fpNext)
         startNode = startNode->fpNext;
      startNode->fpNext = fNodeInstanceList.last();
   }

   startNode = fNodeInstanceList.last();

   // add objects for this node
   objectCount += PartInstance::addObjects( shape, thisNode, startNode, false );

   Vector<PartInstance*> dummyList; // nothing will be added...

   if (thisNode->fpChild)
      startNode->fpChild = PartInstance::addNode(shape,
                                                 this,
                                                 thisNode->fpChild,
                                                 NULL,
                                                 NULL,
                                                 dummyList,
                                                 false,
                                                 true,
                                                 rm);

   AssertFatal(dummyList.empty(),"TSPartInstance::addDetail: dummy list got a part ... oops.");

   return startNode;
}

//---------------------------------------------------------------------------------
// utility routines for constructing part instances
//---------------------------------------------------------------------------------

TSShapeInstance::NodeInstance *
PartInstance::addNode( ShapeInstance * shape, PartInstance * part,
                       NodeInstance * thisNode, NodeInstance * breakNode, 
                       NodeInstance * parentNode,
                       Vector<PartInstance*> & partList, bool separateObjects, bool addSiblings,
                       ResourceManager & rm )
{
   // new node
   part->fTransforms.increment();
   part->fTransforms.last() = new TMat3F;
   *part->fTransforms.last() = *thisNode->fpTransform;
   part->fNodeInstanceList.increment();
   NodeInstance * newNode = 
      part->fNodeInstanceList.last() = new NodeInstance(thisNode->fNode,part->fTransforms.last());
   newNode->fVisible = thisNode->fVisible;

   // handle detailing
   PartInstance::generateDetails(part,
                                 thisNode->fpNextDetail,
                                 parentNode ? parentNode->fpNextDetail : NULL,
                                 newNode,
                                 !addSiblings);

   // should we add the objects to the current part or make new objects of them
   if (separateObjects && thisNode==breakNode)
      PartInstance::makeParts(shape, thisNode, partList, rm);
   else
   {
      part->objectCount += PartInstance::addObjects(shape,thisNode, newNode);
      // ok, that takes care of base detail, now add objects for lower details
      NodeInstance * newNextDetail =  newNode->fpNextDetail;
      NodeInstance * oldNextDetail = thisNode->fpNextDetail;
      while (newNextDetail)
      {
         PartInstance::addObjects(shape,oldNextDetail,newNextDetail);
         oldNextDetail = oldNextDetail->fpNextDetail;
         newNextDetail = newNextDetail->fpNextDetail;
      }   
   }   

   // if we have a child, either add it to part or start a new part
   NodeInstance * childNode = thisNode->fpChild;
   if (childNode)
   {
      if (thisNode==breakNode)
      {
         // make new parts out of children
         while (childNode)
         {
            partList.increment();
            PartInstance * newPart =
               partList.last() = new PartInstance(shape,rm);
            NodeInstance * newPartNode = newPart->fNodeInstanceList.last();
            newPartNode->fpChild =
               PartInstance::addNode(shape,newPart,
                                     childNode,breakNode,
                                     NULL,
                                     partList,separateObjects,false,rm);

            if (!newPartNode->fpChild && newPart==partList.last())
            {
               delete newPart;
               partList.decrement();
            }
            childNode = childNode->fpNext;
         }
      }
      else
         // add child node to firstPart
         newNode->fpChild =
            PartInstance::addNode(shape,part,
                                  childNode,breakNode,
                                  newNode,
                                  partList,separateObjects,true,rm);
   }
   
   // if thisNode (in shape) has a sibling, copy sibling to newNode (in part)
   NodeInstance * siblingNode = thisNode->fpNext;
   if (addSiblings && siblingNode)
      newNode->fpNext =
         PartInstance::addNode(shape,part,
                               siblingNode,breakNode,
                               parentNode,
                               partList,separateObjects,true,rm);

   if (!newNode->fpChild && !newNode->fpNext && newNode->fObjectList.empty())
      return NULL;

   // we're keeping this node, extend the bounding box
   // (this doesn't happen for other detail levels)      
   part->extendBox(newNode);

   return newNode;
}   

// second addNode routine -- this one is for breaking a shape up with a given
// probability, rather than at a given node
TSShapeInstance::NodeInstance *
PartInstance::addNode( ShapeInstance * shape, PartInstance * part,
                       NodeInstance * thisNode, float probBreak,
                       NodeInstance * parentNode,
                       Vector<PartInstance*> & partList, bool addSiblings,
                       ResourceManager & rm )
{
   // new node
   part->fTransforms.increment();
   part->fTransforms.last() = new TMat3F;
   *part->fTransforms.last() = *thisNode->fpTransform;
   part->fNodeInstanceList.increment();
   NodeInstance * newNode = 
      part->fNodeInstanceList.last() = new NodeInstance(thisNode->fNode,part->fTransforms.last());
   newNode->fVisible = thisNode->fVisible;
   
   // handle detailing
   PartInstance::generateDetails(part,
                                 thisNode->fpNextDetail,
                                 parentNode ? parentNode->fpNextDetail : NULL,
                                 newNode,
                                 !addSiblings);

   bool doBreak = breakRand.getFloat(0,1) < probBreak;

   // add objects to part
   part->objectCount += PartInstance::addObjects(shape,thisNode, newNode);
   // ok, that takes care of base detail, now add objects for lower details
   NodeInstance * newNextDetail =  newNode->fpNextDetail;
   NodeInstance * oldNextDetail = thisNode->fpNextDetail;
   while (newNextDetail)
   {
      PartInstance::addObjects(shape,oldNextDetail,newNextDetail);
      oldNextDetail = oldNextDetail->fpNextDetail;
      newNextDetail = newNextDetail->fpNextDetail;
   }   

   // if we have a child, either add it to part or start a new part
   NodeInstance * childNode = thisNode->fpChild;
   if (childNode)
   {
      if (doBreak)
      {
         // make new parts out of children
         while (childNode)
         {
            partList.increment();
            PartInstance * newPart =
               partList.last() = new PartInstance(shape,rm);
            NodeInstance * newPartNode = newPart->fNodeInstanceList.last();
            newPartNode->fpChild =
               PartInstance::addNode(shape,newPart,
                                     childNode,probBreak,
                                     NULL,
                                     partList,false,rm);

            if (!newPartNode->fpChild && newPart==partList.last())
            {
               delete newPart;
               partList.decrement();
            }
            childNode = childNode->fpNext;
         }
      }
      else
         // add child node to firstPart
         newNode->fpChild =
            PartInstance::addNode(shape,part,
                                  childNode,probBreak,
                                  newNode,
                                  partList,true,rm);
   }
   
   // if thisNode (in shape) has a sibling, copy sibling to newNode (in part)
   NodeInstance * siblingNode = thisNode->fpNext;
   if (addSiblings && siblingNode)
      newNode->fpNext =
         PartInstance::addNode(shape,part,
                               siblingNode, probBreak,
                               parentNode,
                               partList,true,rm);

   if (!newNode->fpChild && !newNode->fpNext && newNode->fObjectList.empty())
      return NULL;

   // we're keeping this node, extend the bounding box
   // (this doesn't happen for other detail levels)      
   part->extendBox(newNode);

   return newNode;
}   

void PartInstance::generateDetails( PartInstance * part,
                                    NodeInstance * copyFrom, NodeInstance * parentCopyTo,
                                    NodeInstance * attachTo, bool topOfTree)
{
   while (copyFrom && (parentCopyTo || topOfTree))
   {
      part->fTransforms.increment();
      part->fTransforms.last() = new TMat3F;
      *part->fTransforms.last() = *copyFrom->fpTransform;
      part->fNodeInstanceList.increment();
      part->fNodeInstanceList.last() =
         new NodeInstance(copyFrom->fNode,part->fTransforms.last());
      attachTo->fpNextDetail = part->fNodeInstanceList.last();
      attachTo->fpNextDetail->fVisible = copyFrom->fVisible;

      if (topOfTree)
         attachTo->fpNext = attachTo->fpNextDetail;

      if (parentCopyTo)
      {
         if (parentCopyTo->fpChild)
         {
            NodeInstance * putItHere = parentCopyTo->fpChild;
            while (putItHere->fpNext)
               putItHere = putItHere->fpNext;
            putItHere->fpNext = attachTo->fpNextDetail;   
         }
         else
            parentCopyTo->fpChild = attachTo->fpNextDetail;
         parentCopyTo = parentCopyTo->fpNextDetail;
      }

      copyFrom = copyFrom->fpNextDetail;
      attachTo = attachTo->fpNextDetail;
   }
}

void PartInstance::makeParts(ShapeInstance * shape, NodeInstance * fromNode, 
                             Vector<PartInstance *> & partList,
                             ResourceManager & rm)
{
   for (int i = 0; i < fromNode->fObjectList.size(); i++)
   {
      ShapeObjectInstance * object = 
         dynamic_cast<ShapeObjectInstance*>(fromNode->fObjectList[i]);
      if (object && object->fVisible)
      {
         // new part
         partList.increment();
         partList.last() = new PartInstance(shape,fromNode,object,rm);
         PartInstance * newPart = partList.last();
         object->fActive = object->fVisible = false;
         
         // what about lower detail versions of same object
         // we assume object will be in same position in fObjectList as
         // this one is (but on fpNextDetail node).
         // note:  none of this will affect objectCount on part
         NodeInstance * nextDetail = fromNode->fpNextDetail;
         while (nextDetail && i < nextDetail->fObjectList.size())
         {
            ShapeObjectInstance * object2 =
               dynamic_cast<ShapeObjectInstance*>(nextDetail->fObjectList[i]);
            if (object2 && object2->fVisible)
            {
               // this is the previous details node
               NodeInstance * lastNode = newPart->fNodeInstanceList.last();

               // ok, let's add a node for the new guy
               newPart->fTransforms.increment();
               newPart->fTransforms.last() = new TMat3F;
               NodeInstance * newNode =
                  new NodeInstance(nextDetail->fNode,newPart->fTransforms.last());
               newPart->fNodeInstanceList.increment();
               newPart->fNodeInstanceList.last() = newNode;
               *newNode->fpTransform = *nextDetail->fpTransform;

               // add an object to the new node
               ShapeObjectInstance * newObj =
                  new ShapeInstance::ShapeObjectInstance(object2->fObject,shape->fpShape);
               newObj->fFrameIndex = object2->fFrameIndex;
               newObj->fMatIndex   = object2->fMatIndex;
               newNode->AddDependentObject(newObj);
               
               // deactivate object
               object2->fActive = object2->fVisible = false;

               // attach the new node
               lastNode->fpNextDetail = newNode;
               lastNode->fpNext       = newNode;
            }   
            nextDetail = nextDetail->fpNextDetail;
         }

         // set bounding box
         if (newPart->fNodeInstanceList.size()>1)
            newPart->extendBox(newPart->fNodeInstanceList[1]);
      }
   }
}   

int PartInstance::addObjects(ShapeInstance * shape, NodeInstance * fromNode, 
                              NodeInstance * toNode, bool deactivate)
{
   int newObjects = 0;
   for (int i = 0; i < fromNode->fObjectList.size(); i++)
   {
      // cast object to ShapeObjectInstance
      ShapeObjectInstance * object = 
         dynamic_cast<ShapeObjectInstance*>(fromNode->fObjectList[i]);
      if (object && object->fVisible)
      {
         // add a new object to newNode
         ShapeObjectInstance * newObj =
            new ShapeObjectInstance(object->fObject,&shape->getShape());
         newObj->fFrameIndex = object->fFrameIndex;
         newObj->fMatIndex   = object->fMatIndex;
         toNode->AddDependentObject(newObj);
         newObjects++;
         if (deactivate)
            object->fActive = object->fVisible = false;
      }
   }
   return newObjects;
}

void PartInstance::extendBox(NodeInstance * node, Box3F & box)
{
   for (int i=0; i<node->fObjectList.size(); i++)
   {
      ObjectInstance * object = node->fObjectList[i];
      ShapeObjectInstance * shapeObject = dynamic_cast<ShapeObjectInstance*>(object);
      TMat3F & nmat = *node->fpTransform;
      TMat3F omat = nmat;
      if (shapeObject)
      {
         // save a bit by not doing full matrix mult (fObjectOffset always ident.)
         Point3F pnt;
         m_mul(shapeObject->fObject.fObjectOffset,(RMat3F&)nmat,&pnt);
         omat.p += pnt;
      }   

      Point3F ox,oy,oz;
      omat.getRow(0,&ox);
      omat.getRow(1,&oy);
      omat.getRow(2,&oz);

      Box3F obox;
      object->getBox(this,obox);
      Point3F ocenter = (obox.fMin + obox.fMax) * 0.5f;
      Point3F rad     = (obox.fMax - obox.fMin) * 0.5f;

      float x_extent = fabs(ox.x)*rad.x + 
                       fabs(oy.x)*rad.y + 
                       fabs(oz.x)*rad.z;

      float y_extent = fabs(ox.y)*rad.x + 
                       fabs(oy.y)*rad.y + 
                       fabs(oz.y)*rad.z;

      float z_extent = fabs(ox.z)*rad.x + 
                       fabs(oy.z)*rad.y + 
                       fabs(oz.z)*rad.z;

      // get center in shape space
      Point3F ocenter2;
      m_mul(ocenter,omat,&ocenter2);
      Point3F minExtent = ocenter2;
      Point3F maxExtent = ocenter2;
      minExtent.x -= x_extent;
      minExtent.y -= y_extent;
      minExtent.z -= z_extent;
      maxExtent.x += x_extent;
      maxExtent.y += y_extent;
      maxExtent.z += z_extent;
      box.fMin.setMin(minExtent);
      box.fMax.setMax(maxExtent);
   }
}

void PartInstance::extendBox(NodeInstance * node)
{
   Box3F box;
   boundsObject->getBox(this,box);
   extendBox(node,box);
   boundsObject->setBox(this,box);
}

void PartInstance::setBox()
{
   Box3F box;
   box.fMax.set(-1E20f,-1E20f,-1E20f);
   box.fMin.set( 1E20f, 1E20f, 1E20f);

   int i;

   for (i=1;i<fNodeInstanceList.size();i++)
      extendBox(fNodeInstanceList[i],box);

   boundsObject->setBox(this,box);
   centerBox(fRootDeltaTransform);
}

void PartInstance::centerBox(TMat3F & transform)
{
   // get center of box
   Box3F box;
   boundsObject->getBox(this,box);
   Point3F center = (box.fMin+box.fMax) * 0.5f;

   // shift box, fTransforms, and passed shape transform
   box.fMin -= center;
   box.fMax -= center;
   boundsObject->setBox(this,box);
   for (int i=1;i<fTransforms.size();i++)
      fTransforms[i]->p -= center;
   transform.p += center;
}

void PartInstance::BoundsObject::getBox( ShapeInstance *, Box3F & _box)
{
   _box=bbox;
}

void PartInstance::BoundsObject::setBox( ShapeInstance *, const Box3F & _box)
{
   boxSet = true;
   bbox=_box;
}

}; // namespace TS


