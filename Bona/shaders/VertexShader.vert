// 指定GLSL版本为450（对应Vulkan API）
#version 450

// 启用分离着色器对象扩展（Vulkan必需）
#extension GL_ARB_separate_shader_objects:enable

// ---- 输入顶点属性 ----
layout(location = 0) in vec3 inPosition;  // 顶点位置（模型空间）
//layout(location = 1) in vec3 inColor;     // 顶点颜色（RGB）
layout(location = 1) in vec2 inUV;        // 纹理坐标（UV）

// ---- 输出到片段着色器的数据 ----
//layout(location = 0) out vec3 outColor;   // 传递顶点颜色
layout(location = 1) out vec2 outUV;      // 传递纹理坐标

// ---- 统一缓冲区（Uniform Buffers）----
// 绑定点0：视图和投影矩阵（通常每帧更新一次）
layout(binding = 0) uniform VPMatrices
{
    mat4 mViewMatrix;                     // 世界空间 -> 观察空间变换
    mat4 mProjectionMatrix;               // 观察空间 -> 裁剪空间变换
}vpUBO;

// 绑定点1：模型矩阵（通常每个物体更新一次）
layout(binding = 1) uniform ObjectUniform
{
    mat4 mModelMatrix;                    // 模型空间 -> 世界空间变换
}objectUBO;

// ===== 主函数 =====
void main()
{
    // 顶点位置变换流水线：
    // 1. 模型空间 -> 世界空间 (mModelMatrix)
    // 2. 世界空间 -> 观察空间 (mViewMatrix)
    // 3. 观察空间 -> 裁剪空间 (mProjectionMatrix)
    gl_Position = vpUBO.mProjectionMatrix * vpUBO.mViewMatrix * objectUBO.mModelMatrix * vec4(inPosition, 1.0);

    // 传递颜色和纹理坐标到片段着色器
    //outColor = inColor;  // 输出原始顶点颜色
    outUV = inUV;        // 输出原始UV坐标
}