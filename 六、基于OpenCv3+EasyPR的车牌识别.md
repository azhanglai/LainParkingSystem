[TOC]

### 1、EasyPR 简介

#### 1.1 链接

~~~http
https://blog.csdn.net/qq_31186123/article/details/78661566
~~~

#### 1.2 EasyPR特点

- 它基于openCV这个开源库，这意味着所有它的代码都可以轻易的获取。
- 它能够识别中文，例如车牌为苏EUK722的图片，它可以准确地输出std:string类型的"苏EUK722"的结果。
- 它的识别率较高。目前情况下，字符识别已经可以达到90%以上的精度。
- 把车牌识别划分为了两个过程：即车牌检测（Plate Detection）和字符识别（Chars Recognition）。

#### 1.3 完整的EasyPR的处理流程

![img](https://s2.loli.net/2022/05/24/sM8OCLPYWHBlutD.png)

#### 1.4 车牌检测（Plate Detection）流程

PlateDetect包括的是车牌定位，SVM训练，车牌判断三个过程

![img](https://s2.loli.net/2022/05/24/UeMLcrqvdEp4TNn.png)

#### 1.5 字符识别（Chars Recognition）流程

CharsRecognise包括的是字符分割，ANN训练，字符识别三个过程

![img](https://s2.loli.net/2022/05/24/IaqhJGNfsKBxYZp.png)

### 2、CascadeClassifier(级联分类器) 检测步骤

~~~c++
// 1.load()加载xml级联分类器
CascadeClassifier cascade;
cascade.load("****.xml");
// 2.从视频中取帧，导出image
VideoCapture capture;
Mat picture;
capture.open("****.mp4");
capture.read(picture);
// 3.图像灰度化
Mat grayImg;
cvtColor(picture, grayImg, CV_BGR2GRAY);
// 4.图像resize
Mat smallImg(cvRound(img.rows/scale), cvRound(img.cols/scale), CV_8UC1);
resize(grayImg, smallImg, smallImg.size(), 0, 0);
// 5.直方图均值化
equalizeHist(smallImg,smallImg);
// 6.调用detectMultiScale()实现多尺度检测
//函数原型：
void detectMultiScale( 
	InputArray image, 			// 输入CV_8U的待检测图像
	std::vector<Rect>& objects,	// 输出检测到的目标区域，一组包含检测目标的矩形，其中的矩形可能会部分的在原始图像外侧
	double scaleFactor = 1.1,	// 搜索前后两次窗口大小比例系数，默认1.1，即每次搜索窗口扩大10%
	int minNeighbors = 3,		// opencv识别物体的时候最少检测3次才是算是目标
	int flags=0,				// 设置为CV_HAAR_DO_CANNY_PRUNING 函数将会使用Canny边缘检测来排除边缘过多或过少的区域 
	Size minSize=Size(),		// 目标可检测的最小尺寸，小于该尺寸的目标将被忽略
	Size maxSize=Size())		// 目标可检测的最大尺寸，大于该尺寸的目标将被忽略

cascade.detectMultiScale(smallImg, cars, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30));
// CV_HAAR_SCALE_IMAGE表示不是缩放分类器来检测，而是缩放图像，Size(30, 30)为目标的最小最大尺寸
~~~

### 3、车牌识别步骤

~~~c++
// 1.创建CPlateRecognize对象
CPlateRecognize pr;

// 2.车牌识别参数初始化
// 2.1 setDetectType：设置EasyPR采用的车牌定位算法
// PR_DETECT_CMSER：文字定位方法
// PR_DETECT_COLOR：颜色定位方法
pr.setDetectType(PR_DETECT_CMSER | PR_DETECT_COLOR);
// 2.2 setResultShow：设置EasyPR是否打开结果展示窗口
// 设置为true就是打开，否则就是关闭。在需要观看定位结果时，建议打开，快速运行时关闭。
pr.setResultShow(false);
// 2.3 setMaxPlates：设置EasyPR最多查找多少个车牌
// 当一副图中有大于n个车牌时，EasyPR最终只会输出可能性最高的n个。
pr.setMaxPlates(4);

// 3. 车牌检测
Mat img, plateMat, plateMat_car;
CPlate plate;
vector<CPlate> plateVec;

plateVec.clear(); // 先清空，防止二次点击不识别新车牌
// 参数img：输入图像
// 参数plateVec：输出的车牌CPlate集合
// 返回值：返回结果为0时，代表识别成功，否则失败
pr.plateRecognize(img, plateVec);

// 3.1 车牌识别
plate = plateVec.at(0);
plateMat = plate.getPlateMat();
cvtColor(plateMat, plateMat_car, CV_BGR2RGB);

// 3.2 字符识别
string liences = plate.getPlateStr();
~~~

