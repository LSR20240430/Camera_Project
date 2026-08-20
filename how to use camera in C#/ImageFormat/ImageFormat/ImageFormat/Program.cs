using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using DVPCameraType;
using System.ComponentModel;
using System.Net.NetworkInformation;

namespace ConsoleApp2
{
    internal class Program
    {
        static void Main()
        {
            try
            {
                dvpCameraInfo[] cameraList = CameraHandler.GetCameraList();
                if (cameraList == null || cameraList.Length == 0)
                {
                    Console.WriteLine("未检测到相机！");
                    return;
                }

                CameraHandler.CaptureFrames(cameraList);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"程序崩溃: {ex}");
            }
            finally
            {
                Console.WriteLine("按任意键退出...");
                Console.ReadKey();
            }
        }
    }

    public static class CameraHandler
    {
        private static readonly object _frameLock = new object();
        private static dvpFrame _currentFrame;
        private static IntPtr _currentFrameBuffer;
        private static bool _frameReady = false;
        private static bool _isCapturing = false;
        private static int _capturedCount = 0;
        private const int MAX_CAPTURE_COUNT = 10;

        // LUT曲线设置（完整实现）
        public static void SetLutCurve(uint cameraHandle)
        {
            Console.WriteLine("请选择LUT曲线：按1使用第一组，按2使用第二组");
            string input = Console.ReadLine();

            dvpCurveLut newLut = new dvpCurveLut
            {
                enable = true,
                style = dvpCurveStyle.CURVE_STYLE_LINE,
                array = new dvpCurveArray
                {
                    point = new dvpPoint3c { matrix = new dvpPoint2f[768] },
                    rows = 3,
                    cols = new uint[3] { 256, 256, 256 }
                }
            };

            dvpPoint2f[] lut1 = {
                new dvpPoint2f { X = 0, Y = 0 },
                new dvpPoint2f { X = 0.0930736f, Y = 0.913462f },
                new dvpPoint2f { X = 0.12987f, Y = 0.948077f },
                new dvpPoint2f { X = 0.9960938f, Y = 0.9960938f }
            };

            dvpPoint2f[] lut2 = {
                new dvpPoint2f { X = 0, Y = 0 },
                new dvpPoint2f { X = 0.558442f, Y = 0.0307692f },
                new dvpPoint2f { X = 0.915584f, Y = 0.149688f },
                new dvpPoint2f { X = 0.9960938f, Y = 0.9960938f }
            };

            if (input == "1") Array.Copy(lut1, newLut.array.point.matrix, lut1.Length);
            else if (input == "2") Array.Copy(lut2, newLut.array.point.matrix, lut2.Length);
            else
            {
                Console.WriteLine("输入无效！");
                return;
            }

            dvpStatus status = DVPCamera.dvpSetCurveLut(cameraHandle, newLut);
            Console.WriteLine(status == dvpStatus.DVP_STATUS_OK ?
                "LUT曲线设置成功！" : "LUT曲线设置失败！");
        }

        // 获取相机列表（完整实现）
        public static dvpCameraInfo[] GetCameraList()
        {
            uint cameraCount = 0;
            dvpStatus status = DVPCamera.dvpRefresh(ref cameraCount);
            if (status != dvpStatus.DVP_STATUS_OK || cameraCount == 0)
            {
                Console.WriteLine("未检测到可用相机！");
                return null;
            }

            dvpCameraInfo[] cameras = new dvpCameraInfo[cameraCount];
            for (uint i = 0; i < cameraCount; i++)
            {
                status = DVPCamera.dvpEnum(i, ref cameras[i]);
                if (status == dvpStatus.DVP_STATUS_OK)
                {
                    Console.WriteLine($"{i} -> {cameras[i].FriendlyName} (ID: {cameras[i].UserID})");
                }
            }
            return cameras;
        }

        // 打印帧信息（完整实现）
        private static void PrintFrameInfo(ref dvpFrame frame)
        {
            Console.WriteLine("\n=== 帧详细信息 ===");
            Console.WriteLine($"格式: {frame.format}");
            Console.WriteLine($"位宽: {frame.bits}");
            Console.WriteLine($"字节数: {frame.uBytes}");
            Console.WriteLine($"分辨率: {frame.iWidth}x{frame.iHeight}");
            Console.WriteLine($"帧编号: {frame.uFrameID}");
            Console.WriteLine($"时间戳: {frame.uTimestamp}");
            Console.WriteLine($"曝光时间(μs): {frame.fExposure}");
            Console.WriteLine($"模拟增益: {frame.fAGain}");
            Console.WriteLine($"第一个像素点位置: {frame.position}");
            Console.WriteLine($"水平翻转: {frame.bFlipHorizontalState}");
            Console.WriteLine($"垂直翻转: {frame.bFlipVerticalState}");
            Console.WriteLine($"旋转90度: {frame.bRotateState}");
            Console.WriteLine($"逆时针旋转: {frame.bRotateOpposite}");
            Console.WriteLine($"内部标志位: {frame.internalFlags}");
            Console.WriteLine($"内部信息: {frame.internalValue}");
            Console.WriteLine($"触发ID: {frame.uTriggerId}");
            Console.WriteLine($"用户定制数据: {frame.userValue}");
            Console.WriteLine("=================\n");
        }

        // 帧回调函数（完整实现）
        private static int FrameCallback(uint handle, dvpStreamEvent _event,
    IntPtr pContext, ref dvpFrame frame, IntPtr pBuffer)
        {
            if (!_isCapturing) return 0;

            try
            {
                int expectedBytes = frame.iWidth * frame.iHeight * 2; // 16位/像素
                if (frame.uBytes != expectedBytes)
                {
                    Console.WriteLine($"字节数自动修正: {frame.uBytes}->{expectedBytes}");
                    frame.uBytes = (uint)expectedBytes;
                }

                // 新增代码：将非托管内存复制到托管数组
                byte[] pixelData = new byte[expectedBytes];
                Marshal.Copy(pBuffer, pixelData, 0, expectedBytes);
                // 这里可以添加对pixelData的处理代码
                ushort[] ushortArray = Array.ConvertAll(pixelData, s => (ushort)s);


                lock (_frameLock)
                {
                    _currentFrame = frame;
                    _currentFrameBuffer = pBuffer;
                    _frameReady = true;

                    Console.WriteLine($"收到帧 {frame.uFrameID} - {frame.iWidth}x{frame.iHeight}");
                    PrintFrameInfo(ref frame);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"回调处理异常: {ex.Message}");
            }
            return 0;
        }

        // 获取源格式详细信息（完整实现）
        public static void GetSourceFormatDetails(uint cameraHandle)
        {
            dvpSelectionDescr descr = new dvpSelectionDescr();
            if (DVPCamera.dvpGetSourceFormatSelDescr(cameraHandle, ref descr) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("获取源格式描述失败！");
                return;
            }

            Console.WriteLine($"\n[源格式信息] 总数: {descr.uCount} 默认: {descr.uDefault} 需停止: {descr.bNeedStop == 1}");

            for (uint i = 0; i < descr.uCount; i++)
            {
                dvpFormatSelection detail = new dvpFormatSelection();
                if (DVPCamera.dvpGetSourceFormatSelDetail(cameraHandle, i, ref detail) == dvpStatus.DVP_STATUS_OK)
                {
                    Console.WriteLine($"[{i}] {detail.format} | {detail.selection._string} | 索引: {detail.selection.iIndex}" +
                        $"{(i == descr.uDefault ? " (默认)" : "")}");
                }
            }
        }

        // 设置源格式（完整实现）
        public static bool SetSourceFormat(uint cameraHandle, uint formatIndex)
        {
            dvpSelectionDescr descr = new dvpSelectionDescr();
            if (DVPCamera.dvpGetSourceFormatSelDescr(cameraHandle, ref descr) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("获取源格式信息失败！");
                return false;
            }

            if (formatIndex >= descr.uCount)
            {
                Console.WriteLine($"无效索引！范围: 0-{descr.uCount - 1}");
                return false;
            }

            dvpFormatSelection target = new dvpFormatSelection();
            if (DVPCamera.dvpGetSourceFormatSelDetail(cameraHandle, formatIndex, ref target) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("获取目标格式详情失败！");
                return false;
            }

            Console.WriteLine($"\n将要切换为: {target.format} | {target.selection._string}");

            bool wasCapturing = _isCapturing;
            if (descr.bNeedStop == 1 && _isCapturing)
            {
                Console.WriteLine("正在停止视频流...");
                DVPCamera.dvpStop(cameraHandle);
                _isCapturing = false;
            }

            dvpStatus status = DVPCamera.dvpSetSourceFormatSel(cameraHandle, formatIndex);
            if (status != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine($"设置失败！错误: {status}");
                return false;
            }

            Console.WriteLine("源格式设置成功！");

            if (wasCapturing && descr.bNeedStop == 1)
            {
                Console.WriteLine("重新启动视频流...");
                status = DVPCamera.dvpStart(cameraHandle);
                _isCapturing = status == dvpStatus.DVP_STATUS_OK;
                if (!_isCapturing) Console.WriteLine("警告：流重启失败！");
            }

            return true;
        }

        // 获取当前目标格式（完整实现）
        public static uint GetCurrentTargetFormat(uint cameraHandle)
        {
            uint currentFormat = 0;
            dvpStatus status = DVPCamera.dvpGetTargetFormatSel(cameraHandle, ref currentFormat);
            if (status == dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine($"当前目标格式索引: {currentFormat}");
            }
            else
            {
                Console.WriteLine("获取目标格式失败！");
            }
            return currentFormat;
        }

        // 获取目标格式详细信息（完整实现）
        public static void GetTargetFormatDetails(uint cameraHandle)
        {
            dvpSelectionDescr descr = new dvpSelectionDescr();
            dvpStatus status = DVPCamera.dvpGetTargetFormatSelDescr(cameraHandle, ref descr);

            if (status != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("获取目标格式描述失败！");
                return;
            }

            uint currentFormat = GetCurrentTargetFormat(cameraHandle);

            Console.WriteLine($"\n[目标格式信息] 总数: {descr.uCount} 默认: {descr.uDefault} 需停止: {descr.bNeedStop == 1}");

            for (uint i = 0; i < descr.uCount; i++)
            {
                dvpFormatSelection detail = new dvpFormatSelection();
                status = DVPCamera.dvpGetTargetFormatSelDetail(cameraHandle, i, ref detail);

                if (status == dvpStatus.DVP_STATUS_OK)
                {
                    string currentFlag = (i == currentFormat) ? " [当前]" : "";
                    string defaultFlag = (i == descr.uDefault) ? " (默认)" : "";
                    Console.WriteLine($"[{i}]{currentFlag}{defaultFlag} {detail.format} | {detail.selection._string}");
                }
            }
        }

        // 设置目标格式（完整实现）
        public static bool SetTargetFormat(uint cameraHandle, uint formatIndex)
        {
            dvpSelectionDescr descr = new dvpSelectionDescr();
            if (DVPCamera.dvpGetTargetFormatSelDescr(cameraHandle, ref descr) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("获取目标格式描述失败！");
                return false;
            }

            if (formatIndex >= descr.uCount)
            {
                Console.WriteLine($"无效索引！允许范围: 0-{descr.uCount - 1}");
                return false;
            }

            dvpFormatSelection targetFormat = new dvpFormatSelection();
            if (DVPCamera.dvpGetTargetFormatSelDetail(cameraHandle, formatIndex, ref targetFormat) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("获取目标格式详情失败！");
                return false;
            }

            Console.WriteLine($"\n将要设置为: {targetFormat.format} | {targetFormat.selection._string}");

            bool wasCapturing = _isCapturing;
            if (descr.bNeedStop == 1 && _isCapturing)
            {
                Console.WriteLine("正在停止视频流...");
                DVPCamera.dvpStop(cameraHandle);
                _isCapturing = false;
            }

            dvpStatus status = DVPCamera.dvpSetTargetFormatSel(cameraHandle, formatIndex);
            if (status != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine($"设置失败！错误代码: {status}");
                return false;
            }

            Console.WriteLine("目标格式设置成功！");

            if (wasCapturing && descr.bNeedStop == 1)
            {
                Console.WriteLine("重新启动视频流...");
                status = DVPCamera.dvpStart(cameraHandle);
                _isCapturing = status == dvpStatus.DVP_STATUS_OK;
                if (!_isCapturing) Console.WriteLine("警告：视频流重启失败！");
            }

            return true;
        }

        // 主捕获流程（完整实现）
        public static void CaptureFrames(dvpCameraInfo[] cameras)
        {
            // 相机选择
            Console.Write("\n输入相机索引：");
            if (!uint.TryParse(Console.ReadLine(), out uint index) || index >= cameras.Length)
            {
                Console.WriteLine("无效索引！");
                return;
            }

            // 初始化相机
            uint handle = 0;
            if (DVPCamera.dvpOpenByName(cameras[index].FriendlyName, dvpOpenMode.OPEN_NORMAL, ref handle) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("打开相机失败！");
                return;
            }
            Console.WriteLine($"已打开: {cameras[index].FriendlyName}");

            // 显示格式信息
            GetSourceFormatDetails(handle);
            GetTargetFormatDetails(handle);

            // 源格式设置交互
            Console.Write("\n是否修改源格式？(y/n): ");
            if (Console.ReadLine().Trim().ToLower() == "y")
            {
                Console.Write("输入源格式索引: ");
                if (uint.TryParse(Console.ReadLine(), out uint sourceFormat))
                {
                    SetSourceFormat(handle, sourceFormat);
                    GetSourceFormatDetails(handle); // 再次显示确认
                }
            }

            // 目标格式设置交互
            Console.Write("\n是否修改目标格式？(y/n): ");
            if (Console.ReadLine().Trim().ToLower() == "y")
            {
                Console.Write("输入目标格式索引: ");
                if (uint.TryParse(Console.ReadLine(), out uint targetFormat))
                {
                    SetTargetFormat(handle, targetFormat);
                    GetTargetFormatDetails(handle); // 再次显示确认
                }
            }

            // LUT设置交互
            Console.Write("\n是否配置LUT曲线？(y/n): ");
            if (Console.ReadLine().Trim().ToLower() == "y")
            {
                SetLutCurve(handle);
            }

            // 注册回调
            var callback = new DVPCamera.dvpStreamCallback(FrameCallback);
            if (DVPCamera.dvpRegisterStreamCallback(handle, callback,
                dvpStreamEvent.STREAM_EVENT_FRAME_THREAD, IntPtr.Zero) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("回调注册失败！");
                DVPCamera.dvpClose(handle);
                return;
            }

            // 创建保存目录
            string saveDir = Path.Combine(Directory.GetCurrentDirectory(), "Captures");
            Directory.CreateDirectory(saveDir);

            // 开始捕获
            Console.WriteLine($"\n开始捕获 {MAX_CAPTURE_COUNT} 帧...");
            if (DVPCamera.dvpStart(handle) != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("启动失败！");
                DVPCamera.dvpClose(handle);
                return;
            }
            _isCapturing = true;

            // 主捕获循环
            while (_capturedCount < MAX_CAPTURE_COUNT)
            {
                bool hasFrame = false;
                dvpFrame frame = new dvpFrame();
                IntPtr bufferPtr = IntPtr.Zero;

                lock (_frameLock)
                {
                    if (_frameReady)
                    {
                        frame = _currentFrame;
                        bufferPtr = _currentFrameBuffer;
                        hasFrame = true;
                        _frameReady = false;
                    }
                }

                if (hasFrame && bufferPtr != IntPtr.Zero)
                {
                    string path = Path.Combine(saveDir, $"frame_{_capturedCount:D4}.jpg");

                    try
                    {
                        // 保存图像（使用原始指针）
                        dvpStatus status = DVPCamera.dvpSavePicture(ref frame, bufferPtr, path, 100);

                        if (status == dvpStatus.DVP_STATUS_OK)
                        {
                            Console.WriteLine($"成功保存: {Path.GetFileName(path)}");
                            _capturedCount++;
                        }
                        else
                        {
                            Console.WriteLine($"保存失败! 错误码: {status}");
                        }
                    }
                    catch (AccessViolationException ex)
                    {
                        Console.WriteLine($"内存访问冲突: {ex.Message}");
                        // 可以尝试重新初始化相机连接
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"捕获异常: {ex.Message}");
                    }
                }
                Thread.Sleep(10);
            }

            // 清理资源
            _isCapturing = false;
            DVPCamera.dvpStop(handle);
            DVPCamera.dvpClose(handle);
            Console.WriteLine("\n捕获完成！");
        }

        // 内存安全检查（通过P/Invoke）
        [DllImport("kernel32.dll")]
        private static extern bool IsBadReadPtr(IntPtr lp, int ucb);
    }
}