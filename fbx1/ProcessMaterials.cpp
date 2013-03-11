//
// ProcessMaterials.cpp
// Make a list of all materials in the scene.  Extract all material data.
// Materials are found when going through poly data (ProcessMesh.cpp), and this moduled is called to process them.
// Therefore, if there are materials that don't exist in the poly data, they will not be found/recorded
//



//
// System headers
//
#include <assert.h>




//
// Fbx library headers
//
#include "fbxdefs.h"



//
// Project Includes
//
#include "DisplayCommon.h"
#include "DataTypes.h"
#include "ProcessMaterials.h"
#include "WriteData.h"






////////////////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////////////
// Check to see if this material is already recorded
////////////////////////////////////////////////////////////////////////////////////////
bool ProcessMaterials::IsMaterialRecorded(const char *pName)
{
	string name(pName);

	// iterate through all materials
	for (std::vector<MaterialData>::iterator it = GetFileDataPtr()->materials.begin() ; it != GetFileDataPtr()->materials.end(); ++it)
	{
		if((*it).name == name)
			return true;
	}

	return false;
}





//////////////////////////////////////////////////////////////////////////////////////////////
// RECORD MATERIAL ENTRY FUNCTION
// Each mesh has a list of materials associated with it (one for each 'part' of the mesh)
// Every mesh has at least one 'part'/material (there's a corresponding 'part' per material.
// Every material is checked to see if we've already recorded it.  This is simply done
// by checking the material's name, so if there are two materials with the same name, we're
// screwed.
//////////////////////////////////////////////////////////////////////////////////////////////
void ProcessMaterials::Start(FbxMesh *pMesh)
{
	int materialCnt = pMesh->GetNode()->GetMaterialCount();

	for(int i=0; i < materialCnt; i++)
	{
		FbxSurfaceMaterial *pMaterial = pMesh->GetNode()->GetMaterial(i);

		// Have I already processed this material?
		if(IsMaterialRecorded(pMaterial->GetName()))
			continue; // WARNING!!! For this 'continue' to be safe, make sure nothing important gets done at the end of this loop

	  	if(G_bVerbose)
		    printf("\t\t\tMaterial Name: %s\n", (char *) pMaterial->GetName());

		RecordMaterial(pMaterial, i);
	}
}





////////////////////////////////////////////////////////////////////////////////////////
// RECORD MATERIAL
////////////////////////////////////////////////////////////////////////////////////////
void ProcessMaterials::RecordMaterial(FbxSurfaceMaterial *pMaterial, int materialIndex)
{
	MaterialData matDat; // this is the structure we're going to record in
	bool recordedData = false; // only record this material if we found valid data in it

	//Get the implementation to see if it's a hardware shader (which it shouldn't be)
	const FbxImplementation* lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_HLSL);
	FbxString lImplemenationType = "HLSL";
	if(!lImplementation)
	{
		lImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_CGFX);
		lImplemenationType = "CGFX";
	}

	// we should be reading Maya data, so no implementation should be present
	assert(lImplementation == NULL);

	FbxPropertyT<FbxDouble3> lKFbxDouble3;
	FbxPropertyT<FbxDouble> lKFbxDouble1;
	FbxColor theColor;

	if(pMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId) )
	{
		recordedData = true;

		matDat.shadingModel = SHADING_MODEL_LAMBERT;
	  	if(G_bVerbose)
			printf("\t\t\t\tLambert:\n");

		// We found a Lambert material. Display its properties.
		// Display the Ambient Color
		lKFbxDouble3=((FbxSurfaceLambert *)pMaterial)->Ambient;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		if(G_bVerbose)
			DisplayColor("\t\t\t\t\tAmbient: ", theColor);
		matDat.ambient.r = (float) theColor.mRed;
		matDat.ambient.g = (float) theColor.mGreen;
		matDat.ambient.b = (float) theColor.mBlue;
		matDat.ambient.a = (float) theColor.mAlpha;

		// Display the Diffuse Color
		lKFbxDouble3 =((FbxSurfaceLambert *)pMaterial)->Diffuse;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		if(G_bVerbose)
			DisplayColor("\t\t\t\t\tDiffuse: ", theColor);
		matDat.diffuse.r = (float) theColor.mRed;
		matDat.diffuse.g = (float) theColor.mGreen;
		matDat.diffuse.b = (float) theColor.mBlue;
		matDat.diffuse.a = (float) theColor.mAlpha;

		// Display the Emissive
		lKFbxDouble3 =((FbxSurfaceLambert *)pMaterial)->Emissive;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		if(G_bVerbose)
			DisplayColor("\t\t\t\t\tEmissive: ", theColor);
		matDat.emissive.r = (float) theColor.mRed;
		matDat.emissive.g = (float) theColor.mGreen;
		matDat.emissive.b = (float) theColor.mBlue;
		matDat.emissive.a = (float) theColor.mAlpha;

		// Display the Opacity
		lKFbxDouble1 =((FbxSurfaceLambert *)pMaterial)->TransparencyFactor;
		if(G_bVerbose)
			DisplayDouble("\t\t\t\t\tOpacity: ", 1.0-lKFbxDouble1.Get());
		matDat.opacity = (float) lKFbxDouble1.Get();
	}
	else if (pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		recordedData = true;

		// We found a Phong material.  Display its properties.
		if(G_bVerbose)
			printf("\t\t\t\tPhong:\n");

		// Display the Ambient Color
		lKFbxDouble3 =((FbxSurfacePhong *) pMaterial)->Ambient;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		if(G_bVerbose)
			DisplayColor("\t\t\t\t\tAmbient: ", theColor);
		matDat.ambient.r = (float) theColor.mRed;
		matDat.ambient.g = (float) theColor.mGreen;
		matDat.ambient.b = (float) theColor.mBlue;
		matDat.ambient.a = (float) theColor.mAlpha;

		// Display the Diffuse Color
		lKFbxDouble3 =((FbxSurfacePhong *) pMaterial)->Diffuse;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		if(G_bVerbose)
			DisplayColor("\t\t\t\t\tDiffuse: ", theColor);
		matDat.diffuse.r = (float) theColor.mRed;
		matDat.diffuse.g = (float) theColor.mGreen;
		matDat.diffuse.b = (float) theColor.mBlue;
		matDat.diffuse.a = (float) theColor.mAlpha;

		// Display the Specular Color (unique to Phong materials)
		lKFbxDouble3 =((FbxSurfacePhong *) pMaterial)->Specular;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		if(G_bVerbose)
			DisplayColor("\t\t\t\t\tSpecular: ", theColor);
		matDat.specular.r = (float) theColor.mRed;
		matDat.specular.g = (float) theColor.mGreen;
		matDat.specular.b = (float) theColor.mBlue;
		matDat.specular.a = (float) theColor.mAlpha;

		// Display the Emissive Color
		lKFbxDouble3 =((FbxSurfacePhong *) pMaterial)->Emissive;
		theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
		if(G_bVerbose)
			DisplayColor("\t\t\t\t\tEmissive: ", theColor);
		matDat.emissive.r = (float) theColor.mRed;
		matDat.emissive.g = (float) theColor.mGreen;
		matDat.emissive.b = (float) theColor.mBlue;
		matDat.emissive.a = (float) theColor.mAlpha;

		//Opacity is Transparency factor now
		lKFbxDouble1 =((FbxSurfacePhong *) pMaterial)->TransparencyFactor;
		if(G_bVerbose)
			DisplayDouble("\t\t\t\t\tOpacity: ", 1.0-lKFbxDouble1.Get());
		matDat.opacity = (float) lKFbxDouble1.Get();

		// Display the Shininess
		lKFbxDouble1 =((FbxSurfacePhong *) pMaterial)->Shininess;
		if(G_bVerbose)
			DisplayDouble("\t\t\t\t\tShininess: ", lKFbxDouble1.Get());
		matDat.shininess = (float) lKFbxDouble1.Get();

		// Display the Reflectivity
		lKFbxDouble1 =((FbxSurfacePhong *) pMaterial)->ReflectionFactor;
		if(G_bVerbose)
			DisplayDouble("\t\t\t\t\tReflectivity: ", lKFbxDouble1.Get());
		matDat.reflectivity = (float) lKFbxDouble1.Get();
	}
	else if(G_bVerbose)
		DisplayString("\t\t\t\tUnknown type of Material");



	////////////////////////////////////////////////////////
	// Find texture(s) associated with this material
	////////////////////////////////////////////////////////
	ExtractTextures(pMaterial, materialIndex);



	// DONE
	if(recordedData)
	{
		GetFileDataPtr()->materials.push_back(matDat);
	}
}








///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Extract any textures associated with this material
// I do it this way so that I only record textures used by materials which themselves are used by meshes
// (i.e. avoid any unused data)
// This function loops through all textures looking for one that matches this material.
// Once found, a different function records that texture's data
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessMaterials::ExtractTextures(FbxSurfaceMaterial *pMaterial, int materialIndex)
{
    FbxProperty lProperty;
	bool lDisplayHeader = true;
	int lTextureIndex;

	FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
	{
		lProperty = pMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
		FindTextureInfoByProperty(lProperty, lDisplayHeader, materialIndex); 
	}
}








///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Extract any textures associated with this material
// I do it this way so that I only record textures used by materials which themselves are used by meshes
// (i.e. avoid any unused data)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessMaterials::FindTextureInfoByProperty(FbxProperty pProperty, bool &pDisplayHeader, int materialIndex)
{
    if( pProperty.IsValid() )
    {
		int lTextureCount = pProperty.GetSrcObjectCount<FbxTexture>();

		for (int j = 0; j < lTextureCount; ++j)
		{
			//Here we have to check if it's layeredtextures, or just textures:
			FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
			if (lLayeredTexture)
			{
                DisplayInt("    Layered Texture: ", j);
                FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
                int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();

                for(int k =0; k<lNbTextures; ++k)
                {
                    FbxTexture* lTexture = lLayeredTexture->GetSrcObject<FbxTexture>(k);
                    if(lTexture)
                    {
                        if(pDisplayHeader)
						{                    
                            DisplayInt("    Textures connected to Material ", materialIndex);
                            pDisplayHeader = false;
                        }

                        //NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
                        //Why is that?  because one texture can be shared on different layered textures and might
                        //have different blend modes.

                        FbxLayeredTexture::EBlendMode lBlendMode;
                        lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
                        DisplayString("    Textures for ", pProperty.GetName());
                        DisplayInt("        Texture ", k);  
                        RecordTextureInfo(lTexture, (int) lBlendMode);   
                    }

                }
            }
			else
			{
				//no layered texture simply get on the property
                FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                if(lTexture)
                {
                    //display connected Material header only at the first time
                    if(pDisplayHeader)
					{
                        DisplayInt("    Textures connected to Material ", materialIndex);
                        pDisplayHeader = false;
                    }             

                    DisplayString("    Textures for ", pProperty.GetName());
                    DisplayInt("        Texture ", j);  
                    RecordTextureInfo(lTexture, -1);
                }
            }
        }

    }//end if pProperty

}








///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Record the texture info we've found
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessMaterials::RecordTextureInfo(FbxTexture *pTexture, int blendMode)
{
	FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(pTexture);
	FbxProceduralTexture *lProceduralTexture = FbxCast<FbxProceduralTexture>(pTexture);

    DisplayString("            Name: \"", (char *) pTexture->GetName(), "\"");
	if (lFileTexture)
	{
		DisplayString("            Type: File Texture");
		DisplayString("            File Name: \"", (char *) lFileTexture->GetFileName(), "\"");
	}
	else if (lProceduralTexture)
	{
		DisplayString("            Type: Procedural Texture");
	}
    DisplayDouble("            Scale U: ", pTexture->GetScaleU());
    DisplayDouble("            Scale V: ", pTexture->GetScaleV());
    DisplayDouble("            Translation U: ", pTexture->GetTranslationU());
    DisplayDouble("            Translation V: ", pTexture->GetTranslationV());
    DisplayBool("            Swap UV: ", pTexture->GetSwapUV());
    DisplayDouble("            Rotation U: ", pTexture->GetRotationU());
    DisplayDouble("            Rotation V: ", pTexture->GetRotationV());
    DisplayDouble("            Rotation W: ", pTexture->GetRotationW());

    const char* lAlphaSources[] = { "None", "RGB Intensity", "Black" };

    DisplayString("            Alpha Source: ", lAlphaSources[pTexture->GetAlphaSource()]);
    DisplayDouble("            Cropping Left: ", pTexture->GetCroppingLeft());
    DisplayDouble("            Cropping Top: ", pTexture->GetCroppingTop());
    DisplayDouble("            Cropping Right: ", pTexture->GetCroppingRight());
    DisplayDouble("            Cropping Bottom: ", pTexture->GetCroppingBottom());

    const char* lMappingTypes[] = { "Null", "Planar", "Spherical", "Cylindrical", 
        "Box", "Face", "UV", "Environment" };

    DisplayString("            Mapping Type: ", lMappingTypes[pTexture->GetMappingType()]);

    if (pTexture->GetMappingType() == FbxTexture::ePlanar)
    {
        const char* lPlanarMappingNormals[] = { "X", "Y", "Z" };

        DisplayString("            Planar Mapping Normal: ", lPlanarMappingNormals[pTexture->GetPlanarMappingNormal()]);
    }

    const char* lBlendModes[]   = { "Translucent", "Add", "Modulate", "Modulate2" };   
    if(pBlendMode >= 0)
        DisplayString("            Blend Mode: ", lBlendModes[pBlendMode]);
    DisplayDouble("            Alpha: ", pTexture->GetDefaultAlpha());

	if (lFileTexture)
	{
		const char* lMaterialUses[] = { "Model Material", "Default Material" };
	    DisplayString("            Material Use: ", lMaterialUses[lFileTexture->GetMaterialUse()]);
	}

    const char* pTextureUses[] = { "Standard", "Shadow Map", "Light Map", 
        "Spherical Reflexion Map", "Sphere Reflexion Map", "Bump Normal Map" };

    DisplayString("            Texture Use: ", pTextureUses[pTexture->GetTextureUse()]);
    DisplayString("");
}
