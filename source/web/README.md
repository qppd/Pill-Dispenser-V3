# Pill Dispenser Web App

A modern, user-friendly web application for managing smart pill dispensers. Built with Next.js, Firebase Authentication, and Firebase Realtime Database.

---

## ✨ Features

- **Light Mode Only**: Clean, bright, and accessible interface
- **User Authentication**: Secure login and registration (Firebase Auth)
- **Real-Time Monitoring**: Instantly view pill counts, schedules, and device status
- **Single Device Battery/Status**: One battery and online/offline status for the whole device
- **Schedule Management**: Set 1-3 daily schedules per container (5 containers supported)
- **Manual Dispense**: Dispense pills remotely from the dashboard
- **API Endpoints**: For ESP32/RPi hardware integration
- **Responsive Design**: Mobile, tablet, and desktop support
- **Modern UI/UX**: Beautiful gradients, smooth animations, and intuitive navigation

---

## 🖥️ UI Overview

- **Navigation Bar**: Sticky top bar with links to Dashboard, Logs, Schedule, Login/Register
- **Dashboard**: 
  - Device status (battery %, online/offline)
  - 5 container cards (pills remaining, next dose, last dispensed)
  - Dispense button (disabled if device offline)
- **Logs**: Pill dispense logs and reports, filter/search, export to CSV
- **Schedule**: Manage up to 3 schedules per container, enable/disable times

---

## 🗂️ Project Structure

- `src/app/` - Next.js app router pages
  - `dashboard/` - Main dispenser dashboard
  - `logs/` - Dispense logs and reports
  - `schedule/` - Schedule management
  - `login/`, `register/` - Auth pages
  - `api/` - API routes for hardware
- `src/components/` - Navigation and reusable UI
- `src/lib/` - Firebase config/utilities

---

## 🗄️ Data Structure

- `dispensers/{userId}/device`:
  ```json
  {
    "battery": 85,
    "status": "online" // or "offline"
  }
  ```
- `dispensers/{userId}/containers`:
  ```json
  [
    {
      "id": 1,
      "name": "Container 1",
      "pillsRemaining": 30,
      "lastDispensed": "Never",
      "nextDose": "08:00"
    },
    // ... up to 5 containers
  ]
  ```

---

## 🚀 Getting Started

1. **Clone & Install**
   ```bash
   git clone https://github.com/qppd/Pill-Dispenser-V3.git
   cd Pill-Dispenser-V3/source/web
   npm install
   ```
2. **Configure Firebase**
   - Create a Firebase project
   - Enable Auth & Realtime Database
   - Add your config to `.env.local`
3. **Run Locally**
   ```bash
   npm run dev
   ```
   Visit [http://localhost:3000](http://localhost:3000)

---

## 🔌 API Endpoints

- `GET /api/dispenser?userId={userId}`: Get dispenser/device data
- `POST /api/dispenser`: Update device status from hardware
- `POST /api/dispense`: Trigger pill dispensing

---

## 🛠️ Contribution

1. Fork and clone the repo
2. Create a new branch (`git checkout -b feature/your-feature`)
3. Commit your changes (`git commit -m 'feat: add new feature'`)
4. Push and open a Pull Request

---

## 📄 License

MIT

---

## 📚 Learn More

- [Next.js Documentation](https://nextjs.org/docs)
- [Firebase Documentation](https://firebase.google.com/docs)
- [Tailwind CSS](https://tailwindcss.com/docs)
