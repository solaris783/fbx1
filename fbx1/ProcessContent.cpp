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




///////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR
///////////////////////////////////////////////////////////////////////////////////////
ProcessContent::ProcessContent(string in_filename)
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

    if(lNode)
    {
        for(i = 0; i < lNode->GetChildCount(); i++)
        {
            RecurThroughChildren(lNode->GetChild(i));
        }
    }

	// TODO: Weld all components that can be matched and fix indices into triangle list
	int a;
	a = 1;
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
			//_________________________________________________________
			case FbxNodeAttribute::eMesh:
			{
				m_procMesh.Start(pNode, &m_writeData);

				break;
			}

		} // end of switch(lAttributeType)
	}

    for(i = 0; i < pNode->GetChildCount(); i++)
    {
        RecurThroughChildren(pNode->GetChild(i));
    }
}
