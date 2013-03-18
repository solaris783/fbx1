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
	LightData lightData;
    FbxLight* lLight = (FbxLight*) pNode->GetNodeAttribute();

  	if(G_bVerbose)
	{
	    DisplayString("\t\t\tLight Name: ", (char *) pNode->GetName());
	    DisplayMetaDataConnections(lLight);

		const char* lLightTypes[] = { "Point", "Directional", "Spot" };

		DisplayString("\t\t\t\tType: ", lLightTypes[lLight->LightType.Get()]);
		DisplayBool("\t\t\t\tCast Light: ", lLight->CastLight.Get());
	}

	lightData.name = pNode->GetName();
	lightData.type = (LightTypes) lLight->LightType.Get();
	lightData.isCastLight = lLight->CastLight.Get();
	lightData.isGobo = false;

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

		lightData.isGobo = true;
		lightData.gobo.filename = lLight->FileName.Get().Buffer();
		lightData.gobo.doesProjectToGround = lLight->DrawGroundProjection.Get();
		lightData.gobo.isVolumetricProjection = lLight->DrawVolumetricLight.Get();
		lightData.gobo.isFrontVolumetricProjection = lLight->DrawFrontFacingVolumetricLight.Get();
    }


	FbxDouble3 c = lLight->Color.Get();
	FbxColor lColor(c[0], c[1], c[2]);
	ColorRGBA color((float) c[0], (float) c[1], (float) c[2], 1.0f);

  	if(G_bVerbose)
	{
		DisplayString("\t\t\t\tDefault Animation Values");
		DisplayColor("\t\t\t\t\tDefault Color: ", lColor);
		DisplayDouble("\t\t\t\t\tDefault Intensity: ", lLight->Intensity.Get());
		DisplayDouble("\t\t\t\t\tDefault Outer Angle: ", lLight->OuterAngle.Get());
		DisplayDouble("\t\t\t\t\tDefault Fog: ", lLight->Fog.Get());
	}

	lightData.color = color;
	lightData.intensity = (float) lLight->Intensity.Get();
	lightData.outerAngle = (float) lLight->OuterAngle.Get();
	lightData.fog = (float) lLight->Fog.Get();

	GetFileDataPtr()->lights.push_back(lightData);
}
