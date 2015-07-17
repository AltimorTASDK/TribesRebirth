//-----------------------------------------------------------------------------------//
//                                                                                   //
//       aiGraph.cpp                                                                 //
//                                                                                   //
//    Module for managing a weighted graph to aid in AI navigation.                  //
//                                                                                   //
//    Implemented for Tribes AI path following, but not dependent on any             //
//       Tribes code.                                                                //
//                                                                                   //
//-----------------------------------------------------------------------------------//


/**************************************************************************************

               *** OBJECTS ***

   AIGraph:
      Weighted graph of connected nodes.  It is up to the app to supply the nodes, 
         probably via a console plugin and/or mission editor object. 
   
   xRefTable:
      An NxN table of 2-bit direction entries telling how to get to any point.  

   
               *** CONSOLE ***

   Island counts.
   Query node (by name or number) for it's island # and neighbors.  
   Note nodes can be replaced by a new one coming in with the SAME POINT (i.e. not just 
      the same node).  In fact - we merge points which are isEqual() (or maybe even a 
      looser check using set-able bounds).  


**************************************************************************************/

#include "aiGraph.h"

#include <stdio.h>

#define  ABSURD_DIST    314159265358979.0

static const char trueTxt[] =    "True";


#if INCLUDE_AI_GRAPH_CODE



UInt32   AIGraph::defaultLOSMask;
float    AIGraph::maximumRadius;


/*
   Want to fill out an N x N table (N == number of nodes) telling how to get between
      any two points.  
      
   ALGORITHM:

   Mark all the separate islands.  Simple recursor that is called until everyone
      is marked.  Island numbers start at ZERO and -1 flags a node as not yet
      belonging to an island.  

   For each island BEGIN
   
      In this island we want to fill out all table entries in the table to tell us how 
         to get between any two nodes.  We make use of a big temporary table to track
         our progress.
         
      We make multiple passes until there are no more nodes which need to be informed of
         better potential paths.  On each pass we build a LIST of table entries ((x,y) in
         NxN table) which will be processed on our next pass.  An entry is added to this
         list IFF it has been informed by a neighbor of (and needed to make use of) a better 
         way of getting to a certain point.  
         
      [note we actually use two lists here which swap back and forth. as we're processing the
       last one, we're pushing onto the next one. Each entry in the NxN table has a flag
       so we know if we've already pushed it on a given pass].  Here's the loop:
       
      First we fill in all table entries for immediate neighbors.  i.e. all (X,Y) and (Y,X) 
         where they both have neighbors.  
         
   For each island END
   
*/


// Return a REASON string if we can't build the table, else NULL for success.
//
const char * AIGraph::buildXRefTable(void)
{
   int   numIslands = buildIslands();
   if( numIslands > MaxIslands )
      return avar("there are too many islands (%d) in the table.", numIslands );

   Node::TableBuildHelp  H( nodeList.size() );
   
   while( numIslands-- )
   {
      //helper.gossipStarting();
   
      // Do our "seed" pass of informing.  Each node N tells her neighbors about her
      // other nearby neighbors.  
      for( NodeList::iterator i = nodeList.begin(); i != nodeList.end(); i++ )
        if( i->island == numIslands && i->count > 1 )
          for( int x = 0; x < i->count; x++ )
            for( int y = 0; y < i->count; y++ )
              if( y != x )
                nodeList[i->nodes[y]].tellAboutPath(H,i->xEcho[y],i->nodes[x],i->dists[x]);
   
      // Keep running inform passes as long as there is new information to pass on.  The idea
      //    here is that whenever a node finds a new better way to get from A to B, it then
      //    needs to tell it's other neighbors about this, in case they're interested.  If 
      //    they ARE interested, they then record that as their new better way and they add 
      //    themselves to the (other) list for the next pass.  
      while( H.moreGossipNeeded() )
      {
         Node::TableBuildHelp::Gossip   gossip;
         
         while( H.getGossip( gossip ) )
         {
            Node   & n = nodeList[ gossip.From ];
            for( int x = 0; x < n.count; x++ )
               if( x != gossip.Dir )
                  nodeList[n.nodes[x]].tellAboutPath(H,gossip.Dir,gossip.To,gossip.Dist);
            
         }
      }
   }
   H.fillXRefTable( xRefTable );
   return NULL;
}



AIGraph::AIGraph() 
   : m_islandCount(0)
{
   flags.clear();
}

AIGraph::~AIGraph()
{
}


// DO: make this a static member (and console accessible?).  
static float tooCloseDistance = 2.0;

int AIGraph::addNode( const Node & node )
{
   int   nodeIndex = nodeList.size(), i;
   // Scan the list for nearby nodes and reject this one if a close one is found.  
   for( i = 0; i < nodeIndex; i++ )
      if( m_distf(nodeList[i].point, node.point) < tooCloseDistance ) 
         return -1;
   nodeList.push_back( node );
   flags.set( NodeAdded );
   return nodeIndex;
}

// Add the node to the list.  Mark the graph as needing update.  Return the node index
// in the list (or -1 if it wasn't added due to being too near another). 
int AIGraph::addNode( Point3F & loc, int nDirections )
{
   Node     node(loc, nDirections);
   return addNode( node );
   //int   nodeIndex = nodeList.size();
   //nodeList.push_back( node );
   //flags.set( NodeAdded );
   //return nodeIndex;
}

// loadNode() and listNode() (see aiGraph.h) are mirrors of each other - is for listing
// the list of nodes to save, the other to reload.  Note the stubbed out temps.  
int AIGraph::addNode( const char * printedNode )
{
   Node     node(printedNode);
   return addNode( node );
   //int   nodeIndex = nodeList.size();
   //nodeList.push_back( node );
   //flags.set( NodeAdded );
   //return nodeIndex;
}


AIGraph::NodeSorter::NodeSorter( const Node & gn )
{
   int   i;
   
   nodeCount = 0;                      // this counts 'em, the Node gives max.  
   index = gn.index;
   distanceSortVal = ABSURD_DIST;
   maxDistSoFar = ABSURD_DIST;
   for( i = 0; i < MaxDirections; i++ )
      dists[i] = ABSURD_DIST;
}


int AIGraph::NodeSorter::operator< (const NodeSorter & in_sorter) const 
{
   if( nodeCount != in_sorter.nodeCount )
      return( nodeCount < in_sorter.nodeCount );
   else
      return( distanceSortVal > in_sorter.distanceSortVal );
}


/* 
   Find the neighbors of the nodes - build the hooks.  We would ideally like a way of
      editting these since this method has pitfalls - whoever builds them needs to know
      what they're doing a bit, but this should work for now.  
      
   Algorithm:
      
      First we find all the closest nodes as per the max closest specified.  

      Then we "sort" the list (actually we sort a list of NodeSorters since we want
            to leave our node list alone).  We sort first by least numNodes, then by
            maximum distance.  The idea here is that the far away nodes will go in 
            and override the neighbor points of their neighbors.  
            
      Go through and do the fixups - to make sure that everyone knows their neighbors.  
*/
const char * AIGraph::buildConnections(void)
{
   NodeList::iterator      itOuter, itInner;
   SorterList              sortList;
   
   // Loop to do these: 
   //    build sort list
   //    find nearest nodes and their distances
   //    find maximum number of directions
   //    
   //int   maxDirections = 0;
   for( itOuter = nodeList.begin(); itOuter != nodeList.end(); itOuter++ )
   {
      NodeSorter  sorter( * itOuter );
      
      for( itInner = nodeList.begin(); itInner != nodeList.end(); itInner++ )
      {
         if( itInner != itOuter )
         {
            float thisDistance = m_dist( itOuter->point, itInner->point );
            if( thisDistance < sorter.maxDistSoFar )
            {
               // replace our maximum distance with this one, and figure out a new
               //    maximum distance (which thisDistance isn't necessarily, of course).  
               int      i, replaceIdx = -1;
               float    newMaxDistance = -1.0, curDist;
               for( i = 0; i < itOuter->count; i++ )
               {
                  if( (curDist = sorter.dists[i]) > newMaxDistance )
                     newMaxDistance = curDist;
                  if( curDist == sorter.maxDistSoFar )
                     replaceIdx = i;
               }
               AssertFatal( replaceIdx >= 0, "aiGraph: should have found replace index" );
               
               sorter.maxDistSoFar = newMaxDistance;
               sorter.dists[ replaceIdx ] = thisDistance;
            }
         }
      }
      sortList.push_back( sorter );
   }
   
   
   // Perform the fixups.  We loop on the NodeSorter list and force the longer node spans to 
   //    override the local ones.
   sortList.sort();
   if( nodeList.size() >= 2 )
   {
   }
   
   return trueTxt;
}



// Recurse through the nodes associated with gn which haven't already been visited.  Return
// the number which wind up being visited.  
int AIGraph::islandMarkRecurse( Node * gn, int island )
{
   int    sum = 0;
   if( ! gn->visited() )
   {      
      sum++;
      gn->markVisit();
      gn->island = island;
      for( int d = 0; d < gn->count; d++ )
         sum += islandMarkRecurse( & nodeList [ gn->nodes[d] ], island );
      gn->markLeave();
   }
   return sum;
}


int AIGraph::buildIslands(void)
{
   NodeList::iterator    it;
   
   // First mark all nodes as not belonging to an island (index of -1).  
   for( it = nodeList.begin(); it != nodeList.end(); it++ ){
      AssertFatal( !it->visited(), "aiGraph: node with bad mark bit" );
      it->island = -1;
   }
   
   m_islandCount = 0;
   
   if( nodeList.size() >= 1 )
   {
      int   total = 0;
      do{
         // find first node without island assignment.
         for( it = nodeList.begin(); it != nodeList.end(); it++ )
            if( it->island < 0 )
               break;
         AssertFatal( it != nodeList.end(), "aiGraph: should have found node without island" );
         total += islandMarkRecurse( it, m_islandCount++ );
      } while( total < nodeList.size() );
      AssertFatal( total == nodeList.size(), "aiGraph: total doesn't jibe" );
   }
   
   return m_islandCount;
}


//---------------------------------------------------------------------------------------------
// Cross Reference Table.
// 
// Wrap the BitSet<> class to manage allocation and such.
// 
AIGraph::CrossRefTable::CrossRefTable()
{ 
   data = NULL; 
}

AIGraph::CrossRefTable::~CrossRefTable()
{
   if( data )
      delete [] data;
}

void AIGraph::CrossRefTable::init(int arrayWidth)
{
   if( data )
      delete [] data;
      
   int   arraySize = arrayWidth * arrayWidth + 3 >> 2;
   data = new UInt8 [ arraySize ];
   memset( data, 0xff, arraySize );
   Parent::init( (UInt16 *)(data) );
}


//---------------------------------------------------------------------------------------------
// Node methods.  

void AIGraph::Node::init(void)
{
   flags.clear();
   index = -1;
   count = 0;
   maxDirs = MaxDirections;
   
   for(int i = 0; i < MaxDirections; i++)
   {
      nodes[i] = -1;
      xEcho[i] = -1;
      dists[i] = ABSURD_DIST;
   }
   
   radius = ABSURD_DIST;
   LOSCol = 0xFFFFFFFF;
   island = -1;
}

AIGraph::Node::Node(Point3F & loc, int numDirs )
{
   init();
   point = loc;
   if( (maxDirs = numDirs) > MaxDirections )
      maxDirs = MaxDirections;
   else if( maxDirs < 0 )
      maxDirs = 0;
}

AIGraph::Node::~Node()
{
}

static const char printFormatString[] = "%f %f %f %f %c";

AIGraph::Node::Node(const char * printedInfo)
{
   init();
	sscanf(printedInfo, printFormatString, 
         & point.x, & point.y, & point.z, 
         & radius, 
         & maxDirs
         );
}

char * AIGraph::Node::print(char buff[256])
{
	sprintf(buff, printFormatString, 
         point.x, point.y, point.z, 
         radius, 
         maxDirs
         );
   return buff;
}


//---------------------------------------------------------------------------------------------
// This class helps with the temporary list management so that the main line algorithm is 
//    more to the point. 
// 

AIGraph::Node::TableBuildHelp::TableBuildHelp( int nodeListSz )
{
   AssertFatal( ! lastPassEntries, "aiGraph: only one table allowed at a time" );
   whichTable = 0;
   nodeCount = nodeListSz;
   NxNTracker = new FromTo [ nodeListSz ];
   FromTo   defaultFromTo;
   for( int i = 0; i < nodeListSz; i++ )
      NxNTracker[i] = defaultFromTo;
   nextPassEntries = & passEntries[0];
   lastPassEntries = & passEntries[1];
}

AIGraph::Node::TableBuildHelp::~TableBuildHelp()
{
   delete [] NxNTracker;
   lastPassEntries = nextPassEntries = NULL;
}

// Routine to prep for a pass of gossiping, if needed.  All entries that were 
//    queued up (on the seed pass or on the last pass) in the nextPass tables
//    must now be processed, so this becomes the new last pass.  
bool AIGraph::Node::TableBuildHelp::moreGossipNeeded()
{
   if( ! nextPassEntries->empty() )
   {
      AssertFatal( lastPassEntries->empty(), "aiGraph: last pass must be empty here" );
      lastPassEntries = nextPassEntries;
      nextPassEntries = & passEntries [ whichTable ^= 1 ];
      return true;
   }
   return false;
}

void AIGraph::Node::TableBuildHelp::checkBetterWay( int from, int to, int dir, float dist )
{
   int      index = getIndex( from, to );
   FromTo   & fromTo = NxNTracker[ index ];
   
   if( dist < fromTo.distance )
   {
      fromTo.distance = dist;
      fromTo.direction = dir;
      if( ! fromTo.marked )
      {
         fromTo.marked = true;
         nextPassEntries->push_back( index );
      }
   }
}

// Fetch (and remove) a piece of gossip off the last frame's list.  And return true
// if any was found.  If NONE was found, then we return false AND we set up the 
// lists for the next round of gossip.  
//
// This puts together all the information that comprises a piece of gossip (but which is
// is divided between the index and the FromTo entry):
//    From & To.  
//    Direction from From to get to To in least distance.  
//    Dist distance.  
bool AIGraph::Node::TableBuildHelp::getGossip( Gossip & gossip )
{
   if( ! lastPassEntries->empty() )
   {
      int  gIdx = lastPassEntries->last();
      lastPassEntries->pop_back();
      idx2FromTo( gIdx, gossip.From, gossip.To );
      FromTo   & fromTo = NxNTracker[ gIdx ];
      gossip.Dir = fromTo.direction;
      gossip.Dist = fromTo.distance;
      return true;
   }
   return false;
}


// Allocate a two-dimensional table of two-bit entries and fill with the directions from
// the NxN temporary table.  We're settling on 2 bit entries, 4 bit being a bit too much.
// [ 3-bit might actually be about perfect if such an array type were implemented. ]
void AIGraph::Node::TableBuildHelp::fillXRefTable( CrossRefTable & refTab )
{
   AssertFatal( nodeCount < AbsMaximumNodes, "aiGraph: too many nodes to build cross-"
                        "reference table of reasonable size." );
   AssertFatal( NxNTracker, "aiGraph: tracking table doesn't exist." );
   
   refTab.init( nodeCount );
   for( int from = 0; from < nodeCount; from++ )
   {
      for( int to = 0; to < nodeCount; to++ )
      {
         int   index = getIndex( from, to );
         refTab.set( index, NxNTracker[index].direction );
      }
   }
}


#endif