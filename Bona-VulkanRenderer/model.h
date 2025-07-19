#pragma once

#include "vulkanWrapper/base.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"

namespace LearnVulkan
{
    class Model
    {
    public:
        using Ptr = std::shared_ptr<Model>;

        static Ptr create(const Wrapper::Device::Ptr& device)
        {
            return std::make_shared<Model>(device);
        }

        Model(const Wrapper::Device::Ptr &device)
        {
        }

        void loadModel(const std::string& path, const Wrapper::Device::Ptr& device);

        ~Model() {}

        // ==================================================================
        // 顶点输入状态描述
        // ==================================================================

        std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions()
        {
            std::vector<VkVertexInputBindingDescription> bindingDes{};
            bindingDes.resize(2);

            // 位置属性绑定 (绑定点0)
            bindingDes[0].binding = 0;
            bindingDes[0].stride = sizeof(float) * 3;
            bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            // 颜色属性绑定 (绑定点1)
            //bindingDes[1].binding = 1;
            //bindingDes[1].stride = sizeof(float) * 3;
            //bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            // UV属性绑定 (绑定点2)
            bindingDes[1].binding = 1;
            bindingDes[1].stride = sizeof(float) * 2;
            bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDes;
        }

        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> attributeDes{};
            attributeDes.resize(2);

            // 位置属性 (绑定点0, 位置索引0)
            attributeDes[0].binding  = 0;
            attributeDes[0].location = 0;                           // 对应shader的layout(location = 0)
            attributeDes[0].format   = VK_FORMAT_R32G32B32_SFLOAT;  // XYZ
            //attributeDes[0].offset = offsetof(Vertex, mPosition);
            attributeDes[0].offset   = 0;                           // 在缓冲区起始位置

            // 颜色属性 (绑定点1, 位置索引1)
            // attributeDes[1].binding = 0;
            //attributeDes[1].binding  = 1;
            //attributeDes[1].location = 1;                           // 对应shader的layout(location = 1)
            //attributeDes[1].format   = VK_FORMAT_R32G32B32_SFLOAT;  // RGB
            //attributeDes[1].offset = offsetof(Vertex, mColor);
            //attributeDes[1].offset   = 0;                           // 在缓冲区起始位置

            // UV属性 (绑定点2, 位置索引2)
            attributeDes[1].binding  = 1;
            attributeDes[1].location = 1;                           // 对应shader的layout(location = 2)
            attributeDes[1].format   = VK_FORMAT_R32G32_SFLOAT;     // UV
            //attributeDes[0].offset = offsetof(Vertex, mPosition);
            attributeDes[1].offset   = 0;                           // 在缓冲区起始位置

            return attributeDes;
        }

        // ==================================================================
        // 模型数据访问接口
        // ==================================================================

        //[[nodiscard]] auto getVertexBuffer() const { return mVertexBuffer; }

        /// 获取所有顶点缓冲区数组 (位置/颜色/UV)
        [[nodiscard]] auto getVertexBuffers() const
        {
            std::vector<VkBuffer> buffers
            {
                mPositionBuffer->getBuffer(),
                //mColorBuffer->getBuffer(),
                mUVBuffer->getBuffer()
            };

            return buffers;
        }

        /// 获取索引缓冲区
        [[nodiscard]] auto getIndexBuffer() const
        {
            return mIndexBuffer;
        }

        /// 获取索引数量
        [[nodiscard]] auto getIndexCount() const
        {
            return mIndexDatas.size();
        }

        /// 获取模型统一变量
        [[nodiscard]] auto getUniform() const
        {
            return mUniform;
        }

        auto getVPUniform() const
        {
            return mVPUniform;
        }

        /// 设置模型变换矩阵
        void setModelMatrix(const glm::mat4 matrix)
        {
            mUniform.mModelMatrix = matrix;
        }

        /// 每帧更新模型（旋转动画）
        void update(unsigned int width, unsigned int height)
        {
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime      = std::chrono::high_resolution_clock::now();
            float time            = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
   
            mUniform.mModelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            mVPUniform.mProjectionMatrix = glm::perspective(glm::radians(60.0f), width / (float)height, 0.1f, 1000.0f);

            mVPUniform.mViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
    private:
        // 原始模型数据
        std::vector<float>        mPositions{};          // 顶点位置数据 (XYZ)
        std::vector<float>        mColors{};             // 顶点颜色数据 (RGB)
        std::vector<unsigned int> mIndexDatas{};         // 索引数据 (uint32_t)
        std::vector<float>        mUVs{};                // 纹理UV坐标 (UV)
        
        // GPU缓冲区对象
        Wrapper::Buffer::Ptr mVertexBuffer{ nullptr };
        Wrapper::Buffer::Ptr mPositionBuffer{ nullptr };  // 位置数据缓冲区
        Wrapper::Buffer::Ptr mColorBuffer{ nullptr };     // 颜色数据缓冲区
        Wrapper::Buffer::Ptr mUVBuffer{ nullptr };        // UV数据缓冲区
        Wrapper::Buffer::Ptr mIndexBuffer{ nullptr };     // 索引数据缓冲区

        ObjectUniform        mUniform;                    // 模型统一变量
        VPMatrices           mVPUniform;                  // 视图投影矩阵统一变量
        float                mAngle{ 0.0f };              // 当前旋转角度（度）

    };
}
