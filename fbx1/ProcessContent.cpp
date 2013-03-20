//
// Parse through all the types of nodes in the file: Meshes, Materials, Bones, etc
//



//
// Project Includes
//
#include "fbxdefs.h"
#include "DataTypes.h"
#include "ProcessMesh.h"
#include "WriteData.h"
#include "ProcessContent.h"
#include "Weld.h"
#include "DisplayCommon.h"




///////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR
// This class has a member variables (for example, m_procMesh) with a compulsory initialization parameter (WriteData)
///////////////////////////////////////////////////////////////////////////////////////
ProcessContent::ProcessContent(string in_filename) : m_procMesh(&m_writeData), m_procMat(&m_writeData), m_procLight(&m_writeData)
{
	m_filename = in_filename;
	m_writeData.SetFilename(m_filename);
}



///////////////////////////////////////////////////////////////////////////////////////
// Go through all the children of the root node and call a function that
// recursively iterates through the rest of the tree
///////////////////////////////////////////////////////////////////////////////////////
void ProcessContent::Start(FbxScene* pScene)
{
    int i;
    FbxNode* lNode = pScene->GetRootNode();

	
	
	//////////////////////////////////////////////////////////////
	// GRAB GLOBAL INFO FIRST
	ProcessGlobalData(&pScene->GetGlobalSettings());



	//////////////////////////////////////////////////////////////
	// NOW RECUR THROUGH TREE
    if(lNode)
    {
        for(i = 0; i < lNode->GetChildCount(); i++)
        {
            RecurThroughChildren(lNode->GetChild(i));
        }
    }


	// Weld all components that can be matched and fix indices into triangle list
	m_writeData.WeldData();

	// Get rid of unused materials
	m_procMat.DeleteUnused();
}




///////////////////////////////////////////////////////////////////////////////////////
// This routine does the heavy lifting.
// finds what types of components this scene has, and extracts all the different
// elements one by one: meshes, lights, joints, etc.
///////////////////////////////////////////////////////////////////////////////////////
void ProcessContent::RecurThroughChildren(FbxNode* pNode)
{
    FbxNodeAttribute::EType lAttributeType;
    int i;

    if(pNode->GetNodeAttribute() == NULL)
    {
		if(G_bVerbose)
	        FBXSDK_printf("\t\t\t* NULL Node Attribute\n");
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
			//____________ MESHES + MATERIALS + TEXTURES _____________________________________________
			case FbxNodeAttribute::eMesh:
			{
				m_procMesh.Start(pNode, &m_procMat);
				
				break;
			}

			//_____________ LIGHTS ____________________________________________________________________
	       case FbxNodeAttribute::eLight:
			{
				m_procLight.Start(pNode);
		        break;
			}

		} // end of switch(lAttributeType)
	}

    for(i = 0; i < pNode->GetChildCount(); i++)
    {
        RecurThroughChildren(pNode->GetChild(i));
    }
}




///////////////////////////////////////////////////////////////////////////////////////
// Load pertinent data stored in the fbx global data area
///////////////////////////////////////////////////////////////////////////////////////
void ProcessContent::ProcessGlobalData(FbxGlobalSettings* pGlobalSettings)
{
	if(G_bVerbose)
	    DisplayColor("\t\t\tAmbient Color: ", pGlobalSettings->GetAmbientColor());
	
	FbxColor value = pGlobalSettings->GetAmbientColor();
	ColorRGBA color((float) value.mRed, (float) value.mGreen, (float) value.mBlue, 1.0f);

	m_writeData.GetFileDataPtr()->globals.ambient = color;
}
