
#pragma once

#include "vulkanWrapper/base.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"

namespace LearnVulkan
{
    // 顶点数据结构定义
    struct Vertex
    {
        glm::vec3 mPosition;  // 顶点位置 (xyz)
        glm::vec3 mColor;     // 顶点颜色 (rgb)
    };

    /**
    * @class Model
    * @brief 3D模型资源封装类
    *
    * 管理模型的顶点/索引数据及其缓冲区，包含：
    * - 顶点位置数据
    * - 顶点颜色数据
    * - 纹理UV坐标数据
    * - 索引数据
    * - 模型变换矩阵（支持动态更新）
    */
    class Model
    {
    public:
        using Ptr = std::shared_ptr<Model>;
        static Ptr create(const Wrapper::Device::Ptr& device)
        {
            return std::make_shared<Model>(device);
        }

        /**
        * @brief 构造函数 - 初始化模型数据并创建缓冲区
        * @param device 关联的Vulkan设备
        *
        * 创建默认四边形模型（两个三角形组成）：
        *  1. 定义顶点位置数据
        *  2. 定义顶点颜色数据
        *  3. 定义纹理UV坐标数据
        *  4. 定义索引数据（三角形组成）
        *  5. 创建对应的GPU缓冲区
        */
        Model(const Wrapper::Device::Ptr &device)
        {
            /*mDatas =
            {
                {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            };*/

            // 四边形顶点位置 (四个顶点)
            mPositions =
            {
                0.0f, 0.5f, 0.0f,
                0.5f, 0.0f, 0.0f,
                -0.5f, 0.0f, 0.0f,
                0.0f, -0.5f, 0.0f
            };

            // 每个顶点的颜色 (RGBA)
            mColors =
            {
                1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f,
                1.0f, 0.0f, 0.0f
            };

            // 纹理UV坐标 (纹理映射用)
            mUVs =
            {
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 1.0f,
                1.0f, 0.0f
            };

            // 索引数据 (两个三角形组成四边形: 0-2-1 和 1-2-3)
            mIndexDatas = { 0, 2, 1, 1, 2, 3};

            //mVertexBuffer = Wrapper::Buffer::createVertexBuffer(device, mDatas.size() * sizeof(Vertex), mDatas.data());

            // 创建GPU顶点缓冲区 (位置数据)
            mPositionBuffer = Wrapper::Buffer::createVertexBuffer(
                device,
                mPositions.size() * sizeof(float),
                mPositions.data()
            );

            // 创建GPU顶点缓冲区 (颜色数据)
            mColorBuffer = Wrapper::Buffer::createVertexBuffer(
                device,
                mColors.size() * sizeof(float),
                mColors.data()
            );

            // 创建GPU顶点缓冲区 (UV坐标数据)
            mUVBuffer = Wrapper::Buffer::createVertexBuffer(
                device,
                mUVs.size() * sizeof(float),
                mUVs.data()
            );

            // 创建GPU索引缓冲区
            mIndexBuffer = Wrapper::Buffer::createIndexBuffer(
                device,
                mIndexDatas.size() * sizeof(float),
                mIndexDatas.data()
            );

        }

        ~Model() {}

        // ==================================================================
        // 顶点输入状态描述
        // ==================================================================

        /**
        * @brief 获取顶点输入绑定描述
        * @return 绑定描述数组
        *
        * 描述每个顶点属性绑定的步幅和行为：
        * - 位置属性: 绑定点0, 步幅=3个float
        * - 颜色属性: 绑定点1, 步幅=3个float
        * - UV属性:   绑定点2, 步幅=2个float
        */
        std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions()
        {
            std::vector<VkVertexInputBindingDescription> bindingDes{};
            /*bindingDes.resize(1);

            bindingDes[0].binding = 0;
            bindingDes[0].stride = sizeof(Vertex);
            bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;*/

            bindingDes.resize(3);

            // 位置属性绑定 (绑定点0)
            bindingDes[0].binding = 0;
            bindingDes[0].stride = sizeof(float) * 3;  // XYZ = 3个float
            bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            // 颜色属性绑定 (绑定点1)
            bindingDes[1].binding = 1;
            bindingDes[1].stride = sizeof(float) * 3;  // RGB = 3个float
            bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            // UV属性绑定 (绑定点2)
            bindingDes[2].binding = 2;
            bindingDes[2].stride = sizeof(float) * 2;  // UV = 2个float
            bindingDes[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDes;
        }

        /**
        * @brief 获取顶点属性描述，Attribute相关信息，与VertexShader里面的location相关
        * @return 属性描述数组
        *
        * 描述每个顶点属性的位置和格式：
        * - 位置: 位置索引0, 格式=R32G32B32_SFLOAT
        * - 颜色: 位置索引1, 格式=R32G32B32_SFLOAT
        * - UV: 位置索引2, 格式=R32G32_SFLOAT
        */
        std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> attributeDes{};
            attributeDes.resize(3);

            // 位置属性 (绑定点0, 位置索引0)
            attributeDes[0].binding = 0;
            attributeDes[0].location = 0;  // 对应shader的layout(location = 0)
            attributeDes[0].format = VK_FORMAT_R32G32B32_SFLOAT;  // XYZ
            //attributeDes[0].offset = offsetof(Vertex, mPosition);
            attributeDes[0].offset = 0;  // 在缓冲区起始位置

            // 颜色属性 (绑定点1, 位置索引1)
            //attributeDes[1].binding = 0;
            attributeDes[1].binding = 1;
            attributeDes[1].location = 1;  // 对应shader的layout(location = 1)
            attributeDes[1].format = VK_FORMAT_R32G32B32_SFLOAT;  // RGB
            //attributeDes[1].offset = offsetof(Vertex, mColor);
            attributeDes[1].offset = 0;  // 在缓冲区起始位置

            // UV属性 (绑定点2, 位置索引2)
            attributeDes[2].binding = 2;
            attributeDes[2].location = 2;  // 对应shader的layout(location = 2)
            attributeDes[2].format = VK_FORMAT_R32G32_SFLOAT;  // UV
            //attributeDes[0].offset = offsetof(Vertex, mPosition);
            attributeDes[2].offset = 0;  // 在缓冲区起始位置

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
                mColorBuffer->getBuffer(),
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

        /// 设置模型变换矩阵
        void setModelMatrix(const glm::mat4 matrix)
        {
            mUniform.mModelMatrix = matrix;
        }

        /// 每帧更新模型（旋转动画）
        void update()
        {
            // 创建旋转矩阵（绕Z轴旋转）
            glm::mat4 rotateMatrix = glm::mat4(1.0f);
            rotateMatrix = glm::rotate(
                rotateMatrix,
                glm::radians(mAngle),        // 角度转换
                glm::vec3(0.0f, 0.0f, 1.0f)  // 旋转轴(Z)
            );
            mUniform.mModelMatrix = rotateMatrix;

            // 更新旋转角度（简单动画）
            mAngle += 0.01f;
        }
    private:
        // 原始模型数据
        //std::vector<Vertex> mDatas{};
        std::vector<float> mPositions{};          // 顶点位置数据 (XYZ)
        std::vector<float> mColors{};             // 顶点颜色数据 (RGB)
        std::vector<unsigned int> mIndexDatas{};  // 索引数据 (uint32_t)
        std::vector<float> mUVs{};                // 纹理UV坐标 (UV)

        // GPU缓冲区对象
        //Wrapper::Buffer::Ptr mVertexBuffer{ nullptr };  
        Wrapper::Buffer::Ptr mPositionBuffer{ nullptr };  // 位置数据缓冲区
        Wrapper::Buffer::Ptr mColorBuffer{ nullptr };     // 颜色数据缓冲区
        Wrapper::Buffer::Ptr mUVBuffer{ nullptr };        // UV数据缓冲区
        Wrapper::Buffer::Ptr mIndexBuffer{ nullptr };     // 索引数据缓冲区

        ObjectUniform mUniform;  // 模型统一变量
        float mAngle{ 0.0f };    // 当前旋转角度（度）
    };
}