#include "info.h"

static int blocking(unsigned long, struct mutex *, wait_queue_head_t *);
static wait_queue_head_t * get_lock(object_state *, session *);

#ifndef _COMMONH_
#define _COMMONH_

static int blocking(unsigned long timeout, struct mutex *mutex, wait_queue_head_t *wq)
{
   int val;

   if (timeout == 0) return 0;

   AUDIT printk("%s: thread %d going to usleep for %lu microsecs\n", MODNAME, current->pid, timeout);

   timeout = (timeout*HZ)/1000;   

   //Returns 0, if the condition evaluated to false after the timeout elapsed
   val = wait_event_timeout(*wq, mutex_trylock(mutex), timeout);
   
   AUDIT printk("%s: thread %d go out from usleep\n", MODNAME, current->pid);
   
   if(!val) return 0;

   return 1;
}

static wait_queue_head_t * get_lock(object_state *the_object, session *session){

   int ret;
   wait_queue_head_t *wq;
   wq = &the_object->wq;
   
   // Try to acquire the mutex atomically.
   // Returns 1 if the mutex has been acquired successfully,
   // and 0 on contention.
   ret = mutex_trylock(&(the_object->operation_synchronizer));
   if (!ret)
   {

      printk("%s: unable to get lock now\n", MODNAME);
      if (session->blocking == BLOCKING)
      {

         ret = blocking(session->timeout, &the_object->operation_synchronizer, wq);
         if (ret == 0) return NULL;
      }
      else
         return NULL;
   }

   return wq;
}

#endif
