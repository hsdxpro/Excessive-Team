#pragma once
// Importing 3D models based on Assimp (Open Asset Import Library)

//COMMON INTERCHANGE FORMATS
//Collada(.dae)
//Blender 3D (.blend)
//3ds Max 3DS (.3ds)
//3ds Max ASE(.ase)
//Wavefront Object(.obj)
//Industry Foundation Classes(IFC / Step) (.ifc)
//XGL(.xgl, .zgl)
//Stanford Polygon Library(.ply)
//*AutoCAD DXF(.dxf)
//LightWave(.lwo)
//LightWave Scene(.lws)
//Modo(.lxo)
//Stereolithography(.stl)
//DirectX X(.x)
//AC3D(.ac)
//Milkshape 3D (.ms3d)
//* TrueSpace(.cob, .scn)

//MOTION CAPTURE FORMATS
//Biovision BVH(.bvh)
//* CharacterStudio Motion(.csm)

//GRAPHICS ENGINE FORMATS
//Ogre XML(.xml)
//Irrlicht Mesh(.irrmesh)
//* Irrlicht Scene(.irr)

//GAME FILE FORMATS
//Quake I(.mdl)
//Quake II(.md2)
//Quake III Mesh(.md3)
//Quake III Map / BSP(.pk3)
//* Return to Castle Wolfenstein(.mdc)
//Doom 3 (.md5*)
//*Valve Model(.smd, .vta)
//*Starcraft II M3(.m3)
//*Unreal(.3d)

//OTHER FILE FORMATS
//BlitzBasic 3D (.b3d)
//Quick3D(.q3d, .q3s)
//Neutral File Format(.nff)
//Sense8 WorldToolKit(.nff)
//Object File Format(.off)
//PovRAY Raw(.raw)
//Terragen Terrain(.ter)
//3D GameStudio(3DGS) (.mdl)
//3D GameStudio(3DGS) Terrain(.hmp)
//Izware Nendo(.ndo)

#include "Types.h"
#include "mymath/mymath.h"
#include "PlatformLibrary/File.h"
#include "PlatformLibrary/WindowCommon.h"

#include <vector>
#include <limits>

// Flags for importing
ENUM_CLASS_BITFLAG( eImporter3DFlag, u32 )
{
	VERT_ATTR_POS,
	VERT_ATTR_TEX0,
	VERT_ATTR_NORM,
	VERT_ATTR_TAN,
	VERT_ATTR_BITAN,
	VERT_ATTR_BONE_WEIGHTS,
	VERT_ATTR_BONE_INDICES,
	VERT_BUFF_INTERLEAVED,
	VERT_BUFF_NON_INTERLEAVED,
	PIVOT_RECENTER,
	COUNT,
};

// Configuring importer for importing, for example u dont want tangent for output meshes
struct rImporter3DCfg
{
	rImporter3DCfg()
	{
		// Default importing all attributes in interleaved form
		flags.resize(6);
		flags[0] = eImporter3DFlag::VERT_ATTR_POS;
		flags[1] = eImporter3DFlag::VERT_ATTR_TEX0;
		flags[2] = eImporter3DFlag::VERT_ATTR_NORM;
		flags[3] = eImporter3DFlag::VERT_ATTR_TAN;
		flags[4] = eImporter3DFlag::VERT_ATTR_BITAN;
		flags[5] = eImporter3DFlag::VERT_ATTR_BONE_INDICES;
		flags[6] = eImporter3DFlag::VERT_ATTR_BONE_WEIGHTS;
		flags[7] = eImporter3DFlag::VERT_BUFF_INTERLEAVED;
	}

	rImporter3DCfg(const std::vector<eImporter3DFlag>& flags)
		:flags(flags)
	{}

	bool isContain(eImporter3DFlag f) const
	{
		for (auto& a : flags)
			if (a == f)
				return true;
		return false;
	}

	std::vector<eImporter3DFlag> flags;
};


// Output meshes
struct rImporter3DMesh
{
	rImporter3DMesh() : nVertices(0), vertexSize(0) {}


	~rImporter3DMesh()
	{
		delete[] indices; // Delete indices

		for (auto& v : vertexBuffers)
			delete[] v;
	}

	bool Serialize(File& file)
	{
		file << nVertices;
		file << vertexSize;

		file << (u64)vertexBuffers.size();
		for (auto& ptr : vertexBuffers)
			file.Write(ptr, nVertices * vertexSize);

		file << indexSize;
		file << nIndices;

		file.Write(indices, indexSize * nIndices);

		bool bSuccess = true;
		file << (u64)materials.size();

		for (auto& mat : materials)
			bSuccess &= mat.Serialize(file);
		return bSuccess;
	}

	bool DeSerialize(File& file)
	{
		nVertices << file;
		vertexSize << file;

		size_t nVertexBuffers = file.Read<u64>();

		vertexBuffers.resize(nVertexBuffers);

		for (auto& ptr : vertexBuffers)
		{
			ptr = new u8[nVertices * vertexSize];
			file.Read(ptr, nVertices * vertexSize);
		}

		indexSize << file;
		nIndices << file;

		indices = new u8[indexSize * nIndices];
		file.Read(indices, indexSize * nIndices);

		materials.resize(file.Read<u64>());

		bool bSuccess = true;
		for (auto& mat : materials)
			bSuccess &= mat.DeSerialize(file);
		return bSuccess;
	}

	// Output material for mesh
	struct rMaterial
	{
		rMaterial() : faceStartIdx(std::numeric_limits<u32>::max()), faceEndIdx(std::numeric_limits<u32>::max()) {}

		bool Serialize(File& file)
		{
			file << faceStartIdx;
			file << faceEndIdx;

			file << relTexPathDiffuse;
			file << relTexPathNormal;
			return true;
		}

		bool DeSerialize(File& file)
		{
			faceStartIdx << file;
			faceEndIdx << file;

			relTexPathDiffuse << file;
			relTexPathNormal << file;

			return true;
		}

		u64 faceStartIdx;
		u64 faceEndIdx;

		std::string relTexPathDiffuse;
		std::string relTexPathNormal;
	};

	// Vertices
	u64 nVertices;
	u64 vertexSize;
	std::vector<void*> vertexBuffers;

	// Indices
	void* indices;
	u64 indexSize;
	u64 nIndices;

	// Materials
	std::vector<rMaterial> materials;
};

// Importer output 
struct rImporter3DData
{
	~rImporter3DData()
	{
		for (auto& a : meshes)
			delete a;
	}

	bool Serialize(const std::string& filePath)
	{
		File file(filePath, eIO::OUT_BIN_TRUNC);

		file << (u64)meshes.size();

		bool bSuccess = true;
		for (auto& mesh : meshes)
			bSuccess &= mesh->Serialize(file);

		file.Close();
		return bSuccess;
	}

	bool DeSerialize(const std::string& filePath)
	{
		File file(filePath, eIO::IN_BIN);

		meshes.resize(file.Read<u64>());

		bool bSuccess = true;
		for (auto& mesh : meshes)
		{
			mesh = new rImporter3DMesh();
			bSuccess &= mesh->DeSerialize(file);
		}

		file.Close();
		return bSuccess;
	}

	std::vector<rImporter3DMesh*> meshes;
};

class Importer3D
{
public:
	static bool LoadModelFromFile(const std::string& path, const rImporter3DCfg& cfg, rImporter3DData& data_out);
};