#pragma once
#include <iostream>
#include <stdexcept> 
#include <string>

// 相机异常类，继承自std::exception
//class CameraHikException : public std::exception
//{
//public:
//    // 构造函数，接受一个错误消息字符串
//    CameraHikException(const std::string& message) : message_(message) {}
//
//    // 重载what()方法，返回异常的错误消息
//    virtual const char* what() const noexcept override {
//        return message_.c_str();
//    }
//
//private:
//    // 存储错误消息的字符串
//    std::string message_;
//};
//
//// 一个可能抛出CameraException的函数示例
//void cameraOperation() {
//    // 假设这里有一些相机操作，如果出现问题则抛出异常
//    throw CameraHikException("Hik Camera operation failed!");
//}


class CameraHikException : public std::exception
{
public:
    // 构造函数声明
    explicit CameraHikException(const std::string& message);

    // 重载what()方法声明
    const char* what() const noexcept override;

private:
    std::string message_;
};

