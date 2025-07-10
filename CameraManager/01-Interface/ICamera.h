#pragma once
#include "00-Common/xCameraParam.h"
#include "00-Common/ImageQueue.h"
#include <memory>
#include <functional>
#include <mutex>
#include <queue>
//异常不使用返回值 而是用exception代替
class ICamera
{
public:
	using ImageCallback = std::function<void(std::shared_ptr<xImage>)>;
	using SubscriberID = size_t;

	//TODO:sdk_ver是否都为int 
	virtual int GetSDKVersion() = 0;
	//virtual int EnumDevices(unsigned int nTLayerType, xDeviceInfoList* pstXDevList) = 0;
	//获取相机索引
	virtual int GetIndex() = 0;
	virtual const char* GetSerialNo() = 0;
	virtual void GetConfig(xCameraConfig& config) = 0;
	virtual bool IsOpen() = 0;

	virtual void Open() = 0;
	virtual void Close() = 0;
	virtual void ReleaseCamera(xDeviceInfoList& list) = 0;

	// 开启抓图
	virtual void StartGrabbing() = 0;

	// 停止抓图
	virtual void StopGrabbing() = 0;

	// 设置触模式
	virtual void SetTriggerMode(unsigned int TriggerModeNum) = 0;

	// 设置触发源
	virtual void SetTriggerSource(unsigned int TriggerSourceNum) = 0;

	// 软触发
	virtual void SoftTrigger() = 0;
	////设置相机软触发
	//virtual void SoftwareTrigger() = 0;
	//相机硬触发
	virtual void HardwareTrigger() = 0;
	//取像
	virtual void GetImageBuffer() = 0;
	//virtual bool GetImageBuffer(void* pBuff) = 0;
	//重载
	//获取图像数据
	virtual void GetImageBuffer(xFrameData& frameData) = 0;
	virtual void GetImageBuffer(xImage& ImgData) = 0;
	//设定曝光时间,单位毫秒
	virtual void SetExpTime(double dExpTime) = 0;
	//获取曝光时间，单位毫秒
	virtual double GetExpTime() = 0;

	//设定增益
	virtual void SetGain(double dGain) = 0;
	//获取增益
	virtual double GetGain() = 0;

	//是否软触发
	virtual bool IsSoftwareTrigger() = 0;
	//获取Payload Size
	virtual int GetPayloadSize() = 0;
	//等待捕获事件
	virtual void WaitCaptureEvent(int dwTimeOut = 200) = 0;
	//获取曝光事件的Range
	virtual void GetExpTimeRange(double& minExp, double& maxExp) = 0;
	//获取增益的Range
	virtual void GetGainRange(int& minGain, int& maxGain) = 0;
	//注册回调
	virtual void RegisterCallback() = 0;
	//注销回调
	virtual void UnregisterCallback() = 0;

	//获取灯光
	//virtual ILight* GetLight(int nChannel) = 0;
	////获取相机配置
	//virtual ICameraConfig* GetConfig() = 0;

	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual int GetNumChannels() const = 0;
	virtual ImageQueue& GetImageQueue() = 0;

	virtual std::string GetLastErr(int errorCode) = 0;

	// **（Pub-Sub）**
	virtual SubscriberID AddSubscriber(ImageCallback callback) = 0;

	virtual void RemoveSubscriber(SubscriberID id) = 0;

protected:
	/*int				m_nIndex;
	const char*     m_serialNo;*/

	std::unordered_map<SubscriberID, std::function<void(std::shared_ptr<xImage>)>> subscribers;
	std::mutex subscribersMutex;
	std::queue<SubscriberID> freeIDs; // 存放可复用的ID
	SubscriberID nextID = 1; // 初始 ID 从 1 开始

	bool			m_bOpen           =  false;
	int				m_nHeight         = 0;
	int				m_nChannels       = 0;
	bool			m_bGrabbing       = false;
	bool			m_bSDKInitialized = false;
	xCameraConfig   m_Config;
	ImageQueue      m_imageQueue;
};
