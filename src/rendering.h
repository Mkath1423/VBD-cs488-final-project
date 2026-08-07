#pragma once

#include <vector>
#include <map>
#include "linalg.h"
using namespace linalg::aliases;

#include "resources/shader.h"

constexpr float PI = 3.14159265358979f;
constexpr float DegToRad = PI / 180.0f;
constexpr float RadToDeg = 180.0f / PI;

class Camera {
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
};

class Renderer;

class RenderProxy {
public:
    virtual void sync() = 0;
    virtual void draw(Renderer& renderer) = 0;
};

struct Renderer {
  
    std::shared_ptr<Shader> currentShader;
    Camera camera;

    void updateCamera() {
        if (currentShader) setShaderUniforms();
    }

    void setShaderUniforms() {
        const GLint viewLoc = currentShader->getUniformLocation("view");
        glUniformMatrix4fv(
            viewLoc,
            1,
            GL_FALSE,
            &camera.viewMat()[0][0]
        );

        const GLint projLoc = currentShader->getUniformLocation("projection");
        glUniformMatrix4fv(
            projLoc,
            1,
            GL_FALSE,
            &camera.projMat()[0][0]
        );
    }

    void setShader(std::shared_ptr<Shader> newShader) {
        if(newShader == currentShader) return;
        currentShader = std::move(newShader);

        if (!currentShader) return;
        
        currentShader->useShader();

        setShaderUniforms();
    }

    void setTransform(const float4x4 &modelMat) {
        const GLint modelLoc = currentShader->getUniformLocation("model");
        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            &modelMat[0][0]
        );
    }

    void setColor(const float3& color) {
        const GLint loc = currentShader->getUniformLocation("baseColor");
        if (loc != -1) {
            glUniform3f(
                loc,
                color.x, color.y, color.z
            );
        }
    }

    Renderer(const Camera &camera) : camera{camera} { }

    

    void startFrame(){
         camera.buildMatrices();

         glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};
