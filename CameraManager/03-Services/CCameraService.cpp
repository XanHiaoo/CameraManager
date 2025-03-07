#include "CCameraService.h"
#include "00-Common/xCameraParam.h"
#include "02-Addins/Camera/CCameraHik.h"

CCameraService::CCameraService() {}

CCameraService::~CCameraService()
{
	StopCapturing();
	CloseAll();
}

void CCameraService::Inject(ICamera& cam)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	m_Cameras.push_back(&cam);
	m_CapturingMap[cam.GetSerialNo()] = false;
}

void CCameraService::EnumDevicesForBrand(CameraBrand brand)
{
	std::vector<ICamera*> devices;

	switch (brand)
	{
	case CameraBrand::Hikvision:
		devices = CCameraHik::EnumDevices();
		break;
	case CameraBrand::Dahua:
		// Dahua的设备枚举逻辑
		break;
	default:
		return;
	}

	for (auto* device : devices)
	{
		Inject(*device);
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
		camera->Open();
	}
}

void CCameraService::CloseAll()
{
	for (auto& camera : m_Cameras)
	{
		camera->Close();
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
		if (serialNo == cam->GetSerialNo())
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
		if (serialNo == cam->GetSerialNo())
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
		if (serialNo == camera->GetSerialNo())
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
		StartCapturing(camera->GetSerialNo());
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
		if (serialNo == camera->GetSerialNo())
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
		StopCapturing(camera->GetSerialNo());
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
		RegisterCallback(camera->GetSerialNo());
	}
}

void CCameraService::RegisterCallback(const std::string& serialNo)
{
	std::lock_guard<std::mutex> lock(m_CamMutex);
	for (auto& camera : m_Cameras)
	{
		if (serialNo == camera->GetSerialNo())
		{
			camera->RegisterCallback();
			return;
		}
	}
}

void CCameraService::CaptureImages(ICamera* camera)
{
	std::string serialNo = camera->GetSerialNo();
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
		if (serialNo == camera->GetSerialNo())
		{
			return camera->GetImageQueue();
		}
	}
	static ImageQueue emptyQueue;
	return emptyQueue;
}




