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
		// ��ȡָ�������ͼ������
		xFrameData frameData;
		cameraService.GetImageBuffer(std::string(cameraService.GetAllCameraSerials()[0]), frameData);

		// ע��ص������������
		cameraService.RegisterCallback();

		cameraService.SetTriggerMode(0);

		cameraService.StartGrabbing();

		while (1) {
			ImageQueue& imageQueue = cameraService.GetImageQueue(std::string(cameraService.GetAllCameraSerials()[0]));
		}
		//// ע��ص���ָ�������
		//cameraService.RegisterCallback("67890");

		//// ֹͣץͼ��ָ�������
		//cameraService.StopGrabbing("12345");

		//// ֹͣץͼ�����������
		//cameraService.StopGrabbing();

		//// �ر��������
		//cameraService.CloseAll();

	}
	catch (const std::exception& e) {
		std::cerr << "�쳣: " << e.what() << std::endl;
	}

	return 0;
}
