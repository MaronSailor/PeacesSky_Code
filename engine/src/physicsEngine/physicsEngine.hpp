#pragma once

#include "physicsUnit.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>

class PhysicsEngine : public PhysicsUnit
{
public:
	PhysicsEngine();
	~PhysicsEngine() override;

	void calculateScenePhysics(EngineSceneInfo* sceneInfo) override;
	void startAsyncCalculation(EngineSceneInfo* sceneInfo) override;
	void waitForCalculation() override;

private:
	void threadLoop();

	std::thread             m_thread;
	std::mutex              m_mutex;
	std::condition_variable m_cvWork;
	std::condition_variable m_cvDone;

	bool             m_workReady    = false;
	bool             m_workDone     = true;
	bool             m_running      = false;
	EngineSceneInfo* m_pendingScene  = nullptr;
};