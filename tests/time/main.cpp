#include "link.h"

float perf_time( bool ms = true ){
  static LARGE_INTEGER freq;

  if(freq.LowPart == 0 && freq.HighPart == 0)
    QueryPerformanceFrequency(&freq);

  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);

  if(count.QuadPart == 0 || freq.QuadPart == 0)
    return 0.f;

  float time = ((float)count.QuadPart / (float)freq.QuadPart);

  if(ms)
    time *= 1000.f;

  return time;
}

i32 main(i32 argc, const i8 *argv[]) {

  i32 target_calls = 100;

  // GetTickCount
  {
    float last_time = 0.f;
    float avg       = 0.f;
    i32   count     = 0;

    while(true){
      float new_time = (float)GetTickCount() / 1000.f;

      if(last_time <= 0.f)
        last_time = new_time;

      if(last_time >= new_time)
        continue;

      avg += new_time - last_time;
      count++;

      last_time = new_time;

      if(count > target_calls)
        break;
    }

    printf("GetTickCount: %f (%i calls)\n", avg / (float)count, target_calls);

    // GetTickCount RDTSC Test
    {
      uptr avg    = 0;
      uptr count  = 0;

      while(count < target_calls){
        uptr start = __rdtsc();
        GetTickCount();
        avg += __rdtsc() - start;
        count++;
      }

      printf("GetTickCount RDTSC: %i (%i calls)\n", avg / count, target_calls);
    }
  }

  // QueryPerformanceCounter
  {
    float last_time = 0.f;
    float avg       = 0.f;
    i32   count     = 0;

    while(true){
      float new_time = perf_time(false);

      if(last_time <= 0.f)
        last_time = new_time;

      if(last_time >= new_time)
        continue;

      avg += new_time - last_time;
      count++;

      last_time = new_time;

      if(count > target_calls)
        break;
    }

    printf("QueryPerformanceCounter: %f (%i calls)\n", avg / (float)count, target_calls);

    // QueryPerformanceCounter RDTSC Test
    {
      uptr avg    = 0;
      uptr count  = 0;

      while(count < target_calls){
        uptr start = __rdtsc();
        perf_time(true);
        avg += __rdtsc() - start;
        count++;
      }

      printf("QueryPerformanceCounter RDTSC: %i (%i calls)\n", avg / count, target_calls);
    }
  }

  system("pause");

  return 0;
}
