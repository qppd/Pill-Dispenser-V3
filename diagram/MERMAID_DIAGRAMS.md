# Pill Dispenser V3 - Complete Mermaid Diagrams

Generated from actual codebase analysis on December 10, 2025

---

## 1. SYSTEM ARCHITECTURE DIAGRAM

```mermaid
graph TB
    subgraph "Hardware Layer"
        ESP32[ESP32 Main Controller]
        ESP32CAM[ESP32-CAM Image Capture]
        SERVO[PCA9685 16-Ch Servo Driver]
        IR1[IR Sensor 1 Entrance]
        IR2[IR Sensor 2 Middle]
        IR3[IR Sensor 3 Exit]
        LCD[I2C LCD 20x4 Display]
        RTC[DS1302 RTC Real-Time Clock]
        GSM[SIM800L GSM Module]
        MOTORS[Servo Motors x16]
    end
    
    subgraph "Processing Layer"
        RPI[Raspberry Pi 4 ML Processor]
        YOLO[YOLOv8 Pill Detector]
        DB_LOCAL[SQLite Local Database]
    end
    
    subgraph "Cloud Layer"
        FIREBASE[Firebase Realtime Database]
        AUTH[Firebase Authentication]
    end
    
    subgraph "User Interface"
        WEB[Next.js Web App Dashboard]
        USER[End Users]
    end
    
    ESP32 -->|I2C| SERVO
    ESP32 -->|Digital| IR1
    ESP32 -->|Digital| IR2
    ESP32 -->|Digital| IR3
    ESP32 -->|I2C| LCD
    ESP32 -->|3-Wire| RTC
    ESP32 -->|UART| GSM
    ESP32 -->|Serial UART| RPI
    ESP32 -->|WiFi Commands| ESP32CAM
    
    SERVO -->|PWM| MOTORS
    
    ESP32CAM -->|HTTP Stream| RPI
    RPI -->|Inference| YOLO
    RPI -->|Store| DB_LOCAL
    RPI -->|Results| ESP32
    
    ESP32 -->|WiFi/HTTPS| FIREBASE
    WEB -->|HTTPS| FIREBASE
    WEB -->|Auth| AUTH
    
    USER -->|Browser| WEB
    
    style ESP32 fill:#4A90E2
    style RPI fill:#50C878
    style FIREBASE fill:#FFA500
    style WEB fill:#9B59B6
```

---

## 2. SYSTEM BLOCK DIAGRAM

```mermaid
graph LR
    subgraph "ESP32 Main Controller"
        MAIN[Main Program PillDispenser.ino]
        SERVO_DRV[ServoDriver Module]
        IR_MOD[IRSensor Module]
        LCD_MOD[LCDDisplay Module]
        RTC_MOD[RTClock Module]
        FB_MOD[FirebaseManager Module]
        GSM_MOD[SIM800L Module]
    end
    
    subgraph "External Hardware"
        PCA[PCA9685 16-Ch PWM]
        IR_HW[3x IR Sensors]
        LCD_HW[I2C LCD Display]
        RTC_HW[DS1302 RTC]
        GSM_HW[SIM800L GSM]
        SERVOS[16x Servo Motors]
    end
    
    subgraph "Raspberry Pi System"
        RPI_MAIN[main.py Controller]
        CAM_INT[ESP32CamInterface]
        DETECTOR[EnsemblePillDetector]
        ESP32_COM[ESP32Communicator]
        DB_MGR[DatabaseManager]
    end
    
    subgraph "Web Application"
        NEXT[Next.js Frontend]
        API_DISP["/api/dispenser"]
        API_DISPENSE["/api/dispense"]
        DASH[Dashboard Page]
        LOGS[Logs Page]
        SCHED[Schedule Page]
    end
    
    MAIN --> SERVO_DRV
    MAIN --> IR_MOD
    MAIN --> LCD_MOD
    MAIN --> RTC_MOD
    MAIN --> FB_MOD
    MAIN --> GSM_MOD
    
    SERVO_DRV <-->|I2C| PCA
    IR_MOD <-->|Digital| IR_HW
    LCD_MOD <-->|I2C| LCD_HW
    RTC_MOD <-->|3-Wire| RTC_HW
    GSM_MOD <-->|UART| GSM_HW
    PCA -->|PWM| SERVOS
    
    MAIN <-->|UART| RPI_MAIN
    RPI_MAIN --> CAM_INT
    RPI_MAIN --> DETECTOR
    RPI_MAIN --> ESP32_COM
    RPI_MAIN --> DB_MGR
    
    MAIN <-->|WiFi| FB_MOD
    NEXT <--> API_DISP
    NEXT <--> API_DISPENSE
    NEXT --> DASH
    NEXT --> LOGS
    NEXT --> SCHED
    
    style MAIN fill:#4A90E2
    style RPI_MAIN fill:#50C878
    style NEXT fill:#9B59B6
```

---

## 3. PROGRAM FLOWCHART - ESP32 Main

```mermaid
flowchart TD
    START([System Power On]) --> INIT[Initialize Serial 115200 baud]
    INIT --> I2C[Initialize I2C SDA:21, SCL:22]
    I2C --> MODE{Development Mode?}
    
    MODE -->|Yes| DEV_INIT[Initialize Dev Components: - ServoDriver - IRSensor - LCDDisplay - RTClock]
    MODE -->|No| PROD_INIT[Production Mode Not Implemented]
    
    DEV_INIT --> READY[System Ready LED ON]
    READY --> LOOP{Main Loop}
    
    LOOP --> SERIAL_CHECK{Serial Available?}
    SERIAL_CHECK -->|Yes| READ_CMD[Read Serial Command]
    SERIAL_CHECK -->|No| SENSOR_CHECK
    
    READ_CMD --> PARSE{Parse Command}
    PARSE -->|help| SHOW_HELP[Display Help Menu]
    PARSE -->|status| SHOW_STATUS[Show System Status]
    PARSE -->|test servo| TEST_SERVO[Test Servo Motor]
    PARSE -->|test ir| TEST_IR[Test IR Sensors]
    PARSE -->|test lcd| TEST_LCD[Test LCD Display]
    PARSE -->|test rtc| TEST_RTC[Test RTC Module]
    PARSE -->|dispense| DISPENSE[Dispense Pills]
    PARSE -->|wifi connect| WIFI_CONN[Connect WiFi]
    PARSE -->|firebase test| FB_TEST[Test Firebase]
    
    SHOW_HELP --> LOOP
    SHOW_STATUS --> LOOP
    TEST_SERVO --> LOOP
    TEST_IR --> LOOP
    TEST_LCD --> LOOP
    TEST_RTC --> LOOP
    DISPENSE --> LOOP
    WIFI_CONN --> LOOP
    FB_TEST --> LOOP
    
    SENSOR_CHECK --> TIMER{500ms Elapsed?}
    TIMER -->|Yes| READ_SENSORS[Read All IR Sensors]
    TIMER -->|No| HEARTBEAT
    
    READ_SENSORS --> HEARTBEAT{30s Elapsed?}
    HEARTBEAT -->|Yes| SEND_HB[Send Heartbeat]
    HEARTBEAT -->|No| DELAY
    
    SEND_HB --> DELAY[Delay 100ms]
    DELAY --> LOOP
    
    style START fill:#90EE90
    style READY fill:#FFD700
    style LOOP fill:#87CEEB
```

---

## 4. ACTIVITY DIAGRAM - Pill Dispensing Process

```mermaid
stateDiagram-v2
    [*] --> Idle
    
    Idle --> IRDetection: "User Request or\nScheduled Time"
    
    IRDetection --> ServoActivation: "Pills Detected at\nEntrance (IR1)"
    
    ServoActivation --> DispensePills: "Activate Servo Pair\n(Open/Close)"
    
    DispensePills --> MiddleCheck: Pills Moving
    
    MiddleCheck --> ExitCheck: IR2 Triggered
    
    ExitCheck --> CaptureImage: "IR3 Triggered\n(Exit Detection)"
    
    CaptureImage --> RPiTrigger: "Send UART Trigger\nto Raspberry Pi"
    
    RPiTrigger --> ImageCapture: "RPi Captures\n3 Images via ESP32-CAM"
    
    ImageCapture --> MLInference: "YOLOv8 Ensemble\nDetection"
    
    MLInference --> CountVerify: "Pill Count &\nClassification"
    
    CountVerify --> ResultSend: "Send Results to\nESP32 via UART"
    
    ResultSend --> LCDUpdate: "Update LCD Display"
    
    LCDUpdate --> FirebaseLog: Log to Firebase
    
    FirebaseLog --> DatabaseStore: Store in SQLite
    
    DatabaseStore --> Idle: Process Complete
    
    state IRDetection {
        [*] --> MonitorIR1
        MonitorIR1 --> IR1Blocked: Object Detected
        IR1Blocked --> [*]: Confirmed
    }
    
    state MLInference {
        [*] --> Image1
        Image1 --> Image2
        Image2 --> Image3
        Image3 --> Ensemble
        Ensemble --> [*]: Aggregated Result
    }
```

---

## 5. SEQUENCE DIAGRAM - Complete Pill Dispense Flow

```mermaid
sequenceDiagram
    participant User as User/Schedule
    participant Web as Web App
    participant Firebase as Firebase DB
    participant ESP32 as ESP32 Main
    participant IR as IR Sensors
    participant Servo as Servo Motors
    participant RPI as Raspberry Pi
    participant CAM as ESP32-CAM
    participant YOLO as YOLOv8 Model
    participant LCD as LCD Display
    
    User->>Web: Request Dispense (Manual/Schedule)
    Web->>Firebase: Update Command /dispensers/{uid}/commands
    
    Firebase-->>ESP32: Stream Update dispense_command
    
    ESP32->>IR: Monitor Entrance (IR Sensor 1)
    IR-->>ESP32: Detection: HIGH→LOW
    
    ESP32->>Servo: Activate Channel Pair (e.g., CH0+CH1)
    Servo->>Servo: Open Gate (90°)
    Servo->>Servo: Close Gate (0°)
    
    ESP32->>IR: Monitor Middle (IR Sensor 2)
    IR-->>ESP32: Pills Moving
    
    ESP32->>IR: Monitor Exit (IR Sensor 3)
    IR-->>ESP32: Pills Dispensed
    
    ESP32->>RPI: UART: "TRIGGER_DETECTION"
    
    RPI->>CAM: HTTP GET /flash/on
    CAM-->>RPI: Flash Enabled
    
    RPI->>CAM: Capture Frame 1
    CAM-->>RPI: Image 1 (800x600)
    
    RPI->>CAM: Capture Frame 2 (300ms delay)
    CAM-->>RPI: Image 2
    
    RPI->>CAM: Capture Frame 3 (300ms delay)
    CAM-->>RPI: Image 3
    
    RPI->>YOLO: Detect Pills in Image 1
    YOLO-->>RPI: Detections (bbox, conf, class)
    
    RPI->>YOLO: Detect Pills in Image 2
    YOLO-->>RPI: Detections
    RPI->>YOLO: Detect Pills in Image 3
    YOLO-->>RPI: Detections
    RPI->>RPI: Ensemble Aggregation (Voting & Confidence)
    RPI->>ESP32: UART: "PILLS_DETECTED:3:round_pill"
    ESP32->>LCD: Update Display "3 Pills Dispensed"
    ESP32->>Firebase: Push Log /pilldispenser/pill_logs
    ESP32->>Firebase: Push Report /pilldispenser/reports
    Firebase-->>Web: Real-time Update
    Web-->>User: Show Confirmation
    RPI->>RPI: Store in SQLite pill_dispenser.db
```

---

## 6. USE CASE DIAGRAM

```mermaid
graph TB
    subgraph "Pill Dispenser V3 System"
        UC1[Manual Pill Dispensing]
        UC2[Schedule Management]
        UC3[View Dispense Logs]
        UC4[Monitor Device Status]
        UC5[Pill Count Verification]
        UC6[User Authentication]
        UC7[Export Reports]
        UC8[System Testing]
        UC9[Real-time Monitoring]
        UC10[Remote Control]
        UC11[SMS Notifications]
    end
    
    USER((End User/ Caregiver))
    ADMIN((System Administrator))
    ESP32((ESP32 Controller))
    RPI((Raspberry Pi ML System))
    
    USER --> UC1
    USER --> UC2
    USER --> UC3
    USER --> UC4
    USER --> UC6
    USER --> UC7
    USER --> UC9
    USER --> UC10
    
    ADMIN --> UC8
    ADMIN --> UC4
    ADMIN --> UC7
    
    ESP32 --> UC5
    ESP32 --> UC11
    
    RPI --> UC5
    
    UC1 -.->|includes| UC5
    UC2 -.->|includes| UC1
    UC10 -.->|includes| UC1
    UC11 -.->|extends| UC1
    
    style USER fill:#87CEEB
    style ADMIN fill:#FFD700
    style ESP32 fill:#90EE90
    style RPI fill:#FFA07A
```

---

## 7. DATABASE SCHEMA DIAGRAM - Firebase Realtime Database

```mermaid
erDiagram
    DISPENSERS ||--o{ CONTAINERS : contains
    DISPENSERS ||--|| DEVICE : has
    USERS ||--o{ DISPENSERS : owns
    USERS ||--o{ SCHEDULES : manages
    SCHEDULES ||--o{ SCHEDULE_ITEMS : contains
    PILL_LOGS ||--o| DISPENSERS : references
    REPORTS ||--o| DISPENSERS : references
    
    USERS {
        string uid PK
        string email
        timestamp created_at
    }
    
    DISPENSERS {
        string uid FK
        object containers
        object device
        timestamp last_updated
    }
    
    DEVICE {
        int battery
        string status
        timestamp last_heartbeat
    }
    
    CONTAINERS {
        int id PK
        string name
        int pills_remaining
        string last_dispensed
        string next_dose
    }
    
    SCHEDULES {
        string uid FK
        array dispensers
    }
    
    SCHEDULE_ITEMS {
        int dispenser_id
        string time
        boolean enabled
    }
    
    PILL_LOGS {
        string log_id PK
        int pill_count
        string timestamp
        string device_id
        string description
    }
    
    REPORTS {
        string report_id PK
        int pill_count
        string datetime
        string description
        int status
        string device_id
    }
```

---

## 8. DATABASE SCHEMA DIAGRAM - SQLite (Raspberry Pi)

```mermaid
erDiagram
    DETECTION_SESSIONS ||--o{ DETECTIONS : contains
    DETECTION_SESSIONS ||--o{ CAPTURED_IMAGES : has
    
    DETECTION_SESSIONS {
        string session_id PK
        string timestamp
        string trigger_source
        int image_count
        float processing_time
        int total_detections
        string result_summary
    }
    
    DETECTIONS {
        int id PK
        string session_id FK
        int image_index
        string class_name
        int class_id
        float confidence
        string bbox
        int detection_count
        string timestamp
    }
    
    CAPTURED_IMAGES {
        int id PK
        string session_id FK
        int frame_index
        string file_path
        int width
        int height
        string timestamp
    }
```

---

## 9. NETWORK/COMMUNICATION DIAGRAM

```mermaid
graph TB
    subgraph "Local Network - 192.168.1.x"
        ESP32[ESP32 Main WiFi Client]
        ESP32CAM[ESP32-CAM 192.168.1.100 HTTP Server]
        RPI[Raspberry Pi 4 WiFi/Ethernet]
        ROUTER[WiFi Router Access Point]
    end
    
    subgraph "Serial Communication"
        ESP32 <-->|UART 115200| RPI
        ESP32 -->|Monitor| SERIAL[Serial Monitor USB]
    end
    
    subgraph "Internet"
        FIREBASE[Firebase Realtime DB pilldispenser-5c037]
        WEB_HOST[Vercel Web Hosting]
    end
    
    subgraph "User Devices"
        BROWSER[Web Browser Any Device]
        MOBILE[Mobile Device]
    end
    
    ROUTER --- ESP32
    ROUTER --- ESP32CAM
    ROUTER --- RPI
    
    ESP32 -->|HTTPS| FIREBASE
    RPI -->|HTTP GET| ESP32CAM
    
    ROUTER -->|Internet| FIREBASE
    ROUTER -->|Internet| WEB_HOST
    
    BROWSER -->|HTTPS| WEB_HOST
    MOBILE -->|HTTPS| WEB_HOST
    
    WEB_HOST <-->|WebSocket/REST| FIREBASE
    
    style ESP32 fill:#4A90E2
    style RPI fill:#50C878
    style FIREBASE fill:#FFA500
    style WEB_HOST fill:#9B59B6
```

---

## 10. WEB APPLICATION FLOWCHART

```mermaid
flowchart TD
    START([User Access localhost:3000]) --> AUTH_CHECK{Firebase Configured?}
    
    AUTH_CHECK -->|No| SETUP_MSG[Show Setup Required Message]
    AUTH_CHECK -->|Yes| LOGIN_CHECK{User Logged In?}
    
    LOGIN_CHECK -->|No| LOGIN_PAGE[Login Page]
    LOGIN_CHECK -->|Yes| DASHBOARD
    
    LOGIN_PAGE --> LOGIN_FORM[Enter Email & Password]
    LOGIN_FORM --> AUTH_SUBMIT{Submit Credentials}
    
    AUTH_SUBMIT -->|Success| DASHBOARD[Dashboard Page]
    AUTH_SUBMIT -->|Failed| ERROR[Show Error Message]
    ERROR --> LOGIN_PAGE
    
    DASHBOARD --> LOAD_DATA{Load Data from Firebase}
    
    LOAD_DATA --> DEVICE_STATUS["Get Device Status /dispensers/{uid}/device"]
    LOAD_DATA --> CONTAINERS["Get Containers /dispensers/{uid}/containers"]
    
    DEVICE_STATUS --> DISPLAY_DEVICE[Display: - Battery % - Online/Offline - Low Pills Count]
    
    CONTAINERS --> DISPLAY_CONTAINERS[Display 5 Containers: - Pills Remaining - Next Dose - Last Dispensed]
    
    DISPLAY_DEVICE --> USER_ACTION{User Action?}
    DISPLAY_CONTAINERS --> USER_ACTION
    
    USER_ACTION -->|Dispense Now| DISPENSE_CHECK{Device Online?}
    USER_ACTION -->|View Logs| LOGS_PAGE[Logs Page]
    USER_ACTION -->|Manage Schedule| SCHEDULE_PAGE[Schedule Page]
    USER_ACTION -->|Logout| LOGOUT[Sign Out]
    
    DISPENSE_CHECK -->|Yes| UPDATE_FB[Update Firebase: - Decrement pills - Set timestamp]
    DISPENSE_CHECK -->|No| DISABLED[Button Disabled]
    
    UPDATE_FB --> REALTIME_UPDATE[Real-time Update via onValue()]
    REALTIME_UPDATE --> DASHBOARD
    
    LOGS_PAGE --> LOAD_LOGS[Load pill_logs & reports]
    LOAD_LOGS --> FILTER{Filter/Search}
    FILTER --> EXPORT{Export CSV?}
    EXPORT -->|Yes| DOWNLOAD_CSV[Download Logs.csv]
    EXPORT -->|No| LOGS_PAGE
    
    SCHEDULE_PAGE --> SELECT_CONTAINER[Select Container 1-5]
    SELECT_CONTAINER --> MANAGE_TIMES[Add/Edit/Remove Schedule Times Max 3 per day]
    MANAGE_TIMES --> SAVE_SCHEDULE["Save to Firebase /schedules/{uid}"]
    SAVE_SCHEDULE --> SCHEDULE_PAGE
    
    LOGOUT --> LOGIN_PAGE
    
    style START fill:#90EE90
    style DASHBOARD fill:#87CEEB
    style LOGS_PAGE fill:#FFD700
    style SCHEDULE_PAGE fill:#FFA07A
```

---

## 11. COMPONENT INTERACTION DIAGRAM - ESP32

```mermaid
flowchart LR
    subgraph ESP32_MAIN["ESP32 Main Controller (PillDispenser.ino)"]
        MAIN[Main Loop Development Mode]
    end
    
    subgraph MODULES["Software Modules"]
        SERVO[ServoDriver Class]
        IR[IRSensor Class]
        LCD[LCDDisplay Class]
        RTC[RTClock Class]
        FB[FirebaseManager Class]
        GSM[SIM800L Class]
    end
    
    subgraph HARDWARE["Hardware Interfaces"]
        I2C_BUS[I2C Bus SDA:21 SCL:22]
        GPIO[GPIO Pins 34,35,32]
        WIRE3[3-Wire CLK,DAT,RST]
        UART[UART RX:16 TX:17]
        WIFI[WiFi Radio]
    end
    
    MAIN -->|begin| SERVO
    MAIN -->|begin| IR
    MAIN -->|begin| LCD
    MAIN -->|begin| RTC
    MAIN -->|begin/connectWiFi| FB
    MAIN -->|begin| GSM
    
    SERVO -->|Adafruit_PWMServoDriver| I2C_BUS
    LCD -->|LiquidCrystal_I2C| I2C_BUS
    IR -->|pinMode/digitalRead| GPIO
    RTC -->|ThreeWire| WIRE3
    GSM -->|SoftwareSerial| UART
    FB -->|Firebase_ESP_Client| WIFI
    
    I2C_BUS -.->|0x40| PCA[PCA9685]
    I2C_BUS -.->|0x27| LCD_HW[LCD 20x4]
    GPIO -.-> IR_HW[IR Sensors]
    WIRE3 -.-> RTC_HW[DS1302]
    UART -.-> GSM_HW[SIM800L]
    
    style MAIN fill:#4A90E2
    style SERVO fill:#90EE90
    style FB fill:#FFA500
```

---

## 12. DATA FLOW DIAGRAM - ML Detection Pipeline

```mermaid
flowchart TD
    TRIGGER[ESP32 Trigger UART Command] --> RPI_RECV[Raspberry Pi Receives Trigger]
    
    RPI_RECV --> FLASH_ON[HTTP Request ESP32-CAM Flash ON]
    
    FLASH_ON --> CAP1["Capture Image 1 HTTP GET /stream"]
    CAP1 --> SAVE1["Save to captured_images/"]
    
    SAVE1 --> DELAY1[Delay 300ms]
    DELAY1 --> CAP2[Capture Image 2]
    CAP2 --> SAVE2[Save Image 2]
    
    SAVE2 --> DELAY2[Delay 300ms]
    DELAY2 --> CAP3[Capture Image 3]
    CAP3 --> SAVE3[Save Image 3]
    
    SAVE3 --> FLASH_OFF[Flash OFF]
    
    FLASH_OFF --> PROC1[YOLOv8 Inference Image 1]
    PROC1 --> DET1[Detections 1: bbox, conf, class]
    
    FLASH_OFF --> PROC2[YOLOv8 Inference Image 2]
    PROC2 --> DET2[Detections 2]
    
    FLASH_OFF --> PROC3[YOLOv8 Inference Image 3]
    PROC3 --> DET3[Detections 3]
    
    DET1 --> ENSEMBLE[Ensemble Aggregation]
    DET2 --> ENSEMBLE
    DET3 --> ENSEMBLE
    
    ENSEMBLE --> VOTE[Spatial Grouping & Voting]
    VOTE --> CONF[Confidence Averaging]
    CONF --> FILTER[Threshold Filter 0.7 minimum]
    
    FILTER --> FINAL[Final Detection: Count & Class]
    
    FINAL --> UART_SEND[UART to ESP32: "PILLS_DETECTED:3:round_pill"]
    FINAL --> DB_SAVE[Save to SQLite: detection_sessions]
    
    UART_SEND --> ESP32_PROC[ESP32 Processing]
    DB_SAVE --> RPI_LOG[Logging Complete]
    
    style TRIGGER fill:#FFD700
    style ENSEMBLE fill:#87CEEB
    style FINAL fill:#90EE90
```

---

## 13. STATE MACHINE DIAGRAM - System States

```mermaid
stateDiagram-v2
    [*] --> PowerOn
    
    PowerOn --> Initialization: System Boot
    
    Initialization --> DevMode: DEVELOPMENT_MODE=true
    Initialization --> ProdMode: DEVELOPMENT_MODE=false
    
    DevMode --> Idle: Components OK
    ProdMode --> [*]: Not Implemented
    
    Idle --> CommandWait: Serial Available
    Idle --> SensorMonitor: 500ms Timer
    Idle --> Heartbeat: 30s Timer
    
    CommandWait --> ProcessCommand: Parse Input
    
    ProcessCommand --> TestServo: "test servo"
    ProcessCommand --> TestIR: "test ir"
    ProcessCommand --> TestLCD: "test lcd"
    ProcessCommand --> TestRTC: "test rtc"
    ProcessCommand --> Dispense: "dispense"
    ProcessCommand --> WiFiConnect: "wifi connect"
    ProcessCommand --> FirebaseTest: "firebase test"
    ProcessCommand --> ShowHelp: "help"
    ProcessCommand --> ShowStatus: "status"
    
    TestServo --> Idle: Complete
    TestIR --> Idle: Complete
    TestLCD --> Idle: Complete
    TestRTC --> Idle: Complete
    ShowHelp --> Idle: Display
    ShowStatus --> Idle: Display
    
    Dispense --> ServoActive: Activate Motor
    ServoActive --> PillMoving: IR Sequence
    PillMoving --> WaitML: Trigger RPi
    WaitML --> LogResult: Receive Count
    LogResult --> Idle: Update Firebase
    
    WiFiConnect --> Connecting: Attempt
    Connecting --> Connected: Success
    Connecting --> Idle: Failed
    Connected --> Idle: Return
    
    FirebaseTest --> FBTesting: Run Tests
    FBTesting --> FBConnected: All Pass
    FBTesting --> Idle: Failed
    FBConnected --> Idle: Return
    
    SensorMonitor --> ReadIR: Check All
    ReadIR --> Idle: Update State
    
    Heartbeat --> SendBeat: Log Time
    SendBeat --> Idle: Continue
```

---

## 14. DEPLOYMENT DIAGRAM

```mermaid
graph TB
    subgraph "Physical Device"
        subgraph "ESP32 Node"
            ESP32_FW[ESP32 Firmware Arduino C++ PillDispenser.ino]
            ESP32_LIBS[Libraries: - Firebase_ESP_Client - Adafruit_PWMServo - LiquidCrystal_I2C]
        end
        
        subgraph "ESP32-CAM Node"
            CAM_FW[ESP32-CAM Firmware IPCamera.ino]
            CAM_SERVER[HTTP Server - /stream - /flash/*]
        end
        
        subgraph "Raspberry Pi Node"
            RPI_SW[Python Application main.py]
            RPI_DEPS[Dependencies: - ultralytics (YOLOv8) - opencv-python - pyserial - sqlite3]
            YOLO_MODEL[YOLOv8 Model pill_detector_v1.pt]
            SQLITE[SQLite Database pill_dispenser.db]
        end
    end
    
    subgraph "Cloud Infrastructure"
        subgraph "Firebase Project"
            FB_RTDB[Realtime Database pilldispenser-5c037]
            FB_AUTH[Authentication Email/Password]
        end
        
        subgraph "Vercel Platform"
            NEXT_APP[Next.js 16.0.3 Production Build]
            API_ROUTES["API Routes - /api/dispenser - /api/dispense"]
        end
    end
    
    subgraph "Development Environment"
        ARDUINO_IDE[Arduino IDE ESP32 Board Support]
        VSCODE[VS Code Python/TypeScript]
        NODE[Node.js 18+ npm packages]
    end
    
    ESP32_FW -->|Upload via USB| ARDUINO_IDE
    CAM_FW -->|Upload via USB| ARDUINO_IDE
    RPI_SW -->|Deploy| VSCODE
    NEXT_APP -->|Build & Deploy| NODE
    
    ESP32_FW <-->|WiFi/HTTPS| FB_RTDB
    NEXT_APP <-->|REST API| FB_RTDB
    NEXT_APP -->|Auth| FB_AUTH
    
    RPI_SW -->|UART| ESP32_FW
    RPI_SW -->|HTTP| CAM_FW
    RPI_SW -->|Read/Write| SQLITE
    RPI_SW -->|Inference| YOLO_MODEL
    
    style ESP32_FW fill:#4A90E2
    style RPI_SW fill:#50C878
    style NEXT_APP fill:#9B59B6
    style FB_RTDB fill:#FFA500
```

---

## 15. CLASS DIAGRAM - Core C++ Classes (ESP32)

```mermaid
classDiagram
    class ServoDriver {
        -Adafruit_PWMServoDriver pwm
        -int servoMin
        -int servoMax
        -int servoCount
        +begin() void
        +setServoAngle(channel, angle) void
        +dispenseChannel(channel) void
        +dispensePair(channel1, channel2) void
        +testServo(channel) void
        +testAllServos() void
        +resetAllServos() void
    }
    
    class IRSensor {
        -uint8_t pin1, pin2, pin3
        -bool lastState1, lastState2, lastState3
        -unsigned long debounceTime
        +begin() void
        +isBlocked(sensorNum) bool
        +hasStateChanged(sensorNum) bool
        +readAllSensors() void
        +testSensor(sensorNum) void
        +testAllSensors() void
        +printSensorStates() void
    }
    
    class LCDDisplay {
        -LiquidCrystal_I2C lcd
        -int rows
        -int cols
        +begin() void
        +clear() void
        +print(line, text) void
        +printCentered(line, text) void
        +displayWelcome() void
        +displayStatus(status) void
        +testDisplay() void
    }
    
    class RTClock {
        -ThreeWire myWire
        -RtcDS1302 rtc
        +begin() void
        +setDateTime(dt) void
        +getDateTime() RtcDateTime
        +getTimeString() String
        +getDateString() String
        +isValid() bool
        +testRTC() void
    }
    
    class FirebaseManager {
        -FirebaseData fbdo
        -FirebaseAuth auth
        -FirebaseConfig config
        -String deviceId
        -bool isConnected
        -bool isAuthenticated
        +begin(apiKey, dbURL) bool
        +connectWiFi(ssid, pass) bool
        +sendPillDispenseLog(count, time) bool
        +updateDeviceStatus(status) bool
        +sendHeartbeat() bool
        +uploadSensorData(name, val) bool
        +sendPillReport(count, desc, status) bool
        +checkForCommands() bool
        +testConnection() bool
    }
    
    class SIM800L {
        -SoftwareSerial serial
        -uint8_t resetPin
        -bool isInitialized
        +begin() bool
        +sendSMS(number, message) bool
        +readSMS(index) String
        +makeCall(number) bool
        +hangupCall() bool
        +checkSignalStrength() int
        +testModule() bool
    }
    
    class PillDispenser {
        +setup() void
        +loop() void
        -initializeDevelopmentMode() void
        -handleSerialCommands() void
        -processSerialCommand(cmd) void
        -printHelpMenu() void
        -printSystemStatus() void
    }
    
    PillDispenser --> ServoDriver : uses
    PillDispenser --> IRSensor : uses
    PillDispenser --> LCDDisplay : uses
    PillDispenser --> RTClock : uses
    PillDispenser --> FirebaseManager : uses
    PillDispenser --> SIM800L : uses
```

---

## 16. CLASS DIAGRAM - Python Classes (Raspberry Pi)

```mermaid
classDiagram
    class ESP32CamInterface {
        -str camera_ip
        -str stream_url
        -str flash_url
        -bool is_connected
        +test_connection() bool
        +capture_single_frame() np.ndarray
        +capture_multiple_frames(num, delay) List~ndarray~
        +save_frame(frame, index) void
        +set_flash(state) bool
        +get_flash_status() bool
    }
    
    class EnsemblePillDetector {
        -str model_path
        -float confidence
        -YOLO model
        -dict classes
        +load_model() void
        +detect_single_image(img) List~Dict~
        +ensemble_detection(images) List~Dict~
        +aggregate_detections(all_det) List~Dict~
        +group_similar_detections(dets) List~List~
        +fallback_detection(img) List~Dict~
    }
    
    class ESP32Communicator {
        -str port
        -int baudrate
        -Serial serial_conn
        -bool is_connected
        -list message_queue
        +connect() bool
        +listen_for_triggers() bool
        +send_detection_results(dets) bool
        +send_simple_result(count, class) bool
        +send_command(cmd) str
    }
    
    class DatabaseManager {
        -str db_path
        -Connection conn
        +create_tables() void
        +log_detection_session(data) void
        +log_detection(session_id, det) void
        +get_session_stats(session_id) dict
        +get_all_sessions() List~dict~
        +export_to_csv(path) void
    }
    
    class PillDispenserSystem {
        -EnsemblePillDetector detector
        -ESP32CamInterface camera
        -ESP32Communicator esp32
        -DatabaseManager database
        -bool is_running
        -int session_counter
        +setup_logging() void
        +initialize() bool
        +process_detection_trigger() bool
        +run_triggered_mode() void
        +manual_detection_test() void
        +start() void
        +print_system_status() void
        +shutdown() void
    }
    
    PillDispenserSystem --> ESP32CamInterface : uses
    PillDispenserSystem --> EnsemblePillDetector : uses
    PillDispenserSystem --> ESP32Communicator : uses
    PillDispenserSystem --> DatabaseManager : uses
```

---

## 17. API ROUTE DIAGRAM - Next.js Web App

```mermaid
graph TB
    subgraph "Client Browser"
        CLIENT[React Components]
    end
    
    subgraph "Next.js API Routes"
        API_DISP["/api/dispenser GET & POST"]
        API_DISPENSE["/api/dispense POST"]
    end
    
    subgraph "Firebase Operations"
        GET_REF[ref(db, path)]
        SET_DATA[set(ref, data)]
        GET_DATA[get(ref)]
        ON_VALUE[onValue(ref, callback)]
    end
    
    subgraph "Firebase Database Paths"
        DISP_PATH["/pilldispenser/device/{deviceId}/status"]
        CONT_PATH["/pilldispenser/device/{deviceId}/dispensers"]
        DEV_PATH["/pilldispenser/device/{deviceId}"]
        SCHED_PATH["/pilldispenser/device/{deviceId}/schedules"]
        LOGS_PATH["/pilldispenser/pill_logs"]
        REP_PATH["/pilldispenser/reports"]
    end
    
    CLIENT -->|GET userId| API_DISP
    CLIENT -->|POST status, battery| API_DISP
    CLIENT -->|POST pills, userId| API_DISPENSE
    
    API_DISP -->|Read| GET_DATA
    API_DISP -->|Write| SET_DATA
    API_DISPENSE -->|Read/Write| GET_DATA
    API_DISPENSE -->|Update| SET_DATA
    
    GET_DATA --> DISP_PATH
    SET_DATA --> DISP_PATH
    GET_DATA --> CONT_PATH
    SET_DATA --> CONT_PATH
    
    CLIENT -->|Real-time| ON_VALUE
    ON_VALUE --> CONT_PATH
    ON_VALUE --> DEV_PATH
    ON_VALUE --> SCHED_PATH
    ON_VALUE --> LOGS_PATH
    ON_VALUE --> REP_PATH
    
    style CLIENT fill:#9B59B6
    style API_DISP fill:#4A90E2
    style API_DISPENSE fill:#50C878
```

---

## SUMMARY OF IMPLEMENTED FEATURES

### Hardware Layer
- ✅ ESP32 main controller with I2C, UART, GPIO
- ✅ PCA9685 16-channel servo driver
- ✅ 3 IR sensors for pill detection sequence
- ✅ I2C LCD 20x4 display
- ✅ DS1302 RTC with battery backup
- ✅ SIM800L GSM module (implemented, SMS ready)
- ✅ ESP32-CAM for image capture with flash control

### Processing Layer
- ✅ Raspberry Pi 4 with Python 3.x
- ✅ YOLOv8 machine learning model for pill detection
- ✅ Ensemble detection across 3 images
- ✅ SQLite database for local logging
- ✅ Serial UART communication with ESP32

### Cloud & Web Layer
- ✅ Firebase Realtime Database integration
- ✅ Firebase Authentication (email/password)
- ✅ Next.js 16.0.3 web application
- ✅ Real-time dashboard with 5 containers
- ✅ Schedule management (1-3 times/day per container)
- ✅ Logs page with filter/search/export
- ✅ API endpoints for hardware integration

### Communication Protocols
- ✅ I2C (LCD, PCA9685)
- ✅ UART (ESP32 ↔ RPi, SIM800L)
- ✅ WiFi (ESP32 ↔ Firebase, ESP32-CAM)
- ✅ HTTP (RPi ↔ ESP32-CAM)
- ✅ WebSocket/Real-time (Web ↔ Firebase)

---

*All diagrams generated from actual codebase implementation.*
*No imaginary or speculative features included.*
*Date: December 3, 2025*

