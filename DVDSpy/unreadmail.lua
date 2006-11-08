-- Get information about unread mail.
-- $Header$

--[[

-- Examples:

print(unreadmail:FromRegistry())

print(unreadmail:FromOutlook(true))

--]]


if not mail then mail = {} end

unreadmail = {

  -- Get the information as used by SHGetUnreadMailCount.
  HKCU = 2147483649,
  UnreadMailRoot = "Software\\Microsoft\\Windows\\CurrentVersion\\UnreadMail",
  FromRegistry = function(self, allAccounts)
    local loc, svc, reg
    -- WMI is the only way to get at the registry with key enumeration from automation.
    -- Girder's win package has single value read / write.
    -- Likewise WshShell.
    loc = luacom.CreateObject("WbemScripting.SWbemLocator")
    svc = loc:ConnectServer("", "root\\default")
    reg = svc:Get("StdRegProv")

    local error, ignore, keys
    error, ignore, ignore, keys = reg:EnumKey(self.HKCU, self.UnreadMailRoot, nil)
    if error ~= 0 then return end

    local function getval(key)
      local error, ignore, ignore, ignore, value = 
        reg:GetDWORDValue(self.HKCU, self.UnreadMailRoot .. "\\" .. key, "MessageCount",
                          nil)
      if error == 0 then
        local result = { UnRead = value }
        setmetatable(result, self.UnReadEntry)
        return result
      end
    end

    if allAccounts then
      local result = {}
      for i = 1, table.getn(keys) do
        key = keys[i]
        result[key] = getval(key)
      end
      setmetatable(result, self.UnReadList)
      return result
    else
      return getval(keys[1])
    end
  end,

  -- Get unread mail count from outlook, optionally including amount of
  -- spam removed and counts from other stores with folders named
  -- "Inbox".  This is useful when an inbox rule moves incoming mail
  -- from some account(s) to another store.  Outlook will no longer
  -- report that as unread to the shell.
  FromOutlook = function(self, includeJunk, includeOtherStores)
    -- GetObject or nil
    local function ProtectedGetObject(...)
      local ok, obj = pcall(luacom.GetObject, unpack(arg))
      if ok then return obj end
    end

    -- items(index) or nil
    local function ProtectedGetItem(...)
      local obj = arg[1]
      local ok, item = pcall(obj.Item, unpack(arg))
      if ok then return item end
    end

    local outlook, session, defaultInbox, junkBox, result

    -- Obtain connection each time, in case application has been closed
    -- and reopened.
    outlook = ProtectedGetObject("Outlook.Application")
    if not outlook then return end

    session = outlook.Session
    defaultInbox = session:GetDefaultFolder(6) -- olFolderInbox

    result = { UnRead = defaultInbox.UnReadItemCount }
    setmetatable(result, self.UnReadEntry)

    junkBox = session:getDefaultFolder(23) -- olFolderJunk
    if includeJunk then
      result.Junk = junkBox.Items.Count
    end

    if includeOtherStores then
      result = { Default = result }
      setmetatable(result, self.UnReadList)

      local enum, store, otherBox, otherResult

      enum = luacom.GetEnumerator(session.Folders)
      store = enum:Next()
      while store do
        if store.StoreID ~= defaultInbox.StoreID then
          otherBox = ProtectedGetItem(store.Folders, defaultInbox.Name)
          if otherBox then
            otherResult = { UnRead = otherBox.UnReadItemCount }
            setmetatable(otherResult, self.UnReadEntry)
            result[store.Name] = otherResult
            otherBox = ProtectedGetItem(store.Folders, junkBox.Name)
            if otherBox and includeJunk then
              otherResult.Junk = otherBox.Items.Count
            end
          end
        end
        store = enum:Next()
      end
    end
    
    return result
  end,

  -- Use metatables to make values returned format nicely for display
  -- on LCD automatically.

  UnReadEntry = {
    __tonumber = function(entry)
      return entry.UnRead
    end,
    __tostring = function(entry)
      local result = tostring(entry.UnRead)
      if entry.Junk and entry.Junk > 0 then
        result = result .. "(-" .. entry.Junk .. ")"
      end
      return result
    end
  },
  UnReadList = {
    __tonumber = function(entries)
      local total = 0
      for name,entry in entries do
        total = total + tonumber(entry)
      end
      return total
    end,
    __tostring = function(entries)
      local result
      if entries.Default then result = tostring(entries.Default) end
      for name,entry in entries do
        if name ~= "Default" then
          if result then
            result = result .. "+" .. tostring(entry)
          else
            result = tostring(entry)
          end
        end
      end
      return result or ""
    end
  }

}
