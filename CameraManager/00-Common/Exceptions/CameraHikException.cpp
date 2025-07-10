#include "stdafx.h"
#include "CameraHikException.h"

// 构造函数实现
CameraHikException::CameraHikException(const std::string& message)
    : message_(message)
{}

// what()方法实现
const char* CameraHikException::what() const noexcept
{
    return message_.c_str();
}
