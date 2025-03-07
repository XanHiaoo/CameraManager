#include "CCameraHik.h"

void __stdcall CCameraHik::ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (!pData || !pFrameInfo || !pUser)
	{
		return;
	}

	CCameraHik* pCamera = static_cast<CCameraHik*>(pUser);

	ImageQueue& imageQueue = pCamera->GetImageQueue();

	auto frameData = std::make_shared<xFrameData>();

	frameData->stFrameInfo.nWidth = pFrameInfo->nWidth;
	frameData->stFrameInfo.nHeight = pFrameInfo->nHeight;
	frameData->stFrameInfo.nFrameLen = pFrameInfo->nFrameLen;
	frameData->stFrameInfo.enPixelType = ConvertPixelType(pFrameInfo->enPixelType);

	frameData->pBufAddr = new unsigned char[pFrameInfo->nFrameLen];
	std::memcpy(frameData->pBufAddr, pData, pFrameInfo->nFrameLen);

	imageQueue.PushImage(frameData);
}

CCameraHik::CCameraHik()
{
}

CCameraHik::~CCameraHik()
{
}

void CCameraHik::Open()
{
	int nRet = MV_OK;
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet)
	{
		printf("Enum Devices fail! nRet [0x%x]\n", nRet);
		return;
	}

	// 遍历设备列表，找到匹配的序列号
	MV_CC_DEVICE_INFO* pDeviceInfo = nullptr;
	for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
	{
		MV_CC_DEVICE_INFO* pDev = stDeviceList.pDeviceInfo[i];
		if (pDev == nullptr)
		{
			continue;
		}

		char szSerialNumber[INFO_MAX_BUFFER_SIZE] = { 0 };
		if (pDev->nTLayerType == MV_GIGE_DEVICE)
		{
			strncpy(szSerialNumber, (char*)pDev->SpecialInfo.stGigEInfo.chSerialNumber, sizeof(szSerialNumber) - 1);
		}
		else if (pDev->nTLayerType == MV_USB_DEVICE)
		{
			strncpy(szSerialNumber, (char*)pDev->SpecialInfo.stUsb3VInfo.chSerialNumber, sizeof(szSerialNumber) - 1);
		}

		if (m_serialNo == szSerialNumber)
		{
			pDeviceInfo = pDev;
			break;
		}
	}

	if (pDeviceInfo == nullptr)
	{
		return;
	}

	// ch:创建句柄 | Create handle
	nRet = MV_CC_CreateHandle(&handle, pDeviceInfo);
	if (MV_OK != nRet)
	{
		printf("Create Handle fail! nRet [0x%x]\n", nRet);
		return;
	}

	// ch:打开设备 | Open device
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet)
	{
		printf("Open Device fail! nRet [0x%x]\n", nRet);
		return;
	}

}

void CCameraHik::Close()
{
	int nRet = MV_OK;
	// ch:销毁句柄 | en:Destroy handle
	nRet = MV_CC_DestroyHandle(handle);
	if (MV_OK != nRet)
	{
		return;
	}
}

int CCameraHik::GetSDKVersion()
{
	return 0;
}

std::vector<ICamera*> CCameraHik::EnumDevices()
{
	std::vector<ICamera*> devices;
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

	int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet)
	{
		return devices;
	}

	for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
	{
		MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
		if (pDeviceInfo == nullptr)
		{
			continue;
		}

		std::string serialNumber;
		if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
		{
			serialNumber = reinterpret_cast<char*>(pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
		}
		else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
		{
			serialNumber = reinterpret_cast<char*>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		}

		CCameraHik* camera = new CCameraHik();
		camera->m_serialNo = serialNumber;
		devices.push_back(camera);
	}

	return devices;
}

int CCameraHik::GetIndex()
{
	return 0;
}

const std::string CCameraHik::GetSerialNo()
{
	return m_serialNo;
}

bool CCameraHik::IsOpen()
{
	return true;
}

void CCameraHik::StartGrabbing()
{
	int nRet = MV_OK;
	nRet = MV_CC_StartGrabbing(handle);
	if (MV_OK != nRet)
	{
		return;
	}

	SetTriggerMode(0);
}

void CCameraHik::StopGrabbing()
{
	int nRet = MV_OK;
	nRet = MV_CC_StopGrabbing(handle);
	if (MV_OK != nRet)
	{
		return;
	}
}

void CCameraHik::SetTriggerMode(unsigned int TriggerModeNum)
{
	//0:Off 
	//1:On
	int nRet = MV_OK;
	nRet = MV_CC_SetEnumValue(handle, "TriggerMode", TriggerModeNum);
	if (MV_OK != nRet)
	{
		return;
	}
}

void CCameraHik::SetTriggerSource(unsigned int TriggerSourceNum)
{
	return;
}

void CCameraHik::SoftTrigger()
{
	return;
}

void CCameraHik::SoftwareTrigger()
{
	return;
}

void CCameraHik::HardwareTrigger()
{
	return;
}

void CCameraHik::CaptureImage()
{
	return;
}

void CCameraHik::SetExpTime(double dExpTime)
{
	return;
}

double CCameraHik::GetExpTime()
{
	return 0.0;
}

void CCameraHik::SetGain(int nGain)
{
	return;
}

double CCameraHik::GetGain()
{
	return 0;
}

void CCameraHik::GetImageBuffer(xFrameData& frameData)
{
	int nRet = MV_OK;
	MV_FRAME_OUT stImageInfo = { 0 };
	nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 1000);
	if (nRet == MV_OK && stImageInfo.pBufAddr != nullptr)
	{
		frameData.pBufAddr = stImageInfo.pBufAddr;

		frameData.stFrameInfo.nWidth = stImageInfo.stFrameInfo.nWidth;
		frameData.stFrameInfo.nHeight = stImageInfo.stFrameInfo.nHeight;
		frameData.stFrameInfo.enPixelType = static_cast<xPixelType>(stImageInfo.stFrameInfo.enPixelType);
		frameData.stFrameInfo.nFrameLen = stImageInfo.stFrameInfo.nFrameLen;

		memcpy(frameData.nRes, stImageInfo.nRes, sizeof(stImageInfo.nRes));
	}
}

bool CCameraHik::IsSoftwareTrigger()
{
	return false;
}

int CCameraHik::GetPayloadSize()
{
	return 0;
}

void CCameraHik::WaitCaptureEvent(int dwTimeOut)
{
	return;
}

void CCameraHik::GetExpTimeRange(double & minExp, double & maxExp)
{
}

void CCameraHik::GetGainRange(int & minGain, int & maxGain)
{
}

void CCameraHik::RegisterCallback()
{
	int nRet = MV_OK;
	// ch:注册抓图回调 | en:Register image callback
	nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, this);
	if (MV_OK != nRet)
	{
		return;
	}
}

void CCameraHik::UnregisterCallback()
{
	int nRet = MV_OK;
	// ch:注销抓图回调 | en:Unregister image callback
	nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
	if (MV_OK != nRet)
	{
		return;
	}
}

int CCameraHik::GetWidth() const
{
	return 0;
}

int CCameraHik::GetHeight() const
{
	return 0;
}

int CCameraHik::GetNumChannels() const
{
	return 0;
}

ImageQueue & CCameraHik::GetImageQueue()
{
	return m_imageQueue;
}

xPixelType CCameraHik::ConvertPixelType(MvGvspPixelType mvPixelType)
{
	switch (mvPixelType)
	{
	case PixelType_Gvsp_Mono8:
		return PixelType_X_Mono8;
	case PixelType_Gvsp_RGB8_Packed:
		return PixelType_X_RGB8_Packed;
	default:
		return PixelType_X_Mono8; // 默认值
	}
}
