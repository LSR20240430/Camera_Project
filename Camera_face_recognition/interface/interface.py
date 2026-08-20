import base64
import tkinter as tk
from tkinter import messagebox

import cv2

from face_recognition import FaceRecognizer


class CameraInterface:
    """Tkinter interface for live industrial-camera face detection."""

    def __init__(self, camera, recognizer=None):
        self.camera = camera
        self.recognizer = recognizer or FaceRecognizer()
        self.root = tk.Tk()
        self.root.title("OpenCV Face Recognition")
        self.root.geometry("960x720")
        self.root.protocol("WM_DELETE_WINDOW", self.close)
        self.image_label = tk.Label(self.root, bg="#1f2428")
        self.image_label.pack(fill=tk.BOTH, expand=True, padx=12, pady=12)
        self.status = tk.StringVar(value="Ready to start camera")
        tk.Label(self.root, textvariable=self.status, anchor=tk.W).pack(fill=tk.X, padx=12)
        tk.Button(self.root, text="Close", command=self.close).pack(anchor=tk.E, padx=12, pady=8)
        self.photo = None
        self.running = False

    def start(self):
        try:
            self.camera.start_camera()
            self.running = True
            self.status.set("Detecting faces")
            self.update_frame()
            self.root.mainloop()
        except Exception as error:
            messagebox.showerror("Startup failed", str(error))
            self.close()

    def update_frame(self):
        if not self.running:
            return
        try:
            frame = self.camera.get_single_frame()
            if frame is None:
                self.status.set("Frame capture failed: unsupported image format")
                self.root.after(30, self.update_frame)
                return
            annotated_frame, face_count = self.recognizer.annotate(frame)
            rgb_frame = cv2.cvtColor(annotated_frame, cv2.COLOR_BGR2RGB)
            success, encoded = cv2.imencode(".png", rgb_frame)
            if success:
                image_data = base64.b64encode(encoded.tobytes()).decode("ascii")
                self.photo = tk.PhotoImage(data=image_data)
                self.image_label.configure(image=self.photo)
                self.status.set(f"Faces detected: {face_count}")
        except Exception as error:
            self.status.set(f"Frame capture failed: {error}")
        self.root.after(30, self.update_frame)

    def close(self):
        self.running = False
        try:
            self.camera.close_camera()
        finally:
            self.root.destroy()