#ifndef ZAPPER_H
#define ZAPPER_H

__declspec(dllexport) BOOL ZapWorkrave();
__declspec(dllexport) BOOL FindWorkrave();
__declspec(dllexport) BOOL KillProcess(char *proces_name_to_kill);

#endif /* ZAPPER_H */
