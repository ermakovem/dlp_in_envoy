#include "proxy_wasm_intrinsics.h"
#include <regex>

class DlpFilter : public Context {
 public:
  explicit DlpFilter(uint32_t id, RootContext* root) : Context(id, root) {}

  FilterHeadersStatus onRequestHeaders(uint32_t headers,
                                       bool end_of_stream) override {
    auto result = getRequestHeaderPairs();
    auto pairs = result->pairs();
    for (auto &p : pairs) {
      if (p.first == "content-type") {
        content_type = p.second;
      }
    } 
    return FilterHeadersStatus::Continue;
  }
 
  FilterDataStatus onRequestBody(size_t body_buffer_length,
                                         bool end_of_stream) override {
    WasmDataPtr chunk = getBufferBytes(WasmBufferType::HttpRequestBody, 0, body_buffer_length);
    request_chunks.push_back(std::move(chunk));

    if (end_of_stream) {
      std::string full_body;
      for (auto& chunk : request_chunks) {
        full_body += std::string(chunk->view());
      }
      if (containsRussianPassport(full_body)) {
        LOG_INFO("DLP found passport on request");
      }
    }
                                   
    return FilterDataStatus::Continue;
  }

  FilterHeadersStatus onResponseHeaders(uint32_t headers,
                                        bool end_of_stream) override {
    auto result = getResponseHeaderPairs();
    auto pairs = result->pairs();
    for (auto &p : pairs) {
      if (p.first == "content-type") {
        content_type = p.second;
      }
    }
    return FilterHeadersStatus::Continue;
  }

  FilterDataStatus onResponseBody(size_t body_buffer_length,
                                          bool end_of_stream) override {
    WasmDataPtr chunk = getBufferBytes(WasmBufferType::HttpResponseBody, 0, body_buffer_length);
    response_chunks.push_back(std::move(chunk));

    if (end_of_stream) {
      std::string full_body;
      for (auto& chunk : response_chunks) {
        full_body += std::string(chunk->view());
      }
      if (containsRussianPassport(full_body)) {
        LOG_INFO("DLP found passport on response");
      }
    }
                                        
    return FilterDataStatus::Continue;
  }
 private:
 std::vector<WasmDataPtr> request_chunks;
 std::vector<WasmDataPtr> response_chunks;
 std::string content_type;

 bool containsRussianPassport(const std::string& data) const {
  std::regex pattern(R"(\d{4}\s\d{6})");
  return std::regex_search(data, pattern);
 }

};

static RegisterContextFactory register_StaticContext(
    CONTEXT_FACTORY(DlpFilter), ROOT_FACTORY(RootContext));