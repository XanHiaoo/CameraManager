#include "stdafx.h"
#include "CCameraService.h"
#include "00-Common/xCameraParam.h"
#include "02-Addins/Camera/CCameraHik.h"

CCameraService::CCameraService() {}

CCameraService::~CCameraService()
{
	StopCapturing();
	CloseAll();
	for (auto* cam : m_Cameras)
	{
		SAFE_DELETE(cam);
	}
}
void CCameraService::Inject(ICamera& cam)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	m_Cameras.push_back(&cam);
	m_CapturingMap[std::string(cam.GetSerialNo())] = false;
}

void CCameraService::EnumDevicesByBrand(xCameraBrand& brand, std::vector<xCameraConfig> &xCamCfgList)
{
	xDeviceInfoList xdev;
	switch (brand)
	{
	case xCameraBrand::Hikvision:
		//devices = CCameraHik::EnumDevices();		
		CCameraHik::EnumDevices(-1, xdev, xCamCfgList);
		break;
	case xCameraBrand::Dahua:
		// Dahua的设备枚举逻辑
		break;
	default:
		return;
	}
	/*for (size_t i = 0; i < std::min(static_cast<size_t>(xdev.nDeviceNum), xCamCfgList.size()); i++)
	{
		ICamera* d = new CCameraHik(xCamCfgList[i]);
		Inject(*d);
	}*/
}

// TODO: 实现按协议枚举设备
void CCameraService::EnumDevicesByProtocol(xCameraType& proto)
{
	//xDeviceInfoList xdev;
	//CCameraHik::EnumDevices((int)proto, xdev);
	//switch (proto)
	//{
	//case xCameraType::GigeCamera:
	//	//devices = CCameraHik::EnumDevices();
	//	break;
	//case xCameraType::Usb3Camera:
	//	// Dahua的设备枚举逻辑
	//	break;
	//default:
	//	break;
	//}
}

void CCameraService::AddDevices(std::vector<xCameraConfig>& xCamCfgList) {
	for (size_t i = 0; i < xCamCfgList.size(); i++)
	{
		AddDevice(xCamCfgList[i]);
	}
}

void CCameraService::AddDevice(const xCameraConfig& xCamCfg) {
	switch (xCamCfg.brand)
	{
	case xCameraBrand::Hikvision: 
	{
		ICamera* d = new CCameraHik(xCamCfg);
		Inject(*d);
		break;
	}	
	case xCameraBrand::Dahua:
		break;
	default:
		return;
	}
}

std::vector<std::string> CCameraService::GetAllCameraSerials()
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	std::vector<std::string> serials;
	for (const auto& cam : m_Cameras)
	{
		if (cam)
		{
			serials.push_back(std::string(cam->GetSerialNo()));
		}
	}
	return serials;
}

void CCameraService::OpenAll()
{
	for (auto& camera : m_Cameras)
	{
		Open(std::string(camera->GetSerialNo()));
	}
}

void CCameraService::CloseAll()
{
	for (auto& camera : m_Cameras)
	{
		Close(std::string(camera->GetSerialNo()));
	}
}

void CCameraService::Open(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto cam : m_Cameras)
	{
		if (serialNo == std::string(cam->GetSerialNo()))
		{
			cam->Open();
			return;
		}
	}
}

void CCameraService::Close(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto cam : m_Cameras)
	{
		if (serialNo == std::string(cam->GetSerialNo()))
		{
			cam->Close();
			cam->GetImageQueue().Clear(); 
			return;
		}
	}
}

void CCameraService::StartGrabbing()
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto cam : m_Cameras)
	{
		cam->StartGrabbing();
	}
}

void CCameraService::StartGrabbing(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto cam : m_Cameras)
	{
		if (serialNo == std::string(cam->GetSerialNo()))
		{
			cam->StartGrabbing();
			return;
		}
	}
}

void CCameraService::StopGrabbing()
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto cam : m_Cameras)
	{
		cam->StopGrabbing();
	}
}

void CCameraService::StopGrabbing(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto cam : m_Cameras)
	{
		if (serialNo == std::string(cam->GetSerialNo()))
		{
			cam->StopGrabbing();
			return;
		}
	}
}

void CCameraService::SetTriggerMode(unsigned int mode)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto& camera : m_Cameras)
	{
		camera->SetTriggerMode(mode);
	}
}

void CCameraService::SetTriggerMode(const std::string& serialNo, unsigned int mode)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto& camera : m_Cameras)
	{
		if (serialNo == std::string(camera->GetSerialNo()))
		{
			camera->SetTriggerMode(mode);
			return;
		}
	}
}

void CCameraService::StartCapturing()
{
	for (auto& camera : m_Cameras)
	{
		StartCapturing(std::string(camera->GetSerialNo()));
	}
}

void CCameraService::StartCapturing(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	if (m_CapturingMap[serialNo])
	{
		return;
	}
	m_CapturingMap[serialNo] = true;
	for (auto& camera : m_Cameras)
	{
		if (serialNo == std::string(camera->GetSerialNo()))
		{
			m_CameraThreads[serialNo] = std::thread(&CCameraService::CaptureImages, this, camera);
			break;
		}
	}
}

void CCameraService::StopCapturing()
{
	for (auto& camera : m_Cameras)
	{
		StopCapturing(std::string(camera->GetSerialNo()));
	}
}

void CCameraService::StopCapturing(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	if (m_CapturingMap.find(serialNo) == m_CapturingMap.end() || !m_CapturingMap[serialNo])
	{
		return;
	}
	m_CapturingMap[serialNo] = false;
	if (m_CameraThreads.find(serialNo) != m_CameraThreads.end())
	{
		if (m_CameraThreads[serialNo].joinable())
		{
			m_CameraThreads[serialNo].join();
		}
		m_CameraThreads.erase(serialNo);
	}
}

void CCameraService::GetImageBuffer(const std::string& serialNo, xFrameData& frameData)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (ICamera* cam : m_Cameras)
	{
		if (std::string(cam->GetSerialNo()) == serialNo)
		{
			cam->GetImageBuffer(frameData);
			return;
		}
	}
}

void CCameraService::RegisterCallback()
{
	for (auto& camera : m_Cameras)
	{
		RegisterCallback(std::string(camera->GetSerialNo()));
	}
}

void CCameraService::RegisterCallback(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto& camera : m_Cameras)
	{
		if (serialNo == std::string(camera->GetSerialNo()))
		{
			camera->RegisterCallback();
			return;
		}
	}
}

void CCameraService::UnRegisterCallback()
{
	for (auto& camera : m_Cameras)
	{
		UnRegisterCallback(std::string(camera->GetSerialNo()));
	}
}

void CCameraService::UnRegisterCallback(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto& camera : m_Cameras)
	{
		if (serialNo == std::string(camera->GetSerialNo()))
		{
			camera->UnregisterCallback();
			return;
		}
	}
}

ICamera::SubscriberID CCameraService::AddSubscriber(const std::string& serialNo, ICamera::ImageCallback callback)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto* cam : m_Cameras) {
		if (std::string(cam->GetSerialNo()) == serialNo) {
			return cam->AddSubscriber(std::move(callback));
		}
	}
	return -1;
}

void CCameraService::RemoveSubscriber(const std::string& serialNo, ICamera::SubscriberID id)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto* cam : m_Cameras) {
		if (std::string(cam->GetSerialNo()) == serialNo) {
			cam->RemoveSubscriber(id);
			return;
		}
	}
}

void CCameraService::CaptureImages(ICamera* camera)
{
	std::string serialNo = std::string(camera->GetSerialNo());
	while (m_CapturingMap[serialNo])
	{
		if (!camera->IsOpen())
		{
			break;
		}
		xFrameData frameData{};
		camera->GetImageBuffer(frameData);
		//insert to imagequeue
	}
	return;
}

ImageQueue& CCameraService::GetImageQueue(const std::string& serialNo)
{
	for (auto& camera : m_Cameras)
	{
		if(serialNo == std::string(camera->GetSerialNo()))
		{
			return camera->GetImageQueue();
		}
	}
	static ImageQueue emptyQueue;
	return emptyQueue;
}




