#ifndef ZAPPER_H
#define ZAPPER_H

__declspec(dllexport) BOOL ZapWorkrave(void);
__declspec(dllexport) BOOL FindWorkrave(void);
__declspec(dllexport) BOOL KillProcess(char *proces_name_to_kill);

#endif /* ZAPPER_H */
