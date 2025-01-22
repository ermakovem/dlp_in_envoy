# master's diploma
dlp in service mesh

1. lua
2. wasm (rust)
3. cpp
4. external service

https://cloud.google.com/service-extensions/docs/prepare-plugin-code
docker run -v $PWD:/work -w /work wasmsdk:v3 /build_wasm.sh myproject.wasm

curl -X POST http://localhost:8080 -H "Content-Type: application/json" -d '{"key1": 1234 123456}' -v