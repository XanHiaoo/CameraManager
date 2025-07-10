#pragma once
#include <cstdint>
#include <cstring>

#define X_MAX_DEVICE_NUM               100   ///< \~chinese 支持设备最大个数		\~english The maximum number of supported devices
#define X_INFO_MAX_BUFFER_SIZE           256	 ///< \~chinese 字符串最大长度		\~english The maximum length of string
#define X_MAX_ERROR_LIST_NUM		   128	 ///< \~chinese 失败属性列表最大长度 \~english The maximum size of failed properties list		

///\~chinese 像素格式               \~english Pixel Type
enum xPixelType
{
	PixelType_X_Mono8       = 0x0001,
	PixelType_X_RGB8_Packed = 0x0002,
};
/// \~chinese 输出帧的信息              \~english Output Frame Information
struct xFrameInfo
{
	unsigned short          nWidth;                                 ///< [OUT] \~chinese 图像宽                 \~english Image Width
	unsigned short          nHeight;                                ///< [OUT] \~chinese 图像高                 \~english Image Height
	enum xPixelType         enPixelType;
	unsigned int            nFrameLen;                              ///< [OUT] \~chinese 帧的长度               \~english The Length of Frame

};
/// \~chinese 图像结构体，输出图像地址及图像信息    \~english Image Struct, output the pointer of Image and the information of the specific image
struct xFrameData
{
	unsigned char*			pBufAddr;                               ///< [OUT] \~chinese 图像指针地址           \~english  pointer of image
	xFrameInfo				stFrameInfo;                            ///< [OUT] \~chinese 图像信息               \~english information of the specific image
	unsigned int            nRes[16];                               ///<       \~chinese 预留                   \~english Reserved
};
/// \~chinese
///枚举：设备类型
/// \~english
///Enumeration: device type
enum xCameraType
{
	GigeCamera = 0,						///< \~chinese GIGE相机				\~english GigE Vision Camera
	Usb3Camera = 1,						///< \~chinese USB3.0相机			\~english USB3.0 Vision Camera
	CLCamera = 2,						///< \~chinese CAMERALINK 相机		\~english Cameralink camera
	PCIeCamera = 3,						///< \~chinese PCIe相机				\~english PCIe Camera
	UndefinedCamera = 255				///< \~chinese 未知类型				\~english Undefined Camera
};
/// \~chinese
/// \brief 设备通用信息
/// \~english
/// \brief Device general information
struct xDeviceInfo
{
	xCameraType				     	nCameraType;								///< \~chinese 设备类别			\~english Camera type
	int								nCameraReserved[5];							///< \~chinese 保留				\~english Reserved field

	unsigned char					cameraName[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese 用户自定义名		\~english UserDefinedName
	unsigned char					serialNumber[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese 设备序列号		\~english Device SerialNumber
	unsigned char					vendorName[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese 厂商				\~english Camera Vendor
	unsigned char					modelName[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese 设备型号			\~english Device model
	unsigned char					manufactureName[X_INFO_MAX_BUFFER_SIZE];		///< \~chinese 设备制造信息		\~english Device ManufactureInfo
	unsigned char					deviceVersion[X_INFO_MAX_BUFFER_SIZE];		///< \~chinese 设备版本			\~english Device Version
	unsigned char					cameraReserved[5][X_INFO_MAX_BUFFER_SIZE];	///< \~chinese 保留				\~english Reserved field
};
/// \~chinese
/// \brief 设备信息列表
/// \~english
/// \brief Device information list
struct xDeviceInfoList
{
	unsigned int            nDeviceNum;                             ///< [OUT] \~chinese 在线设备数量           \~english Online Device Number
	xDeviceInfo*			pDeviceInfo[X_MAX_DEVICE_NUM];                            ///< [OUT] \~chinese 支持最多256个设备      \~english Support up to 256 devices
};
/// \~chinese GigE设备信息              \~english GigE device info
struct xGigeDeviceInfo
{
	unsigned int            nIpCfgOption;                           ///< [OUT] \~chinese IP配置选项
	unsigned int            nIpCfgCurrent;                          ///< [OUT] \~chinese 当前IP配置
};
/// \~chinese USB设备信息               \~english USB device info
struct xUsbDeviceInfo
{
	bool bLowSpeedSupported;						///< \~chinese true支持，false不支持，其他值 非法。	\~english true support,false not supported,other invalid
	bool bFullSpeedSupported;						///< \~chinese true支持，false不支持，其他值 非法。	\~english true support,false not supported,other invalid
	bool bHighSpeedSupported;						///< \~chinese true支持，false不支持，其他值 非法。	\~english true support,false not supported,other invalid
	bool bSuperSpeedSupported;						///< \~chinese true支持，false不支持，其他值 非法。	\~english true support,false not supported,other invalid
	bool bDriverInstalled;							///< \~chinese true安装，false未安装，其他值 非法。	\~english true support,false not supported,other invalid
	bool boolReserved[3];							///< \~chinese 保留
	unsigned int Reserved[4];						///< \~chinese 保留									\~english Reserved field

	char configurationValid[X_INFO_MAX_BUFFER_SIZE];	///< \~chinese 配置有效性							\~english Configuration Valid
	char genCPVersion[X_INFO_MAX_BUFFER_SIZE];		///< \~chinese GenCP 版本							\~english GenCP Version
	char u3vVersion[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese U3V 版本号							\~english U3v Version
	char deviceGUID[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese 设备引导号							\~english Device guid number
	char familyName[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese 设备系列号							\~english Device serial number
	char u3vSerialNumber[X_INFO_MAX_BUFFER_SIZE];		///< \~chinese 设备序列号							\~english Device SerialNumber
	char speed[X_INFO_MAX_BUFFER_SIZE];				///< \~chinese 设备传输速度							\~english Device transmission speed
	char maxPower[X_INFO_MAX_BUFFER_SIZE];			///< \~chinese 设备最大供电量						\~english Maximum power supply of device
	char chReserved[4][X_INFO_MAX_BUFFER_SIZE];		///< \~chinese 保留									\~english Reserved field
};
/// \~chinese Int类型值Ex               \~english Int Value Ex
struct xIntValue
{
	int64_t             nCurValue;                                  ///< [OUT] \~chinese 当前值                 \~english Current Value
	int64_t             nMax;                                       ///< [OUT] \~chinese 最大值                 \~english Max
	int64_t             nMin;                                       ///< [OUT] \~chinese 最小值                 \~english Min
	int64_t             nInc;                                       ///< [OUT] \~chinese Inc                    \~english Inc

	unsigned int        nReserved[16];                              ///<       \~chinese 预留                   \~english Reserved
};
/// \~chinese Float类型值               \~english Float Value
struct xFloatValue
{
	float               fCurValue;                                  ///< [OUT] \~chinese 当前值                 \~english Current Value
	float               fMax;                                       ///< [OUT] \~chinese 最大值                 \~english Max
	float               fMin;                                       ///< [OUT] \~chinese 最小值                 \~english Min

	unsigned int        nReserved[4];                               ///<       \~chinese 预留                   \~english Reserved
};
/// \~chinese String类型值              \~english String Value
struct xStringValue
{
	char                chCurValue[256];                            ///< [OUT] \~chinese 当前值                 \~english Current Value

	int64_t             nMaxLength;                                 ///< [OUT] \~chinese 最大长度               \~english MaxLength
	unsigned int        nReserved[2];                               ///<       \~chinese 预留                   \~english Reserved
};
/// \~chinese 保存图片格式              \~english Save image type
enum xSaveImageType
{
	X_Image_Undefined = 0,                        ///< \~chinese 未定义的图像格式             \~english Undefined Image Type
	X_Image_Bmp       = 1,                        ///< \~chinese BMP图像格式                  \~english BMP Image Type
	X_Image_Jpeg      = 2,                        ///< \~chinese JPEG图像格式                 \~english Jpeg Image Type
	X_Image_Png       = 3,                        ///< \~chinese PNG图像格式                  \~english Png  Image Type
	X_Image_Tif       = 4,                        ///< \~chinese TIFF图像格式                 \~english TIFF Image Type
};
/// \~chinese 触发模式                  \~english Trigger Mode
enum xCameraTriggerMode
{
	X_TRIGGER_MODE_OFF = 0,                        ///< \~chinese 关闭                         \~english Off
	X_TRIGGER_MODE_ON  = 1,                        ///< \~chinese 打开                         \~english ON
};
enum xCameraTRIGGER_SOURCE
{
	X_TRIGGER_SOURCE_LINE0 = 0,                        ///< \~chinese Line0                        \~english Line0
	X_TRIGGER_SOURCE_LINE1 = 1,                        ///< \~chinese Line1                        \~english Line1
	X_TRIGGER_SOURCE_LINE2 = 2,                        ///< \~chinese Line2                        \~english Line2
	X_TRIGGER_SOURCE_LINE3 = 3,                        ///< \~chinese Line3                        \~english Line3
	X_TRIGGER_SOURCE_COUNTER0 = 4,                        ///< \~chinese Conuter0                     \~english Conuter0

	X_TRIGGER_SOURCE_SOFTWARE = 7,                        ///< \~chinese 软触发                       \~english Software
	X_TRIGGER_SOURCE_FrequencyConverter = 8,                        ///< \~chinese 变频器                       \~english Frequency Converter
};
enum class xCameraBrand
{
	Hikvision,
	Dahua,
	Basler
};
#include <cstring>

struct xCameraConfig
{
	unsigned int id;
	char* Name;
	char* serialNo;
	xCameraBrand brand;
	xCameraType type;

	xCameraConfig() : id(0), Name(nullptr), serialNo(nullptr), brand(xCameraBrand::Hikvision), type(xCameraType::GigeCamera) {}

	xCameraConfig(const char* serialNo, const char* Name = "", unsigned int id = 0,
		const xCameraBrand& brand = xCameraBrand::Hikvision, const xCameraType& type = xCameraType::GigeCamera)
		: id(id), brand(brand), type(type)
	{
		this->Name = strdup(Name);         
		this->serialNo = strdup(serialNo); 
	}

	xCameraConfig(const xCameraConfig& other)  
		: id(other.id), brand(other.brand), type(other.type)
	{
		this->Name = strdup(other.Name);
		this->serialNo = strdup(other.serialNo);
	}

	xCameraConfig& operator=(const xCameraConfig& other) 
	{
		if (this != &other)
		{
			id = other.id;
			brand = other.brand;
			type = other.type;

			delete[] Name;
			delete[] serialNo;

			Name = strdup(other.Name);
			serialNo = strdup(other.serialNo);
		}
		return *this;
	}

	//~xCameraConfig() 
	//{
	//	delete[] Name;
	//	delete[] serialNo;
	//}

	bool operator==(const xCameraConfig& other) const
	{
		return id == other.id &&
			strcmp(Name, other.Name) == 0 &&  
			strcmp(serialNo, other.serialNo) == 0 &&
			brand == other.brand &&
			type == other.type;
	}
};

struct xImage
{
	void*		 pBufAddr;
	unsigned int nFrameLen;
	int			 nWidth;
	int			 nHeight;
	xPixelType   enPixelType;
};