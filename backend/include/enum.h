#ifndef ENUM_H
#define ENUM_H

#ifdef __cplusplus
namespace workrave {
#endif
  
  /* Mode */
  typedef enum OperationMode
    {
      /* Breaks are reported to the user when due. */
      OPERATION_MODE_NORMAL=0,

      /* Monitoring is suspended. */
      OPERATION_MODE_SUSPENDED,

      /* Breaks are not reported to the user when due. */
      OPERATION_MODE_QUIET,

      /* Number of modes.*/
      OPERATION_MODE_SIZEOF
    } OperationMode;

  typedef enum UsageMode
    {
      /* Normal 'average' PC usage. */
      USAGE_MODE_NORMAL=0,

      /* User is reading. */
      USAGE_MODE_READING,
    }  UsageMode;

#ifdef __cplusplus
}
#endif
  
#endif
