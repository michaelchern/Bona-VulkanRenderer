#pragma once

#include "vulkanWrapper/base.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"

namespace LearnVulkan
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;
    };

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

        void loadModel()
        {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, MODEL_PATH.c_str()))
            {
                throw std::runtime_error(err);
            }

            std::unordered_map<Vertex, uint32_t> uniqueVertices{};

            for (const auto& shape : shapes)
            {
                for (const auto& index : shape.mesh.indices)
                {
                    Vertex vertex{};

                    vertex.pos =
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.texCoord =
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };

                    vertex.color = { 1.0f, 1.0f, 1.0f };

                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }

                    indices.push_back(uniqueVertices[vertex]);
                }
            }
        }

        ~Model() {}

        // ==================================================================
        // 顶点输入状态描述
        // ==================================================================

        //std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions()
        //{
        //    std::vector<VkVertexInputBindingDescription> bindingDes{};
        //    bindingDes.resize(3);

        //    // 位置属性绑定 (绑定点0)
        //    bindingDes[0].binding = 0;
        //    bindingDes[0].stride = sizeof(float) * 3;
        //    bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        //    // 颜色属性绑定 (绑定点1)
        //    bindingDes[1].binding = 1;
        //    bindingDes[1].stride = sizeof(float) * 3;
        //    bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        //    // UV属性绑定 (绑定点2)
        //    bindingDes[2].binding = 2;
        //    bindingDes[2].stride = sizeof(float) * 2;
        //    bindingDes[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        //    return bindingDes;
        //}

        VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding   = 0;
            bindingDescription.stride    = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        //std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        //{
        //    std::vector<VkVertexInputAttributeDescription> attributeDes{};
        //    attributeDes.resize(3);

        //    // 位置属性 (绑定点0, 位置索引0)
        //    attributeDes[0].binding  = 0;
        //    attributeDes[0].location = 0;                           // 对应shader的layout(location = 0)
        //    attributeDes[0].format   = VK_FORMAT_R32G32B32_SFLOAT;  // XYZ
        //    //attributeDes[0].offset = offsetof(Vertex, mPosition);
        //    attributeDes[0].offset   = 0;                           // 在缓冲区起始位置

        //    // 颜色属性 (绑定点1, 位置索引1)
        //    // attributeDes[1].binding = 0;
        //    attributeDes[1].binding  = 1;
        //    attributeDes[1].location = 1;                           // 对应shader的layout(location = 1)
        //    attributeDes[1].format   = VK_FORMAT_R32G32B32_SFLOAT;  // RGB
        //    //attributeDes[1].offset = offsetof(Vertex, mColor);
        //    attributeDes[1].offset   = 0;                           // 在缓冲区起始位置

        //    // UV属性 (绑定点2, 位置索引2)
        //    attributeDes[2].binding  = 2;
        //    attributeDes[2].location = 2;                           // 对应shader的layout(location = 2)
        //    attributeDes[2].format   = VK_FORMAT_R32G32_SFLOAT;     // UV
        //    //attributeDes[0].offset = offsetof(Vertex, mPosition);
        //    attributeDes[2].offset   = 0;                           // 在缓冲区起始位置

        //    return attributeDes;
        //}

        std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
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
            rotateMatrix = glm::rotate(rotateMatrix,
                                       glm::radians(mAngle),          // 角度转换
                                       glm::vec3(0.0f, 0.0f, 1.0f));  // 旋转轴(Z)
            
            mUniform.mModelMatrix = rotateMatrix;

            // 更新旋转角度（简单动画）
            mAngle += 0.01f;
        }
    private:
        // 原始模型数据
        // std::vector<Vertex> mDatas{};
        //std::vector<float>        mPositions{};          // 顶点位置数据 (XYZ)
        //std::vector<float>        mColors{};             // 顶点颜色数据 (RGB)
        //std::vector<unsigned int> mIndexDatas{};         // 索引数据 (uint32_t)
        //std::vector<float>        mUVs{};                // 纹理UV坐标 (UV)
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // GPU缓冲区对象
        //Wrapper::Buffer::Ptr mVertexBuffer{ nullptr };
        Wrapper::Buffer::Ptr mPositionBuffer{ nullptr };  // 位置数据缓冲区
        Wrapper::Buffer::Ptr mColorBuffer{ nullptr };     // 颜色数据缓冲区
        Wrapper::Buffer::Ptr mUVBuffer{ nullptr };        // UV数据缓冲区
        Wrapper::Buffer::Ptr mIndexBuffer{ nullptr };     // 索引数据缓冲区

        ObjectUniform        mUniform;                    // 模型统一变量
        float                mAngle{ 0.0f };              // 当前旋转角度（度）
    };
}
