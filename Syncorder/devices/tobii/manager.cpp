// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/manager_base.h>
#include <Syncorder/devices/tobii/device.cpp>
#include <Syncorder/devices/tobii/callback.cpp>
#include <Syncorder/devices/tobii/buffer.cpp>
#include <Syncorder/devices/tobii/broker.cpp>


/**
 * @class Manager
 */

class TobiiManager : public BManager {
private:
    int device_id_;

    std::unique_ptr<TobiiDevice> device_;
    std::unique_ptr<TobiiCallback> callback_;
    std::unique_ptr<TobiiBuffer> buffer_;
    std::unique_ptr<TobiiBroker> broker_;

public:
    explicit TobiiManager(int device_id)
    : 
        device_id_(device_id) {
            device_ = std::make_unique<TobiiDevice>(device_id);
            callback_ = std::make_unique<TobiiCallback>();
            buffer_ = std::make_unique<TobiiBuffer>();
            broker_ = std::make_unique<TobiiBroker>();
        }

public:
    bool setup() override {
        // device
        device_->pre_setup(callback_.get(), reinterpret_cast<void*>(&TobiiCallback::onGaze));
        device_->setup();

        // callback
        callback_->setup(static_cast<void*>(buffer_.get()));

        // broker
        broker_->setup(buffer_.get(), reinterpret_cast<void*>(&TobiiBuffer::dequeue));

        // flag
        is_setup_.store(true);

        return true;
    }
    
    bool warmup() override {
        device_->warmup();
        callback_->warmup();

        // flag
        is_warmup_.store(true);

        return true;
    }
    
    bool start() override {
        broker_->start();
        buffer_->start();

        // flag
        is_running_.store(true);

        return true;
    }

    bool stop() override {
        broker_->stop();
        buffer_->stop();

        return true;
    }
    bool cleanup() override {
        // broker_->cleanup();
        device_->cleanup();

        return true;
    }

    std::string __name__() const override {
        return "Tobii";
    }
};