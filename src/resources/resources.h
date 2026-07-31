#pragma once
#include <iostream>

#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>


#include <map>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class ResourceManager;



class ManagedResource {
    // std::vector<Resource> dependents;
    
    bool isInitialized;

    protected:

    virtual bool init_impl() = 0;
    virtual void cleanup_impl() = 0;
    
    // void registerDependency(uint64_t id);

    public:

    ManagedResource() : isInitialized{false} {}

    bool init();
    void destroy();

    virtual ~ManagedResource();
};

class ResourceManager {
    std::map<uint64_t, std::unique_ptr<ManagedResource>> resources;

    uint64_t next_id = 1;

    public:

    uint64_t add(std::unique_ptr<ManagedResource> res);

    ManagedResource *get(uint64_t id);

    ~ResourceManager();
};
