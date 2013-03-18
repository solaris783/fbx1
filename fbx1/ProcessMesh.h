//
// Parse through all the types of nodes in the file: Meshes, Materials, Bones, etc
//


#ifndef __PROCESS_MESH__H
#define __PROCESS_MESH__H



//
// Project headers
//
#include "WriteData.h"
#include "BaseProc.h"
#include "ProcessMaterials.h"



///////////////////////////////////////////////////////
// CLASSES
//
// I need it to be a class so that maintainning
// the data structures is easier to program (i.e. 
// not have to pass structures back and forth between
// functions or declare messy globals
///////////////////////////////////////////////////////
class ProcessMesh : public BaseProc
{
	public:
		ProcessMesh(WriteData *pWrtData) : BaseProc(pWrtData) {} // this constructor has a compulsory argument that gets propagated to the base class
		void Start(FbxNode *pNode, ProcessMaterials *pProcMat);

	private:
		bool ProcessPolygonInfo(FbxMesh* pMesh, MaterialMeshXref &matXref); // go through all polygon data and record it into the m_pWriteData structure. Mark any materials used in my global list of materials
};



#endif
