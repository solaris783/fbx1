//
// ProcessLights.cpp
// Make a list of all lights in the scene.  Extract all light data.
//




//
// Project Includes
//
#include "DisplayCommon.h"
#include "DataTypes.h"
#include "ProcessLights.h"
#include "WriteData.h"








//////////////////////////////////////////////////////////////////////////////////////////////////
// RECORD LIGHTS ENTRY FUNCTION
//////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessLights::Start(FbxNode* pNode)
{
    FbxLight* lLight = (FbxLight*) pNode->GetNodeAttribute();

  	if(G_bVerbose)
	{
	    DisplayString("\t\t\tLight Name: ", (char *) pNode->GetName());
	    DisplayMetaDataConnections(lLight);

		const char* lLightTypes[] = { "Point", "Directional", "Spot" };

		DisplayString("\t\t\t\tType: ", lLightTypes[lLight->LightType.Get()]);
		DisplayBool("\t\t\t\tCast Light: ", lLight->CastLight.Get());
	}


    if (!(lLight->FileName.Get().IsEmpty()))
    {
	  	if(G_bVerbose)
		{
			DisplayString("\t\t\t\tGobo");

			DisplayString("\t\t\t\tFile Name: \"", lLight->FileName.Get().Buffer(), "\"");
			DisplayBool("\t\t\t\tGround Projection: ", lLight->DrawGroundProjection.Get());
			DisplayBool("\t\t\t\tVolumetric Projection: ", lLight->DrawVolumetricLight.Get());
			DisplayBool("\t\t\t\tFront Volumetric Projection: ", lLight->DrawFrontFacingVolumetricLight.Get());
		}
    }


  	if(G_bVerbose)
	{
		DisplayString("\t\t\t\tDefault Animation Values");

		FbxDouble3 c = lLight->Color.Get();
		FbxColor lColor(c[0], c[1], c[2]);
		DisplayColor("\t\t\t\t\tDefault Color: ", lColor);
		DisplayDouble("\t\t\t\t\tDefault Intensity: ", pLight->Intensity.Get());
		DisplayDouble("\t\t\t\t\tDefault Outer Angle: ", pLight->OuterAngle.Get());
		DisplayDouble("\t\t\t\t\tDefault Fog: ", pLight->Fog.Get());
	}
}
