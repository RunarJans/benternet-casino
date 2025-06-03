import requests
import time
import zmq

# Hasselt locatie
lat, lon = 50.9307, 5.3370

# ZMQ PUSH setup
context = zmq.Context()
socket = context.socket(zmq.PUSH)
socket.connect("tcp://benternet.pxl-ea-ict.be:24041")

def fetch_weather():
    try:
        url = f"https://api.open-meteo.com/v1/forecast?latitude={lat}&longitude={lon}&current_weather=true"
        response = requests.get(url, timeout=5)
        response.raise_for_status()
        data = response.json()
        if 'current_weather' in data:
            temp = data['current_weather']['temperature']
            cond = data['current_weather']['weathercode']
            tijd = time.strftime("%H:%M:%S%z")
            return f"{tijd} - {temp}°C - Code {cond}"
        else:
            return "Geen actuele weersgegevens"
    except Exception as e:
        return f"Fout bij opvragen weer: {e}"

if __name__ == "__main__":
    print("🌤️ Weer-service actief (Python)...")
    time.sleep(1)  # laat netwerk even opstarten
    while True:
        weerbericht = fetch_weather()
        bericht = f"weer!>{weerbericht}"
        print("📤 Verzonden naar clients:", bericht)
        socket.send_string(bericht)
        time.sleep(60)
