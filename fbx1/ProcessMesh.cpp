//
// Go through the Mesh node and break it down into vertices, normals, uv's, etc.
//




//
// Standard library includes
//



//
// Project Includes
//
#include "fbxdefs.h"
#include "DisplayCommon.h"
#include "DataTypes.h"
#include "ProcessMesh.h"
#include "WriteData.h"
#include "ProcessContent.h"






////////////////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////////////
#define COMPONENT_NOT_FOUND_IDX		-1

// Macros for recording vertex component raw data, and updating indices.
// made into macros as to not obfuscate member functions with pointers or references to arbitrary data, abstracting simple operations making it difficult to follow
#define RECORD_VERTEX_COORD(arg) { RecordVertexCoord(arg); pos.idxs[j] = posIdx; posIdx++; }
#define RECORD_VERTEX_COLOR(arg) { RecordVertexColor(arg); col.idxs[j] = colIdx; colIdx++; }
#define RECORD_VERTEX_TEX_COORD(arg) { RecordVertexTexCoord(arg); uvs.idxs[j] = uvsIdx; uvsIdx++; }
#define RECORD_VERTEX_NORM(arg) { RecordVertexNormal(arg); nrm.idxs[j] = nrmIdx; nrmIdx++; }
#define RECORD_VERTEX_TANG(arg) { RecordVertexTangent(arg); tan.idxs[j] = tanIdx; tanIdx++; }
#define RECORD_VERTEX_BINORM(arg) { RecordVertexBinormal(arg); bin.idxs[j] = binIdx; binIdx++; }






///////////////////////////////////////////////////////////////////////////////////////
// Go through the root node extracting all pertinent info
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::Start(FbxNode* pNode, WriteData *pWrData)
{
    FbxMesh* lMesh = (FbxMesh*) pNode->GetNodeAttribute();
	if(!lMesh)
		return;

	if(G_bVerbose)
	    printf("\t\t\tMesh Name: %s\n", (char *) pNode->GetName());

	m_pWriteData = pWrData;

	// extract data out of mesh
	ProcessPolygonInfo(lMesh);
}





///////////////////////////////////////////////////////////////////////////////////////
// Go through the Mesh node and break it down into vertices, normals, uv's, etc.
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::ProcessPolygonInfo(FbxMesh* pMesh)
{
	bool nonTriangleFound = false;




	//////////////////////////////////////////////////
	// IMPORTANT: Keep track of component indices
	// We have a running list of all raw data added for entire file (not just this mesh)
	// These indices correspond in the order into the MeshData structure
	// *** NOTE: at this point all component indices will match: vPos[0] refers to the same vertex as vNorm[0], vTex[0], etc.  This will change later in the process
	int posIdx = GetDataPtr()->GetCurrVertCoordIndex();
	int colIdx = GetDataPtr()->GetCurrVertColorIndex();
	int uvsIdx = GetDataPtr()->GetCurrVertTexCoordIndex();
	int nrmIdx = GetDataPtr()->GetCurrVertNormIndex();
	int binIdx = GetDataPtr()->GetCurrVertBinormIndex();
	int tanIdx = GetDataPtr()->GetCurrVertTangIndex();



	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// list of component indices for the triangle lists (one for each component: coord, color, uv, etc)
	Int3 pos, col, uvs, nrm, bin, tan;




	/////////////////////////////////////
	// Fbx vertex index for current mesh
    int vertexId = 0;

	//////////////////////////
	// Iteration vars
	// to help go through all mesh data in the fbx file
    int i, j, lPolygonCount = pMesh->GetPolygonCount();
    FbxVector4* lControlPoints = pMesh->GetControlPoints(); 
    char header[100];

	////////////////////////////////
	// Iterate through polygon data
    for (i = 0; i < lPolygonCount; i++)
    {
        int l;

		//////////////////////////
		// FIND GROUP ASSIGNMENTS
		//////////////////////////
        for (l = 0; l < pMesh->GetElementPolygonGroupCount(); l++)
        {
            FbxGeometryElementPolygonGroup* lePolgrp = pMesh->GetElementPolygonGroup(l);

			//
			switch (lePolgrp->GetMappingMode())
			{
				//_______________________________________
				case FbxGeometryElement::eByPolygon:
				{
					if (lePolgrp->GetReferenceMode() == FbxGeometryElement::eIndex)
					{
						if(G_bVerbose)
						{
							int polyGroupId = lePolgrp->GetIndexArray().GetAt(i);
							printf("\t\t\t\tAssigned to group: %d\n", polyGroupId);
						}
						break;
					}
					// no break
				}

				//_______________________________________
				default:
				{
					// any other mapping modes don't make sense
					if(G_bVerbose)
						printf("\t\t\t\t\"unsupported group assignment\"");
					break;
				}

			} // end of switch

		} // end of for(...pMesh->GetElementPolygonGroupCount()...)





		///////////////////////////////////////////
		// FIND RAW DATA (verts, uv's, norms, etc)
		///////////////////////////////////////////

        int lPolygonSize = pMesh->GetPolygonSize(i);
		if(lPolygonSize != 3)
		{
			if(!nonTriangleFound)
			{
				printf("***  WARNING: non-triangles in mesh %s.  ALL non-triangles are discarded\n", pMesh->GetName());
				nonTriangleFound = true;
			}

			vertexId += lPolygonSize;

			// DANGER: 'continue' can screw you up unless you handle any unhandled cases at the end of this loop
			continue;
		}

		for (j = 0; j < lPolygonSize; j++)
		{
			int lControlPointIndex = pMesh->GetPolygonVertex(i, j);

			///////////////////////////////////
			// VERTEX COORDINATES
			///////////////////////////////////
			RECORD_VERTEX_COORD(lControlPoints[lControlPointIndex]);

			if(G_bVerbose)
				Display3DVector("\t\t\t\tCoordinates: ", lControlPoints[lControlPointIndex]);


			///////////////////////////////////
			// VERTEX COLORS
			///////////////////////////////////
			bool storedColor = false;
			for (l = 0; l < pMesh->GetElementVertexColorCount(); l++)
			{
				FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor( l);
				if(G_bVerbose)
					FBXSDK_sprintf(header, 100, "\t\t\t\tColor vertex: "); 

				switch (leVtxc->GetMappingMode())
				{
					case FbxGeometryElement::eByControlPoint:
					{
						switch (leVtxc->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							{
								if(G_bVerbose)
									DisplayColor(header, leVtxc->GetDirectArray().GetAt(lControlPointIndex));
								
								RECORD_VERTEX_COLOR(leVtxc->GetDirectArray().GetAt(lControlPointIndex));
								storedColor = true;

								break;
							}

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leVtxc->GetIndexArray().GetAt(lControlPointIndex);
								if(G_bVerbose)
									DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));

								RECORD_VERTEX_COLOR(leVtxc->GetDirectArray().GetAt(id));
								storedColor = true;

								break;
							}

							default:
								break; // other reference modes not shown here!
						}
						break;
					}

					case FbxGeometryElement::eByPolygonVertex:
					{
						switch (leVtxc->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							{
								if(G_bVerbose)
									DisplayColor(header, leVtxc->GetDirectArray().GetAt(vertexId));
								
								RECORD_VERTEX_COLOR(leVtxc->GetDirectArray().GetAt(vertexId));
								storedColor = true;

								break;
							}

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leVtxc->GetIndexArray().GetAt(vertexId);
								if(G_bVerbose)
									DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));

								RECORD_VERTEX_COLOR(leVtxc->GetDirectArray().GetAt(id));
								storedColor = true;

								break;
							}

							default:
								break; // other reference modes not shown here!
						}

						break;
					}

					case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
					case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
					case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
					{
						break;
					}

				} // end of switch (GetWrappingMode)

			} // end of for(vertexColorCount)

			// if no color found, store a "not found" value
			if(!storedColor)
			{
				col.idxs[j] = COMPONENT_NOT_FOUND_IDX;
			}

			
			///////////////////////////////////
			// VERTEX UV's
			///////////////////////////////////
			bool storedUV = false;

			for (l = 0; l < pMesh->GetElementUVCount(); ++l)
			{
				FbxGeometryElementUV* leUV = pMesh->GetElementUV( l);
				if(G_bVerbose)
					FBXSDK_sprintf(header, 100, "\t\t\t\tTexture UV: "); 

				switch (leUV->GetMappingMode())
				{
					case FbxGeometryElement::eByControlPoint:
					{
						switch (leUV->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							{
								if(G_bVerbose)
									Display2DVector(header, leUV->GetDirectArray().GetAt(lControlPointIndex));

								RECORD_VERTEX_TEX_COORD(leUV->GetDirectArray().GetAt(lControlPointIndex));
								storedUV = true;

								break;
							}

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
								if(G_bVerbose)
									Display2DVector(header, leUV->GetDirectArray().GetAt(id));

								RECORD_VERTEX_TEX_COORD(leUV->GetDirectArray().GetAt(id));
								storedUV = true;

								break;
							}

							default:
								break; // other reference modes not shown here!
						}

						break;
					}

					case FbxGeometryElement::eByPolygonVertex:
					{
						int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);

						switch (leUV->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							case FbxGeometryElement::eIndexToDirect:
							{
								if(G_bVerbose)
									Display2DVector(header, leUV->GetDirectArray().GetAt(lTextureUVIndex));

								RECORD_VERTEX_TEX_COORD(leUV->GetDirectArray().GetAt(lTextureUVIndex));
								storedUV = true;

								break;
							}

							default:
								break; // other reference modes not shown here!
						}

						break;
					}

					case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
					case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
					case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
					{
						break;
					}

				} // end of switch(GetMappingMode)

			} // end of for(...GetElementUVCount...)

			if(!storedUV)
			{
				uvs.idxs[j] = COMPONENT_NOT_FOUND_IDX;
			}



			///////////////////////////////////
			// VERTEX NORMALS
			///////////////////////////////////
			bool foundNormal = false;

			for( l = 0; l < pMesh->GetElementNormalCount(); ++l)
			{
				FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal( l);
				if(G_bVerbose)
					FBXSDK_sprintf(header, 100, "\t\t\t\tNormal: "); 

				if(leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leNormal->GetReferenceMode())
					{
						case FbxGeometryElement::eDirect:
						{
							if(G_bVerbose)
								Display3DVector(header, leNormal->GetDirectArray().GetAt(vertexId));

							RECORD_VERTEX_NORM(leNormal->GetDirectArray().GetAt(vertexId));
							foundNormal = true;

							break;
						}

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray().GetAt(vertexId);
							if(G_bVerbose)
								Display3DVector(header, leNormal->GetDirectArray().GetAt(id));

							RECORD_VERTEX_NORM(leNormal->GetDirectArray().GetAt(id));
							foundNormal = true;

							break;
						}

						default:
							break; // other reference modes not shown here!

					} // end of switch(...GetReferenceMode..)

				} // end of if(...

			}// end of for(...GetElementNormalCount...)

			if(!foundNormal)
			{
				nrm.idxs[j] = COMPONENT_NOT_FOUND_IDX;
			}


			///////////////////////////////////
			// VERTEX TANGENT VECTOR
			///////////////////////////////////
			bool foundTang = false;

			for( l = 0; l < pMesh->GetElementTangentCount(); ++l)
			{
				FbxGeometryElementTangent* leTangent = pMesh->GetElementTangent( l);
				if(G_bVerbose)
					FBXSDK_sprintf(header, 100, "\t\t\t\tTangent: ");

				if(leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leTangent->GetReferenceMode())
					{
						case FbxGeometryElement::eDirect:
						{
							if(G_bVerbose)
								Display3DVector(header, leTangent->GetDirectArray().GetAt(vertexId));

							RECORD_VERTEX_TANG(leTangent->GetDirectArray().GetAt(vertexId));
							foundTang = true;

							break;
						}

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leTangent->GetIndexArray().GetAt(vertexId);
							if(G_bVerbose)
								Display3DVector(header, leTangent->GetDirectArray().GetAt(id));

							RECORD_VERTEX_TANG(leTangent->GetDirectArray().GetAt(id));
							foundTang = true;

							break;
						}

						default:
							break; // other reference modes not shown here!

					} // end of switch(...GetReferenceMode())

				} // end of if(...

			} // end of tangent vector for(...GetElementTangentCount...)

			if(!foundTang)
			{
				tan.idxs[j] = COMPONENT_NOT_FOUND_IDX;
			}


			///////////////////////////////////
			// VERTEX BINORMAL VECTOR
			///////////////////////////////////
			bool foundBinormal = false;

			for( l = 0; l < pMesh->GetElementBinormalCount(); ++l)
			{
				FbxGeometryElementBinormal* leBinormal = pMesh->GetElementBinormal( l);

				if(G_bVerbose)
					FBXSDK_sprintf(header, 100, "\t\t\t\tBinormal: ");

				if(leBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leBinormal->GetReferenceMode())
					{
						case FbxGeometryElement::eDirect:
						{
							if(G_bVerbose)
								Display3DVector(header, leBinormal->GetDirectArray().GetAt(vertexId));

							RECORD_VERTEX_BINORM(leBinormal->GetDirectArray().GetAt(vertexId));
							foundBinormal = true;

							break;
						}

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leBinormal->GetIndexArray().GetAt(vertexId);
							if(G_bVerbose)
								Display3DVector(header, leBinormal->GetDirectArray().GetAt(id));

							RECORD_VERTEX_BINORM(leBinormal->GetDirectArray().GetAt(id));
							foundBinormal = true;

							break;
						}

						default:
							break; // other reference modes not shown here!

					} // end of switch(...GetReferenceMode())

				} // end of if(...

			} // end of binormal loop: for(...GetElementBinormalCount...)

			if(!foundBinormal)
			{
				bin.idxs[j] = COMPONENT_NOT_FOUND_IDX;
			}

			vertexId++;

		} // for polygonSize

		////////////////////////////////////////////////
		// Add found indices
		GetDataPtr()->AddCoordTriIdxs(&pos);
		GetDataPtr()->AddColorTriIdxs(&col);
		GetDataPtr()->AddTexCoordTriIdxs(&uvs);
		GetDataPtr()->AddNormTriIdxs(&nrm);
		GetDataPtr()->AddTangTriIdxs(&tan);
		GetDataPtr()->AddBinormTriIdxs(&bin);

    } // for polygonCount

}









///////////////////////////////////////////////////////////////////////////////////////
// Record a vertex coordinate
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::RecordVertexCoord(FbxVector4 pValue)
{
	Vec3 vertCoord;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertCoord.x = IN_CutPrecision( (float) pValue[0] );
	vertCoord.y = IN_CutPrecision( (float) pValue[1] );
	vertCoord.z = IN_CutPrecision( (float) pValue[2] );

	m_pWriteData->RecordVertCoord(&vertCoord);
}




///////////////////////////////////////////////////////////////////////////////////////
// Record a vertex color
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::RecordVertexColor(FbxColor pValue)
{
	ColorRGBA color;

	color.r = IN_CutPrecision( (float) pValue.mRed );
	color.g = IN_CutPrecision( (float) pValue.mGreen );
	color.b = IN_CutPrecision( (float) pValue.mBlue );
	color.a = IN_CutPrecision( (float) pValue.mAlpha );

	m_pWriteData->RecordVertColor(&color);
}




///////////////////////////////////////////////////////////////////////////////////////
// Record a vertex color
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::RecordVertexTexCoord(FbxVector2 pValue)
{
	TexCoord texC;

	texC.u = IN_CutPrecision( (float) pValue[0] );
	texC.v = IN_CutPrecision( (float) pValue[1] );

	m_pWriteData->RecordVertTexCoord(&texC);
}





///////////////////////////////////////////////////////////////////////////////////////
// Record a vertex normal
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::RecordVertexNormal(FbxVector4 pValue)
{
	Vec3 vertNorm;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertNorm.x = IN_CutPrecision( (float) pValue[0] );
	vertNorm.y = IN_CutPrecision( (float) pValue[1] );
	vertNorm.z = IN_CutPrecision( (float) pValue[2] );

	m_pWriteData->RecordVertNormal(&vertNorm);
}





///////////////////////////////////////////////////////////////////////////////////////
// Record a vertex tangent
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::RecordVertexTangent(FbxVector4 pValue)
{
	Vec3 vertTang;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertTang.x = IN_CutPrecision( (float) pValue[0] );
	vertTang.y = IN_CutPrecision( (float) pValue[1] );
	vertTang.z = IN_CutPrecision( (float) pValue[2] );

	m_pWriteData->RecordVertTangent(&vertTang);
}





///////////////////////////////////////////////////////////////////////////////////////
// Record a vertex binormal
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::RecordVertexBinormal(FbxVector4 pValue)
{
	Vec3 vertBiNorm;

	// All values rounded to 1/10th of a millimeter (enough accuracy I believe - at the time of writing this comment)
	// This will help with vertex welding later on (merging vertex positions that are 'close enough' to be considered the same)

	vertBiNorm.x = IN_CutPrecision( (float) pValue[0] );
	vertBiNorm.y = IN_CutPrecision( (float) pValue[1] );
	vertBiNorm.z = IN_CutPrecision( (float) pValue[2] );

	m_pWriteData->RecordVertBinormal(&vertBiNorm);
}