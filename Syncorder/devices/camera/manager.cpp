// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/manager_base.h>
#include <Syncorder/devices/camera/device.cpp>
#include <Syncorder/devices/camera/callback.cpp>
#include <Syncorder/devices/camera/buffer.cpp>
#include <Syncorder/devices/camera/broker.cpp>


/**
 * @class Manager
 */

class CameraManager : public BManager {
private:
    int device_id_;
    
    std::unique_ptr<CameraDevice> device_;
    Microsoft::WRL::ComPtr<CameraCallback> callback_;
    std::unique_ptr<CameraBuffer> buffer_;
    std::unique_ptr<CameraBroker> broker_;

public:
    explicit CameraManager(int device_id)
    : 
        device_id_(device_id) {
            device_ = std::make_unique<CameraDevice>(device_id);
            callback_ = Microsoft::WRL::Make<CameraCallback>();
            buffer_ = std::make_unique<CameraBuffer>();
            broker_ = std::make_unique<CameraBroker>();
        }

public:
    void setup() override {
        // device
        device_->pre_setup(callback_->getIUnknown());
        device_->setup();
        
        // callback
        callback_->setup(device_->getReader(), static_cast<void*>(buffer_.get()));

        // broker
        broker_->setup(buffer_.get(), reinterpret_cast<void*>(&CameraBuffer::dequeue));
    }
    
    void warmup() override {
        device_->warmup();
        callback_->warmup();
    }
    
    void run() override {
        broker_->run();
        buffer_->run();
    }

    std::string __name__() const override {
        return "Camera";
    }
};