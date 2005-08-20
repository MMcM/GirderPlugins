--[[
NMEA GPS parser for Girder.
$Header$

NOTE: The first half of this file is generic.  To get to the Girder
stuff, search down for Girder.  (LUA pretty much enforces bottom-up.)

Roughly NMEA 0183 v2.2.

NOTE: the NMEA standards organization (like ANSI and ISO, but not UN
EDIFAC or ITU) supports itself by selling printed copies of its
standards at exorbitant prices and not putting them online.  So this
is based on unofficial sources.
--]]

local P
if gir then
  gps = {}
  P = gps
else
  P = _G
end

P.nmea = {
  version = "2.2"
}
P.nmea.__index = P.nmea

-- No standard library function to XOR, try to find it.
local bxor = math.bxor
if not bxor then
  if not bit then
    local l = loadlib("bitlib", "luaopen_bit")
    if l then l() end
  end
  if bit then bxor = bit.bxor end
end

-- Parse a (mostly comma-delimited) sentence into a list of fields.
function P.nmea:ParseSentence(sent)
  if string.sub(sent, 1, 1) ~= "$" then return end

  -- No standard library function to search backwards.
  local e = string.len(sent)
  while string.sub(sent, e, e) ~= "*" do
    e = e - 1
    if e <= 0 then return end
  end

  local csum = tonumber(string.sub(sent, e+1, -1), 16)
  sent = string.sub(sent, 2, e-1)
  if bxor then
    for i = 1,string.len(sent) do
      csum = bxor(csum, string.byte(sent, i))
    end
  end

  local fields = {}
  -- for f in gfind([^,]+) would almost do what you want, except when there is a ,,
  -- and * would generate spurious matches for _every_ comma.
  local i = 1
  while true do
    local ni = string.find(sent, ",", i, true)
    local f = string.sub(sent, i, ni and ni - 1)
    table.insert(fields, f)
    if not ni then break end
    i = ni + 1
  end

  return fields, bxor and csum == 0
end

-- hhmmss.sss UTC to something os.time might return and that os.date likes.
function P.nmea:ConvertUTC(tstr, dstr)
  local hour,min,sec
  hour = tonumber(string.sub(tstr, 1, 2))
  min = tonumber(string.sub(tstr, 3, 4))
  sec = tonumber(string.sub(tstr, 5))

  local now = os.time()
  local nowUTC = os.date("!*t", now)
  
  if not dstr then
    -- The measured time should be in the (very) recent past.  However,
    -- that still might be yesterday (UTC) and today might be the first
    -- of March in a leap year, and so on.  So, rather than trying to be
    -- more portable and construct a table to pass to os.time, we assume
    -- time is in seconds.  It is also possible that the system clock is
    -- slightly off and we don't want that to make us return times 23
    -- hours in the future.  So, we find the time that is closest to the
    -- current time and matches the given fields.
    local delta = nowUTC.sec - sec
    delta = delta + (nowUTC.min - min) * 60
    delta = delta + (nowUTC.hour - hour) * 60 * 60
    while delta < 12 * 60 * 60 do
      delta = delta + 24 * 60 * 60
    end
    while delta >= 12 * 60 * 60 do -- (>= since we prefer the past)
      delta = delta - 24 * 60 * 60
    end
    return now - delta
  else
    local day,month,year
    day = tonumber(string.sub(dstr, 1, 2))
    month = tonumber(string.sub(dstr, 3, 4))
    -- Actually, this won't work if the given date is 311299 and now
    -- is a few seconds later on 1/1/2100.  It's hard to imagine that
    -- this protocol will still be in use then.
    year = tonumber(string.sub(dstr, 5)) + math.floor(nowUTC.year / 100) * 100

    local nowLocal = os.date("*t", now)
    local thenUTC = { 
      year = year, month = month, day = day, 
      hour = hour, min = min, sec = sec,
      isdst = false             -- UTC is never DST.
    }

    -- It's easier when we know the UTC date.  All we need to do is
    -- compute the timezone offset.  Again, we assume seconds so that
    -- we can convert to that and then adjust, rather than needing to
    -- adjust the component representation.
    local tz = nowLocal.sec - nowUTC.sec
    tz = tz + (nowLocal.min - nowUTC.min) * 60
    tz = tz + (nowLocal.hour - nowUTC.hour) * 60 * 60
    -- thenUTC.isdst isn't going to be respected.
    if (nowLocal.isdst) then tz = tz - 60 * 60 end
    return os.time(thenUTC) + tz
  end
end

-- dddmm.mmmm to float degrees.  Also S or W becomes negative.
function P.nmea:ConvertDegrees(deg, compass)
  local ndeg = tonumber(string.sub(deg, 1, 3))
  ndeg = ndeg + tonumber(string.sub(deg, 4)) / 60
  if compass == "S" or compass == "W" then
    ndeg = - ndeg
  end
  return ndeg
end

-- Standard bearing names.
function P.nmea:Compass(deg)
  deg = deg + 11.25
  if deg > 360 then deg = deg - 360 end
  local n = math.floor(deg / 22.5)
  return ({ "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", 
            "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" })[n+1]
end

P.gps = {
}
P.gps.__index = P.gps
setmetatable(P.gps, P.nmea)

-- Create new GPS state holder.
function P.gps:New(o)
  o = o or {}
  o.allSatellites = {}
  o.satellitesInUse = {}
  o.satellitesInView = {}
  setmetatable(o, self) 
  return o
end

-- Log something of interest to console.
function P.gps:Log(msg)
  print(msg)
end

-- Get a satellite with the given id.
function P.gps:GetSatellite(id)
  if not id then return nil end

  s = self.allSatellites[id]
  if not s then
    s = { id = id }
    self.allSatellites[id] = s
  end
  return s
end
  
-- Update GPS state based on the given sentence message.
function P.gps:Update(sent)
  local msg, csok = self:ParseSentence(sent)
  if not msg then
    self:Log(string.format("Couldn't parse '%s', ignored", sent))
    return false
  elseif csok == false then     -- Only explicit failure (not lack of XOR).
    self:Log("Note: bad checksum, continuing")
  end

  local h = self[msg[1]]
  if not h then
    self:Log(string.format("Unknown message '%s', ignored", m[0]))
    return false
  end
  return h(self, msg)  
end

-- Actual NMEA messages

function P.gps:GPGLL(msg)
  if msg[7] ~= "A" then return end -- Only if valid.
  
  self.latitude = self:ConvertDegrees("0" .. msg[2], msg[3])
  self.longitude = self:ConvertDegrees(msg[4], msg[5])
  self.time = self:ConvertUTC(msg[6])
end

function P.gps:GPGGA(msg)
  self.time = self:ConvertUTC(msg[2])
  self.latitude = self:ConvertDegrees("0" .. msg[3], msg[4])
  self.longitude = self:ConvertDegrees(msg[5], msg[6])
  self.fix = ({ "GPS-SPS", "DGPS-SPS", "GPS-PPS" })[tonumber(msg[7])]
  self.satellitesUsed = tonumber(msg[8])
  self.hdop = tonumber(msg[9])
  self.altitude = tonumber(msg[10])
  self.altitudeUnits = msg[11]
  self.geoid = tonumber(msg[12])
  self.geoidUnits = msg[13]
  self.differentialAge = msg[14]
  self.differentialReferenceStationID = msg[15]
end

function P.gps:GPGSA(msg)
  self.modeManual = msg[2] == "M"
  self.mode = ({ nil, "2D", "3D" })[tonumber(msg[3])]
    
  for id,s in pairs(self.allSatellites) do
    s.inUse = false
  end
  for i = 1, 12 do
    local s = self:GetSatellite(tonumber(msg[i + 3]))
    if s then s.inUse = true end
    self.satellitesInUse[i] = s
  end

  self.pdop = tonumber(msg[17])
  self.hdop = tonumber(msg[18])
  self.vdop = tonumber(msg[19])
end

function P.gps:GPGSV(msg)
  local offset = tonumber((msg[3] - 1) * 4)
  table.setn(self.satellitesInView, tonumber(msg[4]))
  for i = 1, 4 do
    local s = self:GetSatellite(tonumber(msg[i * 4 + 1]))
    if s then
      s.elevation = tonumber(msg[i * 4 + 2])
      s.azimuth = tonumber(msg[i * 4 + 3])
      s.snr = tonumber(msg[i * 4 + 4])
    end
    self.satellitesInView[i + offset] = s
  end
end

function P.gps:GPRMC(msg)
  self.time = self:ConvertUTC(msg[2], msg[10])
  if msg[3] ~= "A" then return end -- Only if valid

  self.latitude = self:ConvertDegrees("0" .. msg[4], msg[5])
  self.longitude = self:ConvertDegrees(msg[6], msg[7])
  self.knots = tonumber(msg[8])
  self.mph = self.knots * 1.15077945 -- According to Google
  self.kph = self.knots * 1.85200
  self.course = tonumber(msg[9])
end

-- For debugging, mostly.
function P.gps:LoadFile(file, describe)
  local togo = describe and 10
  for sent in io.lines(file) do
    self:Update(sent)
    if describe then
      togo = togo - 1
      if togo < 0 then
        self:Describe()
        togo = 10
      end
    end
  end
  if describe and togo ~= 10 then self:describe() end
end

-- Show above state.
function P.gps:Describe()
  -- os.date (or Windows strftime) doesn't know about %r.
  print("Time = " .. os.date("%I:%M:%S %p", self.time))
  print("Fix = " .. self.fix)
  if self.latitude and self.longitude then
    print("Lat = " .. string.format("%.2f", self.latitude))
    print("Long = " .. string.format("%.2f", self.longitude))
  end
  if self.altitude then
    print("Alt = " .. string.format("%.0f", self.altitude) .. self.altitudeUnits)
  end
  if self.course then
    print("Course = " .. string.format("%.0f", self.course)
          .. " (" .. self:Compass(self.course) .. ")")
  end
  if self.mph then
    print("Speed = " .. string.format("%.0f", self.mph) .. " MPH")
  end
  
  local n = table.getn(self.satellitesInView)
  print("Satellites in view = " .. n .. " (" .. self.satellitesUsed .. ")")

  if n > 0 then
    print("Sat\tAzim\tElev\tSNR\tin use")
  end
  for i = 1, n do
    local s = self.satellitesInView[i]
    print(s.id .. "\t" .. s.azimuth .. "\t" .. s.elevation .. "\t" ..
          s.snr .. "\t" .. (s.inUse and "*" or ""))
  end
end

--[[

Girder stuff starts here.  To get to the configurable event stuff,
search down for events.

--]]
if gir then

-- Specialized device class.
local super = serial.Classes.Simple
P.device = super:New({

  Name = "GPS",
  Description = "Global Positioning device using standard NMEA protocol.",

-- There should not be any need to change this, the speed is part of
-- the NMEA spec, although many devices may be able to change it.
  BaudRate = 4800,
  Parity = 0,
  StopBits = 0,                 -- i.e., 1 stopbit.
  DataBits = 8,
  FlowControl = 'H',

  Status = "Not Running",
  CallbackType = serial.CB_TERMINATED,
  Terminator = '\r\n',

  IntraCharacterDelay = 0,
  IncompleteResponseTimeout = 100,
  NoreponseTimeout = 200,

  LogLevel = 1,

  GlobalName = true
})

-- Specialized GPS state for Girder.
P.girgps = P.gps:New()
P.girgps.__index = P.girgps

-- This is the configurable event stuff.
-- The key is the event handler (defined below).  
-- The value is some options for it.
-- This is the part of this file that you are most likely to want to customize.
P.girgps.Events = {
  -- Update Girder registry for the current position.
  UpdateLocation = { freq = "once" },

  -- Send GPS.Position lat, long, elev
  PositionEvent = { freq = 5 * 60, onlyIfChanged = true },
    
  -- Send GPS.Course speed, course, compass
  CourseEvent = { freq = 1 * 60, units = "mph", onlyIfChanged = true },
}

-- Initialize serial state.
function P.device:Initialize()
  -- Make a GPS parser / state holder.
  self.gps = P.girgps:New({ name = self.Name })

  -- TODO: Since the device is always running, maybe parse up to a
  -- newline to skip a partial record.
  self.Serial:RxClear()

  self.Status = "Device open"

  -- Setup callback by calling inherited initialize.
  return super.Initialize(self)
end

-- Process line of serial text.
function P.device:ReceiveResponse(data, code)
  super.ReceiveResponse(self)

  if data == nil then return end

  if math.band(code, serial.ERR) ~= 0 then
    gir.LogMessage(self.Name, "Communications Error", 3)
    return
  end

  if math.band(code, serial.RXCHAR) == 0 then return end

  local otime = self.gps.time

  -- Parse it.
  self.gps:Update(data)

  -- If the message had a timestamp (several do), then update status window.
  -- TODO: Does this cause too much traffic?
  if self.gps.time ~= otime then
    self.Status = "Received data at " .. os.date("%I:%M:%S %p", self.gps.time)
    self.Serial:UpdateStatus()
  end

  -- Now see whether there are any events to deal with.
  self.gps:TriggerEvents()
end

-- Make a new Girder-specific GPS state holder.
function P.girgps:New(o)
  o = P.gps.New(self, o)
  o.eventStates = {}
  return o
end

-- Direct messages to the Girder log.
function P.girgps:Log(msg)
  gir.LogMessage(self.name, msg, 3)
end

-- Called periodically (or manually for testing) to send events to Girder.
function P.girgps:TriggerEvents()
  local now = os.time()
  for event, options in self.Events do
    local state = self.eventStates[event]
    if not state then
      state = {}
      self.eventStates[event] = state
    end
    local run = true
    local freq = options.freq
    if freq then
      if freq == "once" then
        if state.lastTime then run = false end
      else
        if state.lastTime and now - state.lastTime < freq then run = false end
      end
    end
    if run then run = self[event](self, options, state) end
    if run then
      state.lastTime = now
    end
  end
end

-- Actual event handlers

P.girgps.PlugInID = 10000       -- TODO: Until Girder supports IDs not from plug-ins.

-- This doesn't actually send an event, but rather updates Girder's location.
function P.girgps:UpdateLocation(options, state)
  if self.latitude and self.longitude then
    local l = gir.GetLocation()
    l.Latitude, l.Longitude = self.latitude, self.longitude
    gir.SetLocation(l)
    self:Log(string.format("Updated geographical location to %.2f, %.2f", 
                           l.Latitude, l.Longitude))
    return true
  end
end

-- Send current position to Girder
function P.girgps:PositionEvent(options, state)
  if self.latitude and self.longitude then
    if options.onlyIfChanged then
      if self.latitude == state.lastLatitude and 
        self.longitude == state.lastLongitude then
        return false
      end
      state.lastLatitude = self.latitude
      state.lastLongitude = self.longitude
    end
    gir.TriggerEvent(self.name .. ".Position", self.PlugInID, 
                     self.latitude, self.longitude, self.elevation)
    return true
  end
end

-- Send current course and speed to Girder
function P.girgps:CourseEvent(options, state)
  if self.knots and self.course then
    if options.onlyIfChanged then
      if self.knots == state.lastKnots and 
        self.course == state.lastBearing then
        return false
      end
      state.lastKnots = self.knots
      state.lastBearing = self.course
    end
    gir.TriggerEvent(self.name .. ".Course", self.PlugInID, 
                     self[options.units or "mph"], 
                     self.course, self:Compass(self.course))
    return true
  end
end

serial.AddDevice(P.device)

-- End of huge Girder conditional.
end
