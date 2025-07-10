#pragma once

#include <memory>
#include "03-Services/CCameraService.h"


class EquipmentManager
{
public:
	~EquipmentManager()
	{
		Shutdown();  
	}

	static EquipmentManager& GetInstance()
	{
		static EquipmentManager instance;
		return instance;
	}

	EquipmentManager(const EquipmentManager&) = delete;
	EquipmentManager& operator=(const EquipmentManager&) = delete;

	CCameraService& GetCameraService() { return *m_CameraService; }

	void Initialize()
	{
		CCameraService_Init();
	}

	void Shutdown()
	{
		CCameraService_Shutdown();
	}

	bool CCameraService_Init();

	bool CCameraService_Shutdown();

private:
	EquipmentManager() : m_CameraService(std::make_unique<CCameraService>()) {}

	std::unique_ptr<CCameraService> m_CameraService;
};

