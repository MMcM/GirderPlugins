/* Display LUA functions
$Header$
*/

#include "stdafx.h"
#include "plugin.h"

int luaSize(lua_State *L)
{
  if (0 != lua_gettop(L)) {
    lua_error(L, "incorrect number of arguments to LCD_Width");
    return 0;
  }
  lua_pushnumber(L, DisplayWidth());
  lua_pushnumber(L, DisplayHeight());
  return 2;
}

int luaClose(lua_State *L)
{
  if (0 != lua_gettop(L)) {
    lua_error(L, "incorrect number of arguments to LCD_Close");
    return 0;
  }
  DisplayClose();
  return 0;
}

int luaString(lua_State *L)
{
  if (4 != lua_gettop(L)) {
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
                lua_tostring(L, 4));
  return 0;
}

int luaCustomCharacter(lua_State *L)
{
  if (3 != lua_gettop(L)) {
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
                         lua_tostring(L, 3));
  return 0;
}

struct luaL_reg luaFunctions[] = {
  { "LCD_Size", luaSize },
  { "LCD_Close", luaClose },
  { "LCD_String", luaString },
  { "LCD_CustomCharacter", luaCustomCharacter },
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
