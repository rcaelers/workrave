#ifndef ZAPPER_H
#define ZAPPER_H

__declspec(dllexport) BOOL ZapWorkrave(void);
__declspec(dllexport) BOOL FindWorkrave(void);

__declspec(dllexport) int TerminateProcessesByNames(const char *directory, const char **executable_names_to_kill, bool dry_run);
__declspec(dllexport) BOOL AreWorkraveProcessesRunning(const char *directory);
__declspec(dllexport) BOOL KillWorkraveProcesses(const char *directory);

#endif /* ZAPPER_H */
