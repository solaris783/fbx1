//
// Go through the Mesh node and break it down into vertices, normals, uv's, etc.
//




//
// Standard library includes
//




//
// Fbx library headers
//
#include "fbxdefs.h"



//
// Project Includes
//
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
#define RECORD_VERTEX_COORD(arg) { GetWrtDataPtr()->RecordVertCoord(arg); pos.idxs[j] = posIdx; posIdx++; }
#define RECORD_VERTEX_COLOR(arg) { GetWrtDataPtr()->RecordVertColor(arg); col.idxs[j] = colIdx; colIdx++; }
#define RECORD_VERTEX_TEX_COORD(arg) { GetWrtDataPtr()->RecordVertTexCoord(arg); uvs.idxs[j] = uvsIdx; uvsIdx++; }
#define RECORD_VERTEX_NORM(arg) { GetWrtDataPtr()->RecordVertNormal(arg); nrm.idxs[j] = nrmIdx; nrmIdx++; }
#define RECORD_VERTEX_TANG(arg) { GetWrtDataPtr()->RecordVertTangent(arg); tan.idxs[j] = tanIdx; tanIdx++; }
#define RECORD_VERTEX_BINORM(arg) { GetWrtDataPtr()->RecordVertBinormal(arg); bin.idxs[j] = binIdx; binIdx++; }






///////////////////////////////////////////////////////////////////////////////////////
// Go through the root node extracting all pertinent info
// If we find any poly data in this mesh, we will go ahead and iterate through all the
// materials used by this mesh and record any used ones.  It may look a bit weird to
// find ProcessMesh calling ProcessMaterials, but those two things are coupled together
// and we only want to record materials we have mesh information for.
///////////////////////////////////////////////////////////////////////////////////////
void ProcessMesh::Start(FbxNode* pNode, ProcessMaterials *pProcMat)
{
    FbxMesh* lMesh = (FbxMesh*) pNode->GetNodeAttribute();
	if(!lMesh)
		return;

	if(G_bVerbose)
	    printf("\t\t\tMesh Name: %s\n", (char *) pNode->GetName());

	// extract data out of mesh
	if(ProcessPolygonInfo(lMesh))
	{
		////////////////////////////////////////////////////////////////////////////////////
		// Found mesh with some poly data in it.  Record any materials associated with it
		pProcMat->Start(lMesh);
	}
}





///////////////////////////////////////////////////////////////////////////////////////
// Go through the Mesh node and break it down into vertices, normals, uv's, etc.
// Return whether it managed to record any data off of this mesh
///////////////////////////////////////////////////////////////////////////////////////
bool ProcessMesh::ProcessPolygonInfo(FbxMesh* pMesh)
{
	bool nonTriangleFound = false;
	bool recordedAny = false;

	 
	

	//////////////////////////////////////////////////
	// IMPORTANT: Keep track of component indices
	// We have a running list of all raw data added for entire file (not just this mesh)
	// These indices correspond in the order into the MeshData structure
	// *** NOTE: at this point all component indices will match: vPos[0] refers to the same vertex as vNorm[0], vTex[0], etc.  This will change later in the process
	int posIdx = GetWrtDataPtr()->GetCurrVertCoordIndex();
	int colIdx = GetWrtDataPtr()->GetCurrVertColorIndex();
	int uvsIdx = GetWrtDataPtr()->GetCurrVertTexCoordIndex();
	int nrmIdx = GetWrtDataPtr()->GetCurrVertNormIndex();
	int binIdx = GetWrtDataPtr()->GetCurrVertBinormIndex();
	int tanIdx = GetWrtDataPtr()->GetCurrVertTangIndex();



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
			recordedAny = true; // yay, we found something!

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
		GetWrtDataPtr()->AddCoordTriIdxs(&pos);
		GetWrtDataPtr()->AddColorTriIdxs(&col);
		GetWrtDataPtr()->AddTexCoordTriIdxs(&uvs);
		GetWrtDataPtr()->AddNormTriIdxs(&nrm);
		GetWrtDataPtr()->AddTangTriIdxs(&tan);
		GetWrtDataPtr()->AddBinormTriIdxs(&bin);

    } // for polygonCount

	return recordedAny;
}
