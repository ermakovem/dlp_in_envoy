import requests
import time
import json

if __name__ == "__main__":
    url = 'http://localhost:8080'

    with open('test_data.json', 'r', encoding='utf-8') as file:
      for test_json in json.load(file):
        start = time.time()
        request = requests.post(url, json=test_json)
        end = time.time()
        print("POST:", end - start)
        time.sleep(2)
        start = time.time()
        request = requests.get(url)
        end = time.time()
        print("GET: ", end - start)
        time.sleep(2)
