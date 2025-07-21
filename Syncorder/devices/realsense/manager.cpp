// local
#include <Syncorder/error/exception.h>
#include <Syncorder/devices/common/manager_base.h>
#include <Syncorder/devices/realsense/device.cpp>
#include <Syncorder/devices/realsense/callback.cpp>
#include <Syncorder/devices/realsense/buffer.cpp>
#include <Syncorder/devices/realsense/broker.cpp>


/**
 * @class Manager
 */

class RealsenseManager : public BManager {
private:
    int device_id_;

    std::unique_ptr<RealsenseDevice> device_;
    std::unique_ptr<RealsenseCallback> callback_;
    std::unique_ptr<RealsenseBuffer> buffer_;
    std::unique_ptr<RealsenseBroker> broker_;

public:
    explicit RealsenseManager(int device_id)
    : 
        device_id_(device_id) {
            device_ = std::make_unique<RealsenseDevice>(device_id);
            callback_ = std::make_unique<RealsenseCallback>();
            buffer_ = std::make_unique<RealsenseBuffer>();
            broker_ = std::make_unique<RealsenseBroker>();
        }

public:
    bool setup() override {
        // device
        device_->pre_setup(reinterpret_cast<void*>(&RealsenseCallback::onFrameset));
        device_->setup();

        // callback
        callback_->setup(static_cast<void*>(buffer_.get()));

        // broker
        broker_->setup(buffer_.get(), reinterpret_cast<void*>(&RealsenseBuffer::dequeue));

        // flag
        is_setup_.store(true);

        // return
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
        std::cout << "[RealsenseManager] Stopping broker and buffer...\n";
        broker_->stop();   // Broker thread 정지
        buffer_->stop();   // Buffer gate 닫기

        return true;
    }

    bool cleanup() override {
        broker_->cleanup();
        device_->cleanup();

        return true;
    }

    std::string __name__() const override {
        return "Realsense";
    }
};