import requests
import time
import json
import matplotlib
import matplotlib.pyplot as plt

def measure_request_time(test_json):
  url = 'http://localhost:8080'
  json_size = len(json.dumps(test_json).encode('utf-8'))
  start = time.time()
  request = requests.post(url, json=test_json)
  end = time.time()
  request = requests.get(url, json=test_json)
  print("POST:", end - start, "SIZE:", json_size)
  return json_size, (end - start) * 1000

  
  

if __name__ == "__main__":
    sizes = []
    post_times = []
    with open('test_data.json', 'r', encoding='utf-8') as file:
      for test_json in json.load(file):
        size, post_time = measure_request_time(test_json)
        sizes.append(size)
        post_times.append(post_time)

    
    plt.plot(sizes, post_times, marker='o')
    plt.xlabel('JSON Size (bytes)')
    plt.ylabel('POST Request Time (ms)')
    plt.title('Dependency of POST Request Time on JSON Size')
    plt.grid(True)
    plt.savefig('output.png')