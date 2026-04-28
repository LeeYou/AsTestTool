#pragma once

#include <memory>
#include <mutex>

namespace AsTestTool {

/**
 * @brief 单例模式模板类
 * 
 * 使用模板实现线程安全的单例模式，支持：
 * - 线程安全初始化（双重检查锁定）
 * - 延迟初始化
 * - 禁止拷贝和赋值
 * 
 * 使用方式：
 *   class MyClass : public Singleton<MyClass> {
 *       friend class Singleton<MyClass>;
 *   private:
 *       MyClass() = default;
 *   };
 */
template<typename T>
class Singleton {
public:
    /**
     * @brief 获取单例实例
     * @return T& 单例实例引用
     */
    static T& Instance() {
        static T instance;  // 延迟初始化，C++11 保证线程安全
        return instance;
    }

    // 禁止拷贝
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // 允许移动（C++17+）
    Singleton(Singleton&&) = default;
    Singleton& operator=(Singleton&&) = default;

protected:
    Singleton() = default;
    virtual ~Singleton() = default;
};

/**
 * @brief 带初始化参数的单例模板
 * 
 * 适用于需要构造函数参数的单例类
 */
template<typename T>
class SingletonWithInit {
public:
    template<typename... Args>
    static T& Instance(Args&&... args) {
        std::call_once(InitFlag, [&]() {
            InstanceHolder() = std::make_unique<T>(std::forward<Args>(args)...);
        });
        return *InstanceHolder();
    }

    static bool IsInitialized() {
        return InstanceHolder() != nullptr;
    }

protected:
    struct InstanceTag {};
    
    SingletonWithInit() = default;
    virtual ~SingletonWithInit() = default;

private:
    static std::unique_ptr<T>& InstanceHolder() {
        static std::unique_ptr<T> instance = nullptr;
        return instance;
    }

    static std::once_flag InitFlag;
};

template<typename T>
std::once_flag SingletonWithInit<T>::InitFlag;

} // namespace AsTestTool
