# Industrial Camera Face Recognition

This project captures images from an industrial camera, converts the camera frames into OpenCV images, detects faces, and displays the result in a live desktop window.

## Features

- Enumerates connected industrial cameras through the `dvp` SDK.
- Opens and configures a selected camera.
- Uses manual exposure control. The default exposure is `30000` microseconds.
- Converts camera frame buffers into BGR OpenCV images.
- Detects faces with OpenCV's Haar cascade classifier.
- Draws a rectangle around every detected face.
- Displays the live result in a Tkinter interface.
- Shows the current number of detected faces.
- Stops acquisition and closes the camera when the window is closed.

## Requirements

- Windows
- Python 3.13
- A supported industrial camera and its DVP driver/SDK
- NumPy
- OpenCV
- Tkinter, normally included with the Windows Python distribution

The project was tested with the following environment:

```text
D:/Software/Miniconda/envs/py313_camera/python.exe
```

Install the Python packages in that environment:

```powershell
D:/Software/Miniconda/envs/py313_camera/python.exe -m pip install numpy opencv-contrib-python
```

The face model is stored locally at:

```text
face_recognition/haarcascade_frontalface_default.xml
```

The `dvp` package is provided by the industrial-camera SDK and may need to be installed or added to the Python path separately.

## Project Structure

```text
Camera_/
├── main.py
├── README.md
├── camera/
│   ├── __init__.py
│   └── camera.py
├── image_process/
│   ├── __init__.py
│   └── image_process.py
├── face_recognition/
│   ├── __init__.py
│   ├── face_recognizer.py
│   └── haarcascade_frontalface_default.xml
├── interface/
│   ├── __init__.py
│   └── interface.py
└── save_picture/
```

## How to Run

Open a terminal in the project root directory and run:

```powershell
D:/Software/Miniconda/envs/py313_camera/python.exe main.py
```

The application will:

1. Enumerate available industrial cameras.
2. Open camera index `0`.
3. Disable automatic exposure and set the exposure to `30000` microseconds.
4. Start continuous image acquisition.
5. Convert each frame to an OpenCV BGR image.
6. Detect and annotate faces.
7. Show the annotated frame in the graphical interface.

Close the window with the **Close** button to stop acquisition and release the camera.

## Adjusting Camera Settings

The camera is created in `main.py`:

```python
cam1 = camera.Camera(0, 100, exposure=30000)
```

- The first argument is the camera index.
- The second argument is the frame count used by the legacy OpenCV capture method.
- `exposure` is the manual exposure time in microseconds.

For a brighter image, increase the exposure value. For example:

```python
cam1 = camera.Camera(0, 100, exposure=50000)
```

A higher exposure can make moving objects more blurred. The best value depends on the lighting and camera model.

## Processing Flow

```text
Industrial camera
       |
       v
DVP frame info + frame buffer
       |
       v
ImageProcess.format_to_OpenCV()
       |
       v
OpenCV BGR NumPy image
       |
       v
FaceRecognizer.detect()
       |
       v
FaceRecognizer.annotate()
       |
       v
Tkinter live interface
```

## Important Notes

- The camera must be connected and recognized by the DVP driver before starting the program.
- Camera index `0` may need to be changed when multiple cameras are connected.
- If the image is too dark, increase the manual exposure or improve the lighting.
- Haar cascade detection is a basic face-detection method, not a face identity-recognition system. It detects faces but does not identify people.
- The current interface processes frames on the Tkinter event loop. For higher frame rates, a worker thread and a frame queue can be added later.
