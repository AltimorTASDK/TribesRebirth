#ifndef SHAPEBUILDER_H_
#define SHAPEBUILDER_H_

#include "MeshBuilder.h"
#include "ts_shapeInst.h"
//--------------------------------------------------------------

#define OBTYPE_MESH 0

//--------------------------------------------------------------

struct IFL_INFO;

class ShapeBuilder
{
public:
	class Sequence
	{
	public:
		int			fName;
		int			fPriority;
		Bool			fCyclic;
		Bool 			fForcevis;
		Bool 			fVisOnly;
		Bool			fUseCelRate;
		int			fStartTime;
		int			fEndTime;
		int 			fFirstFrameTrigger;
		int			fNumFrameTriggers;

		Vector<int>	celKeyTimes;

		Sequence( int name, int start, int end )
		{	
			fName = name;
			fStartTime = start; 
			fEndTime = end; 
			fPriority = 0x1000;
			fCyclic = TRUE;
			fNumFrameTriggers = fFirstFrameTrigger = 0;
			fForcevis = fVisOnly = fUseCelRate = FALSE;
		}
      void SetCyclic( Bool cyclic ) { fCyclic = cyclic; }
		void SetForceVis( Bool forcevis ) { fForcevis = forcevis; }
		void SetVisOnly( Bool visonly ) { fVisOnly = visonly; }
		void SetUseCelRate( Bool useCel ) { fUseCelRate = useCel; }
		void AddCelKeyTime( int time )
		{ 
			celKeyTimes.push_back( time ); 
		}
		void GetCelTimes( int start, int end, Vector<int> * pTimes ) const
		{
			for( Vector<int>::const_iterator ti = celKeyTimes.begin(); 
				ti != celKeyTimes.end(); ti++ )
				if( *ti >= start && *ti <= end )
					pTimes->push_back( *ti );
		}
		int GetCelNumTimes() const
		{
			return celKeyTimes.size();
		}
		void AddFrameTrigger( int index )
		{
			if ( !fNumFrameTriggers++ )
				fFirstFrameTrigger = index;
		}
	};

	class Transition
	{
	public:
		int 	fName;
		int	fStartTime;
		int	fEndTime;
		float duration;
		Point3F gnd_shift;

		Transition( int name, int start, int end, float dur, Point3F *shft )
		{
			fName = name;
			fStartTime = start;
			fEndTime = end;
			gnd_shift = *shft;
			duration = dur;
		}
	};


	class Node
	{
	public:
		TS::Shape::Node snode;
		Point3F scale;
		int mirrored;

		Node( int name, int parent )
		{
			snode.fName = name;
			snode.fParent = parent;
			snode.fnSubSequences = 0;
			snode.fFirstSubSequence = 0;
			snode.fDefaultTransform = 0;
		}

		void SetDefaultTransform( ShapeBuilder & sb, MyTransform const & transform )
		{
			snode.fDefaultTransform = AddTransform( sb, transform );
		}

		int AddTransform( ShapeBuilder & sb, MyTransform const & transform )
		{
			return sb.AddTransform( transform );
		}

		void AddKey( ShapeBuilder & sb, float pos, int transform, Bool visible )
		{
//				TS::Shape::Keyframe key( pos, transform, visible );
			TS::Shape::Keyframe key( pos, transform, 0, FALSE, TRUE, FALSE, TRUE );
			sb.keyframes.push_back( key );
		}

		void AddSubSequence( ShapeBuilder & sb, int seq, int first, int count )
		{
			TS::Shape::SubSequence ss;
			ss.fSequenceIndex = seq;
			ss.fFirstKeyframe = first;
			ss.fnKeyframes = count;
			if( !snode.fnSubSequences )
				snode.fFirstSubSequence = sb.subsequences.size();
			snode.fnSubSequences++;
			sb.subsequences.push_back( ss );
		}
	};
   
   class Statistics 
   {
      public:
         int      num_sequences;
         int      num_transitions;
         int      num_subsequences;
         int      num_nodes;
         int      num_transforms;
         int      num_keyframes;
         int      num_names;
         int      num_meshes;
         int      num_materials;
         int      num_details;
         int      num_frameTriggers;
   };

      // Vectors of exporter-specific structures which converted to 3Space 
      // format on export. 
	VectorPtr<Sequence*> sequences;
	VectorPtr<Transition*> transitions;
	VectorPtr<Node*> nodes;
	Vector<MeshBuilder*> meshes;
      // These vectors just use the straight 3Space RTL format.  
	Vector<TS::Shape::SubSequence> subsequences;
	Vector<TS::Transform> transforms;
	Vector<TS::Shape::Keyframe> keyframes;
	Vector<TS::Name> names;    // these are names going out to dts
	Vector<TS::Name> in_names; // these are names coming in from max
	Vector<TS::Material> materials;
	Vector<TS::Shape::Detail> details;
	Vector<TS::Shape::FrameTrigger> frameTriggers;
	TS::MaterialList materialList;
   
	Point3F	   fCenter;
	RealF		   fRadius;
   Box3F       fBounds;       // version 8 addition
   Point3F     boundsBoxOffset;    //   hack save offset for bounds node offset problem. 
	FILE *fp;
	int 			alwaysNode;

	char * tweakName(char * name);
	int AddName( char *name );
	int FindName( char *name );

   int AddFrameTrigger( float pos, int val, Bool forward )
   {
      TS::Shape::FrameTrigger ft( pos, val, forward );
      frameTriggers.push_back( ft );
		return frameTriggers.size() - 1;
	}
	Sequence * AddSequence( char *name, int startTime, int endTime );
	Transition * AddTransition( char *name, int startTime, int endTime, float duration, Point3F *gnd_shift );
	Node * AddNode( char *name, char *parent );
	int FindNode( char *name );
	void DeleteNode( Node *delnode );
	int AddTransform( MyTransform const & xform );

	Vector<int> * GetCelTimes( int start, int end )
	{
		Vector<int> * pTimes = new Vector<int>;
		for( VectorPtr<Sequence*>::iterator si = sequences.begin(); 
			si != sequences.end(); si++ )
			(*si)->GetCelTimes( start, end, pTimes );
		return pTimes;
	}

	int AddKey( TS::Shape::Keyframe const & key )
	{
		keyframes.push_back( key );
		return keyframes.size() - 1;
	}

	TS::Shape * MakeShape();

	int AddMaterial( INode *, int );
	int AddStandardMaterial( StdMat * pMtl );
	int consider_mat( int mat_index, IFL_INFO *inf );

	void AddDetail( TS::Shape::Detail const & det );
   
   void dumpStatistics(const char * textStr = NULL);
   void captureStats( Statistics & stats );
   void getStatsDifference( Statistics & stats ) ;
   void dumpStats ( const Statistics & stats, const char * txt ) ;

	reset();

	ShapeBuilder();
   ~ShapeBuilder();
};

StdMat *getStdMaterial( INode * pNode, int mtlIndex );

typedef ShapeBuilder::Node SBNode;

//--------------------------------------------------------------
#endif
