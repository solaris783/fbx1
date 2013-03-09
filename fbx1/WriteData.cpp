//
// Write all extracted data out
//



// sytem includes
#include <assert.h>


//
// project includes
//
#include "WriteData.h"
#include "Weld.h"



///////////////////////////////////////////////////////////////////////////////////////
// Hash definitions for data types used by the "WeldData" function
///////////////////////////////////////////////////////////////////////////////////////
namespace std
{
	template <> struct hash<Vec3>
	{
		size_t operator()(Vec3 v)
		{
			const unsigned int * h = (const unsigned int *)(&v);
			unsigned int f = (h[0]+h[1]*11-(h[2]*17))&0x7fffffff;     // avoid problems with +-0
			return (f>>22)^(f>>12)^(f);
		}
	};

	template <> struct hash<TexCoord>
	{
		size_t operator()(TexCoord uv)
		{
			const unsigned int * h = (const unsigned int *)(&uv);
			unsigned int f = (h[0]+h[1]*11)&0x7fffffff;     // avoid problems with +-0
			return (f>>22)^(f>>12)^(f);
		}
	};

	template <> struct hash<ColorRGBA>
	{
		size_t operator()(ColorRGBA v)
		{
			size_t h = 5381;
			unsigned char r = (unsigned char) min(255.0f, ((v.r * 0.5f) * 255.0f));
			unsigned char g = (unsigned char) min(255.0f, ((v.g * 0.5f) * 255.0f));
			unsigned char b = (unsigned char) min(255.0f, ((v.b * 0.5f) * 255.0f));
			unsigned char a = (unsigned char) min(255.0f, ((v.a * 0.5f) * 255.0f));

			h = ((h << 5) + h) ^ r;
			h = ((h << 5) + h) ^ g;
			h = ((h << 5) + h) ^ b;
			h = ((h << 5) + h) ^ a;
			return h;
		}
	};
}






////////////////////////////////////////////////////////////////////////////////////////
// Template for reordering index arrays
////////////////////////////////////////////////////////////////////////////////////////
void ReorderIndices(int originalArraySize, vector<Int3> *pIndexArray, const std::vector<size_t> & xrefs)
{
	for(int i=0; i < originalArraySize; i++)
	{
		for(int j=0; j < 3; j++)
		{
			if((*pIndexArray)[i].idxs[j] >= 0)
				(*pIndexArray)[i].idxs[j] = xrefs[(*pIndexArray)[i].idxs[j]];
		}
	}

}






///////////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////////
WriteData::WriteData()
{
}




///////////////////////////////////////////////////////////////////////////////////////////
// Set the filename for the output file - based on the input file with an added extension
///////////////////////////////////////////////////////////////////////////////////////////
void WriteData::SetFilename(string input_filename)
{
	assert(input_filename.length() > 1);
	m_outputFilename = input_filename + ".res";
}




///////////////////////////////////////////////////////////////////////////////////////////
// Helper function to push a new vertex coordinate into the vertex stl::vector structure
///////////////////////////////////////////////////////////////////////////////////////////
void WriteData::RecordVertCoord(FbxVector4 pValue)
{
	Vec3 vertCoord;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertCoord.x = IN_CutPrecision( (float) pValue[0] );
	vertCoord.y = IN_CutPrecision( (float) pValue[1] );
	vertCoord.z = IN_CutPrecision( (float) pValue[2] );

	m_fileData.meshData.vPos.push_back(vertCoord);
}




///////////////////////////////////////////////////////////////////////////////////////////
// Helper function to push a new vertex color into the color stl::vector structure
///////////////////////////////////////////////////////////////////////////////////////////
void WriteData::RecordVertColor(FbxColor pValue)
{
	ColorRGBA color;

	color.r = IN_CutPrecision( (float) pValue.mRed );
	color.g = IN_CutPrecision( (float) pValue.mGreen );
	color.b = IN_CutPrecision( (float) pValue.mBlue );
	color.a = IN_CutPrecision( (float) pValue.mAlpha );

	m_fileData.meshData.vColor.push_back(color);
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function to push a new vertex texture coordinate into the texCoord stl::vector structure
////////////////////////////////////////////////////////////////////////////////////////////////////
void WriteData::RecordVertTexCoord(FbxVector2 pValue)
{
	TexCoord texC;

	texC.u = IN_CutPrecision( (float) pValue[0] );
	texC.v = IN_CutPrecision( (float) pValue[1] );

	m_fileData.meshData.vTex.push_back(texC);
}




///////////////////////////////////////////////////////////////////////////////////////////
// Helper function to push a new vertex normal into the normal stl::vector structure
///////////////////////////////////////////////////////////////////////////////////////////
void WriteData::RecordVertNormal(FbxVector4 pValue)
{
	Vec3 vertNorm;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertNorm.x = IN_CutPrecision( (float) pValue[0] );
	vertNorm.y = IN_CutPrecision( (float) pValue[1] );
	vertNorm.z = IN_CutPrecision( (float) pValue[2] );

	m_fileData.meshData.vNorm.push_back(vertNorm);
}




///////////////////////////////////////////////////////////////////////////////////////////
// Helper function to push a new vertex tangent into the tangent stl::vector structure
///////////////////////////////////////////////////////////////////////////////////////////
void WriteData::RecordVertTangent(FbxVector4 pValue)
{
	Vec3 vertTang;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertTang.x = IN_CutPrecision( (float) pValue[0] );
	vertTang.y = IN_CutPrecision( (float) pValue[1] );
	vertTang.z = IN_CutPrecision( (float) pValue[2] );

	m_fileData.meshData.vTang.push_back(vertTang);
}




///////////////////////////////////////////////////////////////////////////////////////////
// Helper function to push a new vertex binormal into the binormal stl::vector structure
///////////////////////////////////////////////////////////////////////////////////////////
void WriteData::RecordVertBinormal(FbxVector4 pValue)
{
	Vec3 vertBiNorm;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertBiNorm.x = IN_CutPrecision( (float) pValue[0] );
	vertBiNorm.y = IN_CutPrecision( (float) pValue[1] );
	vertBiNorm.z = IN_CutPrecision( (float) pValue[2] );

	m_fileData.meshData.vBinorm.push_back(vertBiNorm);
}





///////////////////////////////////////////////////////////////////////////////////////////
// WELD data pools
// Remove duplicates from data lists and fix indices
///////////////////////////////////////////////////////////////////////////////////////////
void WriteData::WeldData()
{
	MeshData *pData = &m_fileData.meshData;
	TriList *pIndices = &m_fileData.meshData.tris;
	int num = 0;
	int compCnt = 0;
	vector<size_t> xrefs;

	// Remove POSITION duplicates
	if(pData->vPos.size() > 0)
	{
		num = Weld( pData->vPos, xrefs, std::hash<Vec3>(), std::equal_to<Vec3>() );
		compCnt = pIndices->iPos.size(); // number of items in this component
		ReorderIndices(compCnt, &pIndices->iPos, xrefs);
		xrefs.clear();
	}
	else
	{
		pIndices->iPos.clear();
	}

	// Remove COLOR duplicates
	if(pData->vColor.size() > 0)
	{
		num = Weld( pData->vColor, xrefs, std::hash<ColorRGBA>(), std::equal_to<ColorRGBA>() );
		compCnt = pIndices->iCol.size(); // number of items in this component
		ReorderIndices(compCnt, &pIndices->iCol, xrefs);
		xrefs.clear();
	}
	else
	{
		pIndices->iCol.clear();
	}

	// Remove TEXTURE COORDINATE duplicates
	if(pData->vTex.size() > 0)
	{
		num = Weld( pData->vTex, xrefs, std::hash<TexCoord>(), std::equal_to<TexCoord>() );
		compCnt = pIndices->iTex.size(); // number of items in this component
		ReorderIndices(compCnt, &pIndices->iTex, xrefs);
		xrefs.clear();
	}
	else
	{
		pIndices->iTex.clear();
	}

	// Remove NORMAL duplicates
	if(pData->vNorm.size() > 0)
	{
		num = Weld( pData->vNorm, xrefs, std::hash<Vec3>(), std::equal_to<Vec3>() );
		compCnt = pIndices->iNrm.size(); // number of items in this component
		ReorderIndices(compCnt, &pIndices->iNrm, xrefs);
		xrefs.clear();
	}
	else
	{
		pIndices->iNrm.clear();
	}

	// Remove TANGENT duplicates
	if(pData->vTang.size() > 0)
	{
		num = Weld( pData->vTang, xrefs, std::hash<Vec3>(), std::equal_to<Vec3>() );
		compCnt = pIndices->iTan.size(); // number of items in this component
		ReorderIndices(compCnt, &pIndices->iTan, xrefs);
		xrefs.clear();
	}
	else
	{
		pIndices->iTan.clear();
	}

	// Remove BINORMAL duplicates
	if(pData->vBinorm.size() > 0)
	{
		num = Weld( pData->vBinorm, xrefs, std::hash<Vec3>(), std::equal_to<Vec3>() );
		compCnt = pIndices->iBin.size(); // number of items in this component
		ReorderIndices(compCnt, &pIndices->iBin, xrefs);
		xrefs.clear();
	}
	else
	{
		pIndices->iBin.clear();
	}
}
