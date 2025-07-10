#include "stdafx.h"
#include "EquipmentManager.h"

bool EquipmentManager::CCameraService_Init() 
{
	xCameraBrand brand = xCameraBrand::Hikvision;
	std::vector<xCameraConfig> xCamCfgList;
	m_CameraService->EnumDevicesByBrand(brand, xCamCfgList);
	m_CameraService->AddDevices(xCamCfgList);

	/*m_CameraService->OpenAll();
	m_CameraService->RegisterCallback();
	m_CameraService->StartGrabbing();*/
	return true;
}

bool EquipmentManager::CCameraService_Shutdown()
{
	m_CameraService->StopGrabbing();
	m_CameraService->CloseAll();
	return true;
}