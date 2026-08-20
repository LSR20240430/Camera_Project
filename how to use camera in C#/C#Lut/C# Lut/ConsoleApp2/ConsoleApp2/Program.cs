/*
 You can view the LUT effect in DemoCam3 and the saved images.
 
 */


using System;
using System.IO;
using DVPCameraType;

namespace ConsoleApp2
{
    internal class Program
    {
        static void Main()
        {
            dvpCameraInfo[] cameraList = CameraHandler.GetCameraList();
            if (cameraList == null || cameraList.Length == 0)
            {
                Console.WriteLine("No camera detected!");
                return;
            }

            CameraHandler.CaptureFrames(cameraList);
        }
    }

    public static class CameraHandler
    {
        //public static void SetLutCurve(uint cameraHandle)
        //{
        //    Console.WriteLine("Please select a LUT curve: Press 1 for the first set, press 2 for the second set");
        //    string input = Console.ReadLine();

        //    dvpCurveLut newLut = new dvpCurveLut
        //    {
        //        enable = true,
        //        style = dvpCurveStyle.CURVE_STYLE_LINE,
        //        array = new dvpCurveArray
        //        {
        //            point = new dvpPoint3c
        //            {
        //                matrix = new dvpPoint2f[768]
        //            },
        //            rows = 3,
        //            cols = new uint[3] { 256, 256, 256 }
        //        }
        //    };

        //    dvpPoint2f[] lut1 = new dvpPoint2f[]
        //    {
        //        new dvpPoint2f { X = 0, Y = 0 },
        //        new dvpPoint2f { X = 0.0930736f, Y = 0.913462f },
        //        new dvpPoint2f { X = 0.12987f, Y = 0.948077f },
        //        new dvpPoint2f { X = 0.9960938f, Y = 0.9960938f }
        //    };

        //    dvpPoint2f[] lut2 = new dvpPoint2f[]
        //    {
        //        new dvpPoint2f { X = 0, Y = 0 },
        //        new dvpPoint2f { X = 0.558442f, Y = 0.0307692f },
        //        new dvpPoint2f { X = 0.915584f, Y = 0.149688f },
        //        new dvpPoint2f { X = 0.9960938f, Y = 0.9960938f }
        //    };

        //    if (input == "1")
        //    {
        //        Array.Copy(lut1, 0, newLut.array.point.matrix, 0, lut1.Length);
        //    }
        //    else if (input == "2")
        //    {
        //        Array.Copy(lut2, 0, newLut.array.point.matrix, 0, lut2.Length);
        //    }
        //    else
        //    {
        //        Console.WriteLine("Invalid input!");
        //        return;
        //    }

        //    dvpStatus status = DVPCamera.dvpSetCurveLut(cameraHandle, newLut);
        //    if (status == dvpStatus.DVP_STATUS_OK)
        //    {
        //        Console.WriteLine("LUT curve set successfully!");
        //    }
        //    else
        //    {
        //        Console.WriteLine("Failed to set LUT curve!");
        //        return;
        //    }
        //}



        public static dvpCameraInfo[] GetCameraList()
        {
            uint cameraCount = 0;
            dvpStatus status = DVPCamera.dvpRefresh(ref cameraCount);
            if (status != dvpStatus.DVP_STATUS_OK || cameraCount == 0)
            {
                return null;
            }

            dvpCameraInfo[] cameraList = new dvpCameraInfo[cameraCount];
            for (uint i = 0; i < cameraCount; i++)
            {
                dvpCameraInfo devInfo = new dvpCameraInfo();
                status = DVPCamera.dvpEnum(i, ref devInfo);
                if (status == dvpStatus.DVP_STATUS_OK)
                {
                    cameraList[i] = devInfo;
                    Console.WriteLine($"{i} -> {devInfo.FriendlyName}@{devInfo.UserID}");
                }
            }

            return cameraList;
        }

        public static void CaptureFrames(dvpCameraInfo[] cameraList)
        {
            Console.WriteLine("\nEnter the camera index you want to open:");
            if (!uint.TryParse(Console.ReadLine(), out uint selectedIndex) || selectedIndex >= cameraList.Length)
            {
                Console.WriteLine("Invalid index!");
                return;
            }

            uint cameraHandle = 0;
            dvpStatus status = DVPCamera.dvpOpenByName(cameraList[selectedIndex].FriendlyName, dvpOpenMode.OPEN_NORMAL, ref cameraHandle);
            if (status != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("Failed to open camera!");
                return;
            }
            Console.WriteLine("Camera opened successfully!");

            //SetLutCurve(cameraHandle);

            /*
            status = DVPCamera.dvpSetAeMode(cameraHandle, dvpAeMode.AE_MODE_AE_AG);
            status = DVPCamera.dvpSetAeOperation(cameraHandle, dvpAeOperation.AE_OP_CONTINUOUS);

            status = DVPCamera.dvpSetAeTarget(cameraHandle, 20);
            
             // auto exposure
             */


            status = DVPCamera.dvpStart(cameraHandle);
            if (status != dvpStatus.DVP_STATUS_OK)
            {
                Console.WriteLine("Failed to start camera stream!");
                DVPCamera.dvpClose(cameraHandle);
                return;
            }
            Console.WriteLine("Starting frame capture...");

            string picFolder = Path.Combine(Directory.GetCurrentDirectory(), "pic");
            if (!Directory.Exists(picFolder))
            {
                Directory.CreateDirectory(picFolder);
            }

            int grabCount = 10;
            for (int i = 0; i < grabCount; i++)
            {
                dvpFrame frame = new dvpFrame();
                IntPtr pFrame = IntPtr.Zero;
                status = DVPCamera.dvpGetFrame(cameraHandle, ref frame, ref pFrame, 4000);
                if (status == dvpStatus.DVP_STATUS_OK)
                {
                    Console.WriteLine($"Timestamp: {frame.uTimestamp}");

                    string fileName = Path.Combine(picFolder, $"image_{i}.jpg");
                    status = DVPCamera.dvpSavePicture(ref frame, pFrame, fileName, 100);

                    if (status == dvpStatus.DVP_STATUS_OK)
                    {
                        Console.WriteLine($"Saved at: {fileName}");
                    }
                }
                else
                {
                    Console.WriteLine("Failed to get frame!");
                    break;
                }
            }

            status = DVPCamera.dvpStop(cameraHandle);
            status = DVPCamera.dvpClose(cameraHandle);
        }
    }
}