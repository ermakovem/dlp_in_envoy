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

function envoy_on_request(request_handle)
  local body = request_handle:body()
  
  if (body ~= nil) then
    text = body:getBytes(0,body:length())

    if (contains_russian_passport(text)) then
      request_handle:logWarn("DLP found passport on request")
    end

    if (contains_credit_card(text)) then
      request_handle:logWarn("DLP found credit card on request")
    end
  end
end

function envoy_on_response(response_handle)
  local body = response_handle:body()

  if (body ~= nil) then
    text = body:getBytes(0,body:length())

    if (contains_russian_passport(text)) then
      response_handle:logWarn("DLP found passport on response")
    end

    if (contains_credit_card(text)) then
      response_handle:logWarn("DLP found credit card on response")
    end
  end
end
