#include "proxy_wasm_intrinsics.h"
#include <regex>
#include <iostream>

#include <string>

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
    std::string mode = "Pass";
    WasmDataPtr chunk = getBufferBytes(WasmBufferType::HttpRequestBody, 0, body_buffer_length);
    request_chunks.push_back(std::move(chunk));


    if (end_of_stream) {
      if (DLP(content_type, request_chunks)) {
        if (mode == "Audit") {
          LOG_WARN("Warn by DLP");
        }
        if (mode == "Block") {
          sendLocalResponse(403, "Blocked by DLP", "Blocked by DLP", {});
          LOG_ERROR("Blocked by DLP");
        }
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

    std::string mode = "Block";
    WasmDataPtr chunk = getBufferBytes(WasmBufferType::HttpResponseBody, 0, body_buffer_length);
    response_chunks.push_back(std::move(chunk));

    if (end_of_stream) {
      replaceResponseHeader(":status", "403");
      if (DLP(content_type, response_chunks)) {
        if (mode == "Audit") {
          LOG_WARN("Warn by DLP");
        }
        if (mode == "Block") {
          replaceResponseHeader(":status", "403");
          setBuffer(WasmBufferType::HttpResponseBody, 0, body_buffer_length, "Blocked by DLP");
          LOG_ERROR("Blocked by DLP");
        }
      }
    }                                   
    return FilterDataStatus::Continue;
  }
 private:
 std::vector<WasmDataPtr> request_chunks;
 std::vector<WasmDataPtr> response_chunks;
 std::string content_type;
 
 bool DLP(const std::string& content_type, const std::vector<WasmDataPtr>& chunks) {
  std::string full_body;
  for (auto& chunk : chunks) {
    full_body += std::string(chunk->view());
  }
  return containsRussianPassport(full_body) || contains_credit_card(full_body);
 }

 bool containsRussianPassport(const std::string& data) const {
  std::regex pattern(R"(\d{4}\s\d{6})");
  return std::regex_search(data, pattern);
 }


bool luhn_check(const std::string& num) {
    int sum = 0;
    bool alt = false;
    for (int i = num.length() - 1; i >= 0; --i) {
        int n = num[i] - '0';
        if (alt) {
            n *= 2;
            if (n > 9) n -= 9;
        }
        sum += n;
        alt = !alt;
    }
    return (sum % 10 == 0);
}

bool contains_credit_card(const std::string& data) {
    std::regex card_pattern("\\d{4}[-\\s]?\\d{4}[-\\s]?\\d{4}[-\\s]?\\d{4}");
    std::smatch match;
    if (std::regex_search(data, match, card_pattern)) {
        std::string card_number;
        for (char c : match.str()) {
            if (std::isdigit(c)) {
                card_number += c;
            }
        }
        return luhn_check(card_number);
    }
    return false;
}

};

static RegisterContextFactory register_StaticContext(
    CONTEXT_FACTORY(DlpFilter), ROOT_FACTORY(RootContext));