// Filename:-	CommArea.h
//
// headers for inter-program communication
//

#ifndef COMMAREA_H
#define COMMAREA_H

// setup functions...
//
const char *	CommArea_ServerInitOnceOnly(void);
const char *	CommArea_ClientInitOnceOnly(void);
void	CommArea_ShutDown(void);
//
// size-limit internal buffer-query functions...
//
int		CommArea_GetMaxDataSize(void);
int		CommArea_GetMaxCommandStrlen(void);
int		CommArea_GetMaxErrorStrlen(void);
//
// message-pending query functions...
//
bool	CommArea_IsIdle(void);
const char *	CommArea_IsCommandWaiting(byte **ppbDataPassback, int *piDatasizePassback);
const char *	CommArea_IsErrorWaiting(void);
const char *	CommArea_IsAckWaiting(byte **ppbDataPassback = NULL, int *piDatasizePassback = NULL);
//
// message-acknowledge functions...
//
const char *	CommArea_CommandAck(const char * psCommand = NULL, byte *pbData = NULL, int iDataSize = 0);
const char *	CommArea_CommandClear(void);
const char *	CommArea_CommandError(const char * psError);
//
// message/command-send functions...
//
const char *	CommArea_IssueCommand(const char * psCommand, byte *pbData = NULL, int iDataSize = 0);


#endif	// #ifndef COMMAREA_H

/////////////// eof /////////////

