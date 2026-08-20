# python 相机库的理解

# 由Boost.Python库将c++代码绑定到Python的。分为枚举类型和类类型以及dvpException类型

import dvp
import numpy as np
import cv2
from mypy.build import CacheMeta

import ImageFormats
import DebugReport



# 不用这个了
# from general_inspector_tool import analyze

import CurveLut
import CheckClass

"""
不需要使用下面这个打印相机信息函数了，可以用检测函数，分析dvp.CameraInfo的对象实例信息，甚至方法
"""
# def print_camera_info(camera_info):
#     """打印单个相机的详细信息"""
#     print("\n===== 相机详细信息 =====")
#
#     check_method_overrides(camera_info) #检查dvp.CameraInfo对象
#
#
#     # print(type(camera_info))    #
#     # print(dir(camera_info))     #列出该类实例的所有可调用属性和方法名
#     # print(camera_info)          #默认应该是打印对象地址，但开发者修改了__str__和__repr__
#
#     # # 只保留dvp.CameraInfo的类实例camera_info开发者添加的方法，而不打印python内置方法
#     # custom_methods = [
#     #     attr for attr in dir(camera_info)
#     #     if not attr.startswith('__')  # 过滤掉Python内置方法（__开头的）
#     #        and callable(getattr(camera_info, attr))  # 只保留可调用方法，callable(),准确识别方法，不包括属性
#     # ]
#     # print("开发者添加的方法:", custom_methods)
#
#     # 打印所有非方法属性
#     print("\n--- 打印所有非方法属性 ---")
#     for attr in dir(camera_info): #用attr遍历dir(camera_info)
#         #双条件过滤，① 过滤条件1，检查名字是否以双下划线开头
#         #② getattr(camera_info, attr)，获取该名字对应的属性/方法对象。相当于camera_info.Model或camera_info.SaveConfig
#         #callable()，判断是否是函数/方法，callable(camera_info.SaveConfig) → True，callable(camera_info.Model) → False
#         #not-取反，排除方法
#         if not attr.startswith('__') and not callable(getattr(camera_info, attr)):
#             # getattr(camera_info, attr)  #动态获取属性值，不需要提前知道属性名；以下是等价写法
#             # if attr == 'Model':
#             #     value = camera_info.Model
#             # elif attr == 'SerialNumber':
#             #     value = camera_info.SerialNumber
#             # ...
#             value = getattr(camera_info, attr)
#             print(f"{attr:>20} ({type(value)}): {value if value else '<空>'}")
#             # f-string语法：f"内容{变量}"，大括号内插入变量值
#             # attr:>20：右对齐，占20字符宽度
#             # {value if value else '<空>'} f除了能格式化输出变量外，还可以在{}里加入语句；该括号内容是三元表达式；等价于下面的
#             # if value:
#             #     display_value = value
#             # else:
#             #     display_value = '<空>'

"""
getattr函数定义

value = getattr(object, name[, default])
参数解释：
object：要操作的对象（例如你的 camera_info 对象）
name：要获取的属性/方法名（必须是字符串，例如 "Model"）
default：可选参数，当属性不存在时返回的默认值
返回值：
如果对象有该属性或方法：返回对应的值
如果对象没有该属性：
提供了 default：返回 default
没有提供 default：抛出 AttributeError 异常

"""






"""
在 Python 中，e 是 Exception 类型的一个实例对象。当发生异常时，e 就是捕获到的异常对象。
你完全可以直接打印 e，因为 Exception 对象实现了 __str__ 方法，所以它会以字符串的形式表示错误信息

try —— Except语句
try:
    # 可能抛出异常的代码
except <ExceptionType> as e:
    # 异常处理代码
else:
    # 如果没有发生异常，执行这部分代码
finally:
    # 不管是否发生异常，都会执行这部分代码

try: 放置可能抛出异常的代码块。如果 try 中的代码没有错误，except 不会执行。
except: 捕获特定类型的异常。如果没有发生 try 语句中的异常，except 不会被执行。
else: 如果 try 中的代码没有抛出任何异常，else 中的代码会被执行。这是一个可选的部分。
finally: 无论是否发生异常，都会执行的代码。这也通常用于清理资源（例如关闭文件或释放连接等）。
"""





def main():

    """
    dvp.refresh()返回的是一个列表，调用该功能时，会把每个连接的相机，创建一个dvp.CameraInfo的对象
    列表中的元素存储的是对dvp.CameraInfo对象的引用
    列表中元素的个数，取决于连接的相机数。
    """
    camera_list = dvp.Refresh()
    print("相机枚举返回值",camera_list)
    if not camera_list:
        print("The camera is not found")
        return
    print(f"找到 {len(camera_list)} 台相机:")

    print("\ncamera_list", camera_list) #打印camera_list信息



    """
    enumerate(camera_list):
    enumerate()是Python内置函数，它会在遍历列表时，以元组的方式返回元素的索引和值
    对于camera_list = [cam1, cam2, cam3]，它会生成类似(0,cam1), (1,cam2), (2,cam3)这样的元组序列
    idx, cam_info:
    这是元组解包，每次循环时：
    idx获取索引值（从0开始）
    cam_info获取对应的相机对象
    """

    # 枚举相机
    for idx, cam_info in enumerate(camera_list):
        print(f"\n>>> 相机 {idx + 1} 基本信息:")
        print(f"  型号: {cam_info.Model} (SN: {cam_info.SerialNumber})")
        print(f"  厂商: {cam_info.Manufacturer}")
        print(f"  固件: {cam_info.FirmwareVersion}")

        # 用户可选择是否打印完整信息
        choice = input("是否显示完整信息？ (y/n): ").strip().lower()
        if choice == 'y':
            #print_camera_info(cam_info)   #不用这个函数了，用下面这个函数
            # 直接调用通用工具的函数来显示完整信息

            CheckClass.check_method_property(cam_info,False)
    # 打开相机的三种方式，每种方式分创建对象时用构造函数和先创建空对象，再用三种不同的打开相机函数

    try:
        camera = dvp.Camera(idx,dvp.OpenMode.OPEN_NORMAL)
        print("1",camera)
        # handle = dvp.Find(cam_info.FriendlyName) #使用这个会报错
        # print(type(handle))
        # camera = dvp.Camera(handle)

        #camera = dvp.Camera(cam_info.FriendlyName,dvp.OpenMode.OPEN_NORMAL) #第二个参数可以不加

        # camera = dvp.Camera(cam_info.UserID) #使用会报错

        #camera.Open(0,dvp.OpenMode.OPEN_NORMAL)
        #camera.OpenByName(cam_info.FriendlyName,dvp.OpenMode.OPEN_NORMAL)
        #camera.OpenByUserId(cam_info.UserID,dvp.OpenMode.OPEN_NORMAL)
    except Exception as e:
        print("打开相机错误信息：", e)


   # 检测Camera类
    try:
        # 下面两行代码，需要打开相机才会起作用
        print("\ncamera:", camera)  # 需要打开相机才能打印Camera类实例对象，打印出的不是对象地址，而是相机信息，说明这个类的原始方法被修改了
        CheckClass.check_method_property(camera,False)
    except Exception as e:
        print("Camera类的实例错误信息",e)

    # 加载相机默认参数
    # Status = camera.LoadDefault()

    # 加载指定ini到相机
    # Status = camera.LoadConfig("D:\\Document\\Do3Think\\Python\\MyOwn\\UnderstandingOfPythonAPI\\ZDS(7)@U004600040008.ini")


    # 保存ini到指定路径，路径要存在并且路径名要加上ini文件名，比如该例子ini文件名为example.ini；若每指定路径名会保存在c盘的隐藏路径
    # Status = camera.SaveConfig("D:\\Document\\Do3Think\\Python\\MyOwn\\UnderstandingOfPythonAPI\\example.ini")


    # """
    # 枚举格式信息和设置格式
    # """
    #
    # source_formats = ImageFormats.list_source_formats_by_enum(camera)
    # target_formats = ImageFormats.list_target_formats_by_enum(camera)
    # #提示用户是否要设置格式
    # confirm = input("您是否要设置原始图像格式和目标图像格式？(y/n): ")
    # if confirm.lower() == 'y':
    #     # 用户输入索引值
    #     source_index = int(input("请输入要选择的原始图像格式的索引: "))
    #     target_index = int(input("请输入要选择的目标图像格式的索引: "))
    #
    #     # 设置用户选择的原始格式和目标格式
    #     ImageFormats.set_image_formats(camera, source_index, target_index)
    #
    # #查看ReportPart和ReportLevel枚举值
    # DebugReport.test_debug_report_support(camera)


   # # 检测异常类
   #  try:
   #      dvpException = dvp.dvpException()  # 继承自Exception类
   #      CheckClass.check_method_property(dvpException,False)
   #  except Exception as e:
   #      print("异常类的实例错误信息",e)

   # 检测调试信息报告，功能模块类
   #  try:
   #     ReportPart = dvp.ReportPart()  # 继承自Exception类
   #     CheckClass.check_method_property(ReportPart,False)
   #  except Exception as e:
   #     print("调试信息报告，功能模块类的实例错误信息",e)

   # 检测调试信息报告的级别
   #  try:
   #     ReportLevel = dvp.ReportLevel()  # 继承自Exception类
   #     CheckClass.check_method_property(ReportLevel,False)
   #  except Exception as e:
   #     print("调试信息报告的级别的实例错误信息", e)

   # # 检测CurveLut
   #  try:
   #      CurveLut = dvp.CurveLut()
   #      CheckClass.check_method_property(CurveLut,False)
   #  except Exception as e:
   #      print("curvelut的实例错误信息",e)


    """
    设置Lut，但是array有问题
    """
    # camera.CurveLut = CurveLut.create_default_curve_lut()
    # print(camera.CurveLut)

    """
    获取设备温度
    """
    DeviceTemperature = camera.GetFloatValue("DeviceTemperature")
    print(DeviceTemperature)


    """
    设置roi和获取Roi描述信息
    """

    # print("Get ROI before setting ROI:",camera.Roi)
    # roi = dvp.Region()
    # roi.X = 0
    # roi.Y = 0
    # roi.H = camera.RoiDescr.iMaxH  # 通过获取Roi描述信息，设置最大Roi
    # roi.W = camera.RoiDescr.iMaxW
    # camera.Roi = roi
    # print("Get ROI after setting ROI:",camera.Roi)
    #
    # print("Get the RoiDescr",camera.RoiDescr)

    # def stream_event_callback(pBuffer,W):   # 里面的参数可以是任意的，应该是每一帧数据的首地址
    #     print("Enter Callback")
    #     print(camera.FrameCount)
    #     print(camera.FunctionInfo)
    #     return 1


    # # Flip
    # print("current FlatFieldState status ",camera.FlipHorizontalState)
    # camera.FlipHorizontalState = True
    # print("after setting ",camera.FlipHorizontalState)
    # print("current FlipVerticalState status ", camera.FlipVerticalState)
    # camera.FlipVerticalState = True
    # print("current FlipVerticalState status ", camera.FlipVerticalState)
    #
    # # camera.FunctionInfo
    # print("相机功能的描述信息：",camera.FunctionInfo)
    #
    # # InverseState 负片使能
    # print("current Inverse：",camera.InverseState)
    # camera.InverseState = True
    # print("after setting：", camera.InverseState)
    #
    # # MonoState  去色使能
    # print("current MonoState：",camera.MonoState)
    # camera.MonoState = True
    # print("after setting:",camera.MonoState)



    # 注册视频流事件回调
    #Status = camera.RegisterStreamCallback(stream_event_callback, dvp.StreamEvent.STREAM_EVENT_FRAME_THREAD)
    #print(Status)


    # 锐度和白平衡操作
    # print("current sharpness:", camera.Sharpness)
    # camera.SharpnessState = True
    # camera.Sharpness = 70
    # print("after setting sharpness:", camera.Sharpness)
    # print("current:", camera.AwbOperation)
    #
    # camera.AwbOperation = dvp.AwbOperation.AWB_OP_ONCE  #AWB_OP_OFF，AWB_OP_CONTINUOUS
    # print("after:", camera.AwbOperation)


    """
    Buffer配置
    """
    # print("BufferQueueSizeDescr:",camera.BufferQueueSizeDescr)
    # print("BufferQueueSize:",camera.BufferQueueSize)
    # a,b = camera.AcquisitionBufferModeDescr
    # print("AcquisitionBufferModeDescr:",a,b)
    # print("AcquisitionBufferMode:", camera.AcquisitionBufferMode)
    # print("BufferConfig:",camera.BufferConfig)
    # print("BufferQueueSize:",camera.BufferQueueSize)
    # print("BufferQueueSizeDescr:",camera.BufferQueueSizeDescr)

    """
    获取用户IO有效标志
    """
    #print(camera.UserIoInfo)

    """
    获取sensor信息
    """
    #print(camera.SensorInfo)


    """
    获取当前引脚电平（camera.Line）状态的回读或者自己选择引脚。把strobe驱动设置为disable，回读到LINE_2为False
    """
    # print("a",camera.GetBoolValue("LINE_2"))
    # print(camera.Line)
    # print(camera.GetBoolValue("LineStatus"))

    """
    获取温度信息
    """

    #print(camera.TemperatureInfo)
    camera.Dialog()
    camera.ColorTemperature = 7000
    print( camera.ColorTemperature)
    camera.AeTarget = 90
    print(camera.AeTarget)



    Status = camera.Start()
    print(Status)
    # cv2.namedWindow("Image", cv2.WINDOW_NORMAL)  # 创建一个 OpenCV 窗口（即使不显示图像）
    # cv2.resizeWindow("Image", 800, 600)





    #print("Press ESC to stop the video stream.")

    # while True:
    #     key = cv2.waitKey() & 0xFF  # 获取键盘输入;每1ms检测1次；0xFF 是为了兼容不同系统
    #     try:
    #         print(camera.FrameCount)  # 打印帧统计信息
    #         frame = camera.GetFrame(4000)  # 从相机采集图像数据，超时时间为4000毫秒
    #         print(frame)
    #     except dvp.Exception as e:
    #         print(u"采集图像数据失败:", e.Status)
    #         if (e.Status == Status.DVP_STATUS_TIME_OUT):
    #             continue  # 如果只是超时错误，则继续采集
    #         break
    #     if key == 27:  #  ESC 键的 ASCII 码是 27
    #         Status = camera.Stop()
    #         print(Status)
    #         break



    """
    一、帧信息
    enum dvpImageFormat format = FORMAT_BGR24(10);  // 图像格式为BGR24，即每个像素3字节(B,G,R)；在内存种按B8+G8+R8这种排列
    enum dvpBits bits = BITS_8(0);                 // 每个通道8位(3（通道数） * 8 / 像素)
    dvpUint32 uBytes = 15040512;                   // 图像数据总字节数(2448×2048×3=15040512)
    dvpInt32 iWidth = 2448;                        // 图像宽度2448像素
    dvpInt32 iHeight = 2048;                       // 图像高度2048像素
    dvpUint64 uFrameID = 13;                       // 帧ID为13
    dvpUint64 uTimestamp = 4400130120;             // 时间戳(单位可能是微秒或纳秒)
    double fExposure = 10000;                      // 曝光时间10000(单位可能是微秒)
    float fAGain = 1;                              // 模拟增益为1(无增益)
    enum dvpFirstPosition position = UP_LEFT(0);    // 图像起始位置在左上角
    bool bFlipHorizontalState = 0;                 // 无水平翻转
    bool bFlipVerticalState = 0;                   // 无垂直翻转
    bool bRotateState = 0;                         // 无旋转
    bool bRotateOpposite = 0;                      // 无反向旋转
    dvpUint32 internalFlags = 67108864;            // 内部标志位
    dvpUint32 internalValue = 0;                   // 内部值
    dvpUint64 uTriggerId = 1;                      // 触发ID为1
    dvpUint64 userValue = 0;                       // 用户自定义值
    dvpUint64 pExtra = 0;                          // 额外指针(未使用)
    
    二、帧地址
    帧地址 <memory at 0x00000244FF841A80>
    这是Python中内存对象的表示，表示帧数据在内存中的位置：
    这是一个内存地址引用，指向存储实际图像数据的位置
    地址0x00000244FF841A80是这块内存的起始位置
    结合前面的uBytes=15040512，可以知道这块内存的大小是15040512字节
    
    三、帧信息
    """


    while True:
        try:
            frame,buffer = camera.GetFrame(4000)  # 解包元组
            # print("帧信息",frame)
            # print("帧地址",buffer)
            # print("帧计数",camera.FrameCount)

            # # 提取内存数据的前32字节（十六进制查看）
            # raw_header = bytes(buffer)[:32]
            # print("数据头32字节:", raw_header.hex(' '))
            #
            # # 判断12位数据存储方式
            # if frame.uBytes == frame.iWidth * frame.iHeight * 2:
            #     print("存储方式: 12位数据存储在16位中（高12位有效，低4位填充）")
            # elif frame.uBytes == frame.iWidth * frame.iHeight * 3 // 2:
            #     print("存储方式: 12位紧凑存储（每像素1.5字节）")
            # else:
            #     print("存储方式: 未知（可能是自定义打包格式）")

        except dvp.Exception as e:
            print("Acquire image data Failed:", e.Status)
            if e.Status == Status.DVP_STATUS_TIME_OUT:
                continue
            break



    print("1temperature",camera.ColorTemperature)


    # cv2.destroyAllWindows()
    camera.Stop()
    camera.Close()
    print("Camera closed.")




if __name__ == "__main__":
    main()
    input("\n程序执行完毕，按回车键退出...")