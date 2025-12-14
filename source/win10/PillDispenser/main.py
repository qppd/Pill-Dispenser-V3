from ultralytics import YOLO
import cv2

def main():
    # Load YOLO model (auto-download if not present)
    model = YOLO("yolov8n.pt")  # fast & lightweight

    # Open webcam (0 = default camera)
    source = 0

    # Run YOLO on webcam
    model(
        source=source,
        show=True,      # show live detection window
        conf=0.5,       # confidence threshold
        device="cpu"    # change to "cuda" if GPU available
    )

    # Safety exit
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()