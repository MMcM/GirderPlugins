/***************************************************************************************/
/*                                                                                     */
/*  Girder 3.0 Plugin                                                                  */
/*  User interface                                                                     */
/*                                                                                     */
/*  Copyright 2000 (c) Ron Bessems                                                     */
/*  GNU General Public License                                                         */
/*                                                                                     */
/*                                                                                     */
/***************************************************************************************/

DWORD WINAPI ConfigThread( LPVOID lpParameter );

void Show_Config();
void Update_Config();
void Empty_Config();
void Enable_Config(BOOL bValue);
void Close_Config();
