"""
Pill Dispenser V3 - Raspberry Pi Main Controller
ESP32-Triggered Machine Learning-based Pill Detection System

Features:
- ESP32-triggered image capture and inference
- YOLOv8-based pill detection and classification
- Triple image capture for improved accuracy
- ESP32-CAM integration with flash control
- Real-time communication with ESP32 main controller
- Ensemble detection for higher accuracy
- Data logging and analytics
"""

import cv2
import numpy as np
import time
import json
import logging
import threading
import asyncio
import base64
from datetime import datetime
from pathlib import Path
import requests
import serial
import sqlite3
from typing import Dict, List, Tuple, Optional
from concurrent.futures import ThreadPoolExecutor
import statistics

# Machine Learning imports
try:
    from ultralytics import YOLO
    import torch
    YOLO_AVAILABLE = True
except ImportError:
    print("Warning: YOLOv8 not installed. Install with: pip install ultralytics")
    YOLO_AVAILABLE = False

# Computer Vision imports
import cv2
import numpy as np
from PIL import Image

# Configuration
CONFIG = {
    "model": {
        "path": "models/pill_detector_v1.pt",  # Custom trained YOLOv8 model
        "confidence_threshold": 0.6,
        "iou_threshold": 0.4,
        "input_size": 640,
        "ensemble_threshold": 0.7  # Minimum detections across 3 images
    },
    "camera": {
        "esp32cam_ip": "192.168.1.100",  # ESP32-CAM IP address
        "fallback_camera": 0,
        "width": 800,
        "height": 600,
        "fps": 30,
        "capture_delay": 0.3,  # Delay between captures
        "flash_delay": 0.1     # Flash stabilization delay
    },
    "esp32": {
        "port": "/dev/ttyUSB0",
        "baudrate": 115200,
        "timeout": 2
    },
    "capture": {
        "num_images": 3,  # Number of images to capture per detection cycle
        "save_images": True,
        "images_dir": "captured_images",
        "ensemble_voting": True  # Use ensemble voting for final detection
    },
    "database": {
        "path": "data/pill_dispenser.db"
    },
    "logging": {
        "level": "INFO",
        "file": "logs/pill_dispenser.log"
    }
}

class ESP32CamInterface:
    """
    Interface for communicating with ESP32-CAM module
    """
    
    def __init__(self, camera_ip: str):
        self.camera_ip = camera_ip
        self.stream_url = f"http://{camera_ip}/stream"
        self.flash_url = f"http://{camera_ip}/flash"
        self.is_connected = False
        self.test_connection()
    
    def test_connection(self) -> bool:
        """Test if ESP32-CAM is reachable"""
        try:
            response = requests.get(f"http://{self.camera_ip}/", timeout=3)
            self.is_connected = response.status_code == 200
            logging.info(f"ESP32-CAM connection: {'OK' if self.is_connected else 'FAILED'}")
            return self.is_connected
        except Exception as e:
            logging.error(f"ESP32-CAM connection test failed: {e}")
            self.is_connected = False
            return False
    
    def capture_single_frame(self) -> Optional[np.ndarray]:
        """Capture single frame from ESP32-CAM"""
        if not self.is_connected:
            if not self.test_connection():
                return None
        
        try:
            response = requests.get(self.stream_url, stream=True, timeout=5)
            bytes_data = bytes()
            
            for chunk in response.iter_content(chunk_size=1024):
                bytes_data += chunk
                # Look for JPEG boundaries
                a = bytes_data.find(b'\xff\xd8')  # JPEG start
                b = bytes_data.find(b'\xff\xd9')  # JPEG end
                
                if a != -1 and b != -1:
                    jpg = bytes_data[a:b+2]
                    bytes_data = bytes_data[b+2:]
                    
                    # Convert to OpenCV image
                    img_array = np.frombuffer(jpg, dtype=np.uint8)
                    frame = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
                    return frame
                    
        except Exception as e:
            logging.error(f"Frame capture error: {e}")
            self.is_connected = False
            return None
    
    def capture_multiple_frames(self, num_frames: int = 3, delay: float = 0.3) -> List[np.ndarray]:
        """
        Capture multiple frames with controlled timing and flash
        
        Args:
            num_frames: Number of frames to capture
            delay: Delay between captures
            
        Returns:
            List of captured frames
        """
        frames = []
        
        logging.info(f"Starting capture sequence: {num_frames} frames")
        
        # Turn on flash for consistent lighting
        self.set_flash(True)
        time.sleep(CONFIG['camera']['flash_delay'])  # Allow flash to stabilize
        
        try:
            for i in range(num_frames):
                logging.info(f"Capturing frame {i+1}/{num_frames}")
                
                frame = self.capture_single_frame()
                if frame is not None:
                    frames.append(frame)
                    
                    # Save frame if configured
                    if CONFIG['capture']['save_images']:
                        self.save_frame(frame, i)
                    
                    logging.info(f"Frame {i+1} captured successfully ({frame.shape})")
                else:
                    logging.warning(f"Failed to capture frame {i+1}")
                
                # Delay between captures (except for last frame)
                if i < num_frames - 1:
                    time.sleep(delay)
            
        finally:
            # Turn off flash
            self.set_flash(False)
        
        logging.info(f"Capture sequence complete: {len(frames)}/{num_frames} frames captured")
        return frames
    
    def save_frame(self, frame: np.ndarray, frame_index: int):
        """Save captured frame to disk"""
        try:
            # Create images directory
            images_dir = Path(CONFIG['capture']['images_dir'])
            images_dir.mkdir(parents=True, exist_ok=True)
            
            # Generate filename with timestamp
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"capture_{timestamp}_frame_{frame_index:02d}.jpg"
            filepath = images_dir / filename
            
            # Save image
            cv2.imwrite(str(filepath), frame)
            logging.debug(f"Frame saved: {filepath}")
            
        except Exception as e:
            logging.error(f"Error saving frame: {e}")
    
    def set_flash(self, state: bool) -> bool:
        """Control flash LED"""
        endpoint = "on" if state else "off"
        try:
            response = requests.get(f"{self.flash_url}/{endpoint}", timeout=2)
            success = response.status_code == 200
            if success:
                logging.debug(f"Flash {endpoint}: OK")
            return success
        except Exception as e:
            logging.error(f"Flash control error: {e}")
            return False
    
    def get_flash_status(self) -> bool:
        """Get current flash status"""
        try:
            response = requests.get(f"{self.flash_url}/status", timeout=2)
            data = response.json()
            return data.get("flash") == "on"
        except:
            return False

class EnsemblePillDetector:
    """
    Enhanced pill detector with ensemble inference across multiple images
    """
    
    def __init__(self, model_path: str, confidence: float = 0.6):
        self.model_path = model_path
        self.confidence = confidence
        self.model = None
        self.classes = {}
        self.load_model()
    
    def load_model(self):
        """Load YOLOv8 model for pill detection"""
        try:
            if YOLO_AVAILABLE and Path(self.model_path).exists():
                self.model = YOLO(self.model_path)
                self.classes = self.model.names
                logging.info(f"Loaded YOLOv8 model: {self.model_path}")
                logging.info(f"Available classes: {self.classes}")
            else:
                logging.warning("YOLOv8 model not available, using fallback detection")
                self.load_fallback_model()
        except Exception as e:
            logging.error(f"Error loading model: {e}")
            self.load_fallback_model()
    
    def load_fallback_model(self):
        """Fallback detection using traditional computer vision"""
        self.classes = {
            0: "round_pill",
            1: "oval_pill", 
            2: "capsule",
            3: "tablet"
        }
        logging.info("Using fallback detection method")
    
    def detect_single_image(self, image: np.ndarray) -> List[Dict]:
        """
        Detect pills in a single image
        """
        detections = []
        
        try:
            if self.model and YOLO_AVAILABLE:
                # YOLOv8 detection
                results = self.model(image, conf=self.confidence)
                
                for result in results:
                    boxes = result.boxes
                    if boxes is not None:
                        for i, box in enumerate(boxes):
                            x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                            confidence = box.conf[0].cpu().numpy()
                            class_id = int(box.cls[0].cpu().numpy())
                            
                            detection = {
                                "bbox": [int(x1), int(y1), int(x2), int(y2)],
                                "confidence": float(confidence),
                                "class_id": class_id,
                                "class_name": self.classes.get(class_id, "unknown"),
                                "timestamp": datetime.now().isoformat()
                            }
                            detections.append(detection)
            else:
                # Fallback detection
                detections = self.fallback_detection(image)
                
        except Exception as e:
            logging.error(f"Detection error: {e}")
            detections = self.fallback_detection(image)
        
        return detections
    
    def ensemble_detection(self, images: List[np.ndarray]) -> List[Dict]:
        """
        Perform ensemble detection across multiple images
        
        Args:
            images: List of captured images
            
        Returns:
            Ensemble detection results with improved confidence
        """
        if not images:
            logging.warning("No images provided for ensemble detection")
            return []
        
        logging.info(f"Starting ensemble detection on {len(images)} images")
        
        # Detect pills in each image
        all_detections = []
        for i, image in enumerate(images):
            detections = self.detect_single_image(image)
            all_detections.append(detections)
            logging.info(f"Image {i+1}: {len(detections)} detections")
        
        # Ensemble voting and confidence aggregation
        ensemble_results = self.aggregate_detections(all_detections)
        
        logging.info(f"Ensemble detection complete: {len(ensemble_results)} final detections")
        return ensemble_results
    
    def aggregate_detections(self, all_detections: List[List[Dict]]) -> List[Dict]:
        """
        Aggregate detections from multiple images using ensemble voting
        """
        if not all_detections:
            return []
        
        # Flatten all detections with image index
        flat_detections = []
        for img_idx, detections in enumerate(all_detections):
            for detection in detections:
                detection['image_index'] = img_idx
                flat_detections.append(detection)
        
        if not flat_detections:
            return []
        
        # Group detections by spatial proximity and class
        grouped_detections = self.group_similar_detections(flat_detections)
        
        # Create ensemble results
        ensemble_results = []
        for group in grouped_detections:
            if len(group) >= 2:  # Require detection in at least 2 images
                ensemble_detection = self.create_ensemble_detection(group)
                if ensemble_detection['confidence'] >= CONFIG['model']['ensemble_threshold']:
                    ensemble_results.append(ensemble_detection)
        
        return ensemble_results
    
    def group_similar_detections(self, detections: List[Dict], iou_threshold: float = 0.3) -> List[List[Dict]]:
        """Group spatially similar detections across images"""
        groups = []
        used_indices = set()
        
        for i, detection in enumerate(detections):
            if i in used_indices:
                continue
            
            # Start new group
            group = [detection]
            used_indices.add(i)
            
            # Find similar detections
            for j, other_detection in enumerate(detections):
                if j in used_indices or j == i:
                    continue
                
                # Check if same class and similar position
                if (detection['class_id'] == other_detection['class_id'] and
                    self.calculate_iou(detection['bbox'], other_detection['bbox']) > iou_threshold):
                    group.append(other_detection)
                    used_indices.add(j)
            
            groups.append(group)
        
        return groups
    
    def calculate_iou(self, bbox1: List[int], bbox2: List[int]) -> float:
        """Calculate Intersection over Union (IoU) between two bounding boxes"""
        x1, y1, x2, y2 = bbox1
        x1_2, y1_2, x2_2, y2_2 = bbox2
        
        # Calculate intersection
        xi1 = max(x1, x1_2)
        yi1 = max(y1, y1_2)
        xi2 = min(x2, x2_2)
        yi2 = min(y2, y2_2)
        
        if xi2 <= xi1 or yi2 <= yi1:
            return 0.0
        
        intersection = (xi2 - xi1) * (yi2 - yi1)
        
        # Calculate union
        area1 = (x2 - x1) * (y2 - y1)
        area2 = (x2_2 - x1_2) * (y2_2 - y1_2)
        union = area1 + area2 - intersection
        
        return intersection / union if union > 0 else 0.0
    
    def create_ensemble_detection(self, group: List[Dict]) -> Dict:
        """Create ensemble detection from group of similar detections"""
        # Average bounding box coordinates
        avg_bbox = [
            int(statistics.mean([d['bbox'][0] for d in group])),  # x1
            int(statistics.mean([d['bbox'][1] for d in group])),  # y1
            int(statistics.mean([d['bbox'][2] for d in group])),  # x2
            int(statistics.mean([d['bbox'][3] for d in group]))   # y2
        ]
        
        # Ensemble confidence (weighted average)
        ensemble_confidence = statistics.mean([d['confidence'] for d in group])
        
        # Boost confidence based on number of detections
        detection_count = len(group)
        confidence_boost = min(0.2, detection_count * 0.05)  # Max 0.2 boost
        ensemble_confidence = min(1.0, ensemble_confidence + confidence_boost)
        
        return {
            "bbox": avg_bbox,
            "confidence": ensemble_confidence,
            "class_id": group[0]['class_id'],
            "class_name": group[0]['class_name'],
            "detection_count": detection_count,
            "timestamp": datetime.now().isoformat()
        }
    
    def fallback_detection(self, image: np.ndarray) -> List[Dict]:
        """Traditional computer vision fallback for pill detection"""
        detections = []
        
        # Convert to grayscale
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # Apply Gaussian blur
        blurred = cv2.GaussianBlur(gray, (9, 9), 2)
        
        # Use HoughCircles for round pill detection
        circles = cv2.HoughCircles(
            blurred,
            cv2.HOUGH_GRADIENT,
            dp=1,
            minDist=30,
            param1=50,
            param2=30,
            minRadius=10,
            maxRadius=100
        )
        
        if circles is not None:
            circles = np.round(circles[0, :]).astype("int")
            for (x, y, r) in circles:
                detection = {
                    "bbox": [x-r, y-r, x+r, y+r],
                    "confidence": 0.8,
                    "class_id": 0,
                    "class_name": "round_pill",
                    "timestamp": datetime.now().isoformat()
                }
                detections.append(detection)
        
        return detections

class ESP32Communicator:
    """
    Enhanced communication with ESP32 main controller
    """
    
    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.serial_conn = None
        self.is_connected = False
        self.message_queue = []
        self.response_timeout = CONFIG['esp32']['timeout']
    
    def connect(self) -> bool:
        """Connect to ESP32"""
        try:
            self.serial_conn = serial.Serial(self.port, self.baudrate, timeout=self.response_timeout)
            time.sleep(2)  # Allow ESP32 to reset
            self.is_connected = True
            logging.info(f"Connected to ESP32 on {self.port}")
            return True
        except Exception as e:
            logging.error(f"ESP32 connection error: {e}")
            return False
    
    def listen_for_triggers(self) -> bool:
        """
        Listen for detection trigger commands from ESP32
        
        Returns:
            True if detection trigger received
        """
        if not self.is_connected:
            return False
        
        try:
            if self.serial_conn.in_waiting > 0:
                message = self.serial_conn.readline().decode().strip()
                logging.info(f"ESP32 message: {message}")
                
                # Check for detection trigger
                if "TRIGGER_DETECTION" in message or "DETECT_PILLS" in message:
                    logging.info("Detection trigger received from ESP32")
                    return True
                
                # Store other messages for processing
                self.message_queue.append({
                    "message": message,
                    "timestamp": datetime.now().isoformat()
                })
                
        except Exception as e:
            logging.error(f"Error listening to ESP32: {e}")
            self.is_connected = False
        
        return False
    
    def send_detection_results(self, detections: List[Dict]) -> bool:
        """
        Send detection results back to ESP32
        
        Args:
            detections: List of detected pills
            
        Returns:
            True if sent successfully
        """
        if not self.is_connected or not self.serial_conn:
            return False
        
        try:
            # Format detection results
            result_data = {
                "timestamp": datetime.now().isoformat(),
                "detection_count": len(detections),
                "detections": detections
            }
            
            # Send JSON data
            message = f"DETECTION_RESULTS:{json.dumps(result_data)}\n"
            self.serial_conn.write(message.encode())
            
            logging.info(f"Sent detection results to ESP32: {len(detections)} detections")
            return True
            
        except Exception as e:
            logging.error(f"Error sending detection results: {e}")
            return False
    
    def send_simple_result(self, pill_count: int, dominant_class: str = "unknown") -> bool:
        """
        Send simplified detection result for easier ESP32 parsing
        """
        if not self.is_connected or not self.serial_conn:
            return False
        
        try:
            message = f"PILLS_DETECTED:{pill_count}:{dominant_class}\n"
            self.serial_conn.write(message.encode())
            
            logging.info(f"Sent simple result to ESP32: {pill_count} {dominant_class} pills")
            return True
            
        except Exception as e:
            logging.error(f"Error sending simple result: {e}")
            return False
    
    def send_command(self, command: str) -> str:
        """Send command to ESP32 and get response"""
        if not self.is_connected or not self.serial_conn:
            return "ERROR: Not connected"
        
        try:
            self.serial_conn.write(f"{command}\n".encode())
            time.sleep(0.1)
            response = self.serial_conn.readline().decode().strip()
            return response
        except Exception as e:
            logging.error(f"Communication error: {e}")
            return f"ERROR: {e}"

class DatabaseManager:
    """Enhanced database management for logging and analytics"""
    
    def __init__(self, db_path: str):
        self.db_path = db_path
        self.init_database()
    
    def init_database(self):
        """Initialize database tables"""
        try:
            Path(self.db_path).parent.mkdir(parents=True, exist_ok=True)
            
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            # Enhanced detections table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS detections (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp TEXT,
                    pill_type TEXT,
                    confidence REAL,
                    detection_count INTEGER,
                    bbox_x1 INTEGER,
                    bbox_y1 INTEGER,
                    bbox_x2 INTEGER,
                    bbox_y2 INTEGER,
                    image_count INTEGER,
                    ensemble_used BOOLEAN,
                    trigger_source TEXT
                )
            ''')
            
            # Detection sessions table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS detection_sessions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    session_id TEXT,
                    timestamp TEXT,
                    trigger_source TEXT,
                    images_captured INTEGER,
                    detections_count INTEGER,
                    processing_time REAL,
                    success BOOLEAN
                )
            ''')
            
            conn.commit()
            conn.close()
            logging.info("Database initialized")
            
        except Exception as e:
            logging.error(f"Database initialization error: {e}")
    
    def log_detection_session(self, session_data: Dict):
        """Log complete detection session"""
        try:
            conn = sqlite3.connect(self.db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                INSERT INTO detection_sessions 
                (session_id, timestamp, trigger_source, images_captured, 
                 detections_count, processing_time, success)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (
                session_data.get('session_id'),
                session_data.get('timestamp'),
                session_data.get('trigger_source'),
                session_data.get('images_captured'),
                session_data.get('detections_count'),
                session_data.get('processing_time'),
                session_data.get('success')
            ))
            
            conn.commit()
            conn.close()
            
        except Exception as e:
            logging.error(f"Database logging error: {e}")

class PillDispenserSystem:
    """
    Main system controller with ESP32-triggered detection
    """
    
    def __init__(self):
        self.setup_logging()
        self.detector = EnsemblePillDetector(CONFIG['model']['path'])
        self.camera = ESP32CamInterface(CONFIG['camera']['esp32cam_ip'])
        self.esp32 = ESP32Communicator(CONFIG['esp32']['port'])
        self.database = DatabaseManager(CONFIG['database']['path'])
        self.is_running = False
        self.session_counter = 0
        
    def setup_logging(self):
        """Setup logging configuration"""
        log_dir = Path(CONFIG['logging']['file']).parent
        log_dir.mkdir(parents=True, exist_ok=True)
        
        logging.basicConfig(
            level=getattr(logging, CONFIG['logging']['level']),
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(CONFIG['logging']['file']),
                logging.StreamHandler()
            ]
        )
    
    def initialize(self) -> bool:
        """Initialize all system components"""
        logging.info("Initializing Pill Dispenser System V3 - ESP32 Triggered Mode")
        
        # Test ESP32-CAM connection
        if not self.camera.test_connection():
            logging.error("Failed to connect to ESP32-CAM")
            return False
        
        # Connect to ESP32 main controller
        if not self.esp32.connect():
            logging.error("Failed to connect to ESP32 main controller")
            return False
        
        logging.info("System initialization complete - Ready for ESP32 triggers")
        return True
    
    def process_detection_trigger(self) -> bool:
        """
        Process detection trigger from ESP32
        
        Returns:
            True if detection completed successfully
        """
        session_id = f"session_{self.session_counter:06d}"
        self.session_counter += 1
        start_time = time.time()
        
        logging.info(f"Starting detection session: {session_id}")
        
        # Capture multiple images
        images = self.camera.capture_multiple_frames(
            num_frames=CONFIG['capture']['num_images'],
            delay=CONFIG['camera']['capture_delay']
        )
        
        if not images:
            logging.error("No images captured - aborting detection")
            return False
        
        # Perform ensemble detection
        detections = self.detector.ensemble_detection(images)
        
        processing_time = time.time() - start_time
        
        # Log session data
        session_data = {
            'session_id': session_id,
            'timestamp': datetime.now().isoformat(),
            'trigger_source': 'esp32',
            'images_captured': len(images),
            'detections_count': len(detections),
            'processing_time': processing_time,
            'success': len(detections) > 0
        }
        self.database.log_detection_session(session_data)
        
        # Send results to ESP32
        if detections:
            # Send detailed results
            self.esp32.send_detection_results(detections)
            
            # Send simple summary for easier parsing
            dominant_class = max(detections, key=lambda x: x['confidence'])['class_name']
            self.esp32.send_simple_result(len(detections), dominant_class)
            
            logging.info(f"Detection complete: {len(detections)} pills detected in {processing_time:.2f}s")
        else:
            self.esp32.send_simple_result(0, "none")
            logging.info(f"Detection complete: No pills detected in {processing_time:.2f}s")
        
        return True
    
    def run_triggered_mode(self):
        """
        Main loop for ESP32-triggered detection mode
        """
        logging.info("Starting ESP32-triggered detection mode")
        self.is_running = True
        
        try:
            while self.is_running:
                # Listen for trigger from ESP32
                if self.esp32.listen_for_triggers():
                    # Process detection
                    self.process_detection_trigger()
                
                # Small delay to prevent CPU overload
                time.sleep(0.1)
                
        except KeyboardInterrupt:
            logging.info("Triggered mode interrupted by user")
        except Exception as e:
            logging.error(f"Error in triggered mode: {e}")
        finally:
            self.is_running = False
    
    def manual_detection_test(self):
        """Manual detection test for debugging"""
        logging.info("Starting manual detection test")
        
        # Capture images
        images = self.camera.capture_multiple_frames(
            num_frames=CONFIG['capture']['num_images'],
            delay=CONFIG['camera']['capture_delay']
        )
        
        if not images:
            print("Failed to capture images")
            return
        
        # Perform detection
        detections = self.detector.ensemble_detection(images)
        
        # Display results
        print(f"\n=== Detection Results ===")
        print(f"Images captured: {len(images)}")
        print(f"Detections found: {len(detections)}")
        
        for i, detection in enumerate(detections):
            print(f"  {i+1}. {detection['class_name']} - "
                  f"Confidence: {detection['confidence']:.3f} - "
                  f"Count: {detection.get('detection_count', 1)}")
    
    def start(self):
        """Start the pill dispenser system"""
        if not self.initialize():
            logging.error("System initialization failed")
            return
        
        try:
            while True:
                print("\n=== Pill Dispenser V3 - ESP32 Triggered ===")
                print("1. ESP32-Triggered Mode (Automatic)")
                print("2. Manual Detection Test")
                print("3. Camera Test")
                print("4. ESP32 Communication Test")
                print("5. System Status")
                print("6. Exit")
                
                choice = input("Select option: ").strip()
                
                if choice == '1':
                    self.run_triggered_mode()
                elif choice == '2':
                    self.manual_detection_test()
                elif choice == '3':
                    frame = self.camera.capture_single_frame()
                    if frame is not None:
                        print(f"Camera test OK - Frame size: {frame.shape}")
                        cv2.imshow("Camera Test", frame)
                        cv2.waitKey(3000)
                        cv2.destroyAllWindows()
                    else:
                        print("Camera test failed")
                elif choice == '4':
                    response = self.esp32.send_command("status")
                    print(f"ESP32 Response: {response}")
                elif choice == '5':
                    self.print_system_status()
                elif choice == '6':
                    break
                else:
                    print("Invalid option")
                    
        except KeyboardInterrupt:
            logging.info("System interrupted by user")
        finally:
            self.shutdown()
    
    def print_system_status(self):
        """Print comprehensive system status"""
        print("\n=== System Status ===")
        print(f"ESP32-CAM: {'Connected' if self.camera.is_connected else 'Disconnected'}")
        print(f"ESP32 Main: {'Connected' if self.esp32.is_connected else 'Disconnected'}")
        print(f"YOLO Model: {'Loaded' if self.detector.model else 'Fallback'}")
        print(f"Session Count: {self.session_counter}")
        print(f"Config: {CONFIG['capture']['num_images']} images per detection")
    
    def shutdown(self):
        """Shutdown system gracefully"""
        logging.info("Shutting down system")
        self.is_running = False
        
        if self.esp32.serial_conn:
            self.esp32.serial_conn.close()
        
        cv2.destroyAllWindows()
        logging.info("System shutdown complete")

def main():
    """Main entry point"""
    system = PillDispenserSystem()
    system.start()

if __name__ == "__main__":
    main()
