/*
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
*/

// remove unnecessary timing code to allow cross platform compilation
// Abhishek Dutta <adutta@robots.ox.ac.uk>, 12 Mar. 2020
#include "ttime.h"
void init_time(long* count_time)
{
    *count_time = 0;
}

double tell_time(long count_time) {
    return 0.0;
}
