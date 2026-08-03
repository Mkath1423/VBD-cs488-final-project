#pragma once

#include <vector>

#include "linalg.h"
using namespace linalg::aliases;

#include "engine.h"

constexpr float PI = 3.14159265358979f;
constexpr float DegToRad = PI / 180.0f;
constexpr float RadToDeg = 180.0f / PI;



class Camera : public ManagedResource{
    private:
    
    float4x4 view, proj;

    float4x4 lookAtMatrix() {

        float3 forward = normalize(poi - origin);
        float3 right = normalize(cross(forward, up));
        float3 localUp = normalize(cross(right, forward));
        
        float4x4 out;

        out[0] = { right.x, right.y, right.z, -dot(origin, right) };
        out[1] = { localUp.x, localUp.y, localUp.z, -dot(origin, localUp) };
        out[2] = { -forward.x, -forward.y, -forward.z, dot(origin, forward)};
        out[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

        return transpose(out);
    }

    float4x4 perspectiveMatrix() {
        float4x4 m;
        const float f = 1.0f / (tan(fovy * DegToRad / 2.0f));
        m[0] = { f / aspect, 0.0f, 0.0f, 0.0f };
        m[1] = { 0.0f, f, 0.0f, 0.0f };
        m[2] = { 0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f };
        m[3] = { 0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f };

        return m;
    }

    

    public:

    float fovy, aspect, zNear, zFar;

    float3 origin = { 0.0, 0.0, -10 };
    float3 poi = { 0.0, 0.0, 0.0 };
    float3 up = { 0.0, 1.0, 0.0 };

    // build the matrices once at the start of frame
    // any changes to the parameters will only effect next frame
    void buildMatrices(){
        view = lookAtMatrix();
        proj = perspectiveMatrix();
    }

    const float4x4 &viewMat() {
        return view;
    }
    
    const float4x4 &projMat() {
        return proj;
    }

    // lifetime management is a no-op
    // resource system is used for sharing only
    bool init_impl() { return true; }
    void cleanup_impl() {}    

};

class Renderer {
    
}

class RenderComponent {
    virtual draw() = 0;
};

class ParticleSystemRenderComponent {
    Resource<Shader> 

}

class ParticleSystemNode {
    // physics stuff
    Resource<ParticleSystem> particles;

    Resource<ParticleDrawer> drawer;
    Resource<ParticleCollider> collider;
}