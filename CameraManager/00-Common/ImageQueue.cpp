#include "stdafx.h"
#include "ImageQueue.h"

void ImageQueue::PushImage(std::shared_ptr<xImage> frameData)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_imageQueue.size() >= m_maxSize)
	{
		m_imageQueue.pop();
	}

	m_imageQueue.push(frameData);
	m_condVar.notify_one();
}

std::shared_ptr<xImage> ImageQueue::PopImage()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_condVar.wait(lock, [this] { return !m_imageQueue.empty(); });

	auto frameData = m_imageQueue.front();
	m_imageQueue.pop();
	return frameData;
}

bool ImageQueue::IsEmpty()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_imageQueue.empty();
}

void ImageQueue::SetMaxSize(size_t maxSize)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_maxSize = maxSize;
}

void ImageQueue::Clear()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	while (!m_imageQueue.empty())
	{
		m_imageQueue.pop();
	}
	m_condVar.notify_all();  
}