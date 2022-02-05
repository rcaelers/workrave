#ifndef WINDOWS_WMF_HH
#define WINDOWS_WMF_HH

#include <cstdint>
#include <windows.h>

struct WNF_STATE_NAME
{
  ULONG data[2];

  bool operator==(const WNF_STATE_NAME &other)
  {
    return data[0] == other.data[0] && data[1] == other.data[1];
  }
};

struct WNF_TYPE_ID
{
  GUID type_id;
};

using PWNF_TYPE_ID = WNF_TYPE_ID *;
using PCWNF_TYPE_ID = const WNF_TYPE_ID *;
using WNF_CHANGE_STAMP = ULONG;
using PWNF_CHANGE_STAMP = WNF_CHANGE_STAMP *;

extern "C"
{
  using PWNF_USER_CALLBACK = NTSTATUS (*)(_In_ WNF_STATE_NAME state_name,
                                          _In_ WNF_CHANGE_STAMP change_stamp,
                                          _In_opt_ PWNF_TYPE_ID type_id,
                                          _In_opt_ PVOID context,
                                          _In_ const VOID *buffer,
                                          _In_ ULONG length);

  using PRTLQUERYWNFSTATEDATA = NTSTATUS (*)(_Out_ PWNF_CHANGE_STAMP change_stamp,
                                             _In_ WNF_STATE_NAME state_name,
                                             _In_ PWNF_USER_CALLBACK callback,
                                             _In_opt_ PVOID context,
                                             _In_opt_ PWNF_TYPE_ID type_id);

  using PRTLSUBSCRIBEWNFSTATECHANGENOTIFICATION = NTSTATUS (*)(_Outptr_ DWORD64 *subscription,
                                                               _In_ WNF_STATE_NAME state_name,
                                                               _In_ WNF_CHANGE_STAMP change_stamp,
                                                               _In_ PWNF_USER_CALLBACK callback,
                                                               _In_opt_ PVOID context,
                                                               _In_opt_ PCWNF_TYPE_ID type_id,
                                                               _In_opt_ ULONG serialization_group,
                                                               _In_opt_ ULONG unknown);

  using PRTLUNSUBSCRIBEWNFSTATECHANGENOTIFICATION = NTSTATUS (*)(_In_ DWORD64 subscription);
};
#endif
