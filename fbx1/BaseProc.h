//
// Parse through all the types of nodes in the file: Meshes, Materials, Bones, etc
//


#ifndef __BASEPROC__H
#define __BASEPROC__H



//
// Project headers
//
#include "WriteData.h"



///////////////////////////////////////////////////////
// CLASSES
//
// I want a default class for all processing modules
// that use/access the write data class (WriteData)
// Since it is necessary to have this base class
// in order to access WriteData, the using default 
// constructor parameter will be enforced
///////////////////////////////////////////////////////
class BaseProc
{
	public:
		BaseProc(WriteData *pWrtData) {m_pWriteData = pWrtData;}
		inline WriteData *GetWrtDataPtr() { return m_pWriteData; }
		FileData *GetFileDataPtr() {return m_pWriteData->GetFileDataPtr();}

	private:
		WriteData *m_pWriteData;
};



#endif
