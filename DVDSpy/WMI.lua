-- Interface to Windows Management Instrumentation.
-- $Header$
-- WMI itself is present in 2K, but the formatted data ("cooked") classes are new to XP.

-- The actual implementation.
WMI = {

  -- WMI:Connect([computer],[user],[password])
  -- Make a new connection to WMI and return it.
  -- computer: The name of the host to connect to.  Default: "." (the current machine).
  -- user: User credentials for remote connection.
  -- password: User authentication for remote connection.
  Connect = function(proto, computer, user, password) 
    -- Make a new instance.
    self = {}
    setmetatable(self,proto)
    proto.__index = proto
    
    self.locator = luacom.CreateObject("WbemScripting.SWbemLocator")
    self.services = self.locator:ConnectServer(computer, nil, user, password)

    self.refresher = luacom.CreateObject("WbemScripting.SWbemRefresher")
    self.refresher.AutoReconnect = true

    self.objects = {}

    -- Initially put things in our own table.  Change this with WMI:SetEnvironment.
    self.environment = self

    return self
  end,
  
  -- Close connection and free resources.
  -- It isn't essential to call this; abandoning the WMI instance itself is enough.
  Close = function(self)
    self.locator = nil
    self.refresher = nil
    self.objects = nil
    collectgarbage()
  end,

  -- Set the environment in which results are stored.
  -- env: The table in which values are stored.  Default: _G (the global namespace).
  SetEnvironment = function(self, env)
    self.environment = (env or _G)
  end,

  -- Refresh all previously added objects, setting variables in the environment.
  Refresh = function(self)
    self.refresher:Refresh()
    for name, entry in self.objects do
      entry.handler(self, name, entry.object, entry.arg)
    end
  end,

  -- Add a WMI object to LUA variable mapping.
  -- name: A unique name for the object; a prefix for the variable names in the environment.
  -- path: The local WMI path to the object.  Use the WMI Object Browser if unsure of this.
  -- handler: See WMI:AddObjectEntry.
  AddPath = function(self, name, path, handler)
    local object = self.refresher:Add(self.services, path).Object
    self:AddObjectEntry(name, object, handler)
  end,

  -- Add WMI objects satisfying a query.
  -- name: A unique name for the object; a prefix for the variable names.
  -- query: A WQL query.
  -- handler: See WMI:AddObjectEntry.
  -- max: The maximum number of objects to add; default unbounded.
  AddQuery = function(self, name, query, handler, max)
    local rs, iter, obj, idx, suff
    rs = self.services:ExecQuery(query)
    iter = luacom.GetEnumerator(rs)
    idx = 1
    suff = ""
    if max ~= idx then suff = idx end
    while (not max) or (idx <= max) do
      obj = iter:Next()
      if not obj then break end
      self:AddPath(name .. suff, obj.Path_.Path, handler)
      idx = idx + 1
      suff = idx
    end
  end,

  -- Add a set of WMI objects.
  -- Note: AddQuery adds a set of objects present at the time of the
  -- add.  This function refreshes the set itself each time.  It is
  -- therefore useful for things like active processes.
  -- name: A unique name for the object; a prefix for the variable names.
  -- class: The object class name.
  -- handler: A function.
  AddEnum = function(self, name, class, handler)
    local enum = self.refresher:AddEnum(self.services, class).ObjectSet
    self:AddObjectEntry(name, enum, handler)
  end,

  -- Add a refreshing object to the table.
  -- name: A unique name for the object; used to form global variable names.
  -- object: The COM object.
  -- handler: A handler function, called with name and object.
  --  or a string property name, setting name to the value of that property.
  --  or a table (see WMI:PropertiesHandler)
  AddObjectEntry = function(self, name, object, handler)
    local hfun, harg
    local ht = type(handler)
    if ht == "function" then
      hfun = handler
    else
      harg = handler
      if ht == "string" then
        hfun = self.SinglePropertyHandler
      else
        hfun = self.PropertiesHandler
      end
    end
    local entry = { object = object, handler = hfun, arg = harg }
    self.objects[name] = entry
  end,

  -- Set a variable to the value of a single property.
  SinglePropertyHandler = function(self, var, object, property)
    self.environment[var] = object[property](object)
  end,

  -- Set variables according to a table of properties.
  -- prefix: The prefix for variable names.
  -- object: The WMI COM object.
  -- properties: A table; each key is the variable suffix (including an ordinal when
  -- just value is given to the table constructor); each value is a handler function
  -- called with the object to return a formatted value or a string property name.
  PropertiesHandler = function(self, prefix, object, properties)
    local var, val, fun
    -- It works to just have a property name in the table, in which case
    -- an ordinal name is generated.
    for key, prop in properties do
      var = prefix .. key
      if (type(prop) == "function") then
        fun = prop
      else
        fun = object[prop]
      end
      val = fun(object)
      self.environment[var] = val
    end
  end,

}

-- Some special formatters.

-- Format logical disk free space, showing name, bytes and percentage.
function WMI.FormatDiskFreeSpace(object)
  local freemb, freeb, freepct
  if not object then return end
  freemb = object:FreeMegabytes()
  freepct = object:PercentFreeSpace()
  if not freemb then return nil end
  if freemb > 512 then
    -- Follow the Explorer convention of 1GB = 1024MB.  Even if you
    -- think 1GB = 1000MB, this is close enough.
    freeb = string.format("%.1fGB", freemb / 1024)
  else
    freeb = string.format("%fMB", freemb)
  end
  return object:Name() .. " " .. freeb .. " free (" .. freepct .. "%)"
end

-- Get name and percent for the most active process
function WMI.MostActiveProcess(self, name, enum, ignore1, includeIdle)
  local iter, proc, pct, pname, maxpct, maxname
  iter = luacom.GetEnumerator(enum)
  proc = iter:Next()
  while proc do
    pct = proc:PercentProcessorTime()
    if (not maxpct) or (pct > maxpct) then
      pname = proc:Name()
      if (pname ~= "_Total") and (includeIdle or (pname ~= "Idle")) then
        maxpct, maxname = pct, pname
      end
    end
    proc = iter:Next()
  end
  if maxpct then
    self.environment[name] = maxname .. " (" .. maxpct .. "%)"
  end
end

-- Append tables.
function table.append(...)
  t = {}
  local n = 0
  for i = 1, getn(arg) do
    local at = arg[i]
    if at then
      for j = 1, getn(at) do
        n = n + 1
        t[n] = at[j]
      end
    end
  end
  t.n = n
  return t
end

-- Curry function with additional args on the right.
if not functional then functional = {} end

function functional.rcurry(f, ...)
  local carg = arg
  return function(...) return call(f, table.append(arg, carg)) end
end

-- Example uses:
if false then

  function setup()
    wmi = WMI:Connect()

    -- If MBM is not needed for temperature readings, this is lower overhead.
    wmi:AddPath("CPU", "Win32_PerfFormattedData_PerfOS_Processor.Name='_Total'",
                "PercentProcessorTime")

    wmi:AddPath("Memory", "Win32_PerfFormattedData_PerfOS_Memory=@",
                { Free = "AvailableMBytes", Faults = "PageFaultsPersec" })

    -- For a file server; is there a better metric?
    wmi:AddPath("Server", "Win32_PerfFormattedData_PerfNet_Server=@",
                { Sessions = "ServerSessions", Total = "LogonTotal" })

    -- Add all disks present right now.
    wmi:AddQuery("Disk", "SELECT * FROM Win32_PerfFormattedData_PerfDisk_LogicalDisk WHERE Name <> '_Total'",
                 { Free = WMI.FormatDiskFreeSpace })

    -- Add just the first TCP interface found (max = 1).
    wmi:AddQuery("TCP", "SELECT * FROM Win32_PerfFormattedData_Tcpip_NetworkInterface",
                 { Name = "Name", 
                   Received = "PacketsReceivedPersec", 
                   Sent = "PacketsSentPersec" },
                 1)

    if true then
      -- Get information about the most active process.
      wmi:AddEnum("TopProcess", "Win32_PerfFormattedData_PerfProc_Process",
                  WMI.MostActiveProcess)
    else
      -- Include the idle process (which will often win).
      wmi:AddEnum("TopProcess", "Win32_PerfFormattedData_PerfProc_Process",
                  functional.rcurry(WMI.MostActiveProcess,true))
    end

  end

  function display()
    wmi:Refresh()
    
    print(wmi.CPU)
    print(wmi.MemoryFree)
    print(wmi.TopProcess)

    print(wmi.ServerSessions)
    print(wmi.Disk1Free)
    print(wmi.Disk2Free)

  end

end
