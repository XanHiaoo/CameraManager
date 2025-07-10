#include <iostream>
#include <string>
#include <vector>
#include "01-Interface/ICamera.h"
#include "02-Addins/Camera/CCameraHik.h"
#include "03-Services/CCameraService.h"
#include "00-Common/xCameraParam.h"

int main() {
	try {
		CCameraService cameraService;

		//cameraService.EnumDevicesByBrand(CameraBrand::Hikvision);

		cameraService.OpenAll();

		cameraService.StartGrabbing();
		// 获取指定相机的图像数据
		xFrameData frameData;
		cameraService.GetImageBuffer(std::string(cameraService.GetAllCameraSerials()[0]), frameData);

		// 注册回调（所有相机）
		cameraService.RegisterCallback();

		cameraService.SetTriggerMode(0);

		cameraService.StartGrabbing();

		while (1) {
			ImageQueue& imageQueue = cameraService.GetImageQueue(std::string(cameraService.GetAllCameraSerials()[0]));
		}
		//// 注册回调（指定相机）
		//cameraService.RegisterCallback("67890");

		//// 停止抓图（指定相机）
		//cameraService.StopGrabbing("12345");

		//// 停止抓图（所有相机）
		//cameraService.StopGrabbing();

		//// 关闭所有相机
		//cameraService.CloseAll();

	}
	catch (const std::exception& e) {
		std::cerr << "异常: " << e.what() << std::endl;
	}

	return 0;
}
