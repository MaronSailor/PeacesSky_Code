#pragma once
#include <customEngine.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

// spawns clouds that drift and wrap around the world

class CloudSystem
{
	struct CloudEntry
	{
		uint32_t entityIndex;
		float    offsetX;   // individual horizontal offset for variety
		float    offsetZ;
	};

	std::vector<CloudEntry> m_clouds;

	Scene*                             m_scene         = nullptr;
	CustomEngine::ResourceLoaderUnit*  m_loader        = nullptr;

	// Tunables
	int   m_count          = 150;     // number of cloud entities
	float m_altitude       = 800.0f;  // base Y position of cloud layer
	float m_altitudeVar    = 200.0f;  // random Y variance above/below altitude
	float m_spawnRadius    = 9000.0f; // initial scatter radius — fills the whole sky area
	float m_windX          = 0.4f;    // world units per dT unit along X
	float m_windZ          = 0.05f;   // slight drift on Z for variety

	// Scale ranges  (randomly chosen per cloud)
	float m_scaleXMin = 80.0f,  m_scaleXMax = 350.0f;
	float m_scaleYMin = 20.0f,  m_scaleYMax = 60.0f;
	float m_scaleZMin = 60.0f,  m_scaleZMax = 250.0f;

	float frand(float lo, float hi)
	{
		return lo + (hi - lo) * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
	}

public:
	void init(CustomEngine::Application* app, Scene& scene)
	{
		m_scene  = &scene;
		m_loader = app->getResourceloaderPtr();

		for (int i = 0; i < m_count; ++i)
		{
			uint32_t e = scene.addEntity();

			// random angle + radius for uniform disc layout
			const float angle  = frand(0.0f, 6.2831853f);
			const float radius = frand(200.0f, m_spawnRadius);

			Transform t;
			t.location.x = cosf(angle) * radius;
			t.location.y = m_altitude + frand(-m_altitudeVar, m_altitudeVar);
			t.location.z = sinf(angle) * radius;
			t.scale.x    = frand(m_scaleXMin, m_scaleXMax);
			t.scale.y    = frand(m_scaleYMin, m_scaleYMax);
			t.scale.z    = frand(m_scaleZMin, m_scaleZMax);

			scene.entities[e].addComponent(t);

			RenderComponent rc;
			rc.setResourceLoader(m_loader);
			rc.setMesh("resources/decodeSphere.obj");
			rc.setMaterial("resources/white.png");
			rc.setTransformIndex(scene.entities[e].getComponentIndex<Transform>());
			scene.entities[e].addComponent(rc);

			CloudEntry ce;
			ce.entityIndex = e;
			ce.offsetX     = frand(-0.05f, 0.05f);  // per-cloud speed variance
			ce.offsetZ     = frand(-0.02f, 0.02f);
			m_clouds.push_back(ce);
		}
	}

	// drawDistance  — user setting (slider), capped to sky cylinder radius
	// camPos        — current camera world position (clouds wrap around it)
	void update(float dT, float drawDistance, Vec3 camPos)
	{
		if (!m_scene) return;

		// Hard cap: clouds must disappear before the sky cylinder wall (~10000 units radius)
		const float maxAllowed = 9500.0f;
		const float clampedDist = (drawDistance < maxAllowed) ? drawDistance : maxAllowed;
		const float distSq = clampedDist * clampedDist;

		// Wrap radius: keep clouds in a band of clampedDist around the camera.
		// Using clampedDist as the half-extent ensures the full visible area is covered.
		const float wrapHalf = clampedDist;

		for (auto& ce : m_clouds)
		{
			if (ce.entityIndex >= static_cast<uint32_t>(m_scene->entities.size())) continue;

			Transform& t = m_scene->entities[ce.entityIndex].selectComponent<Transform>();

			// Move cloud with wind
			t.location.x += (m_windX + ce.offsetX) * dT;
			t.location.z += (m_windZ + ce.offsetZ) * dT;

			// Wrap relative to camera so clouds always surround the player
			const float relX = t.location.x - camPos.x;
			const float relZ = t.location.z - camPos.z;

			if (relX > wrapHalf)
				t.location.x -= 2.0f * wrapHalf;
			else if (relX < -wrapHalf)
				t.location.x += 2.0f * wrapHalf;

			if (relZ > wrapHalf)
				t.location.z -= 2.0f * wrapHalf;
			else if (relZ < -wrapHalf)
				t.location.z += 2.0f * wrapHalf;

			// Visibility: hide clouds beyond draw distance from camera (horizontal plane)
			const float dx = t.location.x - camPos.x;
			const float dz = t.location.z - camPos.z;
			const float horizSq = dx * dx + dz * dz;
			m_scene->entities[ce.entityIndex].selectComponent<RenderComponent>().isVisible = (horizSq <= distSq);
		}
	}
};
