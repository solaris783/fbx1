//
// Parse through all the types of nodes in the file: Meshes, Materials, Bones, etc
//


#ifndef __PROCESS_CONTENT__H
#define __PROCESS_CONTENT__H


//
// Project headers
//
#include "DataTypes.h"
#include "WriteData.h"
#include "ProcessMesh.h"



///////////////////////////////////////////////////////
// CLASSES
//
// I need it to be a class so that maintainning
// the data structures is easier to program (i.e. 
// not have to pass structures back and forth between
// functions or declare messy globals
///////////////////////////////////////////////////////
class ProcessContent
{
	public:
		ProcessContent(string filename);
		void Start(FbxScene* pScene);

	private:
		string m_filename;
		WriteData m_writeData;
		ProcessMesh m_procMesh;

		// ProcessContent-specific functions
		void RecurThroughChildren(FbxNode* pNode);
};



#endif
