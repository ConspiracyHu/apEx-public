#pragma once
//#include "phxarray.h"
//#include "phxEngine.h"
//#include "phxMath.h"
//
///*
//	Tree generator based on "Creation and Rendering of Realistic Trees" (Weber/Penn 1995)
//	http://www.cs.duke.edu/courses/fall02/cps124/resources/p119-weber.pdf
//	*/
//
//class CphxMesh;
//
//enum TREESHAPE : unsigned char
//{
//	flareTreeShape_Conical = 0,
//	flareTreeShape_Spherical = 1,
//	flareTreeShape_Hemispherical = 2,
//	flareTreeShape_Cylindrical = 3,
//	flareTreeShape_Tapered_Cylindrical = 4,
//	flareTreeShape_Flame = 5,
//	flareTreeShape_Inverse_Conical = 6,
//	flareTreeShape_Tend_Flame = 7,
//	flareTreeShape_Envelope = 8,
//};
//
//struct TREELEVELPARAMETERS
//{
//	unsigned char nBranches;	//0..inf
//	unsigned char nCurveRes;	//1..inf
//
//	unsigned char nTaper;		//0..2.99999
//	unsigned char nSplitAngle;	//0..180
//	unsigned char nSplitAngleV;	//0..180
//
//	unsigned char nBranchDist;	//0..1
//	short nRotate;				//-360..360
//	short nRotateV;				//-360..360
//	short nDownAngle;			//-179.999..179.999
//	short nDownAngleV;			//-179.999..179.999
//
//	short nCurve;				//-inf..inf
//	short nCurveV;				//-90..inf
//	short nCurveBack;			//-inf..inf
//
//	D3DXFLOAT16 nLength;		//0.00001..inf
//	D3DXFLOAT16 nLengthV;		//0..inf
//	D3DXFLOAT16 nSegSplits;		//0..inf
//};
//
//struct TREEPARAMETERS
//{
//	unsigned char Levels;		//0..9
//	unsigned char Lobes;		//0..inf
//	unsigned char _0BaseSplits;	//0..inf
//	unsigned char LeafShape;	//enum
//	TREESHAPE Shape;			//enum
//	TREESHAPE LeafDistrib;		//enum
//
//	unsigned char BaseSize;		//0..1
//	unsigned char LeafQuality;	//0.00001..1
//	unsigned char LeafBend;		//0..1
//	unsigned char Smooth;		//0..1
//	unsigned char Flare;		//-1..inf realFlare=Flare/25.5-1
//	unsigned char Ratio;		//0.00001..inf realRatio=Ratio/2048
//
//	char AttractionUp;			//-10..10
//	short Leaves;				//-inf..inf
//	char RatioPower;			//-inf..inf realRatioPower=RatioPower/32
//
//  unsigned char PruneRatio;
//  unsigned char PruneWidth;
//  unsigned char PruneWidthPeak;
//  D3DXFLOAT16 PrunePowerLow;
//  D3DXFLOAT16 PrunePowerHigh;
//
//	D3DXFLOAT16 LobeDepth;		//0..inf
//
//	D3DXFLOAT16 Scale;			//0.00001..inf
//	D3DXFLOAT16 _0Scale;		//0.00001..inf
//	D3DXFLOAT16 _0ScaleV;		//0..inf
//	D3DXFLOAT16 LeafScale;		//0.00001..inf
//	D3DXFLOAT16 LeafScaleX;		//0.00001..inf
//	D3DXFLOAT16 LeafStemLen;	//-inf..inf
//};
//
//struct TREESPECIESDESCRIPTOR
//{
//	TREELEVELPARAMETERS Levels[4];
//	TREEPARAMETERS Parameters;
//};
//
//struct TREETRANSFORMATION
//{
//	D3DXMATRIX Rotation;
//	D3DXVECTOR3 Position;
//	TREETRANSFORMATION();
//	TREETRANSFORMATION(D3DXMATRIX m, D3DXVECTOR3 v);
//	D3DXVECTOR3 apply(D3DXVECTOR3 p);
//	D3DXVECTOR3 getY();
//	D3DXVECTOR3 getZ();
//	void RotX(float angle);
//	TREETRANSFORMATION RotY(float angle);
//	void RotZ(float angle);
//	TREETRANSFORMATION translate(D3DXVECTOR3 v);
//	void RotAxisZ(float delta, float rho);
//	void RotAxis(float angle, D3DXVECTOR3 axis);
//	TREETRANSFORMATION ROtXZ(float delta, float rho);
//};
//
//class CphxTreeLeaf
//{
//public:
//
//	TREEPARAMETERS *TreeParams;
//	TREETRANSFORMATION transf;
//
//	CphxTreeLeaf(TREETRANSFORMATION &trf, TREEPARAMETERS *tree);
//	void BuildMesh(CphxMesh *Mesh);
//};
//
//class CphxTreeSegment;
//
//struct TREESUBSEGMENT
//{
//	D3DXVECTOR3 pos;
//	float rad;
//	CphxTreeSegment *segment;
//};
//
//class CphxTreeStem;
//
//class CphxTreeSegment
//{
//public:
//	CphxArray<TREESUBSEGMENT*> subsegments;
//
//	int index;
//	TREETRANSFORMATION transf;
//	float rad_1;
//	CphxTreeStem *stem;
//	TREEPARAMETERS *TreeParams;
//	TREELEVELPARAMETERS *lpar;
//	int Resolution;
//
//	CphxTreeSegment(CphxTreeStem *stm, int inx, TREETRANSFORMATION &trf, float r1, float r2);
//  ~CphxTreeSegment()
//  {
//    subsegments.FreeArray();
//  }
//
//	void BuildMesh(CphxMesh *Mesh, int &cntr);
//	void BuildSegment(CphxMesh *Mesh, int &cntr, float rad, D3DXVECTOR3 pos);
//};
//
//struct STEMDATA
//{
//	float length, segmentLength;
//	float baseRadius;
//	float splitCorrection;
//	float lengthChildMax;
//	float substemsPerSegment;
//	float substemRotangle;
//	float leavesPerSegment;
//};
//
//class CphxTreeGenerator;
//
//class CphxTreeStem
//{
//	CphxArray<CphxTreeSegment*> segments;
//	CphxArray<CphxTreeStem*> children;
//	CphxArray<CphxTreeLeaf*> leaves;
//
//public:
//	STEMDATA StemData;
//	CphxTreeGenerator *Tree;
//	TREEPARAMETERS *TreeParams;
//	CphxTreeStem *parent;
//	int stemlevel;
//	float offset;
//	TREELEVELPARAMETERS *lpar;
//	TREETRANSFORMATION transf;
//
//	CphxTreeStem(CphxTreeGenerator *Tree, CphxTreeStem *Parent, int Level, TREETRANSFORMATION &XForm, float Offset);
//  ~CphxTreeStem()
//  {
//    segments.FreeArray();
//    leaves.FreeArray();
//    children.FreeArray();
//  }
//
//	float stemRadius(float h);
//	TREETRANSFORMATION substemDirection(TREETRANSFORMATION trf, float offset);
//	TREETRANSFORMATION split(TREETRANSFORMATION trf, float s_angle, int nseg, int nsplits);
//
//	CphxTreeStem *Clone(TREETRANSFORMATION trf);
//
//	void Make();
//	void makeSegments(int startseg, int endseg);
//	void makeClones(TREETRANSFORMATION trf, int nseg);
//
//	void BuildMesh(CphxMesh *Mesh);
//	void BuildLeaves(CphxMesh *Mesh);
//};
//
//struct TREELEVELDATA
//{
//	float substemErrorValue;
//	float splitErrorValue;
//	int mesh_points;
//
//	TREELEVELDATA()
//	{
//		substemErrorValue = 0;
//		splitErrorValue = 0;
//		mesh_points = 3;
//	}
//};
//
//class CphxTreeGenerator
//{
//public:
//	CphxArray<CphxTreeStem*> Trunks;
//	TREESPECIESDESCRIPTOR Species;
//
//	TREELEVELDATA Levels[4];
//	float leavesErrorValue;
//
//	void MakeTree(unsigned int seed, TREESPECIESDESCRIPTOR &Species);
//	void BuildTree(CphxMesh *Mesh);
//	void BuildLeaves(CphxMesh *Mesh);
//
//  ~CphxTreeGenerator()
//  {
//    Trunks.FreeArray();
//  }
//};
//
///*
//
//Levels[0].ErrorValue=Levels[1].ErrorValue=Levels[2].ErrorValue=Levels[3].ErrorValue=0;
//
////Parameters.Levels=2;
//
////Parameters.Shape=flareTreeShape_Cylindrical;
//
////Parameters.Scale=1.0f;
////Parameters.ScaleV=0;
////Parameters._0Scale=1;
////Parameters.Ratio=0.03f;
////Levels[0].Length=3;
////Levels[0].LengthV=0;
////Levels[0].Taper=1;
////Levels[0].CurveRes=10;
////Parameters.TrunkBaseSplits=0;
////Levels[0].SplitAngle=45;
////Levels[0].SplitAngleV=0;
////Levels[0].SegSplits=0;
////Levels[0].Curve=45;
////Levels[0].CurveV=0;
////Levels[0].CurveBack=-45;
////Parameters.BaseSize=0;
////Parameters.RatioPower=1;
//
////Levels[1].Branches=11;
////Levels[1].Length=0.5;
////Levels[1].LengthV=0;
////Levels[1].DownAngle=90;
////Levels[1].DownAngleV=0;
////Levels[1].Rotate=10;
////Levels[1].RotateV=0;
////Levels[1].Taper=1;
////Levels[1].SegSplits=0;
////Levels[1].SplitAngle=0;
////Levels[1].SplitAngleV=0;
////Levels[1].CurveRes=3;
////Levels[1].Curve=0;
////Levels[1].CurveBack=0;
////Levels[1].CurveV=0;
//
//
////black tupelo
////Parameters.Shape=flareTreeShape_Tapered_Cylindrical;
////Parameters.BaseSize=0.2;
//
////Parameters.Scale=23;
////Parameters.ScaleV=5;
////Parameters.ZScale=1;
////Parameters.ZScaleV=0;
//
////Parameters.Levels=4;
//
////Parameters.Ratio=0.015f;
////Parameters.RatioPower=1.3;
////Parameters.Lobes=3;
////Parameters.LobeDepth=0.1;
////Parameters._0Scale=1;
////Parameters._0ScaleV=0;
////
////Levels[0].Length=1;
////Levels[0].LengthV=0;
////Levels[0].Taper=1.1;
////Parameters.TrunkBaseSplits=0;
//
////Levels[0].SegSplits=0;
////Levels[0].SplitAngle=0;
////Levels[0].SplitAngleV=0;
//
////Levels[0].CurveRes=10;
////Levels[0].Curve=0;
////Levels[0].CurveBack=0;
////Levels[0].CurveV=30;
//
////Levels[1].DownAngle=60;
////Levels[1].DownAngleV=-40;
////Levels[1].Rotate=140;
////Levels[1].RotateV=0;
////Levels[1].Branches=50;
////Levels[1].Length=0.3;
////Levels[1].LengthV=0.05;
////Levels[1].Taper=1;
////Levels[1].SegSplits=0;
////Levels[1].SplitAngle=0;
////Levels[1].SplitAngleV=0;
////Levels[1].CurveRes=10;
////Levels[1].Curve=0;
////Levels[1].CurveBack=0;
////Levels[1].CurveV=90;
//
////Levels[2].DownAngle=30;
////Levels[2].DownAngleV=10;
////Levels[2].Rotate=140;
////Levels[2].RotateV=0;
////Levels[2].Branches=25;
////Levels[2].Length=0.6;
////Levels[2].LengthV=0.1;
////Levels[2].Taper=1;
////Levels[2].SegSplits=0;
////Levels[2].SplitAngle=0;
////Levels[2].SplitAngleV=0;
////Levels[2].CurveRes=10;
////Levels[2].Curve=-10;
////Levels[2].CurveBack=0;
////Levels[2].CurveV=150;
//
////Levels[3].DownAngle=45;
////Levels[3].DownAngleV=10;
////Levels[3].Rotate=140;
////Levels[3].RotateV=0;
////Levels[3].Branches=12;
////Levels[3].Length=0.4;
////Levels[3].LengthV=0;
////Levels[3].Taper=1;
////Levels[3].SegSplits=0;
////Levels[3].SplitAngle=0;
////Levels[3].SplitAngleV=0;
////Levels[3].CurveRes=1;
////Levels[3].Curve=0;
////Levels[3].CurveBack=0;
////Levels[3].CurveV=0;
//
////quaking aspen
////Parameters.Shape=flareTreeShape_Tend_Flame;
////
////Parameters.Levels=3;
////Parameters.scale_tree=13;
//////Parameters.ScaleV=3.0f;
////Parameters.BaseSize=0.4f;
////Parameters._0BaseSplits=0;
////Parameters.RatioPower=1.2;
////Parameters.AttractionUp=0.5;
////
////Parameters.Ratio=0.015f;
////Parameters.Flare=0.6;
////Parameters.Lobes=5;
////Parameters.LobeDepth=0.07;
////Parameters._0Scale=1;
//////Parameters._0ScaleV=0.2;
////
//////Parameters.Leaves=25;
//////Parameters.LeafShape=0;
//////Parameters.LeafScale=0.17;
//////Parameters.LeafScaleX=1.0;
//////Parameters.LeafBend=0.3;
//////Parameters.LeafStemLen=0.5;
//////Parameters.LeafDistrib=4;
////
//////Parameters.PruneRatio=0;
////Parameters.PruneWidth=0.5;
//////Parameters.PruneWidthPeak=0.5;
//////Parameters.PrunePowerLow=0.5;
//////Parameters.PrunePowerHigh=0.5;
////
//////Parameters.LeafQuality=1;
//////Parameters.Smooth=0.5;
////
//////Parameters.ZScale=1;
//////Parameters.ZScaleV=0;
////
//////level 0
////Levels[0].nLength=1;
////Levels[0].nLengthV=0;
////Levels[0].nTaper=1;
////
////Levels[0].nCurveRes=3;
////Levels[0].nCurve=0;
////Levels[0].nCurveV=20.0;
////Levels[0].nCurveBack=0;
////
////Levels[0].nSegSplits=0;
////Levels[0].nSplitAngle=0;
////Levels[0].nSplitAngleV=0;
////
////Levels[0].nDownAngle=0;
////Levels[0].nDownAngleV=0;
////Levels[0].nRotate=0;
////Levels[0].nRotateV=0;
////Levels[0].nBranches=1;
////Levels[0].nBranchDist=0;
////
//////level 1
////Levels[1].nLength=0.3f;
////Levels[1].nLengthV=0;
////Levels[1].nTaper=1.0;
////
////Levels[1].nCurveRes=5;
////Levels[1].nCurve=-40;
////Levels[1].nCurveV=50;
////Levels[1].nCurveBack=0;
////
////Levels[1].nSegSplits=0;
////Levels[1].nSplitAngle=0;
////Levels[1].nSplitAngleV=0;
////
////Levels[1].nDownAngle=60;
////Levels[1].nDownAngleV=-50;
////Levels[1].nRotate=140;
////Levels[1].nRotateV=0;
////Levels[1].nBranches=50;
////Levels[1].nBranchDist=1;
////
//////level 2
////Levels[2].nLength=0.6f;
////Levels[2].nLengthV=0;
////Levels[2].nTaper=1;
////
////Levels[2].nCurveRes=3;
////Levels[2].nCurve=-40;
////Levels[2].nCurveV=75;
////Levels[2].nCurveBack=0;
////
////Levels[2].nSegSplits=0;
////Levels[2].nSplitAngle=0;
////Levels[2].nSplitAngleV=0;
////
////Levels[2].nDownAngle=45;
////Levels[2].nDownAngleV=10;
////Levels[2].nRotate=140;
////Levels[2].nRotateV=0;
////Levels[2].nBranches=30;
////Levels[2].nBranchDist=1;
//
//////black oak
////Parameters.Shape=flareTreeShape_Hemispherical;
////Parameters.BaseSize=0.05;
//
////Parameters.Scale=10;
////Parameters.ScaleV=0;
////Parameters.ZScale=1;
////Parameters.ZScaleV=0;
//
////Parameters.Levels=2;
//
////Parameters.Ratio=0.018f;
////Parameters.RatioPower=1.3;
////Parameters.Lobes=5;
////Parameters.LobeDepth=0.1;
////Parameters._0Scale=1;
////Parameters._0ScaleV=0;
//
////Levels[0].Length=1;
////Levels[0].LengthV=0;
////Levels[0].Taper=0.95;
////Parameters.TrunkBaseSplits=2;
//
////Levels[0].SegSplits=0.4;
////Levels[0].SplitAngle=10;
////Levels[0].SplitAngleV=0;
//
////Levels[0].CurveRes=8;
////Levels[0].Curve=0;
////Levels[0].CurveBack=0;
////Levels[0].CurveV=90;
//
////Levels[1].DownAngle=30;
////Levels[1].DownAngleV=-30;
////Levels[1].Rotate=80;
////Levels[1].RotateV=0;
////Levels[1].Branches=40;
////Levels[1].Length=0.8;
////Levels[1].LengthV=0.1;
////Levels[1].Taper=1;
////Levels[1].SegSplits=0.2;
////Levels[1].SplitAngle=10;
////Levels[1].SplitAngleV=10;
////Levels[1].CurveRes=10;
////Levels[1].Curve=40;
////Levels[1].CurveBack=-70;
////Levels[1].CurveV=150;
//
////Levels[2].DownAngle=45;
////Levels[2].DownAngleV=10;
////Levels[2].Rotate=140;
////Levels[2].RotateV=0;
////Levels[2].Branches=120;
////Levels[2].Length=0.2;
////Levels[2].LengthV=0.05;
////Levels[2].Taper=1;
////Levels[2].SegSplits=0.1;
////Levels[2].SplitAngle=10;
////Levels[2].SplitAngleV=10;
////Levels[2].CurveRes=3;
////Levels[2].Curve=0;
////Levels[2].CurveBack=0;
////Levels[2].CurveV=-30;
//
////weeping willow
//Parameters.Shape=flareTreeShape_Conical;
//Parameters.Levels=2;
//Parameters.Scale=10;
//Parameters.ScaleV=0;
//Parameters.BaseSize=0.25f;
//Parameters.TrunkBaseSplits=0;
//Parameters.RatioPower=1;
//Parameters.AttractionUp=0;
//
//Parameters.Ratio=0.05f;
//Parameters.Flare=0.5;
//Parameters.Lobes=0;
//Parameters.LobeDepth=0;
//Parameters._0Scale=1;
//Parameters._0ScaleV=0;
//
//Parameters.Leaves=0;
//Parameters.LeafShape=0;
//Parameters.LeafScale=0.2;
//Parameters.LeafScaleX=0.5;
////Parameters.LeafBend=0.3;
////Parameters.LeafStemLen=0.5;
////Parameters.LeafDistrib=4;
//
//Parameters.PruneRatio=0;
//Parameters.PruneWidth=0.5;
//Parameters.PruneWidthPeak=0.5;
//Parameters.PrunePowerLow=0.5;
//Parameters.PrunePowerHigh=0.5;
//
////Parameters.LeafQuality=1;
////Parameters.Smooth=0.5;
//
////Parameters.ZScale=1;
////Parameters.ZScaleV=0;
//
////level 0
//Levels[0].Length=1;
//Levels[0].LengthV=0;
//Levels[0].Taper=1;
//
//Levels[0].CurveRes=3;
//Levels[0].Curve=0;
//Levels[0].CurveV=0;
//Levels[0].CurveBack=0;
//
//Levels[0].SegSplits=0;
//Levels[0].SplitAngle=0;
//Levels[0].SplitAngleV=0;
//
//Levels[0].DownAngle=0;
//Levels[0].DownAngleV=0;
//Levels[0].Rotate=0;
//Levels[0].RotateV=0;
//Levels[0].Branches=1;
////Levels[0].BranchDist=0;
//
////level 1
//Levels[1].Length=0.5f;
//Levels[1].LengthV=0;
//Levels[1].Taper=1;
//
//Levels[1].CurveRes=3;
//Levels[1].Curve=0;
//Levels[1].CurveV=0;
//Levels[1].CurveBack=0;
//
//Levels[1].SegSplits=0;
//Levels[1].SplitAngle=0;
//Levels[1].SplitAngleV=0;
//
//Levels[1].DownAngle=30;
//Levels[1].DownAngleV=0;
//Levels[1].Rotate=120;
//Levels[1].RotateV=0;
//Levels[1].Branches=10;
////Levels[1].BranchDist=1;
//
////level 2
//Levels[2].Length=0.5f;
//Levels[2].LengthV=0;
//Levels[2].Taper=1;
//
//Levels[2].CurveRes=1;
//Levels[2].Curve=0;
//Levels[2].CurveV=0;
//Levels[2].CurveBack=0;
//
//Levels[2].SegSplits=0;
//Levels[2].SplitAngle=0;
//Levels[2].SplitAngleV=0;
//
//Levels[2].DownAngle=30;
//Levels[2].DownAngleV=0;
//Levels[2].Rotate=120;
//Levels[2].RotateV=0;
//Levels[2].Branches=5;
////Levels[2].BranchDist=1;
//
////default test
//Parameters.Shape=flareTreeShape_Conical;
//
//Parameters.Levels=2;
//Parameters.scale_tree=10;
////Parameters.ScaleV=0.0f;
//Parameters.BaseSize=0.25f;
//Parameters._0BaseSplits=0;
//Parameters.RatioPower=1;
//Parameters.AttractionUp=0;
//
//Parameters.Ratio=0.05f;
//Parameters.Flare=0.5;
//Parameters.Lobes=0;
//Parameters.LobeDepth=0;
//Parameters._0Scale=1;
////Parameters._0ScaleV=0;
//
////Parameters.Leaves=25;
////Parameters.LeafShape=0;
////Parameters.LeafScale=0.17;
////Parameters.LeafScaleX=1.0;
////Parameters.LeafBend=0.3;
////Parameters.LeafStemLen=0.5;
////Parameters.LeafDistrib=4;
//
////Parameters.PruneRatio=0;
//Parameters.PruneWidth=0.5;
////Parameters.PruneWidthPeak=0.5;
////Parameters.PrunePowerLow=0.5;
////Parameters.PrunePowerHigh=0.5;
//
////Parameters.LeafQuality=1;
////Parameters.Smooth=0.5;
//
////level 0
//Levels[0].nLength=1;
//Levels[0].nLengthV=0;
//Levels[0].nTaper=1;
//
//Levels[0].nCurveRes=3;
//Levels[0].nCurve=0;
//Levels[0].nCurveV=0.0;
//Levels[0].nCurveBack=0;
//
//Levels[0].nSegSplits=0;
//Levels[0].nSplitAngle=0;
//Levels[0].nSplitAngleV=0;
//
//Levels[0].nDownAngle=0;
//Levels[0].nDownAngleV=0;
//Levels[0].nRotate=0;
//Levels[0].nRotateV=0;
//Levels[0].nBranches=1;
//Levels[0].nBranchDist=0;
//
////level 1
//Levels[1].nLength=0.5f;
//Levels[1].nLengthV=0;
//Levels[1].nTaper=1.0;
//
//Levels[1].nCurveRes=3;
//Levels[1].nCurve=0;
//Levels[1].nCurveV=0;
//Levels[1].nCurveBack=0;
//
//Levels[1].nSegSplits=0;
//Levels[1].nSplitAngle=0;
//Levels[1].nSplitAngleV=0;
//
//Levels[1].nDownAngle=30;
//Levels[1].nDownAngleV=0;
//Levels[1].nRotate=120;
//Levels[1].nRotateV=0;
//Levels[1].nBranches=10;
//Levels[1].nBranchDist=1;
//
////level 2
//Levels[2].nLength=0.5f;
//Levels[2].nLengthV=0;
//Levels[2].nTaper=1;
//
//Levels[2].nCurveRes=1;
//Levels[2].nCurve=0;
//Levels[2].nCurveV=0;
//Levels[2].nCurveBack=0;
//
//Levels[2].nSegSplits=0;
//Levels[2].nSplitAngle=0;
//Levels[2].nSplitAngleV=0;
//
//Levels[2].nDownAngle=30;
//Levels[2].nDownAngleV=0;
//Levels[2].nRotate=120;
//Levels[2].nRotateV=0;
//Levels[2].nBranches=5;
//Levels[2].nBranchDist=1;
//
////desert bush
//Parameters.Shape=flareTreeShape_Conical;
//
//Parameters.Levels=3;
//Parameters.scale_tree=3;
////Parameters.ScaleV=1.0f;
//Parameters.BaseSize=0.1f;
//Parameters._0BaseSplits=0;
//Parameters.RatioPower=1;
//Parameters.AttractionUp=2;
//
//Parameters.Ratio=0.03f;
//Parameters.Flare=0.2;
//Parameters.Lobes=10;
//Parameters.LobeDepth=0.01;
//Parameters._0Scale=1;
////Parameters._0ScaleV=0;
//
////Parameters.Leaves=25;
////Parameters.LeafShape=0;
////Parameters.LeafScale=0.17;
////Parameters.LeafScaleX=1.0;
////Parameters.LeafBend=0.3;
////Parameters.LeafStemLen=0.5;
////Parameters.LeafDistrib=4;
//
////Parameters.PruneRatio=0;
//Parameters.PruneWidth=0.5;
////Parameters.PruneWidthPeak=0.5;
////Parameters.PrunePowerLow=0.5;
////Parameters.PrunePowerHigh=0.5;
//
////Parameters.LeafQuality=1;
////Parameters.Smooth=0.5;
//
////Parameters.ZScale=1;
////Parameters.ZScaleV=0;
//
////level 0
//Levels[0].nLength=1;
//Levels[0].nLengthV=0;
//Levels[0].nTaper=1;
//
//Levels[0].nCurveRes=17;
//Levels[0].nCurve=10;
//Levels[0].nCurveV=360.0;
//Levels[0].nCurveBack=-90;
//
//Levels[0].nSegSplits=0.05;
//Levels[0].nSplitAngle=3;
//Levels[0].nSplitAngleV=0;
//
//Levels[0].nDownAngle=0;
//Levels[0].nDownAngleV=0;
//Levels[0].nRotate=0;
//Levels[0].nRotateV=0;
//Levels[0].nBranches=1;
//Levels[0].nBranchDist=0;
//
////level 1
//Levels[1].nLength=0.8f;
//Levels[1].nLengthV=0.4f;
//Levels[1].nTaper=1.03;
//
//Levels[1].nCurveRes=17;
//Levels[1].nCurve=30;
//Levels[1].nCurveV=480;
//Levels[1].nCurveBack=-20;
//
//Levels[1].nSegSplits=0.25;
//Levels[1].nSplitAngle=5;
//Levels[1].nSplitAngleV=10;
//
//Levels[1].nDownAngle=60;
//Levels[1].nDownAngleV=30;
//Levels[1].nRotate=-90;
//Levels[1].nRotateV=30;
//Levels[1].nBranches=9;
//Levels[1].nBranchDist=1;
//
////level 2
//Levels[2].nLength=0.3f;
//Levels[2].nLengthV=0.2f;
//Levels[2].nTaper=1;
//
//Levels[2].nCurveRes=12;
//Levels[2].nCurve=0;
//Levels[2].nCurveV=480;
//Levels[2].nCurveBack=0;
//
//Levels[2].nSegSplits=0.2;
//Levels[2].nSplitAngle=10;
//Levels[2].nSplitAngleV=10;
//
//Levels[2].nDownAngle=40;
//Levels[2].nDownAngleV=-60;
//Levels[2].nRotate=-90;
//Levels[2].nRotateV=30;
//Levels[2].nBranches=40;
//Levels[2].nBranchDist=1;
//
////wheat
//Parameters.Shape=flareTreeShape_Cylindrical;
//
//Parameters.Levels=2;
//Parameters.scale_tree=0.6;
////Parameters.ScaleV=0.1f;
//Parameters.BaseSize=0.85f;
//Parameters._0BaseSplits=0;
//Parameters.RatioPower=0;
//Parameters.AttractionUp=0;
//
//Parameters.Ratio=0.002f;
//Parameters.Flare=0;
//Parameters.Lobes=0;
//Parameters.LobeDepth=0;
//Parameters._0Scale=1;
////Parameters._0ScaleV=0;
//
////Parameters.Leaves=-1;
////Parameters.LeafShape=sphere;
////Parameters.LeafScale=0.01;
////Parameters.LeafScaleX=0.5;
////Parameters.LeafBend=0.0;
////Parameters.LeafStemLen=-1.2;
////Parameters.LeafDistrib=3;
//
////Parameters.PruneRatio=0;
//Parameters.PruneWidth=0.5;
////Parameters.PruneWidthPeak=0.5;
////Parameters.PrunePowerLow=0.5;
////Parameters.PrunePowerHigh=0.5;
//
////Parameters.LeafQuality=1;
//Parameters.Smooth=0.1;
//
////Parameters.ZScale=1;
////Parameters.ZScaleV=0;
//
////level 0
//Levels[0].nLength=1;
//Levels[0].nLengthV=0;
//Levels[0].nTaper=0.5;
//
//Levels[0].nCurveRes=13;
//Levels[0].nCurve=0;
//Levels[0].nCurveV=20.0;
//Levels[0].nCurveBack=30;
//
//Levels[0].nSegSplits=0;
//Levels[0].nSplitAngle=0;
//Levels[0].nSplitAngleV=0;
//
//Levels[0].nDownAngle=0;
//Levels[0].nDownAngleV=40;
//Levels[0].nRotate=0;
//Levels[0].nRotateV=80;
//Levels[0].nBranches=49;
//Levels[0].nBranchDist=0.1;
//
////level 1
//Levels[1].nLength=0.025f;
//Levels[1].nLengthV=0.0f;
//Levels[1].nTaper=0.5f;
//
//Levels[1].nCurveRes=3;
//Levels[1].nCurve=0;
//Levels[1].nCurveV=0;
//Levels[1].nCurveBack=20;
//
//Levels[1].nSegSplits=0;
//Levels[1].nSplitAngle=0;
//Levels[1].nSplitAngleV=0;
//
//Levels[1].nDownAngle=25;
//Levels[1].nDownAngleV=-5;
//Levels[1].nRotate=120;
//Levels[1].nRotateV=10;
//Levels[1].nBranches=35;
//Levels[1].nBranchDist=1;
//
////weeping willow
//Parameters.Shape=flareTreeShape_Cylindrical;
//
//Parameters.Levels=4;
//Parameters.scale_tree=15;
////Parameters.ScaleV=5.0f;
//Parameters.BaseSize=0.05f;
//Parameters._0BaseSplits=2;
//Parameters.RatioPower=2.0;
//Parameters.AttractionUp=-2.0;
//
//Parameters.Ratio=0.03f;
//Parameters.Flare=0.75;
//Parameters.Lobes=9;
//Parameters.LobeDepth=0.03;
//Parameters._0Scale=1;
//Parameters._0ScaleV=0;
//
//Parameters.Leaves=10;
////Parameters.LeafShape=disc2;
////Parameters.LeafScale=0.012;
////Parameters.LeafScaleX=0.2;
////Parameters.LeafBend=0.3;
////Parameters.LeafStemLen=0.5;
////Parameters.LeafDistrib=3;
//
////Parameters.PruneRatio=1.0;
//Parameters.PruneWidth=0.4;
////Parameters.PruneWidthPeak=0.6;
////Parameters.PrunePowerLow=0.001;
////Parameters.PrunePowerHigh=0.5;
//
////Parameters.LeafQuality=1;
//Parameters.Smooth=0.5;
//
////Parameters.ZScale=1;
////Parameters.ZScaleV=0;
//
////level 0
//Levels[0].nLength=0.8;
//Levels[0].nLengthV=0;
//Levels[0].nTaper=1;
//
//Levels[0].nCurveRes=8;
//Levels[0].nCurve=0;
//Levels[0].nCurveV=120.0;
//Levels[0].nCurveBack=20;
//
//Levels[0].nSegSplits=0.1;
//Levels[0].nSplitAngle=3;
//Levels[0].nSplitAngleV=0;
//
//Levels[0].nDownAngle=0;
//Levels[0].nDownAngleV=0;
//Levels[0].nRotate=0;
//Levels[0].nRotateV=0;
//Levels[0].nBranches=1;
//Levels[0].nBranchDist=0;
//
////level 1
//Levels[1].nLength=0.5f;
//Levels[1].nLengthV=0.1f;
//Levels[1].nTaper=1.0f;
//
//Levels[1].nCurveRes=16;
//Levels[1].nCurve=40;
//Levels[1].nCurveV=90;
//Levels[1].nCurveBack=80;
//
//Levels[1].nSegSplits=0.2;
//Levels[1].nSplitAngle=30;
//Levels[1].nSplitAngleV=10;
//
//Levels[1].nDownAngle=20;
//Levels[1].nDownAngleV=10;
//Levels[1].nRotate=-120;
//Levels[1].nRotateV=30;
//Levels[1].nBranches=25;
//Levels[1].nBranchDist=1;
//
////level 2
//Levels[2].nLength=1.5f;
//Levels[2].nLengthV=0;
//Levels[2].nTaper=1.0f;
//
//Levels[2].nCurveRes=12;
//Levels[2].nCurve=0;
//Levels[2].nCurveV=0;
//Levels[2].nCurveBack=0;
//
//Levels[2].nSegSplits=0.2;
//Levels[2].nSplitAngle=45;
//Levels[2].nSplitAngleV=20;
//
//Levels[2].nDownAngle=30;
//Levels[2].nDownAngleV=10;
//Levels[2].nRotate=-120;
//Levels[2].nRotateV=30;
//Levels[2].nBranches=10;
//Levels[2].nBranchDist=1;
//
////level 3
//Levels[3].nLength=0.1f;
//Levels[3].nLengthV=0;
//Levels[3].nTaper=1.0f;
//
//Levels[3].nCurveRes=1;
//Levels[3].nCurve=0;
//Levels[3].nCurveV=0;
//Levels[3].nCurveBack=0;
//
//Levels[3].nSegSplits=0;
//Levels[3].nSplitAngle=0;
//Levels[3].nSplitAngleV=0;
//
//Levels[3].nDownAngle=20;
//Levels[3].nDownAngleV=10;
//Levels[3].nRotate=140;
//Levels[3].nRotateV=0;
//Levels[3].nBranches=150;
//Levels[3].nBranchDist=1;
//
//
//
//flareTreeTransformation Transform;
//Transform.Position=D3DXVECTOR3(0,0,0);
//D3DXMatrixIdentity(&Transform.Rotation);
//
//ScaleTree=GetVariedValue(Parameters.Scale,Parameters.ScaleV); //meters
//LengthBase=Parameters.BaseSize*ScaleTree;
//
//LengthTrunk=GetVariedValue(Levels[0].Length,Levels[0].LengthV)*ScaleTree;
//float RadiusTrunk=LengthTrunk*Parameters.Ratio*Parameters._0Scale;
//
//Trunk=new flareTreeStem(this,NULL,0,Transform,0,RadiusTrunk,LengthTrunk,0,0);
//
//Trunk->BuildMesh(Mesh);
//Mesh->SmoothGroupSeparation=2;
//
//*/
//
