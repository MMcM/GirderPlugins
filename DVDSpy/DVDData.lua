-- Get DVD title from windowsmedia.com.
-- $Header$

DVDData = {
  CacheDirectory = nil
}

function DVDData.SetCacheDirectory(self, dir)
  if win then
    if not win.PathExists(dir) then
      win.SHCreateDirectory(dir)
    end
  else
    local fso = luacom.CreateObject("Scripting.FileSystemObject")
    if not fso:FolderExists(dir) then
      fso:CreateFolder(dir)
    end
  end
  self.CacheDirectory = dir
end

function DVDData.Lookup(self, discId, title, chapter)

  local data = luacom.CreateObject("Msxml2.DOMDocument")
  if false then data.setProperty("SelectionLanguage", "XPath") end
  data.async = false
  
  local loaded, file
  if self.CacheDirectory then
    file = self.CacheDirectory .. "\\" .. discId .. ".xml"
    local exists
    if win then
      exists = win.FileExists(file)
    else
      local fso = luacom.CreateObject("Scripting.FileSystemObject")
      exists = fso:FileExists(file)
    end
    if exists then
      loaded = data:load(file)
    end
  end
  if not loaded then
    if false then
      -- This just redirects to the one below, but is presumably more stable.
      url = "http://windowsmedia.com/redir/querydvdid.asp?WMPFriendly=true&locale=409&version=8.0.0.4487&DVDID=" .. string.sub(discId, 1, 8) .. "|" .. string.sub(discId, 9)
    else
      url = "http://metaservices.windowsmedia.com/amgvideo_a/template/QueryDVDTOC_v3.xml?TOC=" .. discId
    end
    loaded = data:load(url)
    if loaded and file then
      data:save(file)
    end
  end
  if loaded then
    local xpath = "/DVDData"
    if title then
      xpath = xpath .. "/title[titleNum/text()='" .. title .. "']"
    end
    if chapter then
      xpath = xpath .. "/chapter[chapterNum/text()='" .. chapter .. "']"
    end
    xpath = xpath .. "/name"
    local node = data:selectSingleNode(xpath)
    if node then
      return node.text
    end
  end

end
