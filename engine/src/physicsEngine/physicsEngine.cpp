#include "physicsEngine.hpp"

#include <algorithm>
#include <iostream>

float getMaxDistanceRadius(const PhysicsComponent& component, const Transform& transform)
{
    float maxDistance = component.collisionDimensions.x * 0.5 * transform.scale.x;

    float maxCheck = component.collisionDimensions.y * 0.5 * transform.scale.y;
    if (maxCheck > maxDistance) maxDistance = maxCheck;

    maxCheck = component.collisionDimensions.z * 0.5 * transform.scale.z;
    if (maxCheck > maxDistance) maxDistance = maxCheck;

    return maxDistance;
}

bool checkPossibleOverlaps(const PhysicsComponent& objA, const PhysicsComponent& objB, const Transform& transformA, const Transform& transformB)
{
    float minAX = transformA.location.x - objA.collisionDimensions.x * 0.5f * transformA.scale.x;
    float maxAX = transformA.location.x + objA.collisionDimensions.x * 0.5f * transformA.scale.x;
    float minBX = transformB.location.x - objB.collisionDimensions.x * 0.5f * transformB.scale.x;
    float maxBX = transformB.location.x + objB.collisionDimensions.x * 0.5f * transformB.scale.x;

    float minAY = transformA.location.y - objA.collisionDimensions.y * 0.5f * transformA.scale.y;
    float maxAY = transformA.location.y + objA.collisionDimensions.y * 0.5f * transformA.scale.y;
    float minBY = transformB.location.y - objB.collisionDimensions.y * 0.5f * transformB.scale.y;
    float maxBY = transformB.location.y + objB.collisionDimensions.y * 0.5f * transformB.scale.y;

    float minAZ = transformA.location.z - objA.collisionDimensions.z * 0.5f * transformA.scale.z;
    float maxAZ = transformA.location.z + objA.collisionDimensions.z * 0.5f * transformA.scale.z;
    float minBZ = transformB.location.z - objB.collisionDimensions.z * 0.5f * transformB.scale.z;
    float maxBZ = transformB.location.z + objB.collisionDimensions.z * 0.5f * transformB.scale.z;

    bool overlapX = (minBX <= maxAX) && (maxBX >= minAX);
    bool overlapY = (minBY <= maxAY) && (maxBY >= minAY);
    bool overlapZ = (minBZ <= maxAZ) && (maxBZ >= minAZ);

    return (overlapX && overlapY && overlapZ);
}

bool isComponentOverlap(const PhysicsComponent& componentA, const PhysicsComponent& componentB, const Transform& transformA, const Transform& transformB)
{
    float minAX = transformA.location.x - componentA.collisionDimensions.x * 0.5 * transformA.scale.x;
    float maxAX = transformA.location.x + componentA.collisionDimensions.x * 0.5 * transformA.scale.x;
    float minBX = transformB.location.x - componentB.collisionDimensions.x * 0.5 * transformB.scale.x;
    float maxBX = transformB.location.x + componentB.collisionDimensions.x * 0.5 * transformB.scale.x;

    float minAY = transformA.location.y - componentA.collisionDimensions.y * 0.5 * transformA.scale.y;
    float maxAY = transformA.location.y + componentA.collisionDimensions.y * 0.5 * transformA.scale.y;
    float minBY = transformB.location.y - componentB.collisionDimensions.y * 0.5 * transformB.scale.y;
    float maxBY = transformB.location.y + componentB.collisionDimensions.y * 0.5 * transformB.scale.y;

    float minAZ = transformA.location.z - componentA.collisionDimensions.z * 0.5 * transformA.scale.z;
    float maxAZ = transformA.location.z + componentA.collisionDimensions.z * 0.5 * transformA.scale.z;
    float minBZ = transformB.location.z - componentB.collisionDimensions.z * 0.5 * transformB.scale.z;
    float maxBZ = transformB.location.z + componentB.collisionDimensions.z * 0.5 * transformB.scale.z;

    bool overlapX = (minBX <= maxAX) && (maxBX >= minAX);
    bool overlapY = (minBY <= maxAY) && (maxBY >= minAY);
    bool overlapZ = (minBZ <= maxAZ) && (maxBZ >= minAZ);

    return (overlapX && overlapY && overlapZ);
}

void collisionEventStatic(PhysicsComponent& staticComponent, const Transform& staticTransform, PhysicsComponent& movingComponent, Transform& movingTransform)
{
    if (!isComponentOverlap(staticComponent, movingComponent, staticTransform, movingTransform)) return;

    const float halfAX = staticComponent.collisionDimensions.x * 0.5f * staticTransform.scale.x;
    const float halfAY = staticComponent.collisionDimensions.y * 0.5f * staticTransform.scale.y;
    const float halfAZ = staticComponent.collisionDimensions.z * 0.5f * staticTransform.scale.z;

    const float halfBX = movingComponent.collisionDimensions.x * 0.5f * movingTransform.scale.x;
    const float halfBY = movingComponent.collisionDimensions.y * 0.5f * movingTransform.scale.y;
    const float halfBZ = movingComponent.collisionDimensions.z * 0.5f * movingTransform.scale.z;

    const float minAX = staticTransform.location.x - halfAX;
    const float maxAX = staticTransform.location.x + halfAX;
    const float minAY = staticTransform.location.y - halfAY;
    const float maxAY = staticTransform.location.y + halfAY;
    const float minAZ = staticTransform.location.z - halfAZ;
    const float maxAZ = staticTransform.location.z + halfAZ;

    const float minBX = movingTransform.location.x - halfBX;
    const float maxBX = movingTransform.location.x + halfBX;
    const float minBY = movingTransform.location.y - halfBY;
    const float maxBY = movingTransform.location.y + halfBY;
    const float minBZ = movingTransform.location.z - halfBZ;
    const float maxBZ = movingTransform.location.z + halfBZ;

    const float overlapX = std::min(maxAX, maxBX) - std::max(minAX, minBX);
    const float overlapY = std::min(maxAY, maxBY) - std::max(minAY, minBY);
    const float overlapZ = std::min(maxAZ, maxBZ) - std::max(minAZ, minBZ);

    Vec3 prevPos{
        movingTransform.location.x - movingComponent.locationInertia.x,
        movingTransform.location.y - movingComponent.locationInertia.y,
        movingTransform.location.z - movingComponent.locationInertia.z
    };

    if (overlapX <= overlapY && overlapX <= overlapZ)
    {
        float targetX = (prevPos.x < staticTransform.location.x) ? (minAX - halfBX) : (maxAX + halfBX);
        movingComponent.locationInertia.x = targetX - prevPos.x;
    }
    else if (overlapY <= overlapZ)
    {
        float targetY = (prevPos.y < staticTransform.location.y) ? (minAY - halfBY) : (maxAY + halfBY);
        movingComponent.locationInertia.y = targetY - prevPos.y;
    }
    else
    {
        float targetZ = (prevPos.z < staticTransform.location.z) ? (minAZ - halfBZ) : (maxAZ + halfBZ);
        movingComponent.locationInertia.z = targetZ - prevPos.z;
    }
}

void collisionEvent(PhysicsComponent& componentA, PhysicsComponent& componentB, Transform& transformA, Transform& transformB)
{
    if (componentA.isStatic && componentB.isStatic) return;
    if (componentA.isStatic)
    {
        collisionEventStatic(componentA, transformA, componentB, transformB);
        return;
    }
    if (componentB.isStatic)
    {
        collisionEventStatic(componentB, transformB, componentA, transformA);
        return;
    }
}

// physics async thread (runs on core 2)

PhysicsEngine::PhysicsEngine()
{
    m_running = true;
    m_thread = std::thread(&PhysicsEngine::threadLoop, this);
}

PhysicsEngine::~PhysicsEngine()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running   = false;
        m_workReady = true;
    }
    m_cvWork.notify_one();
    if (m_thread.joinable())
        m_thread.join();
}

void PhysicsEngine::startAsyncCalculation(EngineSceneInfo* sceneInfo)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pendingScene = sceneInfo;
        m_workDone     = false;
        m_workReady    = true;
    }
    m_cvWork.notify_one();
}

void PhysicsEngine::waitForCalculation()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvDone.wait(lock, [this] { return m_workDone; });
}

void PhysicsEngine::threadLoop()
{
    while (true)
    {
        EngineSceneInfo* scene = nullptr;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvWork.wait(lock, [this] { return m_workReady; });
            if (!m_running)
                break;
            scene       = m_pendingScene;
            m_workReady = false;
        }

        if (scene)
            calculateScenePhysics(scene);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_workDone = true;
        }
        m_cvDone.notify_one();
    }
}

void PhysicsEngine::calculateScenePhysics(EngineSceneInfo* sceneInfo)
{
	if (!sceneInfo) return;
	if (!sceneInfo->pPhysicsComponentStorage) return;

	std::vector<PhysicsComponent>& physicsComponents = *sceneInfo->pPhysicsComponentStorage;
    uint32_t componentCount = physicsComponents.size();
	std::vector<Transform>& transforms = *sceneInfo->pTransformComponentStorage;

	// normalize to 144fps; cap at 3x to avoid huge jumps
	constexpr double targetFrameMs = 1000.0 / 144.0;
	const float dtScale = static_cast<float>(
		std::min(sceneInfo->deltaTime / targetFrameMs, 3.0));

    // apply gravity
    const float gravityStep = -0.2f;
    const float maxFallSpeed = -6.0f;

    for (PhysicsComponent& component : physicsComponents)
    {
        if (component.isFalling)
        {
            float nextY = component.locationInertia.y + gravityStep * component.gravityScale * dtScale;
            if (nextY < maxFallSpeed) nextY = maxFallSpeed;
            component.locationInertia.y = nextY;
        }
    }

    Transform premoveTransformI;
    Transform premoveTransformJ;

    for (uint32_t i = 0; i < componentCount - 1; ++i)
    {
        for (uint32_t j = i + 1; j < componentCount; ++j)
        {
            PhysicsComponent& componentI = physicsComponents[i];
            PhysicsComponent& componentJ = physicsComponents[j];
            Transform& transformI = transforms[physicsComponents[i].getTransformIndex()];
            Transform& transformJ = transforms[physicsComponents[j].getTransformIndex()];

            premoveTransformI.location.x = transformI.location.x + componentI.locationInertia.x * dtScale;
            premoveTransformI.location.y = transformI.location.y + componentI.locationInertia.y * dtScale;
            premoveTransformI.location.z = transformI.location.z + componentI.locationInertia.z * dtScale;
            premoveTransformI.scale = transformI.scale;

            premoveTransformJ.location.x = transformJ.location.x + componentJ.locationInertia.x * dtScale;
            premoveTransformJ.location.y = transformJ.location.y + componentJ.locationInertia.y * dtScale;
            premoveTransformJ.location.z = transformJ.location.z + componentJ.locationInertia.z * dtScale;
            premoveTransformJ.scale = transformJ.scale;

            if (checkPossibleOverlaps(physicsComponents[i], physicsComponents[j], premoveTransformI, premoveTransformJ))
            {
                collisionEvent(componentI, componentJ, premoveTransformI, premoveTransformJ);
            }
        }
    }

    for (PhysicsComponent& component : physicsComponents)
    {
        Transform& componentTransform = transforms[component.getTransformIndex()];

        componentTransform.location.x += component.locationInertia.x * dtScale;
        componentTransform.location.y += component.locationInertia.y * dtScale;
        componentTransform.location.z += component.locationInertia.z * dtScale;
    }
}