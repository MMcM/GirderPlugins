' Get DVD title from windowsmedia.com.
' This is a separate script both because LUA does not have easy XML
' access and because it can then run asynchronously.
Option Explicit

' Parse command line arguments.
Dim discId, title, chapter, cacheDir, girderEvent
Dim args, argi, arg, arg2
Set args = Wscript.Arguments
argi = 0
Do While argi < args.Length
  arg = args(argi)
  argi = argi + 1
  If Left(arg, 1) = "-" Then
    arg2 = args(argi)
    argi = argi + 1
    Select Case arg
    Case "-title"
      title = CInt(arg2)
    Case "-chapter"
      chapter = CInt(arg2)
    Case "-cache-directory"
      cacheDir = arg2
    Case "-girder-event"
      girderEvent = arg2
    Case Else
      Err.Raise vbObjectError,, "Unknown argument: " & arg
    End Select
  Else
    discId = arg
  End If
Loop

' Load file from cache or network.
Dim data, loaded, fso, file, url
Set data = CreateObject("Msxml2.DOMDocument")
'data.setProperty("SelectionLanguage", "XPath")
data.async = false
If Not IsEmpty(cacheDir) Then
  Set fso = CreateObject("Scripting.FileSystemObject")
  file = cacheDir & "\" & discId & ".xml"
  If fso.FileExists(file) Then
    loaded = data.load(file)
  End If
End If
If Not loaded Then
 ' This just redirects to the one below, but is presumably more stable.
 'url = "http://windowsmedia.com/redir/querydvdid.asp?WMPFriendly=true&locale=409&version=8.0.0.4487&DVDID=" & Left(discId, 8) & "|" & Right(discId, 8)
  url = "http://services.windowsmedia.com/amgvideo_a/template/QueryDVDTOC_v3.xml?TOC=" & discId
  loaded = data.load(url)
  If loaded And Not IsEmpty(file) Then
    data.save(file)
  End If
End If

' Use XPath to find the desired name element.
Dim xpath, node, text
If loaded Then
  xpath = "/DVDData"
  If Not IsEmpty(title) Then
    xpath = xpath & "/title[titleNum/text()='" & title & "']"
  End If
  If Not IsEmpty(chapter) Then
    xpath = xpath & "/chapter[chapterNum/text()='" & chapter & "']"
  End If
  xpath = xpath & "/name"
  Set node = data.selectSingleNode(xpath)
  If Not node Is Nothing Then text = node.text
End If

' Return the result.
If IsEmpty(girderEvent) Then
  Wscript.echo text
Else
  ' Girder itself has -eventstring, but no payload.  This control
  ' sends an event through DVDSpy.  It has no particular dependencies
  ' on its environment, even though it was designed for use in WMP
  ' skins.
  Dim dvdspy
  Set dvdspy = CreateObject("WMPSpy.SpyCtrl")
  dvdspy.event = girderEvent
  dvdspy.value = text
End If
