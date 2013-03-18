/////////////////////////////////////////////////////////////////
// Define all data type defintions for the data extracted from
// the .fbx file
/////////////////////////////////////////////////////////////////

#ifndef __DATA_TYPES__H
#define __DATA_TYPES__H

// sytem includes
#include <string>
#include <vector>


using namespace std;	// to avoid having to write std:: every time I want to use a standard library func


struct TexCoord
{
	TexCoord() {}
	TexCoord(const TexCoord &uv) : u(uv.u), v(uv.v) {}
	TexCoord(float u, float v) : u(u), v(v) {}

	const TexCoord & operator =(const TexCoord & uv) { u = uv.u; v = uv.v; return *this; }
	bool operator ==(const TexCoord & uv) const { return u == uv.u && v == uv.v; }
	bool operator !=(const TexCoord & uv) const { return u != uv.u || v != uv.v; }

	float u;
	float v;
};

struct Int3
{
	int idxs[3];
};

struct Vec3
{
	Vec3() {}
	Vec3(const Vec3 & v) : x(v.x), y(v.y), z(v.z) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	const Vec3 & operator =(const Vec3 & v) { x = v.x; y = v.y; z = v.z; return *this; }
	bool operator ==(const Vec3 & v) const { return x == v.x && y == v.y && z == v.z; }
	bool operator !=(const Vec3 & v) const { return x != v.x || y != v.y || z != v.z; }

	float x;
	float y;
	float z;
};

struct Vec4
{
	float x;
	float y;
	float z;
	float w;
};

struct ColorRGBA
{
	ColorRGBA() {}
	ColorRGBA(const ColorRGBA & col) : r(col.r), g(col.g), b(col.b), a(col.a) {}
	ColorRGBA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	const ColorRGBA & operator =(const ColorRGBA & col) { r = col.r; g = col.g; b = col.b; a = col.a; return *this; }
	bool operator ==(const ColorRGBA & col) const { return r == col.r && g == col.g && b == col.b && a == col.a; }
	bool operator !=(const ColorRGBA & col) const { return r != col.r || g != col.g || b != col.b || a != col.a; }

	float r;
	float g;
	float b;
	float a;
};

// bits declaring which fields where found in the data file (Maya file, Fbx file)
struct UsingFields
{
	unsigned vpos : 1;
	unsigned vnorm : 1;
	unsigned vbinormal: 1;
	unsigned vtangent : 1;
	unsigned vcolor : 1;
	unsigned vtexCoord : 1;
};

// for holding the list of materials each tri will hold
struct MatList
{
	vector<int> list;
};

// indices into the triangle components.  These correspond 1:1 so that iPos[0] corresponds to iNrm[0], iTex[0], etc.  Done this way for caching performance.
// For example, iPos[0].idxs[0-2] where [0-2] represent the 3 indices into that component's list (i.e. vPos)
struct TriList
{
	vector<Int3> iPos;
	vector<Int3> iNrm;
	vector<Int3> iTex;
	vector<Int3> iCol;
	vector<Int3> iBin;
	vector<Int3> iTan;
	vector<MatList>  iMat; // which material is used by this triangle
};

struct MeshData
{
	// Raw values for all components.  Kept separate for better performance when we weld values later on.
	vector<Vec3> vPos;
	vector<Vec3> vNorm;
	vector<TexCoord> vTex;
	vector<ColorRGBA> vColor;
	vector<Vec3> vBinorm;
	vector<Vec3> vTang;

	UsingFields m_usingFields;

	TriList tris;
};

enum ShadingModel
{
	SHADING_MODEL_LAMBERT,
	SHADING_MODEL_PHONG
};

enum TexAlphaSource
{
	TEX_ALPHA_SOURCE_NONE,
	TEX_ALPHA_SOURCE_RGB_INTENSITY,
	TEX_ALPHA_SOURCE_BLACK
};


enum TexMappingType
{
	TEXMAPPING_TYPE_NULL,
	TEXMAPPING_TYPE_PLANAR,
	TEXMAPPING_TYPE_SPHERICAL,
	TEXMAPPING_TYPE_CYLINDRICAL,
	TEXMAPPING_TYPE_BOX,
	TEXMAPPING_TYPE_FACE,
	TEXMAPPING_TYPE_UV,
	TEXMAPPING_TYPE_ENVIRONMENT
};


enum TexPlanarMappingNormals
{
	TEX_PLANARMAPPING_NORMALS_X,
	TEX_PLANARMAPPING_NORMALS_Y,
	TEX_PLANARMAPPING_NORMALS_Z,
};


enum TexBlendModes
{
	TEX_BLENDMODES_TRANSLUCENT,
	TEX_BLENDMODES_ADD,
	TEX_BLENDMODES_MODULATE,
	TEX_BLENDMODES_MODULATE2
};


enum TexUsedFor
{
	TEXUSE_STANDARD,
	TEXUSE_SHADOWMAP,
	TEXUSE_LIGHTMAP,
	TEXUSE_SPHERICAL_REFLEXION_MAP,
	TEXUSE_SPHERE_REFLEXION_MAP,
	TEXUSE_BUMP_NORMAL_MAP
};


struct TextureData
{
	string name;
	string filename; // full path

	TexAlphaSource alphaSource;
	TexMappingType mappingType;
	TexPlanarMappingNormals planarNormals;
	TexBlendModes blendMode;
	TexUsedFor usedFor;

	vector<int> usedByMaterials;
	bool isProcedural;
	bool isLayered;
	bool usesDefaultMaterial;
	bool swapUV;

	float scaleU;
	float scaleV;
	float translateU;
	float translateV;
	float rotateU;
	float rotateV;
	float rotateW;
	float cropLeft;
	float cropTop;
	float cropRight;
	float cropBottom;
	float defaultAlpha;

	///////////////////////////////////////////////////////////////
	// MUST, MUST, MUST, MUST
	// Initialize all members
	TextureData() : alphaSource(TEX_ALPHA_SOURCE_NONE), 
					mappingType(TEXMAPPING_TYPE_NULL),
					planarNormals(TEX_PLANARMAPPING_NORMALS_X),
					blendMode(TEX_BLENDMODES_TRANSLUCENT),
					usedFor(TEXUSE_STANDARD),
					isProcedural(false),
					isLayered(false),
					usesDefaultMaterial(false),
					swapUV(false),
					scaleU(0.0f),
					scaleV(0.0f),
					translateU(0.0f),
					translateV(0.0f),
					rotateU(0.0f),
					rotateV(0.0f),
					rotateW(0.0f),
					cropLeft(0.0f),
					cropTop(0.0f),
					cropRight(0.0f),
					cropBottom(0.0f),
					defaultAlpha(0.0f) {}
};



struct MaterialData
{
	bool used; // whether this material is referenced by any poly or not
	string name;
	vector<int> textureIdx; // index of the texture(s) in the texture list that this material uses. A single material can have multiple textures associated with it.
	ColorRGBA ambient;
	ColorRGBA diffuse;
	ColorRGBA specular;
	ColorRGBA emissive;
	float opacity; // transparency factor
	float shininess;
	float reflectivity;
	ShadingModel shadingModel;

	///////////////////////////////////////////////////////////////
	// MUST, MUST, MUST, MUST
	// Initialize all members
	MaterialData() : ambient(0.0f, 0.0f, 0.0f, 0.0f),
						diffuse(0.0f, 0.0f, 0.0f, 0.0),
						specular(0.0f, 0.0f, 0.0f, 0.0),
						emissive(0.0f, 0.0f, 0.0f, 0.0),
						opacity(0.0f),
						shininess(0.0f),
						reflectivity(0.0f),
						used(false),
						shadingModel(SHADING_MODEL_LAMBERT) {}
};


//////////////////////////////////////////////////////////////////////
// This will hold a list of materials for a mesh
// The indices in the vector correspond to the mesh material index
// i.e. MaterialMeshXref[0] ==  pMesh->GetNode()->GetMaterial(0)
// The value in stored in the vector corresponds to the the index in
// my list of materials.
///////////////////////////////////////////////////////////////////////
struct MaterialMeshXref
{
	vector<int> newIndices;
};



enum LightTypes
{
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_SPOT
};


struct GoboData
{
	string filename;
	bool doesProjectToGround;
	bool isVolumetricProjection;
	bool isFrontVolumetricProjection;
};


struct LightData
{
	string name;
	LightTypes type;
	bool isCastLight;
	bool isGobo;
	GoboData gobo;
	ColorRGBA color;
	float intensity;
	float outerAngle;
	float fog;
};

struct GlobalData
{
	ColorRGBA ambient;
};



struct FileData
{
	string filename;
	MeshData meshData;
	vector<MaterialData> materials;
	vector<TextureData> textures;
	vector<LightData> lights;
	GlobalData globals;
};



//
// DEFINES
//
// the max precision is based on decimal digits: 10 would be 1/10th of a meter, 100 = 1/100th of a meter, 1000 = millimeter, etc.
#define MAX_PRECISION_POS	10000
#define MAX_PRECISION_NRM	10000
#define MAX_PRECISION_TEX	10000



//
// Inlines
//
inline float IN_CutPrecision(float in, float max_precision = MAX_PRECISION_POS)
{
	return (floor(in * max_precision) / max_precision);
}



#endif
