#include "V4L2Plugin.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <sstream>

namespace AsTestTool {
namespace Plugins {

V4L2Camera::V4L2Camera()
    : m_deviceFd(-1)
    , m_initialized(false)
    , m_cameraOpen(false)
    , m_previewActive(false)
    , m_lastErrorCode(0)
    , m_currentWidth(640)
    , m_currentHeight(480)
    , m_currentFps(30)
    , m_currentFormat(PixelFormat::YUV420)
    , m_bufferCount(0)
    , m_captureRunning(false)
    , m_camerasEnumerated(false) {
}

V4L2Camera::~V4L2Camera() {
    Shutdown();
}

std::string V4L2Camera::GetPluginName() const {
    return "V4L2 Camera Plugin";
}

std::string V4L2Camera::GetPluginVersion() const {
    return "1.0.0";
}

std::string V4L2Camera::GetPlatform() const {
    return "Linux";
}

bool V4L2Camera::Initialize() {
    if (m_initialized) {
        return true;
    }
    
    try {
        // 初始化V4L2
        if (!InitializeV4L2()) {
            return false;
        }
        
        // 枚举可用摄像头
        if (!EnumerateCameras()) {
            SetLastError("Failed to enumerate cameras", -1);
            return false;
        }
        
        m_initialized = true;
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception during initialization: " + std::string(e.what()), -2);
        return false;
    }
}

void V4L2Camera::Shutdown() {
    try {
        // 停止预览
        if (m_previewActive) {
            StopPreview();
        }
        
        // 关闭摄像头
        if (m_cameraOpen) {
            CloseCamera();
        }
        
        // 清理V4L2
        CleanupV4L2();
        
        m_initialized = false;
        
    } catch (const std::exception& e) {
        // 记录错误但不抛出异常
        std::cerr << "Exception during shutdown: " << e.what() << std::endl;
    }
}

std::vector<CameraInfo> V4L2Camera::GetAvailableCameras() {
    if (!m_initialized) {
        SetLastError("Plugin not initialized", -3);
        return {};
    }
    
    if (!m_camerasEnumerated) {
        if (!EnumerateCameras()) {
            SetLastError("Failed to enumerate cameras", -4);
            return {};
        }
    }
    
    return m_availableCameras;
}

bool V4L2Camera::OpenCamera(const std::string& cameraId) {
    if (!m_initialized) {
        SetLastError("Plugin not initialized", -5);
        return false;
    }
    
    if (m_cameraOpen) {
        CloseCamera();
    }
    
    try {
        // 查找指定的摄像头
        CameraInfo* targetCamera = nullptr;
        for (auto& camera : m_availableCameras) {
            if (camera.id == cameraId) {
                targetCamera = &camera;
                break;
            }
        }
        
        if (!targetCamera) {
            SetLastError("Camera not found: " + cameraId, -6);
            return false;
        }
        
        // 打开设备
        if (!OpenDevice(targetCamera->devicePath)) {
            return false;
        }
        
        // 设置设备
        if (!SetupDevice()) {
            CloseDevice();
            return false;
        }
        
        // 分配缓冲区
        if (!AllocateBuffers()) {
            CloseDevice();
            return false;
        }
        
        m_currentCameraId = cameraId;
        m_cameraOpen = true;
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while opening camera: " + std::string(e.what()), -7);
        return false;
    }
}

bool V4L2Camera::CloseCamera() {
    if (!m_cameraOpen) {
        return true;
    }
    
    try {
        // 停止流
        if (m_previewActive) {
            StopStreaming();
        }
        
        // 释放缓冲区
        FreeBuffers();
        
        // 关闭设备
        CloseDevice();
        
        m_cameraOpen = false;
        m_currentCameraId.clear();
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while closing camera: " + std::string(e.what()), -8);
        return false;
    }
}

bool V4L2Camera::IsCameraOpen() const {
    return m_cameraOpen;
}

bool V4L2Camera::StartPreview() {
    if (!m_cameraOpen) {
        SetLastError("Camera not open", -9);
        return false;
    }
    
    if (m_previewActive) {
        return true;
    }
    
    try {
        if (!StartStreaming()) {
            return false;
        }
        
        m_previewActive = true;
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while starting preview: " + std::string(e.what()), -10);
        return false;
    }
}

bool V4L2Camera::StopPreview() {
    if (!m_previewActive) {
        return true;
    }
    
    try {
        if (!StopStreaming()) {
            return false;
        }
        
        m_previewActive = false;
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while stopping preview: " + std::string(e.what()), -11);
        return false;
    }
}

bool V4L2Camera::IsPreviewActive() const {
    return m_previewActive;
}

bool V4L2Camera::CaptureImage(std::vector<uint8_t>& imageData, 
                             PixelFormat& format, 
                             int& width, int& height) {
    if (!m_cameraOpen || !m_previewActive) {
        SetLastError("Camera not ready for capture", -12);
        return false;
    }
    
    try {
        // 设置返回参数
        format = m_currentFormat;
        width = m_currentWidth;
        height = m_currentHeight;
        
        // 分配图像数据缓冲区
        size_t dataSize = width * height * 3; // 假设RGB24格式
        imageData.resize(dataSize);
        
        // 这里应该从V4L2获取实际的图像数据
        // 为了演示，填充一些测试数据
        std::fill(imageData.begin(), imageData.end(), 128);
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while capturing image: " + std::string(e.what()), -13);
        return false;
    }
}

bool V4L2Camera::SetResolution(int width, int height, int fps) {
    if (!m_cameraOpen) {
        m_currentWidth = width;
        m_currentHeight = height;
        m_currentFps = fps;
        return true;
    }
    
    try {
        // 停止当前流
        if (m_previewActive) {
            StopStreaming();
        }
        
        // 释放当前缓冲区
        FreeBuffers();
        
        // 设置新的分辨率
        m_currentWidth = width;
        m_currentHeight = height;
        m_currentFps = fps;
        
        // 重新设置设备
        if (!SetupDevice()) {
            return false;
        }
        
        // 重新分配缓冲区
        if (!AllocateBuffers()) {
            return false;
        }
        
        // 如果之前有预览，重新开始
        if (m_previewActive) {
            if (!StartStreaming()) {
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting resolution: " + std::string(e.what()), -14);
        return false;
    }
}

bool V4L2Camera::SetPixelFormat(PixelFormat format) {
    if (!m_cameraOpen) {
        m_currentFormat = format;
        return true;
    }
    
    try {
        // 停止当前流
        if (m_previewActive) {
            StopStreaming();
        }
        
        // 释放当前缓冲区
        FreeBuffers();
        
        // 设置新的像素格式
        m_currentFormat = format;
        
        // 重新设置设备
        if (!SetupDevice()) {
            return false;
        }
        
        // 重新分配缓冲区
        if (!AllocateBuffers()) {
            return false;
        }
        
        // 如果之前有预览，重新开始
        if (m_previewActive) {
            if (!StartStreaming()) {
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting pixel format: " + std::string(e.what()), -15);
        return false;
    }
}

bool V4L2Camera::SetBrightness(int value) {
    if (!m_cameraOpen || m_deviceFd < 0) {
        SetLastError("Camera not ready for brightness control", -16);
        return false;
    }
    
    try {
        struct v4l2_control ctrl;
        ctrl.id = V4L2_CID_BRIGHTNESS;
        ctrl.value = value;
        
        if (ioctl(m_deviceFd, VIDIOC_S_CTRL, &ctrl) < 0) {
            SetLastError("Failed to set brightness: " + GetV4L2Error(errno), -17);
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting brightness: " + std::string(e.what()), -18);
        return false;
    }
}

bool V4L2Camera::SetContrast(int value) {
    if (!m_cameraOpen || m_deviceFd < 0) {
        SetLastError("Camera not ready for contrast control", -19);
        return false;
    }
    
    try {
        struct v4l2_control ctrl;
        ctrl.id = V4L2_CID_CONTRAST;
        ctrl.value = value;
        
        if (ioctl(m_deviceFd, VIDIOC_S_CTRL, &ctrl) < 0) {
            SetLastError("Failed to set contrast: " + GetV4L2Error(errno), -20);
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting contrast: " + std::string(e.what()), -21);
        return false;
    }
}

bool V4L2Camera::SetSaturation(int value) {
    if (!m_cameraOpen || m_deviceFd < 0) {
        SetLastError("Camera not ready for saturation control", -22);
        return false;
    }
    
    try {
        struct v4l2_control ctrl;
        ctrl.id = V4L2_CID_SATURATION;
        ctrl.value = value;
        
        if (ioctl(m_deviceFd, VIDIOC_S_CTRL, &ctrl) < 0) {
            SetLastError("Failed to set saturation: " + GetV4L2Error(errno), -23);
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting saturation: " + std::string(e.what()), -24);
        return false;
    }
}

std::string V4L2Camera::GetLastError() const {
    return m_lastError;
}

int V4L2Camera::GetLastErrorCode() const {
    return m_lastErrorCode;
}

bool V4L2Camera::InitializeV4L2() {
    // V4L2初始化通常不需要特殊操作
    return true;
}

void V4L2Camera::CleanupV4L2() {
    // V4L2清理通常不需要特殊操作
}

bool V4L2Camera::EnumerateCameras() {
    try {
        m_availableCameras.clear();
        
        // 扫描/dev/video*设备
        for (int i = 0; i < 10; ++i) {
            std::string devicePath = "/dev/video" + std::to_string(i);
            
            // 尝试打开设备
            int fd = open(devicePath.c_str(), O_RDWR);
            if (fd >= 0) {
                // 检查设备能力
                if (CheckDeviceCapabilities(fd)) {
                    CameraInfo camera;
                    camera.id = "camera_" + std::to_string(i);
                    camera.name = "V4L2 Camera " + std::to_string(i);
                    camera.devicePath = devicePath;
                    camera.isAvailable = true;
                    
                    // 获取支持的像素格式
                    camera.supportedFormats = GetSupportedPixelFormats(fd);
                    
                    // 获取支持的分辨率（使用第一个支持的格式）
                    if (!camera.supportedFormats.empty()) {
                        camera.supportedResolutions = GetSupportedResolutions(fd, 
                            PixelFormatToV4L2(camera.supportedFormats[0]));
                    }
                    
                    // 如果没有获取到分辨率，添加默认分辨率
                    if (camera.supportedResolutions.empty()) {
                        camera.supportedResolutions = {
                            Resolution(640, 480, 30, "VGA"),
                            Resolution(1280, 720, 30, "HD"),
                            Resolution(1920, 1080, 30, "FHD")
                        };
                    }
                    
                    m_availableCameras.push_back(camera);
                }
                
                close(fd);
            }
        }
        
        m_camerasEnumerated = true;
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while enumerating cameras: " + std::string(e.what()), -25);
        return false;
    }
}

bool V4L2Camera::OpenDevice(const std::string& devicePath) {
    try {
        m_deviceFd = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);
        if (m_deviceFd < 0) {
            SetLastError("Failed to open device: " + GetV4L2Error(errno), -26);
            return false;
        }
        
        m_devicePath = devicePath;
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while opening device: " + std::string(e.what()), -27);
        return false;
    }
}

bool V4L2Camera::CloseDevice() {
    if (m_deviceFd >= 0) {
        close(m_deviceFd);
        m_deviceFd = -1;
    }
    m_devicePath.clear();
    return true;
}

bool V4L2Camera::SetupDevice() {
    try {
        // 设置视频格式
        struct v4l2_format fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = m_currentWidth;
        fmt.fmt.pix.height = m_currentHeight;
        fmt.fmt.pix.pixelformat = PixelFormatToV4L2(m_currentFormat);
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        
        if (ioctl(m_deviceFd, VIDIOC_S_FMT, &fmt) < 0) {
            SetLastError("Failed to set video format: " + GetV4L2Error(errno), -28);
            return false;
        }
        
        // 设置帧率
        struct v4l2_streamparm parm;
        memset(&parm, 0, sizeof(parm));
        parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        parm.parm.capture.timeperframe.numerator = 1;
        parm.parm.capture.timeperframe.denominator = m_currentFps;
        
        if (ioctl(m_deviceFd, VIDIOC_S_PARM, &parm) < 0) {
            SetLastError("Failed to set frame rate: " + GetV4L2Error(errno), -29);
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while setting up device: " + std::string(e.what()), -30);
        return false;
    }
}

bool V4L2Camera::AllocateBuffers() {
    try {
        // 请求缓冲区
        struct v4l2_requestbuffers req;
        memset(&req, 0, sizeof(req));
        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        
        if (ioctl(m_deviceFd, VIDIOC_REQBUFS, &req) < 0) {
            SetLastError("Failed to request buffers: " + GetV4L2Error(errno), -31);
            return false;
        }
        
        m_bufferCount = req.count;
        m_buffers.resize(m_bufferCount);
        
        // 映射缓冲区
        for (int i = 0; i < m_bufferCount; ++i) {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            
            if (ioctl(m_deviceFd, VIDIOC_QUERYBUF, &buf) < 0) {
                SetLastError("Failed to query buffer: " + GetV4L2Error(errno), -32);
                return false;
            }
            
            m_buffers[i].length = buf.length;
            m_buffers[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE,
                                     MAP_SHARED, m_deviceFd, buf.m.offset);
            
            if (m_buffers[i].start == MAP_FAILED) {
                SetLastError("Failed to map buffer: " + GetV4L2Error(errno), -33);
                return false;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while allocating buffers: " + std::string(e.what()), -34);
        return false;
    }
}

void V4L2Camera::FreeBuffers() {
    for (auto& buffer : m_buffers) {
        if (buffer.start != nullptr && buffer.start != MAP_FAILED) {
            munmap(buffer.start, buffer.length);
            buffer.start = nullptr;
        }
    }
    m_buffers.clear();
    m_bufferCount = 0;
}

bool V4L2Camera::StartStreaming() {
    try {
        // 将缓冲区加入队列
        for (int i = 0; i < m_bufferCount; ++i) {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            
            if (ioctl(m_deviceFd, VIDIOC_QBUF, &buf) < 0) {
                SetLastError("Failed to queue buffer: " + GetV4L2Error(errno), -35);
                return false;
            }
        }
        
        // 开始流
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(m_deviceFd, VIDIOC_STREAMON, &type) < 0) {
            SetLastError("Failed to start streaming: " + GetV4L2Error(errno), -36);
            return false;
        }
        
        // 启动捕获线程
        m_captureRunning = true;
        m_captureThread = std::thread(&V4L2Camera::CaptureLoop, this);
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while starting streaming: " + std::string(e.what()), -37);
        return false;
    }
}

bool V4L2Camera::StopStreaming() {
    try {
        // 停止捕获线程
        m_captureRunning = false;
        if (m_captureThread.joinable()) {
            m_captureThread.join();
        }
        
        // 停止流
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(m_deviceFd, VIDIOC_STREAMOFF, &type) < 0) {
            SetLastError("Failed to stop streaming: " + GetV4L2Error(errno), -38);
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        SetLastError("Exception while stopping streaming: " + std::string(e.what()), -39);
        return false;
    }
}

void V4L2Camera::CaptureLoop() {
    while (m_captureRunning) {
        try {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            
            if (ioctl(m_deviceFd, VIDIOC_DQBUF, &buf) < 0) {
                if (errno != EAGAIN) {
                    break; // 错误，退出循环
                }
                continue; // 暂时没有数据，继续等待
            }
            
            // 处理图像数据
            // 这里可以添加图像处理逻辑
            
            // 将缓冲区重新加入队列
            if (ioctl(m_deviceFd, VIDIOC_QBUF, &buf) < 0) {
                break; // 错误，退出循环
            }
            
        } catch (const std::exception& e) {
            // 记录错误但继续运行
            std::cerr << "Exception in capture loop: " << e.what() << std::endl;
        }
    }
}

void V4L2Camera::SetLastError(const std::string& error, int code) {
    m_lastError = error;
    m_lastErrorCode = code;
}

std::string V4L2Camera::GetV4L2Error(int errno) const {
    return std::string(strerror(errno));
}

uint32_t V4L2Camera::PixelFormatToV4L2(PixelFormat format) const {
    switch (format) {
        case PixelFormat::YUV420: return V4L2_PIX_FMT_YUV420;
        case PixelFormat::YUV422: return V4L2_PIX_FMT_YUYV;
        case PixelFormat::RGB24: return V4L2_PIX_FMT_RGB24;
        case PixelFormat::RGB32: return V4L2_PIX_FMT_RGB32;
        case PixelFormat::MJPG: return V4L2_PIX_FMT_MJPEG;
        default: return V4L2_PIX_FMT_RGB24;
    }
}

PixelFormat V4L2Camera::V4L2ToPixelFormat(uint32_t v4l2Format) const {
    switch (v4l2Format) {
        case V4L2_PIX_FMT_YUV420: return PixelFormat::YUV420;
        case V4L2_PIX_FMT_YUYV: return PixelFormat::YUV422;
        case V4L2_PIX_FMT_RGB24: return PixelFormat::RGB24;
        case V4L2_PIX_FMT_RGB32: return PixelFormat::RGB32;
        case V4L2_PIX_FMT_MJPEG: return PixelFormat::MJPG;
        default: return PixelFormat::Unknown;
    }
}

std::string V4L2Camera::V4L2FormatToString(uint32_t format) const {
    char formatStr[5];
    formatStr[0] = (format >> 0) & 0xFF;
    formatStr[1] = (format >> 8) & 0xFF;
    formatStr[2] = (format >> 16) & 0xFF;
    formatStr[3] = (format >> 24) & 0xFF;
    formatStr[4] = '\0';
    return std::string(formatStr);
}

bool V4L2Camera::CheckDeviceCapabilities(int fd) {
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        return false;
    }
    
    // 检查是否是视频捕获设备
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        return false;
    }
    
    // 检查是否支持流式I/O
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        return false;
    }
    
    return true;
}

std::vector<Resolution> V4L2Camera::GetSupportedResolutions(int fd, uint32_t pixelFormat) {
    std::vector<Resolution> resolutions;
    
    try {
        struct v4l2_frmsizeenum fsize;
        memset(&fsize, 0, sizeof(fsize));
        fsize.pixel_format = pixelFormat;
        
        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fsize) == 0) {
            if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                Resolution res(fsize.discrete.width, fsize.discrete.height, 30,
                              std::to_string(fsize.discrete.width) + "x" + std::to_string(fsize.discrete.height));
                resolutions.push_back(res);
            }
            fsize.index++;
        }
        
    } catch (const std::exception& e) {
        // 如果枚举失败，返回默认分辨率
        resolutions = {
            Resolution(640, 480, 30, "VGA"),
            Resolution(1280, 720, 30, "HD"),
            Resolution(1920, 1080, 30, "FHD")
        };
    }
    
    return resolutions;
}

std::vector<PixelFormat> V4L2Camera::GetSupportedPixelFormats(int fd) {
    std::vector<PixelFormat> formats;
    
    try {
        struct v4l2_fmtdesc fmt;
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
        while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
            PixelFormat format = V4L2ToPixelFormat(fmt.pixelformat);
            if (format != PixelFormat::Unknown) {
                formats.push_back(format);
            }
            fmt.index++;
        }
        
    } catch (const std::exception& e) {
        // 如果枚举失败，返回默认格式
        formats = { PixelFormat::YUV420, PixelFormat::RGB24, PixelFormat::MJPG };
    }
    
    return formats;
}

// V4L2Factory 实现
V4L2Factory::V4L2Factory() {
}

V4L2Factory::~V4L2Factory() {
}

std::unique_ptr<ICameraPlugin> V4L2Factory::CreatePlugin() {
    return std::make_unique<V4L2Camera>();
}

std::string V4L2Factory::GetPluginType() const {
    return "V4L2";
}

} // namespace Plugins
} // namespace AsTestTool

// 导出函数实现
extern "C" {
    AsTestTool::Plugins::IPluginFactory* CreatePluginFactory() {
        return new AsTestTool::Plugins::V4L2Factory();
    }
    
    void DestroyPluginFactory(AsTestTool::Plugins::IPluginFactory* factory) {
        delete factory;
    }
}
