1.项目概述:微型软光栅渲染器，没有借助图形库，该项目旨在实现 opengl  渲染管线

--实现了 Bresenham  画线算法

起点(x1, y1), 终点(x2, y2), 连接他们的理论直线可以用一元一次方程y=mx+b来描述，规定m∈ (0,1)并且这条线是指向第一象限的线，则有以下规律：         
(1).X的值每次增长1           
(2).Y的值会在保持不变和增长1之间选择        
Y的值如何变化取决于斜率的倒数，每当x的值增长1/m，y就会增长1，于是y+=int（m*Δx）即可.      
当然这只是大致原理，实现起来复杂些

显示效果：

![image](https://github.com/Abelabc/SoftRenderer/blob/main/pic/2.png)

![image](https://github.com/Abelabc/SoftRenderer/blob/main/pic/3.png)

--实现了三角形光栅化器和背面剔除

获取了模型的所有微元（三角形），三层循环，第一层为场景中的所有模型，第二层循环以三角形为单位，第三层循环以每个三角形的顶点为单位（三次循环），在内层循环中将每个顶点传入顶点着色器处理，外层循环以三角形为单位传入光栅化器。

显示效果：

![image](https://github.com/Abelabc/SoftRenderer/blob/main/pic/1.png)

![image](https://github.com/Abelabc/SoftRenderer/blob/main/pic/4.png)

![image](https://github.com/Abelabc/SoftRenderer/blob/main/pic/5.png)

![image](https://github.com/Abelabc/SoftRenderer/blob/main/pic/6.png)

对于三角形整体，计算其法向量和摄像机向量，执行背面剔除。只有三角形通过了背面剔除才会进入光栅化阶段。光栅化阶段使用包围盒，在包围盒范围内遍历所有像素，通过重心坐标判断是否在三角形内，如果在三角形内，
则执行深度测试，如果通过了深度测试，则将该像素的重心坐标传入片段着色器处理，前面任何一步不通过，三角形或者像素就会直接被抛弃，直接开始下一循环。

--实现了深度缓冲

定义深度缓冲二维数组，在三角形光栅化阶段，当像素被判定处在三角形内部后，执行深度测试，并更新深度缓冲。
在顶点着色器部分拿到了三角形的各顶点的数据。
执行完深度测试后数据由一开始的三角形顶点数据增加了相关像素的重心坐标和深度缓冲，它们随后会作为参数传入片段着色器。

--实现了 shader类（其中可自定义着色器）

类里的两个函数
virtual Vec4f vertex(int iface, int nthvert)；
virtual bool fragment(Vec3f bar, TGAColor &color)；
可分别作为顶点着色器和片段着色器。

--实现了纹理的使用

主要在片段着色器部分，通过uv坐标可以拿到纹理数据。

--实现了 shadow mapping 

将相机放在光源位置执行第一遍渲染，拿到阴影深度缓冲。然后将相机放回正确的位置，执行第二次渲染，两次渲染使用的着色器是不同的。
第二次渲染时，对于每个通过深度测试的像素，执行阴影测试，声明一个常量矩阵mat<4,4,float> uniform_Mshadow，它允许我将当前片段的屏幕坐标转换为阴影缓冲区内的屏幕坐标，
然后比对深度值来判断像素是否处在阴影之中。

--实现了简单的环境光遮蔽

AO为每个像素定义了一个遮挡度值，通过从像素发射出多条射线，来大致模拟环境光的遮挡度，是一种近似的全局光照技术，近似模拟间接光照，改进了Phong 反射模型的环境光部分。

--成果展示

![image](https://github.com/LGC363/tinyrenderer/blob/main/result.png)

