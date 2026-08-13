from flask import Flask
import subprocess
import time
import threading

app = Flask(__name__)

# ---------------- PHONE DETAILS ----------------
PHONE_NUMBER = "+916381389236"
CALL_DURATION = 10

# ADB connected Android device
ADB_DEVICE = "192.168.43.191:42519"


# ---------------- ADB COMMAND ----------------
def run_adb(cmd):
    full_cmd = ["adb", "-s", ADB_DEVICE] + cmd

    result = subprocess.run(
        full_cmd,
        capture_output=True,
        text=True
    )

    return result.stdout.strip(), result.stderr.strip()


# ---------------- MAKE PHONE CALL ----------------
def make_call():

    print("\n📞 Calling", PHONE_NUMBER)

    out, err = run_adb([
        "shell",
        "am",
        "start",
        "-a",
        "android.intent.action.CALL",
        "-d",
        f"tel:{PHONE_NUMBER}"
    ])

    if err:
        print("⚠ Error starting call:")
        print(err)
        return

    print("✅ Call started")

    # Keep call active for specified duration
    time.sleep(CALL_DURATION)

    # End call
    print("📴 Hanging up...")

    run_adb([
        "shell",
        "input",
        "keyevent",
        "6"
    ])

    print("✅ Call ended\n")


# ---------------- FIRE ALERT API ----------------
@app.route("/firealert", methods=["GET"])
def fire_alert():

    print("\n🔥 FIRE ALERT RECEIVED FROM NODEMCU!")

    # Start call in separate thread
    # so Flask server does not get blocked
    threading.Thread(
        target=make_call,
        daemon=True
    ).start()

    return "OK", 200


# ---------------- START SERVER ----------------
if __name__ == "__main__":

    print("====================================")
    print("🚀 FIRE ALERT SERVER STARTED")
    print("====================================")
    print("Server: http://0.0.0.0:8080")
    print("Waiting for NodeMCU fire alert...")
    print("====================================")

    app.run(
        host="0.0.0.0",
        port=8080
    )
