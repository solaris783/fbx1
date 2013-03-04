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
ColorRGBA g_defaultColorIfNoneFound = {0.5f, 0.5f, 0.5f, 1.0f};		// for verts




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
	//////////////////////////////////////////////////
	// IMPORTANT: Keep track of component indices
	// These indices correspond in the order into the MeshData structure
	// *** NOTE: at this point all component indices will match: vPos[0] refers to the same vertex as vNorm[0], vTex[0], etc.  This will change later in the process
    int vertexId = 0;
	int polyIdx = 0;

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

		for (j = 0; j < lPolygonSize; j++)
		{
			int lControlPointIndex = pMesh->GetPolygonVertex(i, j);

			///////////////////////////////////
			// VERTEX COORDINATES
			///////////////////////////////////
			RecordVertexCoord(lControlPoints[lControlPointIndex]);			

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
								
								RecordVertexColor(leVtxc->GetDirectArray().GetAt(lControlPointIndex));
								storedColor = true;

								break;
							}

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leVtxc->GetIndexArray().GetAt(lControlPointIndex);
								if(G_bVerbose)
									DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));

								RecordVertexColor(leVtxc->GetDirectArray().GetAt(id));
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
								
								RecordVertexColor(leVtxc->GetDirectArray().GetAt(vertexId));
								storedColor = true;

								break;
							}

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leVtxc->GetIndexArray().GetAt(vertexId);
								if(G_bVerbose)
									DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));

								RecordVertexColor(leVtxc->GetDirectArray().GetAt(id));
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

			// if no color found, store a default
			if(!storedColor)
			{
				RecordVertexColor(&g_defaultColorIfNoneFound);
			}

			
			///////////////////////////////////
			// VERTEX UV's
			///////////////////////////////////
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
								break;
							}

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
								if(G_bVerbose)
									Display2DVector(header, leUV->GetDirectArray().GetAt(id));
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


			///////////////////////////////////
			// VERTEX NORMALS
			///////////////////////////////////
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
							break;
						}

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray().GetAt(vertexId);
							if(G_bVerbose)
								Display3DVector(header, leNormal->GetDirectArray().GetAt(id));
							break;
						}

						default:
							break; // other reference modes not shown here!

					} // end of switch(...GetReferenceMode..)

				} // end of if(...

			}// end of for(...GetElementNormalCount...)


			///////////////////////////////////
			// VERTEX TANGENT VECTOR
			///////////////////////////////////
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
							break;
						}

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leTangent->GetIndexArray().GetAt(vertexId);
							if(G_bVerbose)
								Display3DVector(header, leTangent->GetDirectArray().GetAt(id));
							break;
						}

						default:
							break; // other reference modes not shown here!

					} // end of switch(...GetReferenceMode())

				} // end of if(...

			} // end of tangent vector for(...GetElementTangentCount...)


			///////////////////////////////////
			// VERTEX BINORMAL VECTOR
			///////////////////////////////////
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
							break;
						}

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leBinormal->GetIndexArray().GetAt(vertexId);
							if(G_bVerbose)
								Display3DVector(header, leBinormal->GetDirectArray().GetAt(id));
							break;
						}

						default:
							break; // other reference modes not shown here!

					} // end of switch(...GetReferenceMode())

				} // end of if(...

			} // end of binormal loop: for(...GetElementBinormalCount...)

			vertexId++;

		} // for polygonSize

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
