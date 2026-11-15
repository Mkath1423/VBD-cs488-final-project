#pragma once

#include <memory>

template <typename Derived>
class SharableResource {


    protected:
    
    bool initialized;
    SharableResource() : initialized{ false } {}

    public:

    template<typename... Args>
    static std::shared_ptr<Derived> create(Args &&... args) {
        auto resource = std::make_shared<Derived>(
            std::forward<Args>(args)...
        );

        resource->initialized = resource->init();
        if (resource->initialized) {
            return resource;
        }
        else {
            return nullptr;
        }
    }
};
