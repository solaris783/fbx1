//
// ProcessMaterials.h
// Make a list of all materials in the scene.  Extract all material data.
// Materials are found when going through poly data (ProcessMesh.cpp), and this moduled is called to process them.
// Therefore, if there are materials that don't exist in the poly data, they will not be found/recorded
//

#pragma once
//#ifndef __PROCESS_MATERIALS__H
//#define __PROCESS_MATERIALS__H



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
class ProcessMaterials : public BaseProc
{
	public:
		ProcessMaterials(WriteData *pWrtData) : BaseProc(pWrtData) {} // this constructor has a compulsory argument that gets propagated to the base class
		void Start(FbxMesh* pMesh, MaterialMeshXref &matXref);
		void DeleteUnused();

	private:
		int  IsMaterialRecorded(const char *pName);
		int  RecordMaterial(FbxSurfaceMaterial *pMaterial, int materialIndex);
		void ExtractTextures(FbxSurfaceMaterial *pMaterial, int materialIndex);
		void FindTextureInfoByProperty(FbxProperty lProperty, bool &lDisplayHeader, int materialIndex);
		void RecordTextureInfo(FbxTexture *pTexture, int blendMode, TextureData *pTexDat);
		int  IsTextureAlreadyRecorded(FbxTexture *pTexture);
};



//#endif
