# Exoskeleton EMG Control API Deployment Repository

This repository contains the production-ready, minimal files needed to deploy the **Exoskeleton EMG Control API** to [Render](https://render.com) (or any other cloud provider like Heroku, Railway, or AWS).

## 📁 Repository Structure

* 📄 `app.py`: FastAPI server with fully-configured **CORS Middleware** (allowing access from any origin), dynamic host/port detection, and endpoints (`/health` and `/predict`).
* 📄 `pipeline.py`: Digital Signal Processing (DSP) pipeline that handles raw EMG filtering, rectification, low-pass linear envelop extraction, downsampling, and time-domain feature calculations.
* 📄 `best_regressor_model.pkl`: The 19MB production-grade PyCaret regression model trained on biomechanics data to predict joint angles in real-time.
* 📄 `requirements.txt`: Pinpointed, compatible Python packages for PyCaret, FastAPI, and Uvicorn.

---

## 🚀 How to Deploy on Render

### Step 1: Create a GitHub Repository
1. Initialize Git in this directory (or create a new repository on GitHub and upload only the files in this folder).
2. Run these commands in your command prompt:
   ```bash
   git init
   git add .
   git commit -m "Initialize Exoskeleton API deployment repository"
   git branch -M main
   git remote add origin <your-github-repo-url>
   git push -u origin main
   ```

### Step 2: Set up a Web Service on Render
1. Log into your account on [Render.com](https://render.com).
2. Click **New** -> **Web Service**.
3. Connect your GitHub account and select your newly created repository.
4. Configure the Web Service settings as follows:
   * **Name**: `exoskeleton-emg-api` (or any custom name)
   * **Environment / Runtime**: `Python`
   * **Region**: Select the region closest to you
   * **Branch**: `main`
   * **Build Command**: `pip install -r requirements.txt`
   * **Start Command**: `python app.py`
5. Under the **Instance Type**, select **Free** (or any tier).
6. Click **Deploy Web Service** at the bottom of the page!

Render will automatically install all dependencies, load your 19MB PyCaret model, and bring your API live!

---

## 🧪 Testing Your Live API

Once deployed, your API will be live at a URL like `https://exoskeleton-emg-api.onrender.com`.

### 1. Health Check Endpoint
Querying `/health` checks if your model is active:
* **URL**: `https://<your-render-url>/health`
* **Method**: `GET`
* **Response**:
  ```json
  {
    "status": "OK",
    "details": "All systems operational.",
    "loaded_model": "best_regressor_model"
  }
  ```

### 2. Live Joint Angle Prediction Endpoint
Submit raw high-frequency 11-channel EMG signals:
* **URL**: `https://<your-render-url>/predict`
* **Method**: `POST`
* **Headers**: `Content-Type: application/json`
* **Payload**:
  ```json
  {
    "emg_window": [[0.05, 0.02, 0.04, 0.1, 0.12, 0.08, 0.05, 0.03, 0.01, 0.02, 0.01], ...]
  }
  ```
  *(Pass a list of lists representing at least 200 samples of 11-channel raw EMG).*
