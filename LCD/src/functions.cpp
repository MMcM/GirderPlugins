/* Display LUA functions
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

static BOOL GetOptionalDisplayDevice(lua_State *L, int argpos, LPCSTR& dev)
{
  dev = NULL;
  int nargs = lua_gettop(L);
  if (nargs == argpos)
    return TRUE;
  else if (nargs == argpos + 1) {
    if (lua_isstring(L, argpos))
      dev = lua_tostring(L, argpos + 1);
    return TRUE;
  }
  else
    return FALSE;
}

// LCD_Size([dev]) -> width, height
int luaSize(lua_State *L)
{
  LPCSTR dev;
  if (!GetOptionalDisplayDevice(L, 0, dev)) {
    lua_error(L, "incorrect number of arguments to LCD_Width");
    return 0;
  }
  lua_pushnumber(L, DisplayWidth(dev));
  lua_pushnumber(L, DisplayHeight(dev));
  return 2;
}

// LCD_Close([dev])
int luaClose(lua_State *L)
{
  LPCSTR dev;
  if (!GetOptionalDisplayDevice(L, 0, dev)) {
    lua_error(L, "incorrect number of arguments to LCD_Close");
    return 0;
  }
  DisplayClose(dev);
  return 0;
}

// LCD_String(row, col, width, str [, dev])
int luaString(lua_State *L)
{
  LPCSTR dev;
  if (!GetOptionalDisplayDevice(L, 4, dev)) {
    lua_error(L, "incorrect number of arguments to LCD_Display");
    return 0;
  }
  if (!(lua_isnumber(L, 1) &&
        lua_isnumber(L, 2) &&
        lua_isnumber(L, 3) &&
        lua_isstring(L, 4))) {
    lua_error(L, "wrong type argument to LCD_Display");
    return 0;
  }
  DisplayString((int)lua_tonumber(L, 1), 
                (int)lua_tonumber(L, 2),
                (int)lua_tonumber(L, 3),
                lua_tostring(L, 4),
                dev);
  return 0;
}

// LCD_CustomCharacter(row, col, defn [, dev])
int luaCustomCharacter(lua_State *L)
{
  LPCSTR dev;
  if (!GetOptionalDisplayDevice(L, 3, dev)) {
    lua_error(L, "incorrect number of arguments to LCD_CustomCharacter");
    return 0;
  }
  if (!(lua_isnumber(L, 1) &&
        lua_isnumber(L, 2) &&
        lua_isstring(L, 3))) {
    lua_error(L, "wrong type argument to LCD_CustomCharacter");
    return 0;
  }
  DisplayCustomCharacter((int)lua_tonumber(L, 1), 
                         (int)lua_tonumber(L, 2),
                         lua_tostring(L, 3),
                         dev);
  return 0;
}

// LCD_GPO(gpo, state [, dev])
int luaGPO(lua_State *L)
{
  LPCSTR dev;
  if (!GetOptionalDisplayDevice(L, 2, dev)) {
    lua_error(L, "incorrect number of arguments to LCD_GPO");
    return 0;
  }
  if (!(lua_isnumber(L, 1) &&
        (lua_isnumber(L, 2) || lua_isnil(L, 2)))) {
    lua_error(L, "wrong type argument to LCD_GPO");
    return 0;
  }
  DisplayGPO((int)lua_tonumber(L, 1), 
             lua_isnumber(L, 2) && (lua_tonumber(L, 2) != 0.0),
             dev);
  return 0;
}

// LCD_FanPower(fan, power [, dev])
int luaFanPower(lua_State *L)
{
  LPCSTR dev;
  if (!GetOptionalDisplayDevice(L, 2, dev)) {
    lua_error(L, "incorrect number of arguments to LCD_FanPower");
    return 0;
  }
  if (!(lua_isnumber(L, 1) &&
        lua_isnumber(L, 2))) {
    lua_error(L, "wrong type argument to LCD_FanPower");
    return 0;
  }
  DisplayFanPower((int)lua_tonumber(L, 1), 
                  lua_tonumber(L, 2),
                  dev);
  return 0;
}

struct luaL_reg luaFunctions[] = {
  { "LCD_Size", luaSize },
  { "LCD_Close", luaClose },
  { "LCD_String", luaString },
  { "LCD_CustomCharacter", luaCustomCharacter },
  { "LCD_GPO", luaGPO },
  { "LCD_FanPower", luaFanPower },
};

void FunctionsOpen()
{
  lua_State *L = (lua_State *)SF.get_script_state();
  if (NULL == L) return;
  luaL_openl(L, luaFunctions);
}

void FunctionsClose()
{
  lua_State *L = (lua_State *)SF.get_script_state();
  if (NULL == L) return;
  luaL_closel(L, luaFunctions);
}
