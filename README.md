# Bona-VulkanRenderer

**这是我基于Vulkan实现的渲染器Demo。**

## 渲染器介绍

开发语言：C++

编译工具：Cmake

开发环境：Windows 10 - Microsoft Visual Studio 2022

### 控制

- 按W或S可以切换场景
- 长按鼠标左键可以旋转摄像机
- 长按鼠标右键可以移动摄像机
- 按空格键可以复位摄像机
- 按A或D可以旋转灯光方向
- 按E可以切换阴影开关
- 按Q可以切换Shader

## 渲染器启动

主要需要两步操作:

去Vulkan官网下载SDK，看自己电脑是否能够支持Vulkan API运行。

本项目是在VS中用Cmake创建的，右键点击工程目录，最好使用VS打开，如图所示：

![](https://github.com/michaelchern/Bona-VulkanRenderer/blob/main/README_IMG/example1.png)

然后配置根目录下的Cmakelists文件，如图所示，把Vulkan SDK路径替换成你本地电脑的：

![](https://github.com/michaelchern/Bona-VulkanRenderer/blob/main/README_IMG/example2.png)

## 什么是Vulkan

啃了差不多一个月，总算有点眉目了，准备写文章记录一下。去知乎、Github逛了一下，发现大佬已经把文章写好了，那我写啥？大佬都把图画好了，给跪了，这里偷一张图。

![](https://github.com/michaelchern/Bona-VulkanRenderer/blob/main/README_IMG/Vulkan-MindMap.jpg)