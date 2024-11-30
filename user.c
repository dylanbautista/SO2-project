#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  while(1) { 
    char *buffer = "_getKey test;\n";
    write(1, buffer, 15);
    char c = ' ';
    int error = getKey(&c, 1000);
    buffer = "_got this character:";
    write(1, buffer, 21);
    if (error != -1) write(1, &c, 1);
    buffer = "\n_got outside getKey\n";
    write(1, buffer, 22);

  }
}
