-- dlp_filter.lua


local function luhn_check(num)
  local sum, alt = 0, false
  for i = #num, 1, -1 do
      local n = tonumber(num:sub(i, i))
      if alt then
          n = n * 2
          if n > 9 then n = n - 9 end
      end
      sum = sum + n
      alt = not alt
  end
  return sum % 10 == 0
end

local function contains_credit_card(data)
  for card in string.gmatch(data, "%d%d%d%d[%s-]?%d%d%d%d[%s-]?%d%d%d%d[%s-]?%d%d%d%d") do
      if luhn_check(card:gsub("%D", "")) then
          return true
      else
        return false
      end
  end
end


local function contains_russian_passport(data)

  local pattern = "%d%d%d%d%s%d%d%d%d%d%d"

  if string.match(data, pattern) then
      return true
  else
      return false
  end
end


local function DLP(content_type, data)
  return (contains_credit_card(data) or contains_russian_passport(data))
end

function envoy_on_request(request_handle)
  local body = request_handle:body()
  local headers = request_handle:headers()
  local mode = "Block"

  if (body ~= nil) then
    local text = body:getBytes(0,body:length())
    local content_type = headers:get("content-type")

    if (DLP(content_type, text)) then
      if (mode == "Audit") then
        request_handle:logWarn("Warn by DLP")
      end
      if (mode == "Block") then
        request_handle:respond(
          {
            [":status"] = "403",
            ["content-type"] = "text/plain",
            ["x-reason"] = "Blocked by DLP"
          },
          "Blocked by DLP"
        )
        request_handle:logErr("Blocked by DLP")
      end
    end
  end
end

function envoy_on_response(response_handle)
  local body = response_handle:body()
  local headers = response_handle:headers()
  local mode = "Pass"

  if (body ~= nil) then
    local text = body:getBytes(0,body:length())
    local content_type = headers:get("content-type")

    if (DLP(content_type, text)) then
      if (mode == "Audit") then
        response_handle:logWarn("Warn by DLP")
      end
      if (mode == "Block") then
        headers:replace(":status", "403")
        headers:replace("x-reason", "Blocked by DLP")
        response_handle:logErr("Blocked by DLP")
      end
    end
  end
end
