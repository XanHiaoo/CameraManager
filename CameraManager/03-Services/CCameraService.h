#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include "01-Interface/ICamera.h"
#include "00-Common/ImageQueue.h"

class CCameraService
{
private:
	std::vector<ICamera*> m_Cameras;
	std::unordered_map<std::string, std::thread> m_CameraThreads;
	std::unordered_map<std::string, std::atomic<bool>> m_CapturingMap;
	//std::unordered_map<std::string, std::unique_ptr<ImageQueue>> m_cameraImageQueues;
	std::mutex m_CamMutex;

	void CaptureImages(ICamera* camera);

public:
	CCameraService();
	~CCameraService();

	void Inject(ICamera& cam);
	void EnumDevicesForBrand(CameraBrand brand);
	std::vector<std::string> GetAllCameraSerials();


	void OpenAll();
	void CloseAll();

	void StartGrabbing();
	void StartGrabbing(const std::string& serialNo);

	void StopGrabbing();
	void StopGrabbing(const std::string& serialNo);

	void SetTriggerMode(unsigned int mode);
	void SetTriggerMode(const std::string& serialNo, unsigned int mode);

	void StartCapturing();
	void StartCapturing(const std::string& serialNo);
	void StopCapturing();
	void StopCapturing(const std::string& serialNo);

	void GetImageBuffer(const std::string& serialNo, xFrameData& frameData);

	void RegisterCallback();
	void RegisterCallback(const std::string& serialNo);

	ImageQueue& GetImageQueue(const std::string& serialNo);
};
