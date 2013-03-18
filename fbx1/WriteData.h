//
// Write all extracted data out
//


#ifndef __WRITE_DATA__H
#define __WRITE_DATA__H



//
// System headers
//
#include <assert.h>


//
// Fbx library headers
//
#include "fbxdefs.h"


//
// Project headers
//
#include "DataTypes.h"



///////////////////////////////////////////////////////
// CLASSES
//
// This class maintains the file saving information
// and saves data in the appropriate slot.
///////////////////////////////////////////////////////
class WriteData
{
	public:
		WriteData();
		void SetFilename(string input_filename);
		void WeldData(); // remove duplicates from data lists and fix indices

		///////////////////////////////////////////////
		// Functions for recording vertex components
		void RecordVertCoord(FbxVector4 pValue);
		void RecordVertColor(FbxColor pValue);
		void RecordVertTexCoord(FbxVector2 pValue);
		void RecordVertNormal(FbxVector4 pValue);
		void RecordVertTangent(FbxVector4 pValue);
		void RecordVertBinormal(FbxVector4 pValue);

		///////////////////////////////////////////////
		// Functions for getting current component index
		int GetCurrVertCoordIndex() {return m_fileData.meshData.vPos.size();}
		int GetCurrVertColorIndex() {return m_fileData.meshData.vColor.size();}
		int GetCurrVertTexCoordIndex() {return m_fileData.meshData.vTex.size();}
		int GetCurrVertNormIndex() {return m_fileData.meshData.vNorm.size();}
		int GetCurrVertTangIndex() {return m_fileData.meshData.vTang.size();}
		int GetCurrVertBinormIndex() {return m_fileData.meshData.vBinorm.size();}

		///////////////////////////////////////////////
		// Functions for adding new tringle indices
		void AddCoordTriIdxs(Int3 & arg) { m_fileData.meshData.tris.iPos.push_back( arg ); }
		void AddColorTriIdxs(Int3 & arg) { m_fileData.meshData.tris.iCol.push_back( arg ); }
		void AddTexCoordTriIdxs(Int3 & arg) { m_fileData.meshData.tris.iTex.push_back( arg ); }
		void AddNormTriIdxs(Int3 & arg) { m_fileData.meshData.tris.iNrm.push_back( arg ); }
		void AddTangTriIdxs(Int3 & arg) { m_fileData.meshData.tris.iTan.push_back( arg ); }
		void AddBinormTriIdxs(Int3 & arg) { m_fileData.meshData.tris.iBin.push_back( arg ); }
		void AddMaterialIdx(MatList & arg) { m_fileData.meshData.tris.iMat.push_back( arg ); }

		////////////////////////////////////////////////
		// For material, texture, light, etc data access
		FileData *GetFileDataPtr() {return &m_fileData;}

		///////////////////////////////////////////////
		// Flag setting
		void SetMaterialAsUsed(int materialIndex);

	private:
		FileData m_fileData;
		string m_outputFilename;
};





#endif
