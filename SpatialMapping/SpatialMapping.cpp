#include "pch.h"
#include "SpatialMapping.h"

#include "Common\DirectXHelper.h"
#include <ppltasks.h>
#include "GetDataFromIBuffer.h"

using namespace DX;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Perception::Spatial;

//===============================================================================//
/// SurfaceMesh IS WORKING FINE bgfx rendering is disabled
/// GOTO line 155
//===============================================================================//
SurfaceMesh::SurfaceMesh()
{
	m_active = false;
	/*
	m_vbh.idx = bgfx::kInvalidHandle;
	m_ibh.idx = bgfx::kInvalidHandle;

	M_rndClr = (double)rand() / ((double)RAND_MAX + 1);
	*/
}

SurfaceMesh::~SurfaceMesh()
{
	ClearBuffers();
}

void SurfaceMesh::ClearBuffers()
{
	/*
	if (bgfx::isValid(m_vbh)) {
		bgfx::destroy(m_vbh);
	}
	if (bgfx::isValid(m_ibh)) {
		bgfx::destroy(m_ibh);
	}
	*/
}

//bgfx::VertexLayout SurfaceMesh::m_layout;

void SurfaceMesh::s_init()
{
	/*
	m_layout.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Float)
		.end();
	*/
}

void SurfaceMesh::UpdateBuffers(winrt::Windows::Perception::Spatial::Surfaces::SpatialSurfaceMesh mesh
	, winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem)
{

	float* vertexPositions = reinterpret_cast<float*>(DX::GetDataFromIBuffer(mesh.VertexPositions().Data()));
	unsigned short* indices = reinterpret_cast<unsigned short*>(DX::GetDataFromIBuffer(mesh.TriangleIndices().Data()));


	auto try_transform = mesh.CoordinateSystem().TryGetTransformTo(coordinateSystem);
	if (try_transform == NULL || mesh.TriangleIndices().ElementCount() < 3)
	{
		return;
	}
	m_transform = try_transform.Value();

	int vertexCount = mesh.VertexPositions().ElementCount();
	if (vertexCount != m_v.size() && m_active)
	{
		OutputDebugStringW(L"VERTEX DATA UPDATED\n"); // update trigger
		M_rndClr = (double)rand() / ((double)RAND_MAX + 1); // just for testing
	}

	m_v.clear();
	m_v.resize(vertexCount);

	float3 scale = mesh.VertexPositionScale();
	int i = 0;
	for (size_t j = 0; j < vertexCount; j++)
	{
		m_v[j].Position[0] = vertexPositions[i] * scale.x;
		m_v[j].Position[1] = vertexPositions[i + 1] * scale.y;
		m_v[j].Position[2] = vertexPositions[i + 2] * scale.z;
		i += 4; //ignore w value
		m_v[j].Color[0] = 1.0f;
		m_v[j].Color[1] = 1.0f;
		m_v[j].Color[2] = M_rndClr;
	}

	uint32_t vSize = vertexCount * sizeof(float) * 6;
	uint32_t iSize = mesh.TriangleIndices().ElementCount();
	m_i.clear();
	m_i.resize(iSize);
	unsigned short maxTEST = 0;
	for (size_t i = 0; i < iSize; i++)
	{
		m_i[i] = indices[i];
		if (m_i[i] > maxTEST)
		{
			maxTEST = m_i[i]; // just for testing
		}
	}
	/*
	ClearBuffers();
	m_vbh = bgfx::createVertexBuffer(bgfx::makeRef(m_v.data(), vSize), m_layout);
	m_ibh = bgfx::createIndexBuffer(bgfx::makeRef(m_i.data(), iSize));

	if (bgfx::isValid(m_ibh) && bgfx::isValid(m_vbh))
	{
		m_active = true;
	}
	else {
		m_active = false;
	}
	*/
	m_active = true;
}
/*
void SurfaceMesh::Submit(bgfx::ProgramHandle& program,
	bgfx::UniformHandle& viewProjectionCBuffer,
	winrt::Windows::Foundation::Numerics::float4x4* vp)
{
	bgfx::setTransform(&m_transform.m11);
	bgfx::setUniform(viewProjectionCBuffer, &vp[0], 2);
	bgfx::setVertexBuffer(0, m_vbh);
	bgfx::setIndexBuffer(m_ibh);
	bgfx::setInstanceCount(2);
	bgfx::setState(BGFX_STATE_DEFAULT);
	bgfx::submit(0, program);
}

void SpatialMapping::Render(bgfx::ProgramHandle& program,
	bgfx::UniformHandle& viewProjectionCBuffer,
	winrt::Windows::Foundation::Numerics::float4x4* vp)
{
	auto it = m_meshBufferCollection.begin();

	for (; it != m_meshBufferCollection.end(); it++)
	{
		if (it->second.m_active) {
			it->second.Submit(program, viewProjectionCBuffer, vp);
		}
	}
}
*/


//===============================================================================//
//==SpatialMapping===============================================================//
//===============================================================================//

SpatialMapping::SpatialMapping(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem)
{
	SurfaceMesh::s_init();

	m_surfaceMeshOptions.IncludeVertexNormals(false); // not needed for now
	m_surfaceMeshOptions.VertexPositionFormat(
		winrt::Windows::Graphics::DirectX::DirectXPixelFormat::R32G32B32A32Float); //works
	m_surfaceMeshOptions.TriangleIndexFormat(
		winrt::Windows::Graphics::DirectX::DirectXPixelFormat::R16UInt); //works

	m_surfaceObserver = nullptr; // make sure its NULL
	AddOrUpdateSurface(coordinateSystem); // create Observer and callback
}
// called every frame
void SpatialMapping::Update(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem coordinateSystem)
{
	using namespace winrt::Windows::Perception::Spatial::Surfaces;

	static int waitTicks = 0; // testing

	if (waitTicks > 120) {
		std::lock_guard<std::mutex> guard(m_meshCollectionLock);

		auto it_upd = m_updatedSurfaces.begin();
		for (; it_upd != m_updatedSurfaces.end(); it_upd++)
		{
			SurfaceMesh& meshBuffer = m_meshBufferCollection[it_upd->first];
			meshBuffer.UpdateBuffers(it_upd->second, coordinateSystem); // vertex / index update
		}

		//m_updatedSurfaces.clear(); dont clear when manuel
		waitTicks = 0;
	}
	else {
		// MANUEL updating with time date output
		/*
		using namespace winrt::Windows::Foundation;
		if (waitTicks % 30 == 0)
		{
			SpatialBoundingBox axisAlignedBoundingBox =
			{
				{  0.f,  0.f, 0.f },
				{ 5.0f, 5.0f, 5.0f },
			};
			std::lock_guard<std::mutex> guard(m_meshCollectionLock);
			m_bounds = SpatialBoundingVolume::FromBox(coordinateSystem, axisAlignedBoundingBox);
			m_surfaceObserver.SetBoundingVolume(m_bounds); 	// does this change with stationaryFrame?
			auto m_collectionTask = CollectSurfacesManuel();
		}
		*/

		AddOrUpdateSurface(coordinateSystem); // update observer needed? 
		waitTicks++;
	}
}


void
SpatialMapping::AddOrUpdateSurface(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem)
{
	using namespace winrt::Windows::Perception::Spatial::Surfaces;

	SpatialBoundingBox axisAlignedBoundingBox =
	{
		{  0.f,  0.f, 0.f },
		{ 5.0f, 5.0f, 5.0f },
	};
	SpatialBoundingVolume bounds = SpatialBoundingVolume::FromBox(coordinateSystem, axisAlignedBoundingBox); // 

	if (m_surfaceObserver == NULL)
	{
		m_surfaceObserver = SpatialSurfaceObserver();
		m_surfaceObserver.SetBoundingVolume(bounds);

		m_SurfacesChangedEventToken = m_surfaceObserver.ObservedSurfacesChanged(
			winrt::Windows::Foundation::TypedEventHandler
			<SpatialSurfaceObserver, winrt::Windows::Foundation::IInspectable>
			//(std::bind(&SpatialMapping::Observer_ObservedSurfacesChanged, this, std::placeholders::_1, std::placeholders::_2)); // alternative
		{ this, & SpatialMapping::Observer_ObservedSurfacesChanged });
	}
	else
	{
		// does this change with stationaryFrame?
		m_surfaceObserver.SetBoundingVolume(bounds); 
	}
}

winrt::Windows::Foundation::IAsyncAction
SpatialMapping::Observer_ObservedSurfacesChanged(winrt::Windows::Perception::Spatial::Surfaces::SpatialSurfaceObserver sender
	, winrt::Windows::Foundation::IInspectable object)
{
	if (sender != NULL) {
		OutputDebugStringW(L"\n\n\n\n\n LINE 247 TRIGGER EVENT Observer_ObservedSurfacesChanged\n\n\n\n\n\n");
		const auto mapContainingSurfaceCollection = sender.GetObservedSurfaces();

		for (const auto& pair : mapContainingSurfaceCollection)
		{
			auto id = pair.Key();
			auto info = pair.Value();
			auto mesh{ co_await info.TryComputeLatestMeshAsync(m_maxTrianglesPerCubicMeter, m_surfaceMeshOptions) };
			{
				std::lock_guard<std::mutex> guard(m_meshCollectionLock);
				auto it = m_updatedSurfaces.find(id);
				if (it != m_updatedSurfaces.end())
				{
					m_updatedSurfaces.erase(it);
				}
				m_updatedSurfaces.emplace(id, mesh);
			}
		}
	}
}

#pragma warning(disable:4996)
#include <ctime>
#include <sstream>
#include <iomanip>
#define _CRT_SECURE_NO_WARNINGS

winrt::Windows::Foundation::IAsyncAction SpatialMapping::CollectSurfacesManuel()
{
	const auto mapContainingSurfaceCollection = m_surfaceObserver.GetObservedSurfaces();
	for (const auto& pair : mapContainingSurfaceCollection)
	{
		auto id = pair.Key();
		auto info = pair.Value();
		auto mesh{ co_await info.TryComputeLatestMeshAsync(m_maxTrianglesPerCubicMeter, m_surfaceMeshOptions) };
		{
			std::lock_guard<std::mutex> guard(m_meshCollectionLock);

			auto iter = m_updatedSurfaces.find(id);
			if (iter != m_updatedSurfaces.end())
			{
				// check last update time
				std::time_t t = winrt::clock::to_time_t(info.UpdateTime());

				std::wstringstream wss;
				OutputDebugStringW((winrt::to_hstring(id) + L" > ").c_str());
				wss << std::put_time(std::localtime(&t), L"%m/%d/%Y %H:%M:%S\n");
				winrt::hstring hs{ wss.str() };
				OutputDebugStringW(hs.c_str());

				iter->second = mesh;
			}
			else {
				m_updatedSurfaces.emplace(id, mesh);
			}
		}
	}
}
