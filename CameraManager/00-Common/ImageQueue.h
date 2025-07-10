#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "00-Common/xCameraParam.h"

class ImageQueue
{
private:
	std::queue<std::shared_ptr<xImage>> m_imageQueue;
	std::mutex m_mutex;
	std::condition_variable m_condVar;
	size_t m_maxSize = 1;

public:
	void PushImage(std::shared_ptr<xImage> frameData);
	std::shared_ptr<xImage> PopImage();
	bool IsEmpty();
	void SetMaxSize(size_t maxSize);
	void Clear();
};