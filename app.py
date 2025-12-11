# app.py - Complete IoT Device Security Monitoring Web App
from flask import Flask, render_template, jsonify, request
import json
import threading
import time
import random
import datetime
from collections import defaultdict
import os

app = Flask(__name__)

# Global state for IoT devices
devices = {}
alerts_history = []
lock = threading.Lock()

class IoTDevice:
    def __init__(self, device_id, name, ip):
        self.id = device_id
        self.name = name
        self.ip = ip
        self.mac = f"{random.randint(0x00,0xff):02x}:{random.randint(0x00,0xff):02x}:{random.randint(0x00,0xff):02x}:{random.randint(0x00,0xff):02x}:{random.randint(0x00,0xff):02x}:{random.randint(0x00,0xff):02x}"
        self.status = "online"
        self.cpu = random.uniform(5, 25)
        self.memory = random.uniform(10, 40)
        self.disk = random.uniform(20, 60)
        self.ports = random.randint(3, 8)
        self.connections = random.randint(5, 20)
        self.failed_logins = 0
        self.last_seen = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.firmware = f"v{random.randint(1,3)}.{random.randint(0,9)}.{random.randint(0,9)}"
        self.alerts = []
        self.risk_score = 0
    
    def to_dict(self):
        return {
            'id': self.id,
            'name': self.name,
            'ip': self.ip,
            'mac': self.mac,
            'status': self.status,
            'cpu': round(self.cpu, 1),
            'memory': round(self.memory, 1),
            'disk': round(self.disk, 1),
            'ports': self.ports,
            'connections': self.connections,
            'failed_logins': self.failed_logins,
            'last_seen': self.last_seen,
            'firmware': self.firmware,
            'alerts': self.alerts[-5:],  # Last 5 alerts
            'risk_score': round(self.calculate_risk(), 1)
        }
    
    def calculate_risk(self):
        score = 0
        score += self.cpu * 0.4
        score += self.memory * 0.3
        score += (self.ports / 50.0) * 20
        score += (self.failed_logins / 10.0) * 15
        score += (self.connections / 100.0) * 10
        return min(100, score)

def initialize_devices():
    """Initialize sample IoT devices"""
    global devices
    device_list = [
        ("thermostat", "🏠 Smart Thermostat", "192.168.1.101"),
        ("camera", "📹 Security Camera", "192.168.1.102"),
        ("light", "💡 Smart Bulb", "192.168.1.103"),
        ("door", "🚪 Door Sensor", "192.168.1.104"),
        ("speaker", "🔊 Voice Assistant", "192.168.1.105"),
        ("weather", "🌡️ Weather Station", "192.168.1.106"),
        ("shower", "🚿 Smart Shower", "192.168.1.107"),
        ("doorbell", "🛎️ Doorbell Camera", "192.168.1.108"),
        ("plug", "🔌 Smart Plug", "192.168.1.109"),
        ("leak", "🚰 Water Sensor", "192.168.1.110"),
    ]
    
    with lock:
        for device_id, name, ip in device_list:
            devices[device_id] = IoTDevice(device_id, name, ip)

def generate_alert(device, alert_type):
    """Generate security alert"""
    alert_messages = {
        "high_cpu": "🚨 HIGH CPU USAGE (>95%)",
        "high_memory": "⚠️  CRITICAL MEMORY (>90%)",
        "port_scan": "🔍 PORT SCAN DETECTED",
        "bruteforce": "🔒 BRUTE FORCE ATTEMPTS",
        "suspicious": "🌐 SUSPICIOUS CONNECTION",
        "traffic": "📈 UNUSUAL TRAFFIC SPIKE"
    }
    
    alert = f"{alert_messages.get(alert_type, '⚠️  SECURITY ALERT')} - {datetime.datetime.now().strftime('%H:%M:%S')}"
    device.alerts.append(alert)
    alerts_history.append({
        'device': device.name,
        'ip': device.ip,
        'alert': alert,
        'timestamp': datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    })

def simulate_security_events():
    """Simulate real-time IoT security events"""
    while True:
        time.sleep(3)
        
        with lock:
            for device_id, device in devices.items():
                # Update metrics
                device.cpu = random.uniform(5, 100)
                device.memory = random.uniform(10, 95)
                device.disk = random.uniform(20, 85)
                device.ports = random.randint(2, 65)
                device.connections = random.randint(5, 150)
                device.failed_logins = random.randint(0, 15)
                device.last_seen = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                
                # Threat detection
                threats_detected = []
                
                if device.cpu > 95:
                    threats_detected.append("high_cpu")
                if device.memory > 90:
                    threats_detected.append("high_memory")
                if device.ports > 45:
                    threats_detected.append("port_scan")
                if device.failed_logins > 8:
                    threats_detected.append("bruteforce")
                
                # Random threats
                if random.random() < 0.03:
                    threats_detected.append("suspicious")
                if random.random() < 0.02:
                    threats_detected.append("traffic")
                
                # Generate alerts and update status
                if threats_detected:
                    for threat in threats_detected:
                        generate_alert(device, threat)
                    
                    if random.random() < 0.2:
                        device.status = "compromised"
                    else:
                        device.status = "suspicious"
                elif random.random() < 0.08:
                    device.status = "offline"
                else:
                    device.status = "online"
                
                # Limit alerts history
                if len(device.alerts) > 10:
                    device.alerts = device.alerts[-10:]

# Flask Routes
@app.route('/')
def dashboard():
    return render_template('index.html')

@app.route('/api/devices')
def get_devices():
    with lock:
        return jsonify([device.to_dict() for device in devices.values()])

@app.route('/api/stats')
def get_stats():
    with lock:
        total = len(devices)
        online = sum(1 for d in devices.values() if d.status == "online")
        suspicious = sum(1 for d in devices.values() if d.status == "suspicious")
        compromised = sum(1 for d in devices.values() if d.status == "compromised")
        offline = total - online - suspicious - compromised
        
        high_risk = sum(1 for d in devices.values() if d.risk_score > 70)
        
        return jsonify({
            'total': total,
            'online': online,
            'suspicious': suspicious,
            'compromised': compromised,
            'offline': offline,
            'high_risk': high_risk
        })

@app.route('/api/alerts')
def get_alerts():
    return jsonify(alerts_history[-20:])  # Last 20 alerts

@app.route('/api/action/<device_id>/<action>')
def take_action(device_id, action):
    with lock:
        if device_id in devices:
            device = devices[device_id]
            
            if action == "quarantine":
                device.status = "quarantined"
                device.alerts.append(f"🔒 QUARANTINED by admin - {datetime.datetime.now().strftime('%H:%M:%S')}")
            elif action == "restart":
                device.status = "online"
                device.cpu = random.uniform(5, 20)
                device.alerts.append(f"🔄 RESTARTED by admin - {datetime.datetime.now().strftime('%H:%M:%S')}")
            elif action == "scan":
                device.alerts.append(f"🔍 SCAN initiated - {datetime.datetime.now().strftime('%H:%M:%S')}")
            
            generate_alert(device, "action")
            return jsonify({'status': 'success', 'action': action})
    
    return jsonify({'status': 'error', 'message': 'Device not found'}), 404

@app.route('/health')
def health():
    return jsonify({'status': 'healthy', 'timestamp': datetime.datetime.now().isoformat()})

if __name__ == '__main__':
    # Initialize devices and start simulation
    initialize_devices()
    sim_thread = threading.Thread(target=simulate_security_events, daemon=True)
    sim_thread.start()
    
    print("🚀 IoT Security Monitor starting...")
    print("📱 Dashboard: http://localhost:5000")
    print("📊 API Docs: http://localhost:5000/api/devices")
    
    app.run(debug=False, host='0.0.0.0', port=5000)
