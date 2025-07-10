#pragma once
#include "01-Interface/ICamera.h"
#include "MvCameraControl.h"
#include <string.h>
#include "CameraParams.h"

#ifndef MV_NULL
#define MV_NULL    0
#endif

class CCameraHik : public ICamera
{
public:
	CCameraHik(const xCameraConfig& cfg);
	CCameraHik(const char* serialNo);

	~CCameraHik();
	int GetSDKVersion()override;

	/**
	 * @brief 枚举指定类型的设备
	 *
	 * @param nTLayerType 指定需要枚举的设备类型，可以是以下值之一：
	 *        - MV_ALL_DEVICE (0xFFFFFFFF)         : 所有设备类型
	 *        - MV_UNKNOW_DEVICE (0x00000000)      : 未知设备类型，保留意义
	 *        - MV_GIGE_DEVICE (0x00000001)        : GigE 设备
	 *        - MV_1394_DEVICE (0x00000002)        : 1394-a/b 设备
	 *        - MV_USB_DEVICE (0x00000004)         : USB 设备
	 *        - MV_CAMERALINK_DEVICE (0x00000008)  : CameraLink 设备
	 *        - MV_VIR_GIGE_DEVICE (0x00000010)    : 虚拟 GigE 设备
	 *        - MV_VIR_USB_DEVICE (0x00000020)     : 虚拟 USB 设备
	 *        - MV_GENTL_GIGE_DEVICE (0x00000040)  : GenTL GigE 设备
	 * @param pstXDevList 设备信息列表，输出枚举到的设备信息
	 * @xCamCfgList 相机配置列表，输出枚举到的设备对应的配置列表
	 * @return int 返回枚举到的设备数量，若失败则返回错误码
	 */
	static int EnumDevices(int nTLayerType, xDeviceInfoList &pstXDevList, std::vector<xCameraConfig> &xCamCfgList);
	int GetIndex() override;
	const char* GetSerialNo() override;
	void GetConfig(xCameraConfig& config) override;
	bool IsOpen() override;
	void Open() override;
	void Close() override;

	//@TODO:是否改为静态
	void ReleaseCamera(xDeviceInfoList& list) override;
	void StartGrabbing() override;
	void StopGrabbing() override;
	void SetTriggerMode(unsigned int TriggerModeNum) override;
	void SetTriggerSource(unsigned int TriggerSourceNum) override;
	void SoftTrigger() override;
	//void SoftwareTrigger() override;
	void HardwareTrigger() override;
	void GetImageBuffer() override;
	void SetExpTime(double dExpTime) override;
	double GetExpTime() override;
	void SetGain(double dGain) override;
	double GetGain() override;
	void GetImageBuffer(xFrameData& frameData) override;
	void GetImageBuffer(xImage& ImgData)	override;
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

	std::string GetLastErr(int errorCode)override;

	SubscriberID AddSubscriber(ImageCallback callback) override;
	void RemoveSubscriber(SubscriberID id) override;

	static void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

private:

	//bool m_Open;
	//bool m_Grabbing;
	void* handle = NULL;


	//MV_CC_DEVICE_INFO_LIST   m_stDevList;   //设备信息列表
private:
	static xPixelType ConvertPixelType(MvGvspPixelType mvPixelType);
};

