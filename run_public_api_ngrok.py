"""
Exoskeleton API Public Tunneling Service
Uses pyngrok to automatically expose the local FastAPI server to a public HTTPS URL.
"""
import os
import sys
import argparse
import warnings

# Suppress warnings
warnings.filterwarnings("ignore")

# Add the pipeline directory to path first so imports work correctly
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "exoskeleton_pipeline"))

def start_public_api(authtoken=None, port=8000):
    try:
        from pyngrok import ngrok, conf
    except ImportError:
        print("[-] pyngrok package not found. Please install it using: pip install pyngrok")
        return

    # 1. Set Auth Token if provided
    if authtoken:
        print(f"[+] Configuring Ngrok Authtoken...")
        ngrok.set_auth_token(authtoken)

    # 2. Import the FastAPI app
    try:
        from app import app
        import uvicorn
    except ImportError as e:
        print(f"[-] Failed to import FastAPI app: {e}")
        return

    print(f"[+] Starting secure public HTTPS tunnel on port {port}...")
    try:
        # Open tunnel
        tunnel = ngrok.connect(port)
        public_url = tunnel.public_url
        print("=" * 70)
        print("NGROK PUBLIC TUNNEL CREATED SUCCESSFULLY!")
        print(f"Public API URL:   {public_url}")
        print(f"Health Check:     {public_url}/health")
        print(f"Prediction URL:   {public_url}/predict")
        print("=" * 70)
        print("[*] Press Ctrl+C to stop the tunnel and server.\n")
    except Exception as e:
        print(f"[-] Ngrok failed to start tunnel: {e}")
        print("[!] Make sure your ngrok authtoken is correct.")
        print("[!] You can get it from: https://dashboard.ngrok.com/get-started/your-authtoken")
        return

    # 3. Start local Uvicorn server (Ngrok forwards to this local port)
    try:
        uvicorn.run(app, host="127.0.0.1", port=port, log_level="warning")
    except KeyboardInterrupt:
        print("\n[+] Stopping public API tunnel and local server...")
    finally:
        ngrok.kill()
        print("[+] Offlined successfully.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run Exoskeleton API Publicly via Ngrok")
    parser.add_argument("--token", type=str, default=None, help="Your Ngrok Authtoken")
    parser.add_argument("--port", type=int, default=8000, help="Local port to bind FastAPI")
    args = parser.parse_args()

    start_public_api(authtoken=args.token, port=args.port)
