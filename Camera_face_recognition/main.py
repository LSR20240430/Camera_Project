
from camera import camera
from face_recognition import FaceRecognizer
from interface import CameraInterface








def main():
    


    camera.Camera.enumerate_cameras()

    cam1 = camera.Camera(0, 100, exposure=30000)
    cam1.open_camera()
    #cam1.camera_default() 
    cam1.set_camera_params()
    user_interface = CameraInterface(cam1, FaceRecognizer())
    user_interface.start()


if __name__ == "__main__":
    main()