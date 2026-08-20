using OpenCvSharp;
using System;

class Program
{
    static void Main()
    {
        // Initialize the camera (index 1 + DirectShow backend)
        using (VideoCapture cap = new VideoCapture(1, VideoCaptureAPIs.DSHOW))
        {
            if (!cap.IsOpened())
            {
                Console.WriteLine("Failed to open the camera!");
                return;
            }

            using (Window window = new Window("Camera"))
            using (Mat frame = new Mat())
            {
                while (true)
                {
                    cap.Read(frame); // Read a frame
                    if (frame.Empty())
                    {
                        Console.WriteLine("Failed to capture frame");
                        break;
                    }

                    window.ShowImage(frame); // Display the frame
                    if (Cv2.WaitKey(1) == 'q') // Press 'Q' to quit
                        break;
                }
            }
        }
    }
}