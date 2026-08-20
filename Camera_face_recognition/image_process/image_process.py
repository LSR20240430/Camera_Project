# image_process/image_process.py
import cv2
import numpy as np

class ImageProcess:
    """Image processing class"""
    
    def format_to_OpenCV(self, frame_info, frame_buffer):
        """
        Convert DVP frame to OpenCV format
        
        Args:
            frame_info: DVP frame info object
            frame_buffer: DVP frame buffer (memoryview)
            
        Returns:
            numpy.ndarray: OpenCV format image, or None if failed
        """
        try:
            height = frame_info.iHeight
            width = frame_info.iWidth
            pixels = height * width
            buffer = np.frombuffer(frame_buffer, dtype=np.uint8)

            if buffer.size == pixels * 3:
                return buffer.reshape((height, width, 3))
            if buffer.size == pixels * 4:
                image = buffer.reshape((height, width, 4))
                return cv2.cvtColor(image, cv2.COLOR_BGRA2BGR)
            if buffer.size == pixels:
                image = buffer.reshape((height, width))
                return cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)

            print(
                f"Unsupported image buffer: format={frame_info.format}, "
                f"bytes={buffer.size}, expected pixels={pixels}"
            )
            return None
                
        except Exception as e:
            print(f"Format conversion failed: {e}")
            return None
    
    def show(self, img, window_name="Camera"):
        """Display image"""
        if img is not None:
            cv2.imshow(window_name, img)
    
    def wait_key(self, delay=1):
        """Wait for key press"""
        return cv2.waitKey(delay)
    
    def close_windows(self):
        """Close all OpenCV windows"""
        cv2.destroyAllWindows()