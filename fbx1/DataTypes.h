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
};

struct MeshData
{
	string name;

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

struct FileData
{
	string filename;
	MeshData meshData;
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
