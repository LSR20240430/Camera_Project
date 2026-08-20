from pathlib import Path

import cv2


class FaceRecognizer:
    """Detect faces in OpenCV BGR images using a Haar cascade."""

    def __init__(self, scale_factor=1.1, min_neighbors=5, min_size=(40, 40)):
        cascade_name = "haarcascade_frontalface_default.xml"
        candidate_paths = [Path(__file__).resolve().parent / cascade_name]
        if hasattr(cv2, "data"):
            candidate_paths.append(Path(cv2.data.haarcascades) / cascade_name)
        candidate_paths.append(Path(cv2.__file__).resolve().parent / "data" / cascade_name)
        cascade_path = next((path for path in candidate_paths if path.is_file()), None)
        if cascade_path is None:
            raise RuntimeError(
                "Face model not found. Install opencv-contrib-python, or place "
                "haarcascade_frontalface_default.xml in the face_recognition folder."
            )
        self.classifier = cv2.CascadeClassifier(str(cascade_path))
        if self.classifier.empty():
            raise RuntimeError(f"Unable to load face cascade: {cascade_path}")
        self.scale_factor = scale_factor
        self.min_neighbors = min_neighbors
        self.min_size = min_size

    def detect(self, image):
        """Return face rectangles as (x, y, width, height) tuples."""
        if image is None:
            return []
        gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        return self.classifier.detectMultiScale(
            gray_image,
            scaleFactor=self.scale_factor,
            minNeighbors=self.min_neighbors,
            minSize=self.min_size,
        )

    def annotate(self, image):
        """Draw detection boxes and return the image with the face count."""
        faces = self.detect(image)
        result = image.copy()
        for x, y, width, height in faces:
            cv2.rectangle(result, (x, y), (x + width, y + height), (0, 220, 80), 2)
        cv2.putText(
            result,
            f"Faces: {len(faces)}",
            (16, 32),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.9,
            (0, 220, 80),
            2,
        )
        return result, len(faces)