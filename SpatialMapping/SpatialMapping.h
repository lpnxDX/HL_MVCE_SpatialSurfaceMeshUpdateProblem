#pragma once


#include <map>
#include <winrt/Windows.Perception.Spatial.h>
#include <winrt/Windows.Perception.Spatial.Surfaces.h>

//===============================================================================//
/// SurfaceMesh IS WORKING FINE bgfx rendering is disabled
//#include "ExtLibs/bgfx/bgfx/include/bgfx/bgfx.h"
//===============================================================================//
struct SurfaceMesh
{
	struct Vertex {
		//Vertex() {}
		float Position[3];
		float Color[3];
	};

	SurfaceMesh();
	~SurfaceMesh();
	void ClearBuffers();
	static void s_init();
	void UpdateBuffers(winrt::Windows::Perception::Spatial::Surfaces::SpatialSurfaceMesh mesh
	, winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem);
	/*
	void Submit(bgfx::ProgramHandle& program,
		bgfx::UniformHandle& viewProjectionCBuffer,
		winrt::Windows::Foundation::Numerics::float4x4* vp);

	bgfx::VertexBufferHandle m_vbh;
	bgfx::IndexBufferHandle m_ibh;
	static bgfx::VertexLayout m_layout;
	*/
	std::vector<Vertex> m_v;
	std::vector<unsigned short> m_i;

	winrt::Windows::Foundation::Numerics::float4x4 m_transform;
	bool m_active = false;

	float M_rndClr = 1.0f;
};

class SpatialMapping
{
public:
	SpatialMapping(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem) ;

	void Update(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem coordinateSystem);

	/*void Render(bgfx::ProgramHandle& program,
		bgfx::UniformHandle& viewProjectionCBuffer,
		winrt::Windows::Foundation::Numerics::float4x4* vp);*/


	//Concurrency::task<void>
	winrt::Windows::Foundation::IAsyncAction
	Observer_ObservedSurfacesChanged(
		winrt::Windows::Perception::Spatial::Surfaces::SpatialSurfaceObserver sender
		, winrt::Windows::Foundation::IInspectable object);

private:
	void AddOrUpdateSurface(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem);

	winrt::Windows::Foundation::IAsyncAction CollectSurfacesManuel();

	winrt::event_token   m_SurfacesChangedEventToken;

	winrt::Windows::Perception::Spatial::Surfaces::SpatialSurfaceObserver m_surfaceObserver = nullptr;
	winrt::Windows::Perception::Spatial::Surfaces::SpatialSurfaceMeshOptions m_surfaceMeshOptions;
	winrt::Windows::Perception::Spatial::SpatialBoundingVolume m_bounds = nullptr;



	// A way to lock map access.
	std::mutex                                      m_meshCollectionLock;

	// Total number of surface meshes.
	//unsigned int                                    m_surfaceMeshCount;

	// Level of detail setting. The number of triangles that the system is allowed to provide per cubic meter.
	double                                          m_maxTrianglesPerCubicMeter = 1000.0f;

	// The duration of time, in seconds, a mesh is allowed to remain inactive before deletion.
	//const float c_maxInactiveMeshTime = 120.f;

	// The duration of time, in seconds, taken for a new surface mesh to fade in on-screen.
	//const float c_surfaceMeshFadeInTime = 3.0f;

	
	struct guidCompare
	{
		bool operator()(winrt::guid const& left, winrt::guid const& right) const
		{
			return memcmp(&left, &right, sizeof(left)) < 0;
		}
	};
	
	// The set of surfaces in the collection.
	std::map <winrt::guid,
		winrt::Windows::Perception::Spatial::Surfaces::SpatialSurfaceMesh, guidCompare>
													 m_updatedSurfaces;

	std::map<winrt::guid, SurfaceMesh, guidCompare>         	                                 m_meshBufferCollection;
};

