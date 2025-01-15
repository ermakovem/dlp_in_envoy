# Dockerfile
FROM python:3.9-slim

WORKDIR /app

COPY backend_service.py /app/
COPY requirements.txt /app/

RUN pip install -r requirements.txt

CMD ["python", "backend_service.py"]
