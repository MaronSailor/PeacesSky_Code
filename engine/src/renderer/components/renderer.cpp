#include "renderer.hpp"
#include <iostream>
#include "vertex.hpp"

#include "../helper/helperFunctions.hpp"
#include "texture.hpp"
#include "gtc/quaternion.hpp"


#include "gtc/type_ptr.hpp"

#include "shadowMap.hpp"

namespace CustomEngine
{
    Renderer::Renderer() : m_ImageBuffer(1280, 720)
    {
        m_WindowAspect = { 1280, 720 };
    }

    Renderer::~Renderer()
    {
        m_ImageBuffer.destroy();
    }

    void Renderer::draw(const VertexArray& vertArr, const IndexBuffer& indexBuff, const Shader& shader) const
    {
        shader.bind();
        vertArr.bind();
        indexBuff.bind();
        checkOGL(glDrawElements(GL_TRIANGLES, indexBuff.getCount(), GL_UNSIGNED_INT, nullptr));
    }

    void Renderer::setWindowAspect(unsigned int width, unsigned int height)
    {
        if (width == 0 || height == 0) return;
        if (m_WindowAspect.x == static_cast<float>(width) && m_WindowAspect.y == static_cast<float>(height)) return;
        m_WindowAspect.x = static_cast<float>(width);
        m_WindowAspect.y = static_cast<float>(height);

        m_ImageBuffer.resize(width, height);
    }

    void Renderer::renderSceneObjs(EngineSceneInfo* sceneInfo)
    {
        checkOGL(glViewport(0, 0, m_WindowAspect.x, m_WindowAspect.y));

        Camera& camera = *(*sceneInfo).pActiveCamera;

        float aspect = 1;
        if (m_WindowAspect.x > 0 && m_WindowAspect.y > 0) aspect = static_cast<float>(m_WindowAspect.x) / static_cast<float>(m_WindowAspect.y);
        glm::mat4 proj = glm::perspective(glm::radians(camera.fov), aspect, camera.nearP, camera.farP);

        glm::quat orientationCam = { camera.rotation.w, camera.rotation.x, camera.rotation.y, camera.rotation.z };
        glm::vec3 posCam = { camera.location.x, camera.location.y, camera.location.z };
        glm::mat4 view = glm::mat4_cast(glm::conjugate(orientationCam)) * glm::translate(glm::mat4(1.0f), -posCam);


        std::vector<Transform>& transformComponents = *sceneInfo->pTransformComponentStorage;
        std::vector<RenderComponent>& components = *sceneInfo->pRenderComponentStorage;

        m_ImageBuffer.bind();

        checkOGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));


        for (RenderComponent& component : components)
        {
            if (!component.isVisible) continue;
            Transform& objTransform = transformComponents[component.getTransformIndex()];

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(objTransform.location.x, objTransform.location.y, objTransform.location.z));

            glm::quat orientation = { objTransform.rotation.w, objTransform.rotation.x, objTransform.rotation.y, objTransform.rotation.z };
            model *= glm::mat4_cast(orientation);
            model = glm::scale(model, glm::vec3(objTransform.scale.x, objTransform.scale.y, objTransform.scale.z));

            glm::mat4 vp = proj * view;

            Shader& shader = m_ResourceHandler.getShader(0);

            unsigned int indexCount = m_ResourceHandler.bindComponent(component, shader);
            m_ResourceHandler.bindTexture(component, shader);

            shader.setUniformMat4f("u_VP", vp);
            shader.setUniformMat4f("u_Model", model);
            
            checkOGL(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr));
        }
        m_ImageBuffer.unbind();
    }

    void Renderer::renderSceneLighting(EngineSceneInfo* sceneInfo)
    {
        ShadowMap& shadowMap = m_ResourceHandler.getShadowMap();

        checkOGL(glViewport(0, 0, shadowMap.getWidth(), shadowMap.getHeight()));
        
        Shader& shadowMapShader = m_ResourceHandler.getShader(1);

        std::vector<LightSource>& lightSources = *(sceneInfo->pLightComponentStorage);

        checkOGL(glBindFramebuffer(GL_FRAMEBUFFER, m_OutputFbo));
        checkOGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        for (LightSource& light : lightSources)
        {
            Vec3& lightPosition = light.location;
            Vec3& lightRotation = light.rotation;

            glm::mat4 lightProj;

            if (light.type == Spotlight)
            {
                lightProj = glm::perspective(glm::radians(45.0f), static_cast<float>(shadowMap.getWidth()) / static_cast<float>(shadowMap.getHeight()), 0.1f, 100.0f);
            }
            else
            {
                float orthoSize = 300.0f;
                lightProj = glm::ortho(-orthoSize, orthoSize, orthoSize, -orthoSize, 0.1f, 1000.0f);
            }

            glm::mat4 lightView = glm::mat4(1.0f);
            lightView = glm::rotate(lightView, glm::radians(-lightRotation.y), glm::vec3(0, 1, 0));
            lightView = glm::rotate(lightView, glm::radians(-lightRotation.x), glm::vec3(1, 0, 0));
            lightView = glm::rotate(lightView, glm::radians(-lightRotation.z), glm::vec3(0, 0, 1));
            lightView = glm::translate(lightView, glm::vec3(-lightPosition.x, -lightPosition.y, -lightPosition.z));

            glm::mat4 lightVP =  lightProj * lightView;

            std::vector<Transform>& transformComponents = *sceneInfo->pTransformComponentStorage;
            std::vector<RenderComponent>& shadowMapComponents = *sceneInfo->pRenderComponentStorage;


            shadowMap.bind();

            checkOGL(glClear(GL_DEPTH_BUFFER_BIT));

            for (RenderComponent& shadowMapComponent : shadowMapComponents)
            {
                if (shadowMapComponent.getTransformIndex() != ~0u)
                {
                    Transform& objTransform = transformComponents[shadowMapComponent.getTransformIndex()];

                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(objTransform.location.x, objTransform.location.y, objTransform.location.z));
                    model = glm::rotate(model, glm::radians(objTransform.rotation.y), glm::vec3(0, 1, 0));
                    model = glm::rotate(model, glm::radians(objTransform.rotation.x), glm::vec3(1, 0, 0));
                    model = glm::rotate(model, glm::radians(objTransform.rotation.z), glm::vec3(0, 0, 1));
                    model = glm::scale(model, glm::vec3(objTransform.scale.x, objTransform.scale.y, objTransform.scale.z));

                    glm::mat4 shadowMapMvp = lightProj * lightView * model;

                    unsigned int indexCount = m_ResourceHandler.bindComponent(shadowMapComponent, shadowMapShader);

                    shadowMapShader.setUniformMat4f("u_MVP", shadowMapMvp);

                    checkOGL(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr));
                }
            }
            shadowMap.unbind();
            checkOGL(glBindFramebuffer(GL_FRAMEBUFFER, m_OutputFbo));
            checkOGL(glViewport(0, 0, m_WindowAspect.x, m_WindowAspect.y));

            Camera& camera = *(*sceneInfo).pActiveCamera;

            float aspect = 1;
            if (m_WindowAspect.x > 0 && m_WindowAspect.y > 0) aspect = static_cast<float>(m_WindowAspect.x) / static_cast<float>(m_WindowAspect.y);
            glm::mat4 proj = glm::perspective(glm::radians(camera.fov), aspect, camera.nearP, camera.farP);

            glm::quat orientationCam = { camera.rotation.w, camera.rotation.x, camera.rotation.y, camera.rotation.z };
            glm::vec3 posCam = { camera.location.x, camera.location.y, camera.location.z };
            glm::mat4 view = glm::mat4_cast(glm::conjugate(orientationCam)) * glm::translate(glm::mat4(1.0f), -posCam);

            glm::mat4 vp = proj * view;

            Shader& lightingShader = m_ResourceHandler.getShader(2);
            lightingShader.bind();
            uint32_t quadIndexCount = m_ResourceHandler.bindQuad();
            m_ImageBuffer.bindTextures(1,2);
            shadowMap.bindMapTexture(3);
            lightingShader.setUniform1i("u_SceneColorTexture", 1);
            lightingShader.setUniform1i("u_SceneDepthTexture", 2);
            lightingShader.setUniform1i("u_ShadowMap", 3);
            lightingShader.setUniformMat4f("u_LightVP", lightVP);
            lightingShader.setUniformMat4f("u_CameraVP", vp);
            lightingShader.setUniform4f("u_LightColor", light.color.r, light.color.g, light.color.b, light.color.a);
            checkOGL(glDrawElements(GL_TRIANGLES, quadIndexCount, GL_UNSIGNED_INT, nullptr));

            m_ImageBuffer.unbind();
        }
        
        shadowMap.unbind();
    }

    void Renderer::renderScene(EngineSceneInfo* sceneInfo)
    {
        if (!sceneInfo) return;
        if (!(*sceneInfo).pActiveCamera) return;

        renderSceneObjs(sceneInfo);
        renderSceneLighting(sceneInfo);
    }

}