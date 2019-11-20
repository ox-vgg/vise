#include <sys/types.h>
#include <sys/times.h>
#include <stdio.h>
#include "ttime.h"
  
struct tms colin_time;

void init_time (long *count_time)
{
times(&colin_time);
*count_time = colin_time.tms_utime + colin_time.tms_stime;
 
return;
} 

double tell_time (long count_time)
{
double local_time;
times (&colin_time);
local_time = (double)(colin_time.tms_utime + colin_time.tms_stime - count_time);
return (local_time /60.0);
}
