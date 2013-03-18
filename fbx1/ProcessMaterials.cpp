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
// returns the index of the material if it does, -1 if it doesn't
////////////////////////////////////////////////////////////////////////////////////////
int ProcessMaterials::IsMaterialRecorded(const char *pName)
{
	string name(pName);
	int index = 0;

	// iterate through all materials
	for (std::vector<MaterialData>::iterator it = GetFileDataPtr()->materials.begin() ; it != GetFileDataPtr()->materials.end(); ++it)
	{
		if((*it).name == name)
			return index;
		index++;
	}

	return -1;
}





//////////////////////////////////////////////////////////////////////////////////////////////////
// RECORD MATERIAL ENTRY FUNCTION
// Each mesh has a list of materials associated with it (one for each 'part' of the mesh)
// Every mesh has at least one 'part'/material (there's a corresponding 'part' per material.
// Every material is checked to see if we've already recorded it.  This is simply done
// by checking the material's name, so if there are two materials with the same name, we're
// screwed.
// A list that crossreferences the Fbx list of materials to my global list of materials is made
//////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessMaterials::Start(FbxMesh *pMesh, MaterialMeshXref &matXref)
{
	int materialCnt = pMesh->GetNode()->GetMaterialCount();

	for(int i=0; i < materialCnt; i++)
	{
		FbxSurfaceMaterial *pMaterial = pMesh->GetNode()->GetMaterial(i);

		// Have I already processed this material?
		int matIdx = IsMaterialRecorded(pMaterial->GetName());
		if(matIdx >= 0)
		{
			matXref.newIndices.push_back(matIdx);
			continue; // WARNING!!! For this 'continue' to be safe, make sure nothing important gets done at the end of this loop
		}

	  	if(G_bVerbose)
		    printf("\t\t\tMaterial Name: %s\n", (char *) pMaterial->GetName());

		// if a valid material, record it's xref index
		matIdx = RecordMaterial(pMaterial, i);

		// record what index this material ended up in (-1 if invalid)
		matXref.newIndices.push_back(matIdx);
	}
}





////////////////////////////////////////////////////////////////////////////////////////
// RECORD MATERIAL
// returns the index of the newly added material, -1 if invalid
////////////////////////////////////////////////////////////////////////////////////////
int ProcessMaterials::RecordMaterial(FbxSurfaceMaterial *pMaterial, int materialIndex)
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



	// didn't find a valid material
	if(!recordedData)
		return -1;


	////////////////////////////////////////////////////////
	// Find texture(s) associated with this material
	// Extracting textures happens here as opposed to its
	// own module to cut on the amount of iterations
	// necessary per file.
	////////////////////////////////////////////////////////
	ExtractTextures(pMaterial, materialIndex);

	// DONE
	GetFileDataPtr()->materials.push_back(matDat);

	return (GetFileDataPtr()->materials.size()-1);
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
	TextureData texDat;
	bool recordedData = false;

    if( pProperty.IsValid() )
    {
		int lTextureCount = pProperty.GetSrcObjectCount<FbxTexture>();

		for (int j = 0; j < lTextureCount; ++j)
		{
			//Here we have to check if it's layeredtextures, or just textures:
			FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
			if (lLayeredTexture)
			{
				texDat.isLayered = true;
				if(G_bVerbose)
	                DisplayInt("\t\t\t\t\tLayered Texture: ", j);
                FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
                int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();

                for(int k =0; k<lNbTextures; ++k)
                {
                    FbxTexture* lTexture = lLayeredTexture->GetSrcObject<FbxTexture>(k);
                    if(lTexture)
                    {
                        if(pDisplayHeader)
						{
							if(G_bVerbose)
	                            DisplayInt("\t\t\t\t\tTextures connected to Material ", materialIndex);
                            pDisplayHeader = false;
                        }

                        //NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
                        //Why is that?  because one texture can be shared on different layered textures and might
                        //have different blend modes.

                        FbxLayeredTexture::EBlendMode lBlendMode;
                        lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
						if(G_bVerbose)
						{
	                        DisplayString("\t\t\t\t\tTextures for ", pProperty.GetName());
		                    DisplayInt("\t\t\t\t\tTexture ", k);
						}

						// see if this texture already exists, if not, then record it
						int texId = IsTextureAlreadyRecorded(lTexture);
						if(texId >= 0)
						{
							GetFileDataPtr()->textures[texId].usedByMaterials.push_back(texId);
						}
						else
						{
	                        RecordTextureInfo(lTexture, (int) lBlendMode, &texDat);
							recordedData = true;
						}
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
						if(G_bVerbose)
	                        DisplayInt("\t\t\t\t\tTextures connected to Material ", materialIndex);
                        pDisplayHeader = false;
                    }

                    DisplayString("\t\t\t\t\tTextures for ", pProperty.GetName());
                    DisplayInt("\t\t\t\t\tTexture ", j);

					// see if this texture already exists, if not, then record it
					int texId = IsTextureAlreadyRecorded(lTexture);
					if(texId >= 0)
					{
						GetFileDataPtr()->textures[texId].usedByMaterials.push_back(texId);
					}
					else
					{
	                    RecordTextureInfo(lTexture, -1, &texDat);
						recordedData = true;
					}
                }
            }
        }

    }//end if pProperty



	/////////////////////////////////////////////
	// Record the texture data if we found it
	if(recordedData)
	{
		GetFileDataPtr()->textures.push_back(texDat);
	}
}












///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Find out if this texture is already recorded.
// If so, return the index of the texture in the m_fileDataPtr->textures list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ProcessMaterials::IsTextureAlreadyRecorded(FbxTexture *pTexture)
{
	FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(pTexture);
	string name;

	// The full path to the file is the best name to use to assure uniqueness.
	// If full path name doesn't exist, use the Maya name of the texture.
	if(lFileTexture)
	{
		name = lFileTexture->GetFileName();
	}
	else
	{
		name = pTexture->GetName();
	}

	int texIdx = 0;

	// iterate through all textures
	for (std::vector<TextureData>::iterator it = GetFileDataPtr()->textures.begin() ; it != GetFileDataPtr()->textures.end(); ++it)
	{
		if((*it).name == name)
			return true;

		texIdx++;
	}

	return -1;
}











///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Record the texture info we've found
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessMaterials::RecordTextureInfo(FbxTexture *pTexture, int blendMode, TextureData *pTexDat)
{
	FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(pTexture);
	FbxProceduralTexture *lProceduralTexture = FbxCast<FbxProceduralTexture>(pTexture);

	if(G_bVerbose)
	    DisplayString("\t\t\t\t\t\tName: \"", (char *) pTexture->GetName(), "\"");
	pTexDat->name = pTexture->GetName();

	if (lFileTexture)
	{
		if(G_bVerbose)
		{
			DisplayString("\t\t\t\t\t\tType: File Texture");
			DisplayString("\t\t\t\t\t\tFile Name: \"", (char *) lFileTexture->GetFileName(), "\"");
		}
		pTexDat->filename = lFileTexture->GetFileName();
	}
	else if (lProceduralTexture)
	{
		if(G_bVerbose)
		{
			DisplayString("\t\t\t\t\t\tType: Procedural Texture");
		}
		pTexDat->isProcedural = true;
	}

	if(G_bVerbose)
	{
		DisplayDouble("\t\t\t\t\t\tScale U: ", pTexture->GetScaleU());
		DisplayDouble("\t\t\t\t\t\tScale V: ", pTexture->GetScaleV());
		DisplayDouble("\t\t\t\t\t\tTranslation U: ", pTexture->GetTranslationU());
		DisplayDouble("\t\t\t\t\t\tTranslation V: ", pTexture->GetTranslationV());
		DisplayBool("\t\t\t\t\t\tSwap UV: ", pTexture->GetSwapUV());
		DisplayDouble("\t\t\t\t\t\tRotation U: ", pTexture->GetRotationU());
		DisplayDouble("\t\t\t\t\t\tRotation V: ", pTexture->GetRotationV());
		DisplayDouble("\t\t\t\t\t\tRotation W: ", pTexture->GetRotationW());
	}

	pTexDat->scaleU = (float) pTexture->GetScaleU();
	pTexDat->scaleV = (float) pTexture->GetScaleV();

	pTexDat->translateU = (float) pTexture->GetTranslationU();
	pTexDat->translateV = (float) pTexture->GetTranslationV();

	pTexDat->swapUV = pTexture->GetSwapUV();

	pTexDat->rotateU = (float) pTexture->GetRotationU();
	pTexDat->rotateV = (float) pTexture->GetRotationV();
	pTexDat->rotateW = (float) pTexture->GetRotationW();

    const char* lAlphaSources[] = { "None", "RGB Intensity", "Black" };

	if(G_bVerbose)
	{
		DisplayString("\t\t\t\t\t\tAlpha Source: ", lAlphaSources[pTexture->GetAlphaSource()]);
		DisplayDouble("\t\t\t\t\t\tCropping Left: ", pTexture->GetCroppingLeft());
		DisplayDouble("\t\t\t\t\t\tCropping Top: ", pTexture->GetCroppingTop());
		DisplayDouble("\t\t\t\t\t\tCropping Right: ", pTexture->GetCroppingRight());
		DisplayDouble("\t\t\t\t\t\tCropping Bottom: ", pTexture->GetCroppingBottom());
	}

	pTexDat->alphaSource = (TexAlphaSource) pTexture->GetAlphaSource();

	pTexDat->cropLeft = (float) pTexture->GetCroppingLeft();
	pTexDat->cropRight = (float) pTexture->GetCroppingRight();
	pTexDat->cropTop = (float) pTexture->GetCroppingTop();
	pTexDat->cropBottom = (float) pTexture->GetCroppingBottom();

    const char* lMappingTypes[] = { "Null", "Planar", "Spherical", "Cylindrical", 
        "Box", "Face", "UV", "Environment" };

	if(G_bVerbose)
	    DisplayString("\t\t\t\t\t\tMapping Type: ", lMappingTypes[pTexture->GetMappingType()]);

	pTexDat->mappingType = (TexMappingType) pTexture->GetMappingType();

    if (pTexture->GetMappingType() == FbxTexture::ePlanar)
    {
        const char* lPlanarMappingNormals[] = { "X", "Y", "Z" };

		if(G_bVerbose)
	        DisplayString("\t\t\t\t\t\tPlanar Mapping Normal: ", lPlanarMappingNormals[pTexture->GetPlanarMappingNormal()]);

		pTexDat->planarNormals = (TexPlanarMappingNormals) pTexture->GetPlanarMappingNormal();
    }

    const char* lBlendModes[]   = { "Translucent", "Add", "Modulate", "Modulate2" };   
    if(blendMode >= 0)
	{
		if(G_bVerbose)
	        DisplayString("\t\t\t\t\t\tBlend Mode: ", lBlendModes[blendMode]);

		pTexDat->blendMode = (TexBlendModes) blendMode;
	}

	if(G_bVerbose)
	    DisplayDouble("\t\t\t\t\t\tAlpha: ", pTexture->GetDefaultAlpha());

	pTexDat->defaultAlpha = (float) pTexture->GetDefaultAlpha();

	if (lFileTexture)
	{
		const char* lMaterialUses[] = { "Model Material", "Default Material" };
		if(G_bVerbose)
		    DisplayString("\t\t\t\t\t\tMaterial Use: ", lMaterialUses[lFileTexture->GetMaterialUse()]);

		int material_index = GetFileDataPtr()->materials.size(); // this will be the index of the material currently being added
		pTexDat->usedByMaterials.push_back(material_index);
	}

    const char* pTextureUses[] = { "Standard", "Shadow Map", "Light Map", 
        "Spherical Reflexion Map", "Sphere Reflexion Map", "Bump Normal Map" };

	if(G_bVerbose)
	{
	    DisplayString("\t\t\t\t\t\tTexture Use: ", pTextureUses[pTexture->GetTextureUse()]);
		DisplayString("");
	}

	pTexDat->usedFor = (TexUsedFor) pTexture->GetTextureUse();
}
