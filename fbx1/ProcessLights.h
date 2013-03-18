#pragma once

//
// Project headers
//
#include "WriteData.h"
#include "BaseProc.h"



///////////////////////////////////////////////////////
// CLASSES
//
// I need it to be a class so that maintainning
// the data structures is easier to program (i.e. 
// not have to pass structures back and forth between
// functions or declare messy globals
///////////////////////////////////////////////////////
class ProcessLights : public BaseProc
{
	public:
		ProcessLights(WriteData *pWrtData) : BaseProc(pWrtData) {} // this constructor has a compulsory argument that gets propagated to the base class
		void Start(FbxNode* pNode);

	private:
};
