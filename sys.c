/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <circular_buffer.h>

#include <list.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }

  int found = 0;
  int next_start_pos = NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA;
  page_table_entry *PT = get_PT(current());

  //Need to find temporal free virtual addresses.
  while (!found) { //repeat until found. If not enoguh space, returns.
    int pag = next_start_pos;
    int inner_error = 0;
    for (; pag < TOTAL_PAGES && (pag < next_start_pos + NUM_PAG_DATA); pag++) { //Check if gap is large enough
      if(PT[pag].bits.present != 0) {
        //Search for a new start position
        int temp_pag = pag;
        while (temp_pag < TOTAL_PAGES && PT[temp_pag].bits.present != 0) temp_pag++;
        if (temp_pag >= TOTAL_PAGES) {inner_error = 1; break;} //Not enough free consecutive pages found
        next_start_pos = temp_pag; //Start from the new position.
        break;
      }
    }
    if (pag >= TOTAL_PAGES || inner_error) {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      // Deallocate task_struct
      list_add_tail(lhcurrent, &freequeue);
      //Return error
      return -EAGAIN;
    }
    if (pag >= next_start_pos + NUM_PAG_DATA) found = 1; //previous break instruction has not been reached
  }

  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, next_start_pos+(pag-NUM_PAG_KERNEL+NUM_PAG_CODE), get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((next_start_pos+(pag-NUM_PAG_KERNEL+NUM_PAG_CODE))<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, next_start_pos+(pag-NUM_PAG_KERNEL+NUM_PAG_CODE));
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;
  uchild->task.master_thread = uchild->task.PID;
  uchild->task.master_thread_address = (DWord) uchild;
  uchild->task.pending_unblocks=0;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  //Prepare thread list, if it is needed in a future.
  uchild->task.has_threads = 0;
  INIT_LIST_HEAD(&uchild->task.thread_child_list);

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

void print_hex(int hex) {
  int eip_val = hex; //Get the hardware context's eip value
  char buff[9] = "00000000";
  //Convert decimal to hexadecimal (as string);
  //it uses a friendly algorithm in order to maintain 0's at the beginning
  //if needed.
  for (int i = 0; i < 8; i++) {
    int aux = eip_val;
    aux = aux >> 28;
    aux = (0x0000000F) & aux; //base-2 magic
    if (aux < 10) {
      buff[i] = '0' + aux;
    } else {
      buff[i] = 'A' + (aux - 10);
    }
    eip_val = eip_val << 4;
  }

  printk("\n Value: 0x");
  printk(buff);
  printk("\n");
}

int sys_threadCreateWithStack( void (*function)(void* arg), int N, void* parameter, void (*ext)(void))
{
  //Check user parameters
  //Access ok for function address (read)
  if (!access_ok(VERIFY_READ, function, sizeof(void*))) return -EFAULT;
  //N has a suitable value (< TOTAL_PAGES - (NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA))
  if (N >= TOTAL_PAGES - (NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA)) return -EINVAL;


  //Busquem un thread a la freequeue
  if (list_empty(&freequeue)) return -ENOMEM;

  struct list_head * newthreadlist = list_first(&freequeue);
  list_del(newthreadlist);
  struct task_struct * newthread = list_head_to_task_struct(newthreadlist);
  union task_union * u_newthread = (union task_union *) newthread;

  copy_data(current(), u_newthread, sizeof(union task_union));
  //newthread->dir_pages_baseAddr = get_DIR(current());
  page_table_entry *PT = get_PT(newthread);

  int found = 0;

  int next_start_pos = NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA;

  while (!found) { //repeat until found. If not enoguh space, returns.
    int pag = next_start_pos;
    int inner_error = 0;
    for (; pag < TOTAL_PAGES && (pag < next_start_pos + N); pag++) { //Check if gap is large enough
      if(PT[pag].bits.present != 0) {
        //Search for a new start position
        int temp_pag = pag;
        while (temp_pag < TOTAL_PAGES && PT[temp_pag].bits.present != 0) temp_pag++;
        if (temp_pag >= TOTAL_PAGES) {inner_error = 1; break;} //Not enough free consecutive pages found
        next_start_pos = temp_pag; //Start from the new position.
        break;
      }
    }
    if (pag >= TOTAL_PAGES || inner_error) {
      // Deallocate task_struct
      list_add_tail(newthreadlist, &freequeue);
      //Return error
      return -EAGAIN;
    }
    if (pag >= next_start_pos + N) found = 1; //previous break instruction has not been reached
  }

  //Search enough free physical addresses for the USER STACK
  int new_pag, pag;
  for (pag = 0; pag < N; ++pag) {
    new_pag = alloc_frame();
    if (new_pag != -1) {
      set_ss_pag(PT, next_start_pos+pag, new_pag);
    } else {
      // Deallocate allocated pages. Up to pag.
      for (int i=0; i<pag; i++)
      {
        free_frame(get_frame(PT, next_start_pos+i));
        del_ss_pag(PT, next_start_pos+i);
      }
      // Deallocate task_struct
      list_add_tail(newthreadlist, &freequeue);

      // Return error 
      return -EAGAIN; 
    }
  }

  //Assignem PID i altres dedes del PCB
  newthread->PID=++global_PID;
  newthread->state=ST_READY;
  newthread->pending_unblocks=0;
  newthread->master_thread = current()->master_thread;
  newthread->master_thread_address = current()->master_thread_address;
  newthread->thread_user_stack_base_page = (DWord) (next_start_pos);
  newthread->thread_user_stack_num_page = N;
  INIT_LIST_HEAD(&(newthread->memoria));

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(u_newthread);

  newthread->register_esp=register_ebp;

  DWord temp_ebp=(DWord) 244003; //Random value for ebp
  /* Prepare child stack for context switch */
  *(DWord*)(newthread->register_esp)=(DWord)&ret_from_fork; //Why not?
  newthread->register_esp-=sizeof(DWord);
  *(DWord*)(newthread->register_esp)=temp_ebp;

  init_stats(&(u_newthread->task.p_stats));

  DWord new_esp = (DWord) ((next_start_pos + N) << 12) - 4;
  
	//newthread->register_esp = (DWord) &newthread[KERNEL_STACK_SIZE-19];
  u_newthread->stack[KERNEL_STACK_SIZE - 2] = (DWord) new_esp - 4; //Change user stack address
  u_newthread->stack[KERNEL_STACK_SIZE - 5] = (DWord) function; //Change saved eip value

  //Prepare child thread user stack
  copy_to_user(&ext,(void*) new_esp-4,sizeof(DWord)); //Copy exit function to thread's user stack
  copy_to_user(&parameter,(void*) new_esp,sizeof(DWord)); //Copy parameter to thread's user stack

  //Add thread in parent's thread list
  if (current()->master_thread == current()->PID) current()->has_threads = 1;
  newthread->has_threads = 0;
  list_add(&newthread->thread_child_anchor, &((struct task_struct*) current()->master_thread_address)->thread_child_list);

  //Add thread to ready queue
  u_newthread->task.state=ST_READY;
  list_add_tail(newthreadlist, &readyqueue);

  return newthread->PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
  char localbuffer [TAM_BUFFER];
  int bytes_left;
  int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}

extern struct list_head blocked;
extern struct list_head getKey_blocked;

int sys_block() {

  if (current()->pending_unblocks == 0) {
    update_process_state_rr(current(), &blocked);
    sched_next_rr();
  } else {
    if (current()->pending_unblocks > 0) --(current()->pending_unblocks);
  }

  return 0;
}

extern int zeos_ticks;
extern struct circular_buffer keyboard_buffer;
extern int pending_to_service;

int sys_getKey(char* b, int timeout) {

  if (!access_ok(VERIFY_WRITE, b, sizeof(char))) return -EFAULT;
  current()->key_timeout = timeout;

  //Block process, to the getKey_blocked list, if:
  //1. There are ready processes pending to get a char from the buffer.
  //2. No chars are available in the buffer. 
  //3. There are processes already blocked.
  if (pending_to_service > 0 || circular_buffer_empty(&keyboard_buffer) || !list_empty(&getKey_blocked)) {
    update_process_state_rr(current(), &getKey_blocked);
    sched_next_rr();
  }

  if (current()->key_timeout <= 0) return -ETIME;

  pending_to_service--; //process has been served 

  char c = circular_buffer_pop(&keyboard_buffer);

  if ((int) c == -1) return -EAGAIN;

  copy_to_user(&c, b, sizeof(char));
  return 0;
}

int sys_gettime()
{
  return zeos_ticks;
}

int sys_clrscr(char* b) {
  if (!access_ok(VERIFY_READ, b, sizeof(DWord [25][80]))) return -EFAULT;

  if (b != NULL) {
    Word (*matrix)[80] = (Word (*)[80])b;
    clear_paint_screen(matrix);
  } else {
    clear_screen();
  }

  return 0;
}

int sys_gotoXY(int x, int y) {

  if (x > 80 || y > 25 || x < 0 || y < 0) return -EINVAL;

  setXY(x,y);

  return 0;
}

int sys_changeColor(int fg, int bg) {
  change_screen_colors(fg, bg);

  return 0;
}

int sys_memRegGet(int num_pages) {

  if (num_pages >= TOTAL_PAGES - (NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA)) return -EINVAL;

  page_table_entry *PT = get_PT(current());
  int found = 0;
  int next_start_pos = NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA;

  while (!found) { //repeat until found. If not enoguh space, returns.
    int pag = next_start_pos;
    int inner_error = 0;
    for (; pag < TOTAL_PAGES && (pag < next_start_pos + num_pages); pag++) { //Check if gap is large enough
      if(PT[pag].bits.present != 0) {
        //Search for a new start position
        int temp_pag = pag;
        while (temp_pag < TOTAL_PAGES && PT[temp_pag].bits.present != 0) temp_pag++;
        if (temp_pag >= TOTAL_PAGES) {inner_error = 1; break;} //Not enough free consecutive pages found
        next_start_pos = temp_pag; //Start from the new position.
        break;
      }
    }
    if (pag >= TOTAL_PAGES || inner_error) {
      //Return error
      return -EAGAIN;
    }
    if (pag >= next_start_pos + num_pages) found = 1; //previous break instruction has not been reached
  }

  int new_pag, pag;
  for (pag = 0; pag < num_pages; ++pag) {
    new_pag = alloc_frame();
    if (new_pag != -1) {
      set_ss_pag(PT, next_start_pos+pag, new_pag);
    } else {
      // Deallocate allocated pages. Up to pag.
      for (int i=0; i<pag; i++)
      {
        free_frame(get_frame(PT, next_start_pos+i));
        del_ss_pag(PT, next_start_pos+i);
      }

      // Return error 
      return -EAGAIN; 
    }
  }

  //Add info in dynamic memory data structure

  return next_start_pos << 12; //Return logical address
}

int sys_memRegDel(char* m) {

}

void sys_exit()
{  
  if (current()->master_thread != current()->PID) { //If it is a thread
    terminate_thread(current());
    /* Restarts execution of the next process */
    sched_next_rr();
    
  } else {
    int i;

    page_table_entry *process_PT = get_PT(current());

    // Deallocate all the propietary physical pages
    for (i=0; i<NUM_PAG_DATA; i++)
    {
      free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
      del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
    }

    if (current()->has_threads) {
      struct list_head * e;

      //Also exit child thread execution.
      list_for_each(e, &current()->thread_child_list) {
        terminate_thread(list_head_to_task_struct(e));
        INIT_LIST_HEAD(&list_head_to_task_struct(e)->thread_child_anchor);
      }
    }

    INIT_LIST_HEAD(&current()->thread_child_list);
    
    /* Free task_struct */
    list_add_tail(&(current()->list), &freequeue);
    
    current()->PID=-1;
    
    /* Restarts execution of the next process */
    sched_next_rr();
  }
}

void terminate_thread(struct task_struct * thread_ts) {
  int i;

  page_table_entry *thread_PT = get_PT(thread_ts);

  // Deallocate all thread's user stack
  for (i=0; i<thread_ts->thread_user_stack_num_page; i++)
  {
    free_frame(get_frame(thread_PT, thread_ts->thread_user_stack_base_page+i));
    del_ss_pag(thread_PT, thread_ts->thread_user_stack_base_page+i);
  }
  
  list_add_tail(&(thread_ts->list), &freequeue);
  thread_ts->PID=-1;

}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}


char* sys_memRegGet(int num_pages)
{
  page_table_entry *PT = get_PT(current());
  if (num_pages < 0) return (char*)-EINVAL;
  ++num_pages;

  int found = 0;
  int next_start_pos = NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA;

  //printk("Buscant memoria\n");
  while (!found) { //repeat until found. If not enoguh space, returns.
    int pag = next_start_pos;
    int inner_error = 0;
    for (; pag < TOTAL_PAGES && (pag < next_start_pos + num_pages); pag++) { //Check if gap is large enough
      if(PT[pag].bits.present != 0) {
        //Search for a new start position
        int temp_pag = pag;
        while (temp_pag < TOTAL_PAGES && PT[temp_pag].bits.present != 0) temp_pag++;
        if (temp_pag >= TOTAL_PAGES) {inner_error = 1; break;} //Not enough free consecutive pages found
        next_start_pos = temp_pag; //Start from the new position.
        break;
      }
    }
    if (pag >= TOTAL_PAGES || inner_error) {
      //Return error
      return (char*) -EAGAIN;
    }
    if (pag >= next_start_pos + num_pages) found = 1; //previous break instruction has not been reached
  }

  //printk("Assignant memoria\n");

  //Search enough free physical addresses for the USER STACK
  int new_pag, pag;
  for (pag = 0; pag < num_pages; ++pag) {
    new_pag = alloc_frame();
    if (new_pag != -1) {
      set_ss_pag(PT, next_start_pos+pag, new_pag);
    } else {
      // Deallocate allocated pages. Up to pag.
      for (int i=0; i<pag; i++)
      {
        free_frame(get_frame(PT, next_start_pos+i));
        del_ss_pag(PT, next_start_pos+i);
      }
      // Return error 
      return (char*) -EAGAIN; 
    }
  }

  struct list_head *list = (struct list_head *) (next_start_pos <<12);
  INIT_LIST_HEAD(list);
  list_add_tail(list, &(current()->memoria));
  //printk("list hecho\n");
  int * tam = (int *) ((next_start_pos <<12) + sizeof(struct list_head));
  *tam = num_pages;
  print_hex(num_pages);

  //printk("Retornant\n");
  //print_hex((next_start_pos << 12));
  return (char*) (next_start_pos <<12) + sizeof(int) + sizeof(struct list_head);
}

int sys_memRegDel(char* m)
{
  printk("Eliminant memoria\n");
  //print_hex((m  - sizeof(int) - sizeof(struct list_head)));
  struct list_head *l = (struct list_head *) (m  - sizeof(int) - sizeof(struct list_head));
  list_del(l);

  page_table_entry *PT = get_PT(current());
  int pag = ((int)l >>12);
  int * a = (m  - sizeof(int));
  int num_pag = *a;
  print_hex(num_pag);
  for (int i = 0; i < num_pag; i++) {
    free_frame(get_frame(PT, pag+i));
    del_ss_pag(PT, pag+i);
  }
  set_cr3(get_DIR(current()));
  return 0;
}