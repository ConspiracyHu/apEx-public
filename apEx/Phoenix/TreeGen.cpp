#include "TreeGen.h"
//#include "Mesh.h"
////#define MIN_STEM_LEN 0.0005 
////#define MIN_STEM_RADIUS MIN_STEM_LEN/10
//
//#pragma warning( disable : 4244 )
//
//
//float GetVariedValue(float base, float variation)
//{
//	return base + variation*((rand() / (float)RAND_MAX) * 2 - 1);
//}
//
//int PropagateError(float &err, float Val)
//{
//	int eff = (int)(Val + err + 0.5);
//	err -= (eff - Val);
//	return eff;
//}
//
//TREETRANSFORMATION::TREETRANSFORMATION()
//{
//	D3DXMatrixIdentity(&Rotation);
//	Position = D3DXVECTOR3(0, 0, 0);
//}
//
//TREETRANSFORMATION::TREETRANSFORMATION(D3DXMATRIX m, D3DXVECTOR3 v)
//{
//	Rotation = m;
//	Position = v;
//}
//
//D3DXVECTOR3 TREETRANSFORMATION::apply(D3DXVECTOR3 p)
//{
//	D3DXVECTOR3 v;
//	D3DXMATRIX rm;
//	D3DXMatrixTranspose(&rm, &Rotation);
//	D3DXVec3TransformNormal(&v, &p, &rm);
//	v = v + Position;
//	return D3DXVECTOR3(v.x, v.z, v.y); //<-!!!!!!!! this swaps y and z!!!
//}
//
//D3DXVECTOR3 TREETRANSFORMATION::getY()
//{
//	return D3DXVECTOR3(Rotation.m[0][1], Rotation.m[1][1], Rotation.m[2][1]);
//}
//
//D3DXVECTOR3 TREETRANSFORMATION::getZ()
//{
//	return D3DXVECTOR3(Rotation.m[0][2], Rotation.m[1][2], Rotation.m[2][2]);
//}
//
//void TREETRANSFORMATION::RotX(float angle)
//{
//	D3DXMATRIX rm;
//	D3DXMatrixMultiply(&Rotation, &Rotation, D3DXMatrixRotationX(&rm, -angle));
//}
//
//TREETRANSFORMATION TREETRANSFORMATION::RotY(float angle)
//{
//	D3DXMATRIX rm;
//	D3DXMatrixMultiply(&rm, &Rotation, D3DXMatrixRotationY(&rm, -angle));
//	return TREETRANSFORMATION(rm, Position);
//}
//
//void TREETRANSFORMATION::RotZ(float angle)
//{
//	D3DXMATRIX rm;
//	D3DXMatrixMultiply(&Rotation, &Rotation, D3DXMatrixRotationZ(&rm, -angle));
//}
//
//TREETRANSFORMATION TREETRANSFORMATION::translate(D3DXVECTOR3 v)
//{
//	return TREETRANSFORMATION(Rotation, Position + v);
//}
//
//void TREETRANSFORMATION::RotAxisZ(float delta, float rho)
//{
//	RotAxis(delta, D3DXVECTOR3(cos(rho), sin(rho), 0));
//}
//
//void TREETRANSFORMATION::RotAxis(float angle, D3DXVECTOR3 axis)
//{
//	D3DXMATRIX rm;
//	D3DXMatrixMultiply(&Rotation, &Rotation, D3DXMatrixRotationAxis(&rm, &axis, -angle));
//}
//
//TREETRANSFORMATION TREETRANSFORMATION::ROtXZ(float delta, float rho)
//{
//	D3DXMATRIX rx, rz, rm;
//	D3DXMatrixMultiply(&rm, &Rotation, D3DXMatrixMultiply(&rm, D3DXMatrixRotationZ(&rx, -rho), D3DXMatrixRotationX(&rz, -delta)));
//	return TREETRANSFORMATION(rm, Position);
//}
//
//CphxTreeLeaf::CphxTreeLeaf(TREETRANSFORMATION &trf, TREEPARAMETERS *treep)
//{
//	transf = trf;
//	TreeParams = treep;
//	if ((float)TreeParams->LeafBend == 0) return;
//
//	D3DXVECTOR3 norm = transf.getY();
//
//	float tbend = atan2(transf.Position.y, transf.Position.x) - atan2(norm.y, norm.x);
//	transf.RotZ(TreeParams->LeafBend / 255.0f*tbend);
//
//	norm = transf.getY();
//
//	float fbend = atan2(sqrt(norm.x*norm.x + norm.y*norm.y), norm.z);
//	transf.RotX(TreeParams->LeafBend / 255.0f*fbend);
//}
//
//void CphxTreeLeaf::BuildMesh(CphxMesh *Mesh)
//{
//	int vxc = Mesh->Vertices.NumItems();
//
//	char v[4][2] = { { -1, 0 }, { 1, 0 }, { 1, 2 }, { -1, 2 } };
//	for (int x = 0; x < 4; x++)
//		Mesh->AddVertex(transf.apply(D3DXVECTOR3(v[x][0] * TreeParams->LeafScaleX, 0, v[x][1] + TreeParams->LeafStemLen * 2)*TreeParams->LeafScale*0.5));
//
//	Mesh->AddPolygon(vxc + 0, vxc + 1, vxc + 2, vxc + 3, D3DXVECTOR2(0, 0), D3DXVECTOR2(1, 0), D3DXVECTOR2(1, 1), D3DXVECTOR2(0, 1));
//}
//
//CphxTreeSegment::CphxTreeSegment(CphxTreeStem *stm, int inx, TREETRANSFORMATION &trf, float r1, float r2)
//{
//	index = inx;
//	transf = trf;
//	rad_1 = r1;
//	stem = stm;
//	lpar = stem->lpar;
//	TreeParams = stem->TreeParams;
//	Resolution = stem->Tree->Levels[stem->stemlevel].mesh_points;
//
//	int cnt = 2;
//	int typ = 0;
//
//	//if (lpar->nCurveV<0) //helix not implemented
//	//{ 
//	//	typ=3;
//	//	cnt=11;
//	//}
//
//	if (lpar->nTaper / 255.0f * 3 > 1 && lpar->nTaper / 255.0f * 3 <= 2 && index == lpar->nCurveRes - 1)
//	{
//		typ = 2;
//		cnt = 10;
//	}
//
//	if (lpar->nTaper / 255.0f * 3 > 2) cnt = 21;
//
//	if (stem->stemlevel == 0 && index == 0)
//	{
//		cnt = 10;
//		typ = 1;
//	}
//
//	D3DXVECTOR3 dir = transf.getZ();
//	for (int i = 1; i < cnt; i++)
//	{
//		TREESUBSEGMENT *n = new TREESUBSEGMENT;
//		subsegments.Add(n);
//		n->segment = this;
//
//		float pos = i / (float)(cnt - 1); //make sub segments
//
//		if (typ == 1) pos = 1 / pow(2.0f, cnt - i); //make flare
//		if (typ == 2) pos = 1 - 1 / pow(2.0f, i); //make spherical end
//
//		//switch (typ)
//		//{
//		////case 3: //make helix
//		////	{
//		////		float angle = abs(lpar->nCurveV)/180*pi;
//		////		float rad = sqrt(1.0f/(cos(angle)*cos(angle))-1)*stem->segmentLength/pi/2.0f;
//
//		////		float an=2*pi*i/(cnt-1);
//
//		////		n->rad=stem->stemRadius(index*stem->segmentLength + i*stem->segmentLength/(cnt-1));
//		////		n->pos=transf.apply(D3DXVECTOR3(rad*(cos(an)-1),rad*sin(an),i*stem->segmentLength/(cnt-1)));
//		////		//apply swapped the axes
//		////		n->pos=D3DXVECTOR3(n->pos.x,n->pos.z,n->pos.y); 
//		////	}
//		////	break;
//		//}
//
//		//if (typ<3) //needed only if helix is in
//		{
//			pos *= stem->StemData.segmentLength;
//			n->rad = stem->stemRadius(index*stem->StemData.segmentLength + pos);
//			n->pos = transf.Position + dir*pos;
//		}
//	}
//
//	if (typ == 2)
//	{
//		TREESUBSEGMENT *n = new TREESUBSEGMENT();
//		subsegments.Add(n);
//		n->segment = this;
//		n->rad = r2;
//		n->pos = transf.Position + dir*stem->StemData.segmentLength;
//	}
//}
//
//float ShapeRatio(TREESHAPE Shape, float Ratio)
//{
//	switch (Shape)
//	{
//		case flareTreeShape_Conical: return 0.2f + 0.8f*Ratio;
//		case flareTreeShape_Spherical: return 0.2f + 0.8f*sin(Ratio*pi);
//		case flareTreeShape_Hemispherical: return 0.2f + 0.8f*sin(0.5f*Ratio*pi);
//		case flareTreeShape_Cylindrical: return 1.0f;
//		case flareTreeShape_Tapered_Cylindrical: return 0.5f*(Ratio + 1);
//		case flareTreeShape_Flame: return Ratio < 0.7 ? Ratio / 0.7f : (1.0f - Ratio) / 0.3f;
//		case flareTreeShape_Inverse_Conical: return 1 - 0.8f*Ratio;
//		case flareTreeShape_Tend_Flame: return Ratio < 0.7f ? 0.5f + 0.5f*Ratio / 0.7f : 0.5f + 0.5f*(1 - Ratio) / 0.3f;
//	}
//	return 0;
//}
//
//void CphxTreeSegment::BuildSegment(CphxMesh *Mesh, int &cntr, float rad, D3DXVECTOR3 pos)
//{
//	TREETRANSFORMATION trf = transf.translate(pos - transf.Position);
//
//	int vxb = Mesh->Vertices.NumItems() - Resolution;
//
//	for (int i = 0; i < Resolution; i++)
//	{
//		float angle = i*pi * 2 / Resolution;
//
//		if (!stem->stemlevel)
//		{
//			if (TreeParams->Lobes != 0) angle -= 0.125f / TreeParams->Lobes;
//			rad *= (1 + GetVariedValue(0, 1)*TreeParams->_0ScaleV / subsegments.NumItems())*(1.0f + TreeParams->LobeDepth*cos(TreeParams->Lobes*angle));
//		}
//
//		Mesh->AddVertex(trf.apply(D3DXVECTOR3(cos(angle)*rad, sin(angle)*rad, 0)));
//		if (cntr) Mesh->AddPolygon(vxb + i + Resolution, vxb + (i + 1) % Resolution + Resolution, vxb + (i + 1) % Resolution, vxb + i);
//	}
//	cntr++;
//}
//
//void CphxTreeSegment::BuildMesh(CphxMesh *Mesh, int &cntr)
//{
//	if (!subsegments.NumItems() || cntr == 0)
//		BuildSegment(Mesh, cntr, rad_1, transf.Position);
//
//	for (int x = 0; x < subsegments.NumItems(); x++)
//		BuildSegment(Mesh, cntr, subsegments[x]->rad, subsegments[x]->pos);
//}
//
//void CphxTreeStem::BuildMesh(CphxMesh *Mesh)
//{
//	int cntr = 0;
//
//	for (int x = 0; x < segments.NumItems(); x++)
//		segments[x]->BuildMesh(Mesh, cntr);
//
//	for (int x = 0; x < children.NumItems(); x++)
//		children[x]->BuildMesh(Mesh);
//}
//
//CphxTreeStem::CphxTreeStem(CphxTreeGenerator *tr, CphxTreeStem *GrowsFrom, int stlev, TREETRANSFORMATION &trf, float offs)
//{
//	Tree = tr;
//	TreeParams = &Tree->Species.Parameters;
//	stemlevel = stlev;
//	transf = trf;
//	offset = offs;
//
//	parent = GrowsFrom;
//
//	//////////////////////////////////////////////////////////////////////////
//	//this may be unnecessary
//	if (GrowsFrom && GrowsFrom->stemlevel >= stemlevel)
//		parent = GrowsFrom->parent;
//	//////////////////////////////////////////////////////////////////////////
//
//	lpar = &Tree->Species.Levels[stemlevel];
//
//	StemData.splitCorrection = 0;
//}
//
//void CphxTreeStem::Make()
//{
//	//base level
//	StemData.length = GetVariedValue(lpar->nLength, lpar->nLengthV) * TreeParams->Scale;
//	StemData.baseRadius = StemData.length * TreeParams->Ratio / 2048.0f;
//
//	if (stemlevel)
//	{
//		//higher levels:
//		StemData.length = parent->StemData.lengthChildMax*(parent->StemData.length - 0.6f*offset);
//
//		//level 1:
//		if (stemlevel == 1)
//		{
//			float parlen = parent->StemData.length;
//			StemData.length = parlen*parent->StemData.lengthChildMax*ShapeRatio(TreeParams->Shape, (parlen - offset) / (parlen - TreeParams->BaseSize / 255.0f*TreeParams->Scale));
//		}
//
//		float radius = parent->StemData.baseRadius * pow(StemData.length / parent->StemData.length, TreeParams->RatioPower / 32.0f);
//		StemData.baseRadius = min(radius, parent->stemRadius(offset));
//	}
//
//	StemData.segmentLength = StemData.length / lpar->nCurveRes;
//
//	// this is where pruning would be if it'd be implemented
//	//if (stemlevel>0 && TreeParams->nPruneRatio > 0) pruning();
//
//	//if (StemData.length < MIN_STEM_LEN || StemData.baseRadius < MIN_STEM_RADIUS) return false;
//
//	TREELEVELPARAMETERS *lpar_1 = &Tree->Species.Levels[stemlevel + 1];
//
//	StemData.lengthChildMax = GetVariedValue(lpar_1->nLength, lpar_1->nLengthV);
//	float stems_max = lpar_1->nBranches;
//
//	switch (stemlevel)
//	{
//		case 0:
//			StemData.substemsPerSegment = stems_max / (1 - TreeParams->BaseSize / 255.0f);
//			break;
//		case 1:
//			StemData.substemsPerSegment = (int)(stems_max * (0.2f + 0.8f*StemData.length / parent->StemData.length / parent->StemData.lengthChildMax));
//			break;
//		default:
//			StemData.substemsPerSegment = (int)(stems_max * (1.0f - 0.5f * offset / parent->StemData.length));
//	}
//
//	StemData.substemsPerSegment /= (float)lpar->nCurveRes;
//	StemData.substemRotangle = 0;
//
//	if (stemlevel && stemlevel == TreeParams->Levels - 1)
//		StemData.leavesPerSegment = abs(TreeParams->Leaves)*ShapeRatio(TreeParams->LeafDistrib, offset / parent->StemData.length)*TreeParams->LeafQuality / 255.0f / lpar->nCurveRes;
//
//	makeSegments(0, lpar->nCurveRes);
//}
//
//float CphxTreeStem::stemRadius(float h)
//{
//	float Z, Z2, Z3;
//
//	Z = min(h / StemData.length, 1.0f);
//	Z3 = Z2 = (1 - Z)*StemData.length;
//
//	float taper = lpar->nTaper / 255.0f * 3;
//
//	float unit_taper = 0;
//	if (taper <= 2) unit_taper = 2 - taper;
//	if (taper <= 1) unit_taper = taper;
//
//	float radius = StemData.baseRadius*(1 - unit_taper*Z);
//
//	if (taper > 1)
//	{
//		float depth = taper - 2;
//		if (taper < 2 || Z2<radius) depth = 1;
//
//		if (taper >= 2)
//			Z3 = abs(Z2 - 2 * radius*(int)(Z2 / 2 / radius + 0.5));
//
//		if (taper>2 || Z3 < radius)
//			radius += depth*(sqrt(radius*radius - (Z3 - radius)*(Z3 - radius)) - radius);
//	}
//
//	if (stemlevel == 0)
//		radius *= TreeParams->_0Scale*(1 + (TreeParams->Flare / 25.5f - 1)*(pow(100, max(0, 1 - 8 * Z)) - 1) / 100.0f);
//
//	return radius;
//}
//
//void CphxTreeStem::makeSegments(int start_seg, int end_seg)
//{
//	TREETRANSFORMATION trf = transf;
//
//	for (int s = start_seg; s < end_seg; s++)
//	{
//		if (s)
//		{
//			float delta = lpar->nCurveBack * 2;
//
//			if (delta == 0) delta = lpar->nCurve;
//			else
//				if (s < (lpar->nCurveRes + 1) / 2)
//					delta = lpar->nCurve * 2;
//
//			delta /= (float)lpar->nCurveRes;
//			delta += StemData.splitCorrection;
//
//			trf.RotX(delta*pi / 180.0f);
//
//			delta = GetVariedValue(0, lpar->nCurveV) / lpar->nCurveRes;
//			trf.RotAxisZ(delta*pi / 180.0f, GetVariedValue(pi, pi));
//
//			if (stemlevel >= 2)
//			{
//				float declination = acos(trf.getZ().z);
//				float curve_up = TreeParams->AttractionUp / 12.7f * abs(declination * sin(declination)) / lpar->nCurveRes;
//				D3DXVECTOR3 z = trf.getZ();
//				trf.RotAxis(-curve_up, D3DXVECTOR3(-z.y, z.x, 0));
//			}
//		}
//
//		float rad_1 = stemRadius(s*StemData.segmentLength);
//		float rad_2 = stemRadius((s + 1)*StemData.segmentLength);
//
//		CphxTreeSegment *segment = new CphxTreeSegment(this, s, trf, rad_1, rad_2);
//		segments.Add(segment);
//
//		if (stemlevel < TreeParams->Levels - 1)
//		{
//			float subst_per_segm = StemData.substemsPerSegment;
//			float offs = 0;
//
//			if (!stemlevel)
//			{
//				if (segment->index*StemData.segmentLength <= TreeParams->BaseSize / 255.0f*StemData.length)
//				{
//					offs = (TreeParams->BaseSize / 255.0f*StemData.length - segment->index*StemData.segmentLength) / StemData.segmentLength;
//					subst_per_segm = StemData.substemsPerSegment*max(0, 1 - offs);
//				}
//			}
//			else
//				if (segment->index == 0)
//					offs = parent->stemRadius(offset) / StemData.segmentLength;
//
//			int substems_eff = PropagateError(Tree->Levels[stemlevel].substemErrorValue, subst_per_segm);
//
//			float dist = (1.0f - offs) / substems_eff*Tree->Species.Levels[stemlevel + 1].nBranchDist / 255.0f;
//			float distv = dist*0.25f;
//
//			for (int x = 0; x < substems_eff; x++)
//			{
//				float w = GetVariedValue(offs + dist / 2 + x*dist, distv);
//				float offset = (segment->index + w) * StemData.segmentLength;
//
//				TREETRANSFORMATION trf2 = substemDirection(segment->transf, offset);
//
//				//substemposition
//
//				//if (lpar->nCurveV<0) //helix
//				//{
//				//	int i=(int)(w*(segment->subsegments.NumItems()-1));
//				//	D3DXVECTOR3 p1 = segment->subsegments[i]->pos;
//				//	D3DXVECTOR3 p2 = segment->subsegments[i+1]->pos;
//				//	D3DXVECTOR3 pos = p1+((p2-p1)*(w - i/(segment->subsegments.NumItems()-1)));
//				//	trf=trf.translate(pos-segment->transf.Position);
//				//} 
//				//else 
//
//				trf2.Position += segment->transf.getZ()*(w*StemData.segmentLength);
//
//				CphxTreeStem *substem = new CphxTreeStem(Tree, this, stemlevel + 1, trf2, offset);
//				children.Add(substem);
//
//				substem->Make();
//			}
//		}
//
//		if (stemlevel == TreeParams->Levels - 1)
//		{
//			//make leaves
//			if (TreeParams->Leaves > 0)
//			{
//				float leaves_eff = PropagateError(Tree->leavesErrorValue, StemData.leavesPerSegment);
//
//				float offs = 0;
//				if (segment->index == 0 && stemlevel) //stemlevel needed for parent availability check
//					offs = parent->stemRadius(offset) / StemData.segmentLength;
//
//				for (int x = 0; x < leaves_eff; x++)
//				{
//					float dist = (1.0f - offs) / leaves_eff;
//					float w = GetVariedValue(offs + dist / 2 + x*dist, dist / 2);
//
//					float loffs = (segment->index + w)*StemData.segmentLength;
//					TREETRANSFORMATION trf2 = substemDirection(segment->transf, loffs);
//					trf2.Position += segment->transf.getZ()*w*StemData.segmentLength;
//
//					leaves.Add(new CphxTreeLeaf(trf2, TreeParams));
//				}
//			}
//			else
//				if (segment->index == lpar->nCurveRes - 1) //leaves<0 = fan mode => ~383 bytes
//				{
//					TREELEVELPARAMETERS *lpar_1 = &Tree->Species.Levels[stemlevel + 1];
//					int cnt = (int)(StemData.leavesPerSegment*lpar->nCurveRes + 0.5);
//
//					TREETRANSFORMATION trf2 = segment->transf.translate(segment->transf.getZ()*StemData.segmentLength);
//					float distangle = lpar_1->nRotate / (float)cnt;
//					float varangle = lpar_1->nRotateV / (float)cnt;
//					float downangle = lpar_1->nDownAngle;
//					float vardown = lpar_1->nDownAngleV;
//					float offsetangle = distangle / 2;
//
//					if (cnt % 2)
//					{
//						leaves.Add(new CphxTreeLeaf(trf2, TreeParams));
//						offsetangle = distangle;
//					}
//
//					for (int x = 0; x < cnt / 2; x++)
//					{
//						for (int rot = 1; rot >= -1; rot -= 2)
//						{
//							TREETRANSFORMATION transf1 = trf2.RotY(rot*pi / 180.0f*(GetVariedValue(offsetangle + x*distangle, varangle)));
//							transf1.RotX(GetVariedValue(downangle, vardown)*pi / 180.0f);
//							leaves.Add(new CphxTreeLeaf(transf1, TreeParams));
//						}
//					}
//				}
//		}
//
//		trf.Position += StemData.segmentLength*trf.getZ();
//		if (s < end_seg - 1) makeClones(trf, s);
//	}
//}
//
//void CphxTreeStem::makeClones(TREETRANSFORMATION trf, int nseg)
//{
//	int seg_splits_eff = TreeParams->_0BaseSplits;
//
//	if (stemlevel || nseg || !TreeParams->_0BaseSplits)
//		seg_splits_eff = PropagateError(Tree->Levels[stemlevel].splitErrorValue, lpar->nSegSplits);
//
//	if (seg_splits_eff < 1) return;
//
//	for (int i = 0; i < seg_splits_eff; i++)
//	{
//		CphxTreeStem *clon = Clone(trf);
//		clon->transf = clon->split(trf, 360.0f / (seg_splits_eff + 1)*(1 + i), nseg, seg_splits_eff);
//		clon->makeSegments(nseg + 1, clon->lpar->nCurveRes);
//	}
//
//	trf = split(trf, 0, nseg, seg_splits_eff);
//}
//
//CphxTreeStem *CphxTreeStem::Clone(TREETRANSFORMATION trf)
//{
//	CphxTreeStem *clon = new CphxTreeStem(Tree, this, stemlevel, trf, offset);
//	children.Add(clon);
//	clon->StemData = StemData;
//	clon->StemData.substemRotangle += 180;
//	return clon;
//}
//
//TREETRANSFORMATION CphxTreeStem::split(TREETRANSFORMATION trf, float s_angle, int nseg, int nsplits)
//{
//	float declination = acos(trf.getZ().z) * 180 / pi;
//	float split_angle = max(0, (GetVariedValue(lpar->nSplitAngle, lpar->nSplitAngleV) - declination));
//
//	trf.RotX(split_angle*pi / 180.0f);
//
//	StemData.splitCorrection -= split_angle / (lpar->nCurveRes - nseg - 1);
//
//	if (s_angle > 0)
//	{
//		float split_diverge = GetVariedValue(s_angle, lpar->nSplitAngleV);
//
//		if (TreeParams->_0BaseSplits == 0 || stemlevel != 0 || nseg != 0)
//		{
//			split_diverge = 20 + 0.75f*(30 + abs(declination - 90))*pow(GetVariedValue(1, 1) / 2.0f, 2);
//			if (GetVariedValue(0, 1) >= 0) split_diverge = -split_diverge;
//		}
//
//		trf.RotAxis(split_diverge*pi / 180.0f, D3DXVECTOR3(0, 0, 1));
//	}
//
//	StemData.substemsPerSegment /= (float)(nsplits + 1);
//	return trf;
//}
//
//
//TREETRANSFORMATION CphxTreeStem::substemDirection(TREETRANSFORMATION trf, float offset)
//{
//	TREELEVELPARAMETERS *lpar_11 = &Tree->Species.Levels[min(stemlevel + 1, 3)];
//
//	float rndv = GetVariedValue(lpar_11->nRotate, lpar_11->nRotateV);
//
//	float rotangle;
//	if (lpar_11->nRotate >= 0)
//	{
//		rotangle = StemData.substemRotangle = StemData.substemRotangle + rndv;
//	}
//	else
//	{
//		if (abs(StemData.substemRotangle) != 1) StemData.substemRotangle = 1;
//		StemData.substemRotangle *= -1;
//		rotangle = StemData.substemRotangle*(180 + rndv);
//	}
//
//	float downangle = GetVariedValue(lpar_11->nDownAngle, lpar_11->nDownAngleV);
//	if (lpar_11->nDownAngleV < 0)
//	{
//		float len = StemData.length;
//		if (!stemlevel) len *= 1 - TreeParams->BaseSize / 255.0f;
//		downangle = lpar_11->nDownAngle + lpar_11->nDownAngleV*(0.6f - 1.6f*(StemData.length - offset) / len);
//	}
//
//	return trf.ROtXZ(downangle*pi / 180.0f, rotangle*pi / 180.0f);
//}
//
//void CphxTreeStem::BuildLeaves(CphxMesh *Mesh)
//{
//	for (int x = 0; x < leaves.NumItems(); x++)
//		leaves[x]->BuildMesh(Mesh);
//	for (int x = 0; x < children.NumItems(); x++)
//		children[x]->BuildLeaves(Mesh);
//}
//
//void CphxTreeGenerator::MakeTree(unsigned int seed, TREESPECIESDESCRIPTOR &spec)
//{
//	Species = spec;
//
//	//default test
//	//Parameters.Shape=flareTreeShape_Conical;
//
//	//Parameters.Levels=2;
//	//Parameters.scale_tree=10;
//	////Parameters.ScaleV=0.0f;
//	//Parameters.BaseSize=0.25f;
//	//Parameters._0BaseSplits=0;
//	//Parameters.RatioPower=1;
//	//Parameters.AttractionUp=0;
//
//	//Parameters.Ratio=0.05f;
//	//Parameters.Flare=0.5;
//	//Parameters.Lobes=0;
//	//Parameters.LobeDepth=0;
//	//Parameters._0Scale=1;
//	//Parameters._0ScaleV=0;
//
//	////Parameters.Leaves=25;
//	////Parameters.LeafShape=0;
//	////Parameters.LeafScale=0.17;
//	////Parameters.LeafScaleX=1.0;
//	////Parameters.LeafBend=0.3;
//	////Parameters.LeafStemLen=0.5;
//	////Parameters.LeafDistrib=4;
//
//	////Parameters.PruneRatio=0;
//	//Parameters.PruneWidth=0.5;
//	////Parameters.PruneWidthPeak=0.5;
//	////Parameters.PrunePowerLow=0.5;
//	////Parameters.PrunePowerHigh=0.5;
//
//	////Parameters.LeafQuality=1;
//	////Parameters.Smooth=0.5;
//
//	////level 0
//	//Levels[0].nLength=1;
//	//Levels[0].nLengthV=0;
//	//Levels[0].nTaper=1;
//
//	//Levels[0].nCurveRes=3;
//	//Levels[0].nCurve=0;
//	//Levels[0].nCurveV=0.0;
//	//Levels[0].nCurveBack=0;
//
//	//Levels[0].nSegSplits=0;
//	//Levels[0].nSplitAngle=0;
//	//Levels[0].nSplitAngleV=0;
//
//	//Levels[0].nDownAngle=0;
//	//Levels[0].nDownAngleV=0;
//	//Levels[0].nRotate=0;
//	//Levels[0].nRotateV=0;
//	//Levels[0].nBranches=1;
//	//Levels[0].nBranchDist=0;
//
//	////level 1
//	//Levels[1].nLength=0.5f;
//	//Levels[1].nLengthV=0;
//	//Levels[1].nTaper=1.0;
//
//	//Levels[1].nCurveRes=3;
//	//Levels[1].nCurve=0;
//	//Levels[1].nCurveV=0;
//	//Levels[1].nCurveBack=0;
//
//	//Levels[1].nSegSplits=0;
//	//Levels[1].nSplitAngle=0;
//	//Levels[1].nSplitAngleV=0;
//
//	//Levels[1].nDownAngle=30;
//	//Levels[1].nDownAngleV=0;
//	//Levels[1].nRotate=120;
//	//Levels[1].nRotateV=0;
//	//Levels[1].nBranches=10;
//	//Levels[1].nBranchDist=1;
//
//	////level 2
//	//Levels[2].nLength=0.5f;
//	//Levels[2].nLengthV=0;
//	//Levels[2].nTaper=1;
//
//	//Levels[2].nCurveRes=1;
//	//Levels[2].nCurve=0;
//	//Levels[2].nCurveV=0;
//	//Levels[2].nCurveBack=0;
//
//	//Levels[2].nSegSplits=0;
//	//Levels[2].nSplitAngle=0;
//	//Levels[2].nSplitAngleV=0;
//
//	//Levels[2].nDownAngle=30;
//	//Levels[2].nDownAngleV=0;
//	//Levels[2].nRotate=120;
//	//Levels[2].nRotateV=0;
//	//Levels[2].nBranches=5;
//	//Levels[2].nBranchDist=1;
//
//
//	//make like a tree...
//
//	leavesErrorValue = 0;
//
//	Levels[0].mesh_points = 4;
//	if (Species.Parameters.Lobes > 0)
//	{
//		Levels[0].mesh_points = (int)(Species.Parameters.Lobes*(pow(2.0f, (int)(1 + 2.5f*Species.Parameters.Smooth / 255.0f))));
//		Levels[0].mesh_points = max(Levels[0].mesh_points, (int)(4 * (1 + 2 * Species.Parameters.Smooth / 255.0f)));
//	}
//	for (int i = 1; i < 4; i++)
//		Levels[i].mesh_points = max(3, (int)(Levels[i].mesh_points*(1 + 1.5*Species.Parameters.Smooth / 255.0f)));
//
//	srand(seed);
//
//	float trunk_rotangle = 0;
//
//	for (int x = 0; x < Species.Levels[0].nBranches; x++)
//	{
//		trunk_rotangle = fmod(GetVariedValue(trunk_rotangle + Species.Levels[0].nRotate + 360, Species.Levels[0].nRotateV), 360);
//		float DownAngle = GetVariedValue(Species.Levels[0].nDownAngle, Species.Levels[0].nDownAngleV);
//
//		float angle = GetVariedValue(0, pi);
//		float dist = GetVariedValue(0, Species.Levels[0].nBranchDist / 255.0f);
//		TREETRANSFORMATION trf;
//		trf = trf.ROtXZ(DownAngle*pi / 180.0f, trunk_rotangle*pi / 180.0f);
//		trf.Position = D3DXVECTOR3(sin(angle), cos(angle), 0)*dist;
//
//		CphxTreeStem *trunk = new CphxTreeStem(this, NULL, 0, trf, 0);
//		Trunks.Add(trunk);
//		trunk->Make();
//	}
//
//}
//
//void CphxTreeGenerator::BuildTree(CphxMesh *Mesh)
//{
//	for (int x = 0; x < Trunks.NumItems(); x++)
//		Trunks[x]->BuildMesh(Mesh);
//
//	Mesh->SmoothGroupSeparation = DEFAULTSMOOTHGROUPSEPARATION;
//}
//
//void CphxTreeGenerator::BuildLeaves(CphxMesh *Mesh)
//{
//	for (int x = 0; x < Trunks.NumItems(); x++)
//		Trunks[x]->BuildLeaves(Mesh);
//	Mesh->SmoothGroupSeparation = DEFAULTSMOOTHGROUPSEPARATION;
//}