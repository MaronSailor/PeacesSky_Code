#pragma once

#include <customEngine.hpp>
#include "blockManager.hpp"

#include <cmath>
#include <random>
	
class MenuScene
{
	CustomEngine::Application* m_app          = nullptr;
	Scene*                     m_scene        = nullptr;
	BlockManager*              m_blockManager = nullptr;

	uint32_t m_blockEntity = 0;
	bool     m_hasEntity   = false; // entity slot was ever allocated
	bool     m_created     = false; // scene is currently visible

	MeshData m_blockMesh{};
	float    m_orbitTime = 0.0f;

	static constexpr float k_orbitRadius    = 12.0f;
	static constexpr float k_orbitYawSpeed  = 0.001f;
	static constexpr float k_orbitPitchFreq = 0.0007f;
	static constexpr float k_orbitPitchAmp  = 0.4f;

public:
	// call once before create() / update()
	void init(CustomEngine::Application* app, Scene& scene, BlockManager& blockManager)
	{
		m_app          = app;
		m_scene        = &scene;
		m_blockManager = &blockManager;
	}

	// call when main menu becomes active
	void create()
	{
		if (m_created)
			return;

		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<size_t> dist(0, m_blockManager->getBlockTypeCount() - 1);
		const char* material = m_blockManager->getBlockTypeMaterial(dist(rng));

		buildBlockMesh(m_blockMesh);

		if (!m_hasEntity)
		{
			m_blockEntity = m_scene->addEntity();

			Transform       t;
			RenderComponent r;
			r.setResourceLoader(m_app->getResourceloaderPtr());

			m_scene->entities[m_blockEntity].addComponent(t);
			m_scene->entities[m_blockEntity].selectComponent<Transform>().location = { 0.0f, 0.0f, 0.0f };
			m_scene->entities[m_blockEntity].selectComponent<Transform>().rotation = { 1.0f, 0.0f, 0.0f, 0.0f };
			m_scene->entities[m_blockEntity].selectComponent<Transform>().scale    = { 1.0f, 1.0f, 1.0f };
			m_scene->entities[m_blockEntity].addComponent(r);
			m_scene->entities[m_blockEntity].selectComponent<RenderComponent>().setMeshData(&m_blockMesh);
			m_scene->entities[m_blockEntity].selectComponent<RenderComponent>().setMaterial(material);
			m_scene->entities[m_blockEntity].selectComponent<RenderComponent>().setTransformIndex(
				m_scene->entities[m_blockEntity].getComponentIndex<Transform>());

			m_hasEntity = true;
		}
		else
		{
			// reuse entity, just swap material
			m_scene->entities[m_blockEntity].selectComponent<RenderComponent>().setMaterial(material);
		}

		m_orbitTime = 0.0f;
		m_created   = true;
	}

	// Call when a game session begins — empties the mesh so nothing renders
	void destroy()
	{
		if (!m_created)
			return;

		m_blockMesh.vertexBufferData.clear();
		m_blockMesh.indexBufferData.clear();
		++m_blockMesh.revision;

		m_created = false;
	}

	// call every frame while on main menu
	void update(float dT)
	{
		if (!m_created)
			return;

		m_orbitTime += dT;

		const float yaw      = m_orbitTime * k_orbitYawSpeed;
		const float pitch    = k_orbitPitchAmp * sinf(m_orbitTime * k_orbitPitchFreq);
		const float cosPitch = cosf(pitch);

		const Vec3 eye =
		{
			k_orbitRadius * cosPitch * sinf(yaw),
			k_orbitRadius * sinf(pitch),
			k_orbitRadius * cosPitch * cosf(yaw)
		};

		m_scene->sceneCamera.location = eye;
		m_scene->sceneCamera.rotation = lookAt(eye, { 0.0f, 0.0f, 0.0f });
	}

	// Forward from SetupLayer's entity-removal callback
	void onEntityRemoved(uint32_t removedIndex)
	{
		if (m_hasEntity && m_blockEntity > removedIndex)
			--m_blockEntity;
	}

	bool isActive() const { return m_created; }

private:
	// Full cube, half-size = 2 (matches BlockManager blockSize/2 = 4/2)
	static void buildBlockMesh(MeshData& mesh)
	{
		mesh.vertexBufferData.clear();
		mesh.indexBufferData.clear();

		constexpr float h = 2.0f;

		auto appendFace = [&](Vec3 v0, Vec3 v1, Vec3 v2, Vec3 v3, Vec3 n)
		{
			const unsigned int base = static_cast<unsigned int>(mesh.vertexBufferData.size());
			mesh.vertexBufferData.push_back({ v0, { 0.0f, 0.0f }, n });
			mesh.vertexBufferData.push_back({ v1, { 1.0f, 0.0f }, n });
			mesh.vertexBufferData.push_back({ v2, { 1.0f, 1.0f }, n });
			mesh.vertexBufferData.push_back({ v3, { 0.0f, 1.0f }, n });
			mesh.indexBufferData.push_back(base);
			mesh.indexBufferData.push_back(base + 1);
			mesh.indexBufferData.push_back(base + 2);
			mesh.indexBufferData.push_back(base);
			mesh.indexBufferData.push_back(base + 2);
			mesh.indexBufferData.push_back(base + 3);
		};

		appendFace({ h,-h,-h }, { h, h,-h }, { h, h, h }, { h,-h, h }, { 1, 0, 0 }); // +X
		appendFace({-h,-h, h }, {-h, h, h }, {-h, h,-h }, {-h,-h,-h }, {-1, 0, 0 }); // -X
		appendFace({-h, h,-h }, {-h, h, h }, { h, h, h }, { h, h,-h }, { 0, 1, 0 }); // +Y
		appendFace({-h,-h, h }, {-h,-h,-h }, { h,-h,-h }, { h,-h, h }, { 0,-1, 0 }); // -Y
		appendFace({-h,-h, h }, { h,-h, h }, { h, h, h }, {-h, h, h }, { 0, 0, 1 }); // +Z
		appendFace({ h,-h,-h }, {-h,-h,-h }, {-h, h,-h }, { h, h,-h }, { 0, 0,-1 }); // -Z

		++mesh.revision;
	}

	// OpenGL -Z forward look-at quaternion (Shepperd's method)
	static Quaternion lookAt(const Vec3& eye, const Vec3& target)
	{
		Vec3 z = { eye.x - target.x, eye.y - target.y, eye.z - target.z };
		const float zLen = sqrtf(z.x*z.x + z.y*z.y + z.z*z.z);
		if (zLen > 0.0001f) { z.x /= zLen; z.y /= zLen; z.z /= zLen; }

		const Vec3 worldUp = { 0.0f, 1.0f, 0.0f };
		Vec3 x = { worldUp.y*z.z - worldUp.z*z.y,
		           worldUp.z*z.x - worldUp.x*z.z,
		           worldUp.x*z.y - worldUp.y*z.x };
		const float xLen = sqrtf(x.x*x.x + x.y*x.y + x.z*x.z);
		if (xLen > 0.0001f) { x.x /= xLen; x.y /= xLen; x.z /= xLen; }
		else                { x = { 1.0f, 0.0f, 0.0f }; }

		const Vec3 y = { z.y*x.z - z.z*x.y, z.z*x.x - z.x*x.z, z.x*x.y - z.y*x.x };

		const float trace = x.x + y.y + z.z;
		Quaternion q{ 1.0f, 0.0f, 0.0f, 0.0f };
		if (trace > 0.0f)
		{
			const float s = sqrtf(trace + 1.0f) * 2.0f;
			q.w = 0.25f * s;
			q.x = (y.z - z.y) / s;
			q.y = (z.x - x.z) / s;
			q.z = (x.y - y.x) / s;
		}
		else if (x.x > y.y && x.x > z.z)
		{
			const float s = sqrtf(1.0f + x.x - y.y - z.z) * 2.0f;
			q.w = (y.z - z.y) / s;
			q.x = 0.25f * s;
			q.y = (y.x + x.y) / s;
			q.z = (z.x + x.z) / s;
		}
		else if (y.y > z.z)
		{
			const float s = sqrtf(1.0f + y.y - x.x - z.z) * 2.0f;
			q.w = (z.x - x.z) / s;
			q.x = (y.x + x.y) / s;
			q.y = 0.25f * s;
			q.z = (z.y + y.z) / s;
		}
		else
		{
			const float s = sqrtf(1.0f + z.z - x.x - y.y) * 2.0f;
			q.w = (x.y - y.x) / s;
			q.x = (z.x + x.z) / s;
			q.y = (z.y + y.z) / s;
			q.z = 0.25f * s;
		}
		return q;
	}
};