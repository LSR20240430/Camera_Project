import dvp
import time

from image_process import image_process


import cv2
import numpy as np


class Camera:
    
    def __init__(self, index, frame_count, exposure=30000):
       
        self.index = index
        self.frame_count = frame_count
        self.exposure = exposure
        self.camera_object = None
        self.acquisition_started = False

        # Create image processor object
        self.image_processor = image_process.ImageProcess()
        
    
    @classmethod
    def enumerate_cameras(cls):
        
        camera_info = dvp.Refresh() #Return [camera1_info_object1,camera_info_object2,...] if found
        #print(camera_info)
        if(len(camera_info) == 0):
          print("No devices found")
        else:
         for k,v in enumerate(camera_info):  #Return [(0,'camera1_info_object1'), (1,'camera1_info_object2')]
            print(k, "->", v.FriendlyName, "->", v.PortInfo) #Print index and camera name for each camera you have found


    def open_camera(self):
        try:
            self.camera_object = dvp.Camera(self.index) #Create and open a camera, return camera object
            print(self.camera_object) #camera object nearly has the same attributes as the camera_info_object.
    
            #print(type(self.camera_object)) #Return camera object
            
            
              
            print(f"Open successfully camera {self.index}")
            

        except dvp.dvpException as e:
            #If the camera driver inside has captured the dvpException, it will create an instance of dvp.dvpException. And it will be assigned to variable e in try-except statement.
            #e is a instance of dvp.dvpException class, and its attribute Status(e.Status) is the instance of dvp.Status class
            
            print(type(e),type(e.Status)) #Return dvpException
            print("Failed to open camera:", e.Status) #If it's a standard exception of DVP
        except BaseException as e:
            print("Invalid index:", self.index) #Other exceptions


    
    def set_camera_params(self):
        if self.camera_object != None:
            try:
                self.camera_object.TriggerState = False 
                self.camera_object.AeOperation = dvp.AeOperation.AE_OP_OFF
                self.camera_object.Exposure = self.exposure
                
                roiDescr = self.camera_object.RoiDescr
                roi = dvp.Region()  
                roi.x = 0
                roi.y = 0   
                roi.w = roiDescr.iMaxW
                roi.h = roiDescr.iMaxH
                print(self.camera_object.Roi)
                
                #self.camera_object.AcquisitionFrameRateEnable = True
                #self.camera_object.AcquisitionFrameRateValue = 15.0

                #self.camera_object.AeOperation = dvp.AeOperation.AE_OP_CONTINUOUS
                #self.camera_object.AeMode = dvp.AeMode.AE_MODE_AE_AG
                
                self.camera_object.AwbOperation = dvp.AwbOperation.AWB_OP_ONCE
                
                #self.camera_object.Dialog() 
                
                
                print(f"Set camera {self.index} parameters successfully")
            except dvp.dvpException as e:
                print("Failed to set camera parameters:", e.Status) #If it's a standard exception of DVP
        else:
            print("Camera is not opened yet. Please open the camera first.")
    

    def get_frame(self):
        if self.camera_object != None:
            try:
                self.camera_object.Start() #Ready to capture image
                for i in range(self.frame_count):
                    frame = self.camera_object.GetFrame(4000) #Get image data with timeout of 4000ms
                    
                    frame_info , frame_buffer = frame
                    self.print_frame_info(frame_info)

                  # time.sleep(4)  
                  # After Start, frames are stored in the hardware buffer and
                  # transferred to the host through GetFrame. Delayed reads drop frames.
                  #比如睡眠4s后，取的帧就是帧id为280的了，这就是没有及时取帧
                  # Reduce the camera frame rate, process frames faster, or use FIFO mode
                  # to reduce frame loss.
                          
                    #self.save_frame(frame_info, frame_buffer)

                    #print(type(frame_info)) #Return dvp.Frame
                    #print(type(frame_buffer)) #Return buffer memory address


                      # Convert to OpenCV format
                    img = self.image_processor.format_to_OpenCV(frame_info, frame_buffer)
                    
                    if img is not None:
                      # Display the image
                       self.image_processor.show(img)
                    
                      # Check for 'q' key to exit early
                       key = self.image_processor.wait_key(1)
                       if key == ord('q'):
                        print("User pressed 'q', exiting...")
                        break
            
            # Stop after the loop (only once)
                self.camera_object.Stop()
                self.image_processor.close_windows()




                   
                
            except dvp.dvpException as e:
                print("Failed to get frame:", e.Status) #If it's a standard exception of DVP.
        else:
            print("Camera is not opened yet. Please open the camera first.")

    def get_single_frame(self):
        """Capture one frame and return it as an OpenCV image."""
        if self.camera_object is None:
            raise RuntimeError("Camera is not opened yet")

        frame = self.camera_object.GetFrame(4000)
        frame_info, frame_buffer = frame
        return self.image_processor.format_to_OpenCV(frame_info, frame_buffer)

    def start_camera(self):
        """Start continuous acquisition for consumers such as a GUI."""
        if self.camera_object is None:
            raise RuntimeError("Camera is not opened yet")
        if not self.acquisition_started:
            self.camera_object.Start()
            self.acquisition_started = True

    @classmethod
    def format_to_OpenCV(cls, frame_info, frame_buffer):
         frame, buffer = frameBuffer
         bits = np.uint8 if(frame.bits == Bits.BITS_8) else np.uint16
         shape = None
         convertType = None
         if(frame.format >= ImageFormat.FORMAT_MONO and frame.format <= ImageFormat.FORMAT_BAYER_RG):
          shape = 1
         elif(frame.format == ImageFormat.FORMAT_BGR24 or frame.format == ImageFormat.FORMAT_RGB24):
          shape = 3
         elif(frame.format == ImageFormat.FORMAT_BGR32 or frame.format == ImageFormat.FORMAT_RGB32):
          shape = 4
         else:
          return None

         mat = np.frombuffer(buffer, bits)
         mat = mat.reshape(frame.iHeight, frame.iWidth, shape)   # Reshape the image.
         return mat




    

    def print_frame_info(self, frame_info):
        print(f"FrameID_{frame_info.uFrameID} FrameFormat_{frame_info.format} FrameBits_{frame_info.bits}")
        print(f"FrameBytes_{frame_info.uBytes}_bytes FrameTimestamp_{frame_info.uTimestamp}_us")
        print(f"FrameWidth_{frame_info.iWidth}_pixels FrameHeight_{frame_info.iHeight}_pixels")
        #Timestamp is in microseconds, and Timestamp between two frames can be used to calculate the actual frame rate.
        #uBytes is the actual size of the image data in bytes.
        #Under raw8 + BGR24,  UserLayer: iwidth * iheight * bits(8) * 3 / 8 == uBytes CameraLayer: don't need to multiply by 3, because the frame format is raw8 in camera, which was interpolated into bgr in driver layer.
        #Under raw12 + Mono12, UserLayer: iwidth + iheight * 16 * 1 / 8 == uBytes  CameraLayer: 16 need to change to 12. 
        
        #print(type(self.camera_object.FrameCount))
        print(f"FrameCount_{self.camera_object.FrameCount} ") 
        #Return the instance of dvp.FrameCount class, and the camera's attribute FrameCount is an instance of dvp.FrameCount class.
        
        #fProcRate is the frame rate between the previous GetFrame and the Current GetFrame, it show the frame rate that your program can process the frames. 
        #If fProcRate is much lower than the fFrameRate, it means your program is not fast enough to process the frames, 
        #and you may need to optimize your program or reduce the fFrameRate to avoid losing frames.
        print("_______________________________")

    
    def stop_camera(self):
        if self.camera_object != None:
            try:
                if self.acquisition_started:
                    self.camera_object.Stop() #Stop camera
                    self.acquisition_started = False
                print(f"Camera {self.index} stopped successfully")
            except dvp.dvpException as e:
                print("Failed to stop camera:", e.Status) #If it's a standard exception of DVP
        else:
            print("Camera is not opened yet. Please open the camera first.")
    

    def close_camera(self):
        if self.camera_object != None:
            try:
                self.stop_camera()
                self.camera_object.Close() #Close camera
                self.camera_object = None
                print(f"Camera {self.index} closed successfully")
            except dvp.dvpException as e:
                print("Failed to close camera:", e.Status) #If it's a standard exception of DVP
        else:
            print("Camera is not opened yet. Please open the camera first.")
    



    def save_frame(self, frame_info, frame_buffer):
        try:
            start = time.time()
            
            file_path = f"save_picture\\frame_{frame_info.uFrameID:04d}.bmp"
            dvp.SavePicture((frame_info, frame_buffer), file_path)
            print(f"Frame {frame_info.uFrameID} saved successfully at {file_path}")
            
            save_time = time.time() - start #Calculate the time taken to save the picture.
            
            print(f"The time taken to save the picture: {save_time*1000:.2f} ms")
        except dvp.dvpException as e:
            print("Failed to save frame:", e.Status) #If it's a standard exception of DVP

        
    def camera_default(self):
        if self.camera_object != None:
            try:
                self.camera_object.LoadDefault() #Reset camera parameters to default values
                print(f"Camera {self.index} reset to default parameters successfully")
            except dvp.dvpException as e:
                print("Failed to reset camera parameters to default:", e.Status) #If it's a standard exception of DVP       
        else:
            print("Camera is not opened yet. Please open the camera first.")
