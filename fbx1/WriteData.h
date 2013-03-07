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
		void AddCoordTriIdxs(Int3 *pInt3) { m_fileData.meshData.tris.iPos.push_back( *pInt3 ); }
		void AddColorTriIdxs(Int3 *pInt3) { m_fileData.meshData.tris.iCol.push_back( *pInt3 ); }
		void AddTexCoordTriIdxs(Int3 *pInt3) { m_fileData.meshData.tris.iTex.push_back( *pInt3 ); }
		void AddNormTriIdxs(Int3 *pInt3) { m_fileData.meshData.tris.iNrm.push_back( *pInt3 ); }
		void AddTangTriIdxs(Int3 *pInt3) { m_fileData.meshData.tris.iTan.push_back( *pInt3 ); }
		void AddBinormTriIdxs(Int3 *pInt3) { m_fileData.meshData.tris.iBin.push_back( *pInt3 ); }

	private:
		FileData m_fileData;
		string m_outputFilename;
};





#endif
