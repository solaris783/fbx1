//
// Parse through all the types of nodes in the file: Meshes, Materials, Bones, etc
//


#ifndef __PROCESS_MESH__H
#define __PROCESS_MESH__H



//
// Project headers
//
#include "WriteData.h"



///////////////////////////////////////////////////////
// CLASSES
//
// I need it to be a class so that maintainning
// the data structures is easier to program (i.e. 
// not have to pass structures back and forth between
// functions or declare messy globals
///////////////////////////////////////////////////////
class ProcessMesh
{
	public:
		void Start(FbxNode *pNode, WriteData *pWrData);

	private:
		WriteData *m_pWriteData;
		void ProcessPolygonInfo(FbxMesh* pMesh); // go through all polygon data and record it into the m_pWriteData structure

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Functions for recording component values into our data structure (of class MeshData)
		void RecordVertexCoord(FbxVector4 pValue);
		void RecordVertexColor(FbxColor pValue);
		void RecordVertexTexCoord(FbxVector2 pValue);
		void RecordVertexNormal(FbxVector4 pValue);
};



#endif
