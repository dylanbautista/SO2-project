/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <sched.h>
#include <io.h>

#include <sched.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

int zeos_ticks = 0;
extern struct list_head blocked;

//Temp systems

void key_timout_system() {
  struct list_head * cursor;
  struct list_head * next;

  //Search for a process that's waiting a key.
  //Linear search, not good if there are too many blocked processors...
  list_for_each_safe(cursor, next, &blocked) {
    struct task_struct * cursor_ts = list_head_to_task_struct(cursor);
    if (cursor_ts->waiting_for_key && cursor_ts->key_timeout > 0) {
      cursor_ts->key_timeout--;
      if (cursor_ts->key_timeout == 0) {
        update_process_state_rr(cursor_ts, &readyqueue); //Put process to ready queue
      }
    }
  }

}


void clock_routine()
{
  zeos_show_clock();
  zeos_ticks ++;

  key_timout_system();
  
  schedule();
}

void keyboard_routine()
{
  unsigned char c = inb(0x60);

  struct list_head * cursor;
  struct list_head * next;

  //Search for a process that's waiting a key.
  //Linear search, not good if there are too many blocked processors...
  list_for_each_safe(cursor, next, &blocked) {
    struct task_struct * cursor_ts = list_head_to_task_struct(cursor);
    if (cursor_ts->waiting_for_key) {
      cursor_ts->pressed_key = char_map[c&0x7f];
      update_process_state_rr(cursor_ts, &readyqueue); //Put process to ready queue
    }
  }
  
  //if (c&0x80) printc_xy(0, 0, char_map[c&0x7f]);
}

int get_ctxt_eip();

void page_fault_ext_routine()
{
  int eip_val = get_ctxt_eip(); //Get the hardware context's eip value
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

  printk("\nProcess generates a PAGE FAULT exception at EIP: 0x");
  printk(buff);
  printk("\n");
  while(1) {}
}

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void clock_handler();
void keyboard_handler();
void system_call_handler();
void page_fault_ext_handler();

void setMSR(unsigned long msr_number, unsigned long high, unsigned long low);

void setSysenter()
{
  setMSR(0x174, 0, __KERNEL_CS);
  setMSR(0x175, 0, INITIAL_ESP);
  setMSR(0x176, 0, (unsigned long)system_call_handler);
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(14, &page_fault_ext_handler, 0);
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(33, keyboard_handler, 0);

  setSysenter();

  set_idt_reg(&idtR);
}

