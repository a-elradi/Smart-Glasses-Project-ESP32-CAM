from flask import Flask, request
import os
import time

app = Flask(__name__)
UPLOAD_FOLDER = 'received_images'

if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

@app.route('/upload', methods=['POST'])
def upload_file():
    if request.content_type != 'image/jpeg':
        return "Invalid content type", 400
    
    # Save the frame with a timestamp
    filename = f"capture_{int(time.time())}.jpg"
    filepath = os.path.join(UPLOAD_FOLDER, filename)
    
    with open(filepath, 'wb') as f:
        f.write(request.data)
    
    print(f"Received {filename} - Size: {len(request.data)} bytes")
    return "Image Received", 200

if __name__ == '__main__':
    # Use 0.0.0.0 to listen on all interfaces (important for WiFi connection)
    app.run(host='0.0.0.0', port=5000)
