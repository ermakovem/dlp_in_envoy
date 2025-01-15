# backend_service.py
from flask import Flask, jsonify, request

app = Flask(__name__)

last_message = ''

@app.route('/', methods=['GET'])
def get():
    return jsonify(last_message)

@app.route('/', methods=['POST'])
def post():
    global last_message
    last_message = request.get_json()
    return jsonify(message="POST request succsesfull!")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)