/*********************************************************************
 *  @file   : PluginIF.cpp
 *
 *  Project : 
 *
 *  Company : 
 *
 *  Author  : Tom Skoglund
 *
 *  Purpose : Handle the Girder API
 *
 *********************************************************************
 * $Id$
 ********************************************************************/


#include "stdafx.h"
#include "PluginIF.h"

#include "Anir2Girder.h"


#define GIR_API_VERSION		1


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


/*!
 * App is fetching plugin version
 *
 * @param data : 
 * @param size : 
 *
 * @return void : 
 */
void WINAPI gir_version(PCHAR data, int size) 
{
	strncpy(data, PLUGINVERSION, size);
}



/*!
 * App is fetching Plugin name
 *
 * @param data : 
 * @param size : 
 *
 * @return void: 
 */
void WINAPI gir_name(PCHAR data, int size)
{
	strncpy(data, PLUGINNAME, size);
}



/*!
 * App is fetching plugin description
 *
 * @param data : 
 * @param size : 
 *
 * @return void : 
 */
void WINAPI gir_description(PCHAR data, int size)
{
	strncpy(data, PLUGINDESCRIPTION, size);
}




/*!
 * App is fetching plugin Device ID
 *
 * @param none
 *
 * @return int : 
 */
int WINAPI gir_devicenum()
{
	return PLUGINDEVICEID;
}




/*!
 * Which API are we using
 *
 * @param max_api : 
 *
 * @return int : 
 */
int WINAPI gir_requested_api(int max_api)
{
	return GIR_API_VERSION;
}




/*!
 * App call to open plugin
 *
 * @param gir_major_ver : 
 * @param gir_minor_ver : 
 * @param gir_micro_ver : 
 * @param api_functions : 
 *
 * @return int : 
 */
int WINAPI gir_open(int gir_major_ver, int gir_minor_ver, int gir_micro_ver, p_functions api_functions )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.PluginOpen(gir_major_ver, gir_minor_ver, gir_micro_ver, api_functions);
}




/*!
 * Close request
 *
 * @param none
 *
 * @return int : 
 */
int WINAPI gir_close()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.PluginClose();
}




/*!
 * Settings for the plugin
 *
 * @param none
 *
 * @return void : 
 */
void WINAPI gir_config()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.PluginConfig();
}




/*!
 * Command dialog for plugin
 *
 * @param none
 *
 * @return void : 
 */
void WINAPI gir_command_gui()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.PluginCommandGui();
}




/*!
 * Start sending events
 *
 * @param none
 *
 * @return int WINAPI  : 
 */
int WINAPI gir_start()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.PluginStart();
}



/*!
 * Stop sending events
 *
 * @param none
 *
 * @return int WINAPI  : 
 */
int WINAPI gir_stop()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.PluginStop();
}




/*!
 * A command is selected in the tree
 *
 * @param command : 
 *
 * @return void WINAPI  : 
 */
void WINAPI gir_command_changed(p_command command)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.PluginCommandChanged(command);
}



#ifdef __cplusplus
}
#endif