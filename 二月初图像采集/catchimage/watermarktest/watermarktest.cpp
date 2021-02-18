#include <iostream>
#include <string>
#include <stdlib.h>
#include <Windows.h>
#include <CString>
#include <vector>
#include <thread>
#include "Include/imageConvert.h"
#include <stdio.h>
#include "opencv2/opencv.hpp"
#define DLL_FILENAME L"WatermarkAPI.dll"     //水印dll
#define DETFUN_NAME  "detectWatermark"
#define IMAGE_HEIGHT 256
#define IMAGE_WIDTH  256
using namespace cv;
using namespace std;

#include "Include/mysql/mysql.h"
#include "Include/Dahua/GenICam/System.h"
#include "Include/Dahua/GenICam/Camera.h"
#include "Include/Dahua/GenICam/GigE/GigECamera.h"
#include "Include/Dahua/GenICam/GigE/GigEInterface.h"
#include "Include/Dahua/Infra/PrintLog.h"
#include "Include/Dahua/Memory/SharedPtr.h"
#include "Include/imageConvert.h"

#pragma comment(lib, "Lib/ImageConvert.lib")
#pragma comment(lib, "Lib/MVSDKmd.lib")
#pragma comment(lib, "Lib/libmysql.lib")
#ifdef _DEBUG
#pragma comment(lib, "OpenCV64/lib/debug/opencv_ts340d.lib")
#pragma comment(lib, "OpenCV64/lib/debug/opencv_world340d.lib")
#else
#pragma comment(lib, "OpenCV64/lib/release/opencv_ts340.lib")
#pragma comment(lib, "OpenCV64/lib/release/opencv_world340.lib")
#endif

#pragma warning(disable:4996)
typedef unsigned char uchar;
using namespace Dahua::GenICam;
using namespace Dahua::Infra;


// 全局变量定义
ICameraPtr cameraSptr;
IStreamSourcePtr streamPtr;
bool isGrabbingFlag;
string caremaIp = "127.0.0.1";
double CAREMA_EXPOSURE_TIME = 200;
double CAREMA_GAIN_VALUE = 3.00;


//【设置相机信息】
static void setCameralInfor(ICameraPtr& cameraSptr)
{
	IGigECameraPtr gigeCameraPtr = IGigECamera::getInstance(cameraSptr);
	if (NULL == gigeCameraPtr)
	{
		return;
	}

	//获取Gige相机相关信息
	Dahua::Infra::CString ip = gigeCameraPtr->getIpAddress();
	Dahua::Infra::CString subnetMask = gigeCameraPtr->getSubnetMask();
	Dahua::Infra::CString gateway = gigeCameraPtr->getGateway();
	Dahua::Infra::CString macAddress = gigeCameraPtr->getMacAddress();


	//获取对应接口的网卡信息
	IGigEInterfacePtr gigeInterfaceSPtr = IGigEInterface::getInstance(cameraSptr);
	if (NULL == gigeInterfaceSPtr)
	{
		return;
	}

	Dahua::Infra::CString interfaceIp = gigeInterfaceSPtr->getIpAddress();
	Dahua::Infra::CString interfaceSubnetMask = gigeInterfaceSPtr->getSubnetMask();
	Dahua::Infra::CString interfaceGateway = gigeInterfaceSPtr->getGateway();
	Dahua::Infra::CString interfaceMacAddress = gigeInterfaceSPtr->getMacAddress();

	//设置Gige相机的IP和MAC地址
	Dahua::Infra::CString newip = interfaceIp.c_str();
	newip = newip.substr(0, 8);
	newip = newip.append(caremaIp.c_str());
	gigeCameraPtr->forceIpAddress(newip.c_str());
	ip = gigeCameraPtr->getIpAddress();
	subnetMask = gigeCameraPtr->getSubnetMask();
	gateway = gigeCameraPtr->getGateway();
	macAddress = gigeCameraPtr->getMacAddress();
	//printf("ip address is %s.\r\n", ip.c_str());
	//printf("subnetMask is %s.\r\n", subnetMask.c_str());
	//printf("gateway is %s.\r\n", gateway.c_str());
	//printf("macAddress is %s.\r\n", macAddress.c_str());
	//printf("\n");
	//cout << "newip:" <<newip.c_str()<< endl;

}

//【相机信息展示】
static void displayCameralInfor(ICameraPtr& cameraSptr)
{
	IGigECameraPtr gigeCameraPtr = IGigECamera::getInstance(cameraSptr);
	if (NULL == gigeCameraPtr)
	{
		return;
	}

	//获取Gige相机相关信息
	Dahua::Infra::CString ip = gigeCameraPtr->getIpAddress();
	Dahua::Infra::CString subnetMask = gigeCameraPtr->getSubnetMask();
	Dahua::Infra::CString gateway = gigeCameraPtr->getGateway();
	Dahua::Infra::CString macAddress = gigeCameraPtr->getMacAddress();
	printf("ip address is %s.\r\n", ip.c_str());
	printf("subnetMask is %s.\r\n", subnetMask.c_str());
	printf("gateway is %s.\r\n", gateway.c_str());
	printf("macAddress is %s.\r\n", macAddress.c_str());
	printf("\n");


	//获取对应接口的网卡信息
	IGigEInterfacePtr gigeInterfaceSPtr = IGigEInterface::getInstance(cameraSptr);
	if (NULL == gigeInterfaceSPtr)
	{
		return;
	}

	Dahua::Infra::CString interfaceIp = gigeInterfaceSPtr->getIpAddress();
	Dahua::Infra::CString interfaceSubnetMask = gigeInterfaceSPtr->getSubnetMask();
	Dahua::Infra::CString interfaceGateway = gigeInterfaceSPtr->getGateway();
	Dahua::Infra::CString interfaceMacAddress = gigeInterfaceSPtr->getMacAddress();
	printf("ip address of interface  is %s.\r\n", interfaceIp.c_str());
	printf("subnetMask of interface is %s.\r\n", interfaceSubnetMask.c_str());
	printf("gateway of interface is %s.\r\n", interfaceGateway.c_str());
	printf("macAddress of interface is %s.\r\n", interfaceMacAddress.c_str());
	printf("\n");
}

//【设置相机参数】
static bool setLineTriggerConf(IAcquisitionControlPtr sptrAcquisitionCtl, IAnalogControlPtr sptrAnalogControl, IImageFormatControlPtr imageFormatControl)
{
	cout << "[相机]: 正在设置相机参数..." << endl;
	cout << "------> 开始设置触发源！" << endl;
	
	// 设置触发源为Line1触发
	CEnumNode enumNode = sptrAcquisitionCtl->triggerSource();
	bool bRet = enumNode.setValueBySymbol("Line1");
	if (bRet != true)
	{
		cout << "------> 设置触发源失败！" << endl;
		return false;
	}

	// 设置触发器
	enumNode = sptrAcquisitionCtl->triggerSelector();
	bRet = enumNode.setValueBySymbol("FrameStart");
	if (bRet != true)
	{
		cout << "------> 设置触发器失败！" << endl;
		return false;
	}

	// 设置触发模式
	enumNode = sptrAcquisitionCtl->triggerMode();
	bRet = enumNode.setValueBySymbol("On");
	if (bRet != true)
	{
		cout << "------> 设置触发模式失败！" << endl;
		return false;
	}

	// 设置外触发为上升沿（下降沿为FallingEdge）
	enumNode = sptrAcquisitionCtl->triggerActivation();
	bRet = enumNode.setValueBySymbol("RisingEdge");
	if (bRet != true)
	{
		cout << "------> 设置外触发为上升沿失败！" << endl;
		return false;
	}

	//设置白平衡
	enumNode = sptrAnalogControl->balanceWhiteAuto();
	bRet = enumNode.setValueBySymbol("Continuous");
	if (bRet != true) {
		cout << "------> 设置白平衡失败！" << endl;
	}
	cout << "------> 设置触发源结束！" << endl;
	
	cout << "------> 设置曝光时间" << endl;
	double exposureTimeValue = CAREMA_EXPOSURE_TIME;
	CDoubleNode exposureTime = sptrAcquisitionCtl->exposureTime();
	exposureTime.setValue(exposureTimeValue);
	exposureTime.getValue(exposureTimeValue);
	//printf("after change ,exposureTime is %f. thread ID :%d\n", exposureTimeValue, CThread::getCurrentThreadID());
	cout << "------> 设置曝光时间结束" << endl;
	
	cout << "------> 设置gain值" << endl;
	double gainRawValue = CAREMA_GAIN_VALUE;
	CDoubleNode gainRaw = sptrAnalogControl->gainRaw();
	gainRaw.setValue(gainRawValue);
	cout << "------> 设置gain值结束" << endl;

	return true;
}

// 获取一次外触发信号后，相机所执行的操作
void onGetFrame(const CFrame &pFrame)
{
	DWORD curtime = GetTickCount();
	// 判断帧的有效性
	bool isValid = pFrame.valid();
	if (!isValid)
	{
		cout << "[相机]: 获得图像帧失败！" << endl;
		return;
	}
	else
	{
		uint64_t blockId = pFrame.getBlockId();
		isGrabbingFlag = false;
	}

	int ng = 2560 * 2560 * 3;
	uchar *p = new uchar[ng];
	IMGCNV_SOpenParam openParam;
	openParam.width = pFrame.getImageWidth();
	openParam.height = pFrame.getImageHeight();
	openParam.paddingX = pFrame.getImagePadddingX();
	openParam.paddingY = pFrame.getImagePadddingY();
	openParam.dataSize = pFrame.getImageSize();
	openParam.pixelForamt = pFrame.getImagePixelFormat();
	IMGCNV_EErr status = IMGCNV_ConvertToBGR24((uchar*)pFrame.getImage(), &openParam, p, &ng);
	Mat src = Mat(Size(2560, 2560), CV_8UC3, p);

	// 存储图片
	/*ostringstream imagename;
	String imagename = countn + ".jpg";
	imagename << countn << ".bmp";
	String str = imagename.str();
	cout << "nowimagecount" << imagename.str() << endl;
	vector <int>compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	imwrite(str, src, compression_params);*/
	

	// 清除缓存
	delete p;
}

// 调用此函数，开启相机，进而捕捉图像
void watermarkMain() {
	bool offFlag = true;
	connect: if (offFlag) {
		//发现设备
		CSystem &systemObj = CSystem::getInstance();
		TVector<ICameraPtr> vCameraPtrList;
		bool bRet = systemObj.discovery(vCameraPtrList);
		if (!bRet){ cout << "[相机]: 没有发现设备！" << endl; Sleep(3000); goto connect; }
		if (0 == vCameraPtrList.size()){ cout << "[相机]: 没有发现相机！" << endl; Sleep(3000); goto connect; }
		
		cameraSptr = vCameraPtrList[0];

		//打印设备和网口的信息
		//displayCameralInfor(cameraSptr);
		
		//设置设备和网口的信息
		setCameralInfor(cameraSptr);

		if (!vCameraPtrList[0]->connect()){ Sleep(3000); cout << "[相机]: 相机连接失败，正在重新连接..." << endl; goto connect; }

		//创建AcquisitionControl对象
		IAcquisitionControlPtr sptrAcquisitionControl = systemObj.createAcquisitionControl(cameraSptr);

		//创建AnalogControl对象
		IAnalogControlPtr sptrAnalogControl = systemObj.createAnalogControl(cameraSptr);

		//创建ImageFormatControl对象
		IImageFormatControlPtr imageFormatControl = systemObj.createImageFormatControl(cameraSptr);

		if (NULL == sptrAcquisitionControl){ cout << "[相机]: 创建控制对象失败，正在重新连接..." << endl; return; }

		//设置外部触发配置和相关配置
		bRet = setLineTriggerConf(sptrAcquisitionControl, sptrAnalogControl, imageFormatControl);
		if (!bRet){ cout << "[相机]: 设置触发模式失败，正在重新连接..." << endl; return; }

		//createThread();

		//创建流对象
		streamPtr = systemObj.createStreamSource(cameraSptr);

		if (NULL == streamPtr){ cout << "[相机]: 创建传输流对象失败！" << endl; return; }

		cout << "[相机]: 相机连接成功！" << endl;

		while (offFlag) {
			//注册回调函数
			bRet = streamPtr->attachGrabbing(onGetFrame);
			if (!bRet){ Sleep(3000); goto connect; }
			
			//设置图像缓存数
			//streamPtr->setBufferCount(4);

			//开始拉流
			bool isStartGrabbingSuccess = streamPtr->startGrabbing();
			if (!isStartGrabbingSuccess){ cout << "[相机]: 相机拉流失败，正在重新连接..." << endl; isGrabbingFlag = false; Sleep(3000); goto connect; }
			else{ isGrabbingFlag = true; }

			//等待外部触发
			int cnt = 0;
			while (isGrabbingFlag&&offFlag)
			{
				Sleep(50);
				cnt++;
				//if (cnt >= 10000)		break;    // [*] 根据实际情况注释掉
			}

			//注销回调函数
			bRet = streamPtr->detachGrabbing(onGetFrame);
			if (!bRet){ Sleep(3000); continue; }


			//停止相机拉流
			bRet = streamPtr->stopGrabbing();
			if (!bRet){ Sleep(3000); continue; }
		}

		//断开设备
		cameraSptr->disConnect();
	}
}


// 【主函数】
int main()
{
	watermarkMain();
	

	system("pause");
	return 0;
}

