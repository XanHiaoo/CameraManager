#include "stdafx.h"
#include "CCameraHik.h"
#include <sstream>
#include <string>
#include "00-Common/Exceptions/CameraHikException.h"
#include "03-Services/Equipment/EquipmentManager.h"
class ThreadPool
{
public:
	ThreadPool(size_t numThreads)
	{
		for (size_t i = 0; i < numThreads; ++i)
		{
			workers.emplace_back([this]()
			{
				while (true)
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(queueMutex);
						condition.wait(lock, [this] { return !tasks.empty() || stop; });
						if (stop && tasks.empty()) return;
						task = std::move(tasks.front());
						tasks.pop();
					}
					task();
				}
			});
		}
	}

	~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers) worker.join();
	}

	void EnqueueTask(std::function<void()> task)
	{
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			tasks.push(std::move(task));
		}
		condition.notify_one();
	}

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop = false;
};
ThreadPool threadPool(4);

void __stdcall CCameraHik::ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (!pData || !pFrameInfo || !pUser)
	{
		return;
	}
	CCameraHik* pCamera = static_cast<CCameraHik*>(pUser);
	ImageQueue& imageQueue = pCamera->GetImageQueue();
	//绑定 pBufAddr 的释放
	/*std::shared_ptr<xFrameData> frameData(new xFrameData, [](xFrameData* ptr) {
		if (ptr) {
			delete[] ptr->pBufAddr;
			delete ptr;
		}
	});
	frameData->stFrameInfo.nWidth = pFrameInfo->nWidth;
	frameData->stFrameInfo.nHeight = pFrameInfo->nHeight;
	frameData->stFrameInfo.nFrameLen = pFrameInfo->nFrameLen;
	frameData->stFrameInfo.enPixelType = ConvertPixelType(pFrameInfo->enPixelType);
	frameData->pBufAddr = new unsigned char[pFrameInfo->nFrameLen];
	std::memcpy(frameData->pBufAddr, pData, pFrameInfo->nFrameLen);
		imageQueue.PushImage(frameData);
	*/
	std::shared_ptr<xImage> imgData(new xImage, [](xImage * ptr)
	{
		if (ptr)
		{
			delete[] ptr->pBufAddr;
			delete ptr;
		}
	});
	imgData->nWidth = pFrameInfo->nWidth;
	imgData->nHeight = pFrameInfo->nHeight;
	imgData->nFrameLen = pFrameInfo->nFrameLen;
	imgData->enPixelType = ConvertPixelType(pFrameInfo->enPixelType);
	imgData->pBufAddr = new unsigned char[pFrameInfo->nFrameLen];
	std::memcpy(static_cast<unsigned char*>(imgData->pBufAddr), pData, pFrameInfo->nFrameLen);
	//std::memcpy(imgData->pBufAddr, pData, pFrameInfo->nFrameLen);
	imageQueue.PushImage(imgData);
	// **Pub**
	{
		std::vector<std::pair<ICamera::SubscriberID, ICamera::ImageCallback>> subscribersCopy;
		{
			std::lock_guard<std::mutex> lock(pCamera->subscribersMutex);
			subscribersCopy.assign(pCamera->subscribers.begin(), pCamera->subscribers.end());
		}
		// 通过全局线程池发布图像
		for (const auto&[id, callback] : subscribersCopy)
		{
			threadPool.EnqueueTask([callback, imgData]()
			{
				callback(imgData);
			});
		}
	}
}

//CCameraHik::CCameraHik(const char* serialNo) : CCameraHik(xCameraConfig(serialNo))
//{
//	//m_Config = xCameraConfig(serialNo);
//}

CCameraHik::CCameraHik(const xCameraConfig& cfg)
/*:m_Config(cfg)*/
{
	m_Config = cfg;
}

CCameraHik::CCameraHik(const char* serialNo)
{
	m_Config.serialNo = strdup(serialNo);  
}

CCameraHik::~CCameraHik()
{
	delete[] m_Config.serialNo;
	UnregisterCallback();
	Close();
}

void CCameraHik::Open()
{
	CCameraService& c= EquipmentManager::GetInstance().GetCameraService();
	int nRet = MV_OK;
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	// 遍历设备列表，找到匹配的序列号
	MV_CC_DEVICE_INFO* pCurDeviceInfo = nullptr;
	for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
	{
		MV_CC_DEVICE_INFO* pDev = stDeviceList.pDeviceInfo[i];
		if (pDev == nullptr)
		{
			continue;
		}
		char szSerialNumber[INFO_MAX_BUFFER_SIZE] = { 0 };
		switch (pDev->nTLayerType)
		{
		case MV_GIGE_DEVICE:
			strncpy((char*)szSerialNumber, (char*)pDev->SpecialInfo.stGigEInfo.chSerialNumber, sizeof(szSerialNumber) - 1);
			break;
		case MV_USB_DEVICE:
			strncpy((char*)szSerialNumber, (char*)pDev->SpecialInfo.stUsb3VInfo.chSerialNumber, sizeof(szSerialNumber) - 1);
			break;
		case MV_CAMERALINK_DEVICE:
			strncpy((char*)szSerialNumber, (char*)pDev->SpecialInfo.stCamLInfo.chSerialNumber, sizeof(szSerialNumber) - 1);
			break;
		case MV_VIR_GIGE_DEVICE:
			strncpy((char*)szSerialNumber, (char*)pDev->SpecialInfo.stGigEInfo.chSerialNumber, sizeof(szSerialNumber) - 1);
			break;
		default:
			break;
		}
		if (strcmp(m_Config.serialNo, szSerialNumber) == 0)
		{
			pCurDeviceInfo = pDev;
			break;
		}
	}
	if (pCurDeviceInfo == nullptr)
	{
		return;
	}
	// ch:创建句柄 | Create handle
	nRet = MV_CC_CreateHandle(&handle, pCurDeviceInfo);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	if (!m_bOpen)
		nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet)
	{
		int nRet2 = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet2)
		{
			handle = nullptr;
			throw CameraHikException(GetLastErr(nRet2));
		}
		else
			throw CameraHikException(GetLastErr(nRet));
	}
	m_bOpen = true;
	return;
}

void CCameraHik::Close()
{
	int nRet = MV_OK;
	if (m_bOpen)
	{
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK == nRet)
		{
			nRet = MV_CC_DestroyHandle(handle);
			if (MV_OK == nRet)
			{
				m_bOpen = false;
			}
			else
			{
				throw CameraHikException(GetLastErr(nRet));
			}
			m_bOpen = false;
		}
		else
		{
			nRet = MV_CC_DestroyHandle(handle);
			throw CameraHikException(GetLastErr(nRet));
		}
	}
}

void CCameraHik::ReleaseCamera(xDeviceInfoList& list)
{
	for (unsigned int i = 0; i < list.nDeviceNum; i++)
	{
		if (list.pDeviceInfo[i])
		{
			delete list.pDeviceInfo[i]; //
			// 或 free(list.pDeviceInfo[i])
			list.pDeviceInfo[i] = nullptr;
		}
	}
	list.nDeviceNum = 0;
}

int CCameraHik::GetSDKVersion()
{
	return MV_CC_GetSDKVersion();
}

int CCameraHik::EnumDevices(int nTLayerType, xDeviceInfoList& pstXDevList, std::vector<xCameraConfig>& xCamCfgList)
{
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	int nRet;
	if (-1 == nTLayerType)
	{
		nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE | MV_VIR_USB_DEVICE, &stDeviceList);
	}
	else
	{
		nRet = MV_CC_EnumDevices(nTLayerType, &stDeviceList);
	}
	if (nRet != MV_OK)
	{
		return nRet;
	}
	pstXDevList.nDeviceNum = stDeviceList.nDeviceNum;
	xCamCfgList.clear();
	for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
	{
		if (stDeviceList.pDeviceInfo[i] == nullptr)
		{
			continue;
		}
		pstXDevList.pDeviceInfo[i] = new xDeviceInfo();
		xCameraConfig xCamCfg;
		switch (stDeviceList.pDeviceInfo[i]->nTLayerType)
		{
		case MV_GIGE_DEVICE:
			xCamCfg.Name = strdup(reinterpret_cast<const char*>(stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName));
			xCamCfg.serialNo = strdup(reinterpret_cast<const char*>(stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber));

			xCamCfg.brand = xCameraBrand::Hikvision;
			xCamCfg.type = GigeCamera;
			pstXDevList.pDeviceInfo[i]->nCameraType = GigeCamera;
			strcpy((char*)pstXDevList.pDeviceInfo[i]->cameraName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chUserDefinedName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->serialNumber, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chSerialNumber);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->vendorName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chManufacturerName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->modelName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chModelName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->manufactureName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chManufacturerName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->deviceVersion, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chDeviceVersion);
			break;
		case MV_USB_DEVICE:
			xCamCfg.Name = reinterpret_cast<char*>(stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chUserDefinedName);
			xCamCfg.serialNo = reinterpret_cast<char*>(stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chSerialNumber);
			xCamCfg.brand = xCameraBrand::Hikvision;
			xCamCfg.type = Usb3Camera;
			pstXDevList.pDeviceInfo[i]->nCameraType = Usb3Camera;
			strcpy((char*)pstXDevList.pDeviceInfo[i]->cameraName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chUserDefinedName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->serialNumber, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chSerialNumber);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->vendorName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chManufacturerName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->modelName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chModelName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->manufactureName, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chManufacturerName);
			strcpy((char*)pstXDevList.pDeviceInfo[i]->deviceVersion, (char*)stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chDeviceVersion);
			break;
		default:
			break;
		}
		xCamCfgList.push_back(xCamCfg);
	}
	// 确保未使用的指针置为 nullptr，防止访问越界
	for (unsigned int i = stDeviceList.nDeviceNum; i < X_MAX_DEVICE_NUM; i++)
	{
		pstXDevList.pDeviceInfo[i] = nullptr;
	}
	return pstXDevList.nDeviceNum;
}

int CCameraHik::GetIndex()
{
	return m_Config.id;
}

const char* CCameraHik::GetSerialNo()
{
	return m_Config.serialNo;
}

void CCameraHik::GetConfig(xCameraConfig& config)
{
	config = m_Config;
}

bool CCameraHik::IsOpen()
{
	return m_bOpen;
}

void CCameraHik::StartGrabbing()
{
	int nRet = MV_OK;
	if (IsOpen())
	{
		nRet = MV_CC_StartGrabbing(handle);
		if (MV_OK != nRet)
		{
			m_bGrabbing = false;
			throw CameraHikException(GetLastErr(nRet));
			//destroy
			return;
		}
		m_bGrabbing = true;
		//SetTriggerMode(0);
	}
}

void CCameraHik::StopGrabbing()
{
	int nRet = MV_OK;
	if (m_bGrabbing)
	{
		nRet = MV_CC_StopGrabbing(handle);
		if (MV_OK != nRet)
		{
			m_bGrabbing = false;
			throw CameraHikException(GetLastErr(nRet));
			//destroy
			return;
		}
		m_bGrabbing = false;
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
		throw CameraHikException(GetLastErr(nRet));
	}
}

void CCameraHik::SetTriggerSource(unsigned int TriggerSourceNum)
{
	//0:Line0
	//1:Line1
	//2:Line2
	//3:Line3
	//4:Counter0
	//7:Software
	//8:FrequencyConverter
	int nRet = MV_OK;
	switch (TriggerSourceNum)
	{
	case X_TRIGGER_SOURCE_LINE0:
		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 0);
		break;
	case X_TRIGGER_SOURCE_LINE1:
		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 1);
		break;
	case X_TRIGGER_SOURCE_LINE2:
		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 2);
		break;
	case X_TRIGGER_SOURCE_LINE3:
		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 3);
		break;
	case X_TRIGGER_SOURCE_COUNTER0:
		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 4);
		break;
	case X_TRIGGER_SOURCE_SOFTWARE:
		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 7);
		break;
	case X_TRIGGER_SOURCE_FrequencyConverter:
		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", 8);
		break;
	default:
		break;
	}
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
}

void CCameraHik::SoftTrigger()
{
	int nRet = MV_OK;
	nRet = MV_CC_SetCommandValue(handle, "TriggerSoftware");
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
}

void CCameraHik::HardwareTrigger()
{
	return;
}

void CCameraHik::GetImageBuffer()
{
	return;
}

void CCameraHik::SetExpTime(double dExpTime)
{
	int nRet = MV_OK;
	nRet = MV_CC_SetEnumValue(handle, "ExposureAuto", 0);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	nRet = MV_CC_SetFloatValue(handle, "ExposureTime", dExpTime);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
}

double CCameraHik::GetExpTime()
{
	MVCC_FLOATVALUE nExposureTime = { 0 };
	int nRet = MV_OK;
	nRet = MV_CC_GetFloatValue(handle, "ExposureTime", &nExposureTime);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	return nExposureTime.fCurValue;
}

void CCameraHik::SetGain(double dGain)
{
	int nRet = MV_OK;
	nRet = MV_CC_SetEnumValue(handle, "GainAuto", 0);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	nRet = MV_CC_SetFloatValue(handle, "Gain", dGain);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
}

double CCameraHik::GetGain()
{
	MVCC_FLOATVALUE nGain = { 0 };
	int nRet = MV_OK;
	nRet = MV_CC_GetFloatValue(handle, "Gain", &nGain);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	return nGain.fCurValue;
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
		MV_CC_FreeImageBuffer(handle, &stImageInfo);
	}
	else
	{
		MV_CC_FreeImageBuffer(handle, &stImageInfo);
		throw CameraHikException(GetLastErr(nRet));
	}
}

void CCameraHik::GetImageBuffer(xImage& ImgData)
{
	int nRet = MV_OK;
	MV_FRAME_OUT stImageInfo = { 0 };
	nRet = MV_CC_GetImageBuffer(handle, &stImageInfo, 1000);
	if (nRet == MV_OK && stImageInfo.pBufAddr != nullptr)
	{
		ImgData.pBufAddr = stImageInfo.pBufAddr;
		ImgData.nWidth = stImageInfo.stFrameInfo.nWidth;
		ImgData.nHeight = stImageInfo.stFrameInfo.nHeight;
		ImgData.enPixelType = static_cast<xPixelType>(stImageInfo.stFrameInfo.enPixelType);
		ImgData.nFrameLen = stImageInfo.stFrameInfo.nFrameLen;
		//memcpy(ImgData.nRes, stImageInfo.nRes, sizeof(stImageInfo.nRes));
		MV_CC_FreeImageBuffer(handle, &stImageInfo);
	}
	else
	{
		MV_CC_FreeImageBuffer(handle, &stImageInfo);
		throw CameraHikException(GetLastErr(nRet));
	}
}

bool CCameraHik::IsSoftwareTrigger()
{
	MVCC_ENUMVALUE nMode = { 0 };
	MVCC_ENUMVALUE nSource = { 0 };
	int nRet = MV_OK;
	nRet = MV_CC_GetEnumValue(handle, "TriggerMode", &nMode);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	nRet = MV_CC_GetEnumValue(handle, "TriggerSource", &nSource);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	if (nMode.nCurValue == 1 && nSource.nCurValue == 7)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CCameraHik::GetPayloadSize()
{
	MVCC_INTVALUE nPayloadSize = { 0 };
	int nRet = MV_OK;
	nRet = MV_CC_GetIntValue(handle, "PayloadSize", &nPayloadSize);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	return nPayloadSize.nCurValue;
}

void CCameraHik::WaitCaptureEvent(int dwTimeOut)
{
	return;
}

void CCameraHik::GetExpTimeRange(double& minExp, double& maxExp)
{
	MVCC_FLOATVALUE nExposureTime = { 0 };
	int nRet = MV_CC_GetExposureTime(handle, &nExposureTime);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	minExp = nExposureTime.fMin;
	maxExp = nExposureTime.fMax;
}

void CCameraHik::GetGainRange(int& minGain, int& maxGain)
{
	MVCC_FLOATVALUE nGain = { 0 };
	int nRet = MV_CC_GetGain(handle, &nGain);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
	minGain = nGain.fMin;
	maxGain = nGain.fMax;
}

void CCameraHik::RegisterCallback()
{
	int nRet = MV_OK;
	// ch:注册抓图回调 | en:Register image callback
	nRet = MV_CC_RegisterImageCallBackEx(handle, ImageCallBackEx, this);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
}

void CCameraHik::UnregisterCallback()
{
	int nRet = MV_OK;
	// ch:注销抓图回调 | en:Unregister image callback
	nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
	if (MV_OK != nRet)
	{
		throw CameraHikException(GetLastErr(nRet));
	}
}

ICamera::SubscriberID CCameraHik::AddSubscriber(ImageCallback callback)
{
	std::lock_guard<std::mutex> lock(subscribersMutex);
	SubscriberID id;
	if (!freeIDs.empty())
	{
		// 复用空闲 ID
		id = freeIDs.front();
		freeIDs.pop();
	}
	else
	{
		id = nextID++;
	}
	subscribers[id] = callback;
	return id;
}

void CCameraHik::RemoveSubscriber(SubscriberID id)
{
	std::lock_guard<std::mutex> lock(subscribersMutex);
	auto it = subscribers.find(id);
	if (it != subscribers.end())
	{
		subscribers.erase(it);
		freeIDs.push(id); // 供后续复用
	}
}

int CCameraHik::GetWidth() const
{
	MVCC_INTVALUE nCamWidth;
	int nRet = MV_OK;
	nRet = MV_CC_GetIntValue(handle, "Width", &nCamWidth);
	if (MV_OK != nRet)
	{
		//throw CameraHikException(GetLastErr(nRet));
	}
	return nCamWidth.nCurValue;
}

int CCameraHik::GetHeight() const
{
	MVCC_INTVALUE nCamHeight;
	int nRet = MV_OK;
	nRet = MV_CC_GetIntValue(handle, "Height", &nCamHeight);
	if (MV_OK != nRet)
	{
		//throw CameraHikException(GetLastErr(nRet));
	}
	return nCamHeight.nCurValue;
}

int CCameraHik::GetNumChannels() const
{
	return m_nChannels;
}

ImageQueue& CCameraHik::GetImageQueue()
{
	return m_imageQueue;
}

std::string CCameraHik::GetLastErr(int errorCode)
{
	std::stringstream errorStream;
	errorStream << "camera : " << m_Config.serialNo;
	switch (errorCode)
	{
	case MV_E_HANDLE:
		errorStream << " Error or invalid handle ";
		break;
	case MV_E_SUPPORT:
		errorStream << " Not supported function ";
		break;
	case MV_E_BUFOVER:
		errorStream << " Cache is full ";
		break;
	case MV_E_CALLORDER:
		errorStream << " Function calling order error ";
		break;
	case MV_E_PARAMETER:
		errorStream << " Incorrect parameter ";
		break;
	case MV_E_RESOURCE:
		errorStream << " Applying resource failed ";
		break;
	case MV_E_NODATA:
		errorStream << " No data ";
		break;
	case MV_E_PRECONDITION:
		errorStream << " Precondition error, or running environment changed ";
		break;
	case MV_E_VERSION:
		errorStream << " Version mismatches ";
		break;
	case MV_E_NOENOUGH_BUF:
		errorStream << " Insufficient memory ";
		break;
	case MV_E_UNKNOW:
		errorStream << " Unknown error ";
		break;
	case MV_E_GC_GENERIC:
		errorStream << " General error ";
		break;
	case MV_E_GC_ACCESS:
		errorStream << " Node accessing condition error ";
		break;
	case MV_E_ACCESS_DENIED:
		errorStream << " No permission ";
		break;
	case MV_E_BUSY:
		errorStream << " Device is busy, or network disconnected ";
		break;
	case MV_E_NETER:
		errorStream << " Network error ";
		break;
	default:
		errorStream << " undefined status:" << errorCode;
		break;
	}
	return errorStream.str();
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