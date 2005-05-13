<?xml version="1.0" encoding="UTF-8"?>
<!-- LCD Command Tests
$Header$
-->
<Girder Version="400">
  <FileRoot Name="C:\Program Files\Promixis\Girder\plugins\LCD\LCDTests.gml" Identifier="-1" Enabled="TRUE" FileGUID="{C94C17F9-8E25-4615-80AD-94F20257BB45}" TimeStamp="0" Locked="FALSE">
    <Group Name="Display Commands" Identifier="1" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
      <Comments>Test each command type</Comments>
      <Command Name="String" Identifier="2" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Display string</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789</sValue1>
        <sValue2>s</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>1</iValue1>
        <iValue3>10</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
      <Command Name="Variable" Identifier="3" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Display variable value</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>math.pi</sValue1>
        <sValue2>v</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>4</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
      <Command Name="Date / Time" Identifier="4" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Display current date</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>%Y-%m-%d %H:%M:%S</sValue1>
        <sValue2>t</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
      <MultiGroup Name="Filename" Identifier="5" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Display trimmed filename</Comments>
        <Command Name="Variable" Identifier="6" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <Comments>Compose filename</Comments>
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>FileName = win.GetDirectory("GIRDERDIR") .. "\\girder.exe"
</sValue1>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <ActionType>4</ActionType>
          <ActionSubType>34</ActionSubType>
        </Command>
        <Command Name="Display" Identifier="7" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>FileName</sValue1>
          <sValue2>f</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
      </MultiGroup>
      <Command Name="Close" Identifier="8" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Close display</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue2>x</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>2</ActionSubType>
      </Command>
      <Command Name="Clear" Identifier="9" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Clear display</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue2>c</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>2</ActionSubType>
      </Command>
      <Command Name="Screen" Identifier="10" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Multi-line screen w/ variable &amp; marquee</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>[_VERSION]
abcdefghijklmnopqrstuvwxyz

</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>12</iValue1>
        <iValue2>2</iValue2>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="Character" Identifier="11" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Character by code</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>42</sValue1>
        <sValue2>*</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>1</iValue1>
        <iValue2>1</iValue2>
        <iValue3>1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
      <Command Name="Custom Char" Identifier="12" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Custom character (NE arrow)</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0b01111 0b00011 0b00101 0b01000</sValue1>
        <sValue2>$</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>1</iValue1>
        <iValue2>2</iValue2>
        <iValue3>1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
      <Command Name="Start Timer" Identifier="13" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE" lFileGUID3="{C94C17F9-8E25-4615-80AD-94F20257BB45}">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <bValue1>TRUE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>5000</iValue1>
        <iValue2>2</iValue2>
        <lValue3>4</lValue3>
        <ActionType>122</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
      <Command Name="Stop Timer" Identifier="14" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue2>2</iValue2>
        <ActionType>122</ActionType>
        <ActionSubType>1</ActionSubType>
        <EventString Name="AfterScriptReset" Identifier="15" Enabled="TRUE" Invert="FALSE" Device="18" IrStr="AfterScriptReset"/>
      </Command>
    </Group>
    <Group Name="Custom" Identifier="16" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
      <Comments>Test custom characters</Comments>
      <MultiGroup Name="00-07" Identifier="17" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Command Name="Legend" Identifier="18" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>01234567</sValue1>
          <sValue2>s</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="00" Identifier="19" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b00010
0b00100
0b01000
0b01000
0b01000
0b00100
0b00010
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="01" Identifier="20" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b01110
0b10001
0b10000
0b10000
0b10000
0b10001
0b01110
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>1</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="02" Identifier="21" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b10001
0b10001
0b10001
0b10001
0b10001
0b10001
0b01110
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>2</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="03" Identifier="22" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b01111
0b10000
0b10000
0b01110
0b00001
0b00001
0b11110
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>3</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="04" Identifier="23" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b11111
0b00100
0b00100
0b00100
0b00100
0b00100
0b00100
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>4</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="05" Identifier="24" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b01110
0b10001
0b10001
0b10001
0b10001
0b10001
0b01110
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>5</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="06" Identifier="25" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b10001
0b11011
0b10101
0b10101
0b10001
0b10001
0b10001
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>6</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="07" Identifier="26" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
0b01000
0b00100
0b00010
0b00010
0b00010
0b00100
0b01000
0b11111
</sValue1>
          <sValue2>$</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>7</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
      </MultiGroup>
    </Group>
    <Group Name="All Chars" Identifier="27" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
      <Comments>Test every possible character code</Comments>
      <MultiGroup Name="00-0F" Identifier="28" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Command Name="Legend" Identifier="29" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0123456789ABCDEF  0</sValue1>
          <sValue2>s</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="00" Identifier="30" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x00</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="01" Identifier="31" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x01</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>1</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="02" Identifier="32" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x02</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>2</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="03" Identifier="33" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x03</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>3</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="04" Identifier="34" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x04</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>4</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="05" Identifier="35" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x05</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>5</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="06" Identifier="36" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x06</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>6</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="07" Identifier="37" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x07</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>7</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="08" Identifier="38" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x08</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>8</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="09" Identifier="39" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x09</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>9</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="0A" Identifier="40" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x0A</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>10</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="0B" Identifier="41" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x0B</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>11</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="0C" Identifier="42" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x0C</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>12</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="0D" Identifier="43" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x0D</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>13</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="0E" Identifier="44" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x0E</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>14</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="0F" Identifier="45" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x0F</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>15</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
      </MultiGroup>
      <MultiGroup Name="10-1F" Identifier="46" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Command Name="Legend" Identifier="47" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0123456789ABCDEF  1</sValue1>
          <sValue2>s</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="10" Identifier="48" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x10</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="11" Identifier="49" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x11</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>1</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="12" Identifier="50" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x12</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>2</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="13" Identifier="51" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x13</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>3</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="14" Identifier="52" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x14</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>4</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="15" Identifier="53" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x15</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>5</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="16" Identifier="54" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x16</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>6</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="17" Identifier="55" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x17</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>7</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="18" Identifier="56" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x18</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>8</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="19" Identifier="57" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x19</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>9</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="1A" Identifier="58" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x1A</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>10</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="1B" Identifier="59" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x1B</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>11</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="1C" Identifier="60" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x1C</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>12</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="1D" Identifier="61" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x1D</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>13</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="1E" Identifier="62" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x1E</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>14</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="1F" Identifier="63" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x1F</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>15</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
      </MultiGroup>
      <Command Name="20-2F" Identifier="64" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  2
 !"#$%&amp;'()*+,-./</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="30-3F" Identifier="65" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  3
0123456789:;&lt;=&gt;?</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="40-4F" Identifier="66" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  4
@ABCDEFGHIJKLMNO</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="50-5F" Identifier="67" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  5
PQRSTUVWXYZ[[\]^_</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="60-6F" Identifier="68" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  6
`abcdefghijklmno</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <MultiGroup Name="70-7F" Identifier="69" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Command Name="70-7E" Identifier="70" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0123456789ABCDEF  7
pqrstuvwxyz{|}~</sValue1>
          <sValue2>#</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>3</ActionSubType>
        </Command>
        <Command Name="7F" Identifier="71" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x7F</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>15</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>3</ActionSubType>
        </Command>
      </MultiGroup>
      <MultiGroup Name="80-8F" Identifier="72" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Command Name="Legend" Identifier="73" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0123456789ABCDEF  0</sValue1>
          <sValue2>s</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="80" Identifier="74" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x80</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="81" Identifier="75" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x81</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>1</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="82" Identifier="76" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x82</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>2</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="83" Identifier="77" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x83</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>3</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="84" Identifier="78" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x84</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>4</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="85" Identifier="79" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x85</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>5</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="86" Identifier="80" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x86</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>6</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="87" Identifier="81" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x87</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>7</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="88" Identifier="82" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x88</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>8</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="89" Identifier="83" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x89</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>9</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="8A" Identifier="84" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x8A</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>10</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="8B" Identifier="85" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x8B</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>11</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="8C" Identifier="86" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x8C</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>12</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="8D" Identifier="87" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x8D</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>13</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="8E" Identifier="88" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x8E</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>14</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="8F" Identifier="89" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x8F</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>15</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
      </MultiGroup>
      <MultiGroup Name="90-9F" Identifier="90" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Command Name="Legend" Identifier="91" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0123456789ABCDEF  0</sValue1>
          <sValue2>s</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="90" Identifier="92" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x90</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="91" Identifier="93" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x91</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>1</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="92" Identifier="94" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x92</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>2</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="93" Identifier="95" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x93</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>3</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="94" Identifier="96" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x94</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>4</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="95" Identifier="97" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x95</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>5</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="96" Identifier="98" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x96</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>6</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="97" Identifier="99" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x97</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>7</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="98" Identifier="100" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x98</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>8</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="99" Identifier="101" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x99</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>9</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="9A" Identifier="102" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x9A</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>10</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="9B" Identifier="103" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x9B</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>11</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="9C" Identifier="104" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x9C</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>12</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="9D" Identifier="105" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x9D</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>13</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="9E" Identifier="106" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x9E</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>14</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="9F" Identifier="107" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>0x9F</sValue1>
          <sValue2>*</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue2>15</iValue2>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
      </MultiGroup>
      <Command Name="A0-AF" Identifier="108" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  A
 ¡¢£¤¥¦§¨©ª«¬­®¯</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="B0-BF" Identifier="109" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  B
°±²³´µ¶·¸¹º»¼½¾¿</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="C0-CF" Identifier="110" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  C
ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="D0-DF" Identifier="111" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  D
ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="E0-EF" Identifier="112" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  E
àáâãäåæçèéêëìíîï</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
      <Command Name="F0-FF" Identifier="113" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>0123456789ABCDEF  F
ðñòóôõö÷øùúûüýþÿ</sValue1>
        <sValue2>#</sValue2>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>3</ActionSubType>
      </Command>
    </Group>
    <Group Name="GPO" Identifier="114" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
      <Comments>Test General Purpose Output</Comments>
      <Command Name="Simple" Identifier="115" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Turn on low-power GPO</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue2>o</sValue2>
        <bValue1>TRUE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>5</iValue1>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>5</ActionSubType>
      </Command>
      <Command Name="Start Counter" Identifier="116" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE" lFileGUID3="{C94C17F9-8E25-4615-80AD-94F20257BB45}">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <bValue1>TRUE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>2000</iValue1>
        <iValue2>1</iValue2>
        <lValue3>117</lValue3>
        <ActionType>122</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
      <Command Name="Counter" Identifier="117" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>GPOCounter = (GPOCounter or 0) + 1
lcd.GPO(5, band(GPOCounter,1))
lcd.GPO(6, band(GPOCounter,2))
lcd.GPO(7, band(GPOCounter,4))
</sValue1>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <ActionType>4</ActionType>
        <ActionSubType>34</ActionSubType>
      </Command>
      <Command Name="End Counter" Identifier="118" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue2>1</iValue2>
        <ActionType>122</ActionType>
        <ActionSubType>1</ActionSubType>
      </Command>
    </Group>
    <Group Name="Fans" Identifier="119" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
      <Comments>Test fan power and monitoring</Comments>
      <Command Name="Simple" Identifier="120" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>General purpose output</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>75</sValue1>
        <sValue2>p</sValue2>
        <bValue1>TRUE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <iValue1>1</iValue1>
        <iValue3>-1</iValue3>
        <ActionType>118</ActionType>
        <ActionSubType>6</ActionSubType>
      </Command>
      <MultiGroup Name="Initialize Feedback" Identifier="121" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Command Name="Define functions" Identifier="122" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <Comments>Define Lua functions for feedback handling</Comments>
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>
-- Map from event to fan,sensor entry.
FanSensors = {}

-- Add a new fan,sensor entry with the given target temperature.
function AddFanSensor(fan, sensor, target)
  local entry = {}
  entry.fan = fan
  entry.fanEvent = "Fan"..fan
  entry.sensor = sensor
  entry.sensorEvent = "Sensor"..sensor

  entry.minPower = .1
  entry.maxPower = 1.0
  entry.power = 0.75
  entry.powerIncrement = .05
  entry.failsafePower = entry.maxPower

  entry.hysteresis = .5
  entry.target = target

  function entry:OnEvent(event, payload)
    if event == self.fanEvent then
      self.rpm = tonumber(payload)
    elseif event == self.sensorEvent then
      self.temp = tonumber(payload)
      self:CheckTemp()
    end
  end

  function entry:CheckTemp()
    if not self.temp then return end
    local changed
    if self.failsafeTemp and (self.temp &gt;= self.failsafeTemp) then
      -- If the temperature is too high, immediately switch the power
      -- up to max rather than stepping gradually.
      self.power = self.failsafePower
      changed = 1
    elseif self.temp &gt;= self.target + self.hysteresis then
      self.power = self.power + self.powerIncrement
      if self.power &gt; self.maxPower then
        self.power = self.maxPower
      end
      changed = 1
    elseif self.temp &lt;= self.target - self.hysteresis then
      self.power = self.power - self.powerIncrement
      if self.power &lt; self.minPower then
        self.power = self.minPower
      end
      changed = 1
    end
    if changed then
      lcd.FanPower(self.fan, self.power)
      print(self.fanEvent .. " power now " .. self.power)
    end
  end

  FanSensors[entry.fanEvent] = entry
  FanSensors[entry.sensorEvent] = entry
  return entry
end
</sValue1>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <ActionType>4</ActionType>
          <ActionSubType>34</ActionSubType>
        </Command>
        <Command Name="Setup Feedback" Identifier="123" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <Comments>Define a feedback entry</Comments>
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>-- Note: this test is for a room temperature fan, not one inside the case.
AddFanSensor(1, 1, 23).failsafeTemp = 30
</sValue1>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <ActionType>4</ActionType>
          <ActionSubType>34</ActionSubType>
        </Command>
        <Command Name="Enable Event" Identifier="124" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE" lFileGUID1="{C94C17F9-8E25-4615-80AD-94F20257BB45}">
          <Comments>Enable feedback event</Comments>
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <lValue1>126</lValue1>
          <ActionType>4</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="Start Timer" Identifier="125" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE" lFileGUID3="{C94C17F9-8E25-4615-80AD-94F20257BB45}">
          <Comments>Start periodic timer for unchanging temperature</Comments>
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <bValue1>TRUE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>5000</iValue1>
          <iValue2>3</iValue2>
          <lValue3>128</lValue3>
          <ActionType>122</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
      </MultiGroup>
      <Command Name="Feedback" Identifier="126" Enabled="FALSE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Check for feedback related event</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>local entry
entry = FanSensors[EventString]
if entry then
  entry:OnEvent(EventString, pld1)
end
</sValue1>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <ActionType>4</ActionType>
        <ActionSubType>34</ActionSubType>
        <EventString Name="On LCD Event" Identifier="127" Enabled="TRUE" Invert="FALSE" Device="-2" IrStr="On LCD Event"/>
      </Command>
      <Command Name="Timer" Identifier="128" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Called periodically because temperature might not change</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>for key,entry in FanSensors do
  entry:CheckTemp()
end
</sValue1>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <ActionType>4</ActionType>
        <ActionSubType>34</ActionSubType>
      </Command>
      <MultiGroup Name="Disable Feedback" Identifier="129" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <EventString Name="AfterScriptReset" Identifier="130" Enabled="TRUE" Invert="FALSE" Device="18" IrStr="AfterScriptReset"/>
        <Command Name="Stop Timer" Identifier="131" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <Comments>Stop periodic timer</Comments>
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue2>3</iValue2>
          <ActionType>122</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="Disable Event" Identifier="132" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE" lFileGUID1="{C94C17F9-8E25-4615-80AD-94F20257BB45}">
          <Comments>Disable feedback event</Comments>
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <lValue1>126</lValue1>
          <ActionType>4</ActionType>
          <ActionSubType>2</ActionSubType>
        </Command>
      </MultiGroup>
    </Group>
    <Group Name="Keypad" Identifier="133" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
      <Comments>Test keypad button input</Comments>
      <MultiGroup Name="LCD Event" Identifier="134" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Display LCD event and payload</Comments>
        <EventString Name="On LCD Event" Identifier="135" Enabled="TRUE" Invert="FALSE" Device="-2" IrStr="On LCD Event"/>
        <Command Name="Display" Identifier="136" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>[EventString] [pld1]</sValue1>
          <sValue2>s</sValue2>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue1>1</iValue1>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>1</ActionSubType>
        </Command>
        <Command Name="OSD" Identifier="137" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>[EventString] [pld1]</sValue1>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <ActionType>4</ActionType>
          <ActionSubType>9</ActionSubType>
        </Command>
      </MultiGroup>
      <MultiGroup Name="Set Legends" Identifier="138" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Set Crystalfontz button legends</Comments>
        <Command Name="UL" Identifier="139" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>UL</sValue1>
          <sValue2>k</sValue2>
          <sValue3>Up</sValue3>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>4</ActionSubType>
        </Command>
        <Command Name="UR" Identifier="140" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>UR</sValue1>
          <sValue2>k</sValue2>
          <sValue3>Enter</sValue3>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>4</ActionSubType>
        </Command>
        <Command Name="LL" Identifier="141" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>LL</sValue1>
          <sValue2>k</sValue2>
          <sValue3>Down</sValue3>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>4</ActionSubType>
        </Command>
        <Command Name="LR" Identifier="142" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
          <OSD>FALSE</OSD>
          <Topmost>TRUE</Topmost>
          <MatchNum>1</MatchNum>
          <sValue1>LR</sValue1>
          <sValue2>k</sValue2>
          <sValue3>Cancel</sValue3>
          <bValue1>FALSE</bValue1>
          <bValue2>FALSE</bValue2>
          <bValue3>FALSE</bValue3>
          <iValue3>-1</iValue3>
          <ActionType>118</ActionType>
          <ActionSubType>4</ActionSubType>
        </Command>
      </MultiGroup>
    </Group>
    <Group Name="Functions" Identifier="143" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
      <Comments>Test LCD LUA extension functions</Comments>
      <Command Name="Basic" Identifier="144" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Open in script editor</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>-- Reset display
lcd.Close()

-- Size
local w,h = lcd.Size()
print(w, h)

-- String
lcd.String(0, w / 2, 2, "**")

-- Custom characters
lcd.CustomCharacter(0, 0, "0b11111 0b10101 0b11101 0b10001 0b10001 0b11111")
lcd.CustomCharacter(0, w - 1, "0b11111 0b10101 0b10111 0b10001 0b10001 0b11111")
lcd.CustomCharacter(h - 1, 0, "0b11111 0b10001 0b10001 0b11101 0b10101 0b11111")
lcd.CustomCharacter(h - 1, w - 1, "0b11111 0b10001 0b10001 0b10111 0b10101 0b11111")
</sValue1>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <ActionType>4</ActionType>
        <ActionSubType>34</ActionSubType>
      </Command>
      <Command Name="Settings" Identifier="145" Enabled="TRUE" ErrorBreak="FALSE" ErrorJump="FALSE" ErrorEmail="FALSE" ErrorGirderLog="FALSE" ErrorSystemLog="FALSE">
        <Comments>Open in script editor</Comments>
        <OSD>FALSE</OSD>
        <Topmost>TRUE</Topmost>
        <MatchNum>1</MatchNum>
        <sValue1>-- These only really work with Simulated LCD or LcdHype display devices.
print(lcd.GetSetting("SimLCDFont"))
print(lcd.GetSetting("SimLCDFontSize"))

lcd.SetSetting("SimLCDFont", "Lucida Console")
lcd.SetSetting("SimLCDFontSize", 18)

</sValue1>
        <bValue1>FALSE</bValue1>
        <bValue2>FALSE</bValue2>
        <bValue3>FALSE</bValue3>
        <ActionType>4</ActionType>
        <ActionSubType>34</ActionSubType>
      </Command>
    </Group>
  </FileRoot>
</Girder>
