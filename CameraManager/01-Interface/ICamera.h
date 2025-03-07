#pragma once
#include "00-Common/xCameraParam.h"
#include "00-Common/ImageQueue.h"
//异常不使用返回值 而是用exception代替
class ICamera
{
public:
	/* virtual void Open() = 0;
	 virtual void Close() = 0;*/
	virtual int GetSDKVersion() = 0;
	//获取相机索引
	virtual int GetIndex() = 0;
	virtual const std::string GetSerialNo() = 0;
	//打开相机
	//virtual bool Open() = 0;
	//是否打开
	virtual bool IsOpen() = 0;

	virtual void Open() = 0;
	virtual void Close() = 0;
	//关闭相机
	//virtual bool Close() = 0;

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
	//设置相机软触发
	virtual void SoftwareTrigger() = 0;
	//设置相机硬触发
	virtual void HardwareTrigger() = 0;
	//取像
	virtual void CaptureImage() = 0;
	//virtual bool CaptureImage(void* pBuff) = 0;
	//重载

	//设定曝光时间,单位毫秒
	virtual void SetExpTime(double dExpTime) = 0;
	//获取曝光时间，单位毫秒
	virtual double GetExpTime() = 0;
	//设定增益
	virtual void SetGain(int nGain) = 0;
	//获取增益
	virtual double GetGain() = 0;
	//获取图像数据
	virtual void GetImageBuffer(xFrameData& frameData) = 0;
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

protected:
	int				m_nIndex;
	std::string		m_serialNo;
	bool			m_bOpen;
	int				m_nNumChannels;
	int				m_nHeight;
	int				m_nChannels;
	bool			m_bGrabbing;

	ImageQueue m_imageQueue;
};
