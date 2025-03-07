#pragma once

enum xPixelType
{
	PixelType_X_Mono8		= 0x0001,
	PixelType_X_RGB8_Packed = 0x0002,
};
struct xFrameInfo
{
	unsigned short          nWidth;                                 ///< [OUT] \~chinese 图像宽                 \~english Image Width
	unsigned short          nHeight;                                ///< [OUT] \~chinese 图像高                 \~english Image Height
	enum xPixelType         enPixelType;
	unsigned int            nFrameLen; 
	///< [OUT] \~chinese 帧的长度               \~english The Length of Frame

};
struct xFrameData                                  
{
	unsigned char*			pBufAddr;                               ///< [OUT] \~chinese 图像指针地址           \~english  pointer of image
	xFrameInfo				stFrameInfo;                            ///< [OUT] \~chinese 图像信息               \~english information of the specific image
	unsigned int            nRes[16];                               ///<       \~chinese 预留                   \~english Reserved
};

enum class CameraBrand
{
	Hikvision,
	Dahua,
	Basler
};
