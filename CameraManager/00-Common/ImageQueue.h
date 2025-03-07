#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "00-Common/xCameraParam.h"

class ImageQueue
{
private:
	std::queue<std::shared_ptr<xFrameData>> m_imageQueue;
	std::mutex m_mutex;
	std::condition_variable m_condVar;
	size_t m_maxSize = 12; 

public:
	void PushImage(std::shared_ptr<xFrameData> frameData);
	std::shared_ptr<xFrameData> PopImage();
	bool IsEmpty();
	void SetMaxSize(size_t maxSize);
};