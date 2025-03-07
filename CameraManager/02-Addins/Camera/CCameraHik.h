#pragma once

#include "01-Interface/ICamera.h"
#include "MvCameraControl.h"
#include <string.h>

#ifndef MV_NULL
#define MV_NULL    0
#endif

class CCameraHik : public ICamera
{
public:
	CCameraHik();
	~CCameraHik();

	int GetSDKVersion() override;
	static std::vector<ICamera*> EnumDevices();
	int GetIndex() override;
	const std::string GetSerialNo() override;
	bool IsOpen() override;
	void Open() override;
	void Close() override;
	void StartGrabbing() override;
	void StopGrabbing() override;
	void SetTriggerMode(unsigned int TriggerModeNum) override;
	void SetTriggerSource(unsigned int TriggerSourceNum) override;
	void SoftTrigger() override;
	void SoftwareTrigger() override;
	void HardwareTrigger() override;
	void CaptureImage() override;
	void SetExpTime(double dExpTime) override;
	double GetExpTime() override;
	void SetGain(int nGain) override;
	double GetGain() override;
	void GetImageBuffer(xFrameData& frameData) override;
	bool IsSoftwareTrigger() override;
	int GetPayloadSize() override;
	void WaitCaptureEvent(int dwTimeOut = 200) override;
	void GetExpTimeRange(double& minExp, double& maxExp) override;
	void GetGainRange(int& minGain, int& maxGain) override;
	void RegisterCallback() override;
	void UnregisterCallback() override;
	int GetWidth() const override;
	int GetHeight() const override;
	int GetNumChannels() const override;
	ImageQueue& GetImageQueue() override;

	static void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

private:
	bool m_bSDKInitialized;
	void* handle = NULL;

private:
	static xPixelType ConvertPixelType(MvGvspPixelType mvPixelType);
};
