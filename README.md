# ⚙️ ServoControlMsp430LabView  

## 📌 Project Overview  
This project demonstrates **servo motor control** using an **MSP430** microcontroller, with a **LabVIEW interface** for user input. The **SG90 servo motor** is controlled via **UART communication**, allowing real-time adjustments from a LabVIEW control panel.  

## 🛠️ Features  
- **MSP430 Board** used for servo control  
- **SG90 Servo Motor** connected to the microcontroller  
- **Serial Print Feature** similar to Arduino environments, using **UART**  
- **LabVIEW Interface** for user commands and real-time control  

## 📸 Project Images  
### 🔹 LabVIEW Control Panel  
<p align="center">
    <img src="https://github.com/user-attachments/assets/3b8b3e6c-df04-4c57-9684-d140adeb16b7" 
         alt="LabVIEW Control Panel" height="400"/>
</p>
<p align="center"><em>Figure 1: LabVIEW interface for servo control</em></p>

### 🔹 Block Diagram  
<p align="center">
    <img src="https://github.com/user-attachments/assets/a373e4b2-f182-4835-a3d3-1f16b4c0c5d1" 
         alt="Block Diagram" height="400"/>
</p>
<p align="center"><em>Figure 2: LabVIEW block diagram for serial communication</em></p>

## 🚀 How It Works  
1. **User inputs commands** from the **LabVIEW control panel**  
2. **MSP430 reads UART signals** and processes the command  
3. The **servo motor adjusts its position** accordingly  
4. A **feedback mechanism** prints status updates via **serial communication**  

## 🔧 Setup & Installation  
1. **Clone the repository:**  
   ```sh
   git clone https://github.com/your-username/ServoControlMsp430LabView.git

