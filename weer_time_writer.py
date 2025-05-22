import requests
import time

def haal_weer_op():
    url = f"https://wttr.in/?format=%T - %t - %C&{int(time.time())}"
    try:
        response = requests.get(url)
        if response.status_code == 200:
            with open("weerinfo.txt", "w", encoding="utf-8") as f:
                f.write(response.text.strip())
        else:
            with open("weerinfo.txt", "w", encoding="utf-8") as f:
                f.write("Fout bij ophalen van weerinformatie")
    except Exception as e:
        with open("weerinfo.txt", "w", encoding="utf-8") as f:
            f.write(f"Netwerkfout: {e}")

if __name__ == "__main__":
    while True:
        haal_weer_op()
        time.sleep(60)
