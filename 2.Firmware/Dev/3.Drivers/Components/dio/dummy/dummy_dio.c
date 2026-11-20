#include "dio.h"

__attribute__(( weak )) void do_set(dio_t *me)
{
  // Dummy implementation: Set the status to true
  me->bStatus = true;
  return;
}
__attribute__(( weak )) void do_reset(dio_t *me)
{
  me->bStatus = false;
   return;
}
__attribute__(( weak )) void do_toggle(dio_t *me)
{
  me->bStatus = !me->bStatus;
   return;
}