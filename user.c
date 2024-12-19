#include <libc.h>
#include <colors.h>
#include <screen_matrix.h>

static char_mat ascii_art = {
" ________  _______   ________  ________",
"|\\_____  \\|\\  ___ \\ |\\   __  \\|\\   ____\\ ",
" \\|___/  /\\ \\   __/|\\ \\  \\|\\  \\ \\  \\___|_ ",
"     /  / /\\ \\  \\_|/_\\ \\  \\\\\\  \\ \\_____  \\",
"    /  /_/__\\ \\  \\_|\\ \\ \\  \\\\\\  \\|____|\\  \\",
"   |\\________\\ \\_______\\ \\_______\\____\\_\\  \\ ",
"    \\|_______|\\|_______|\\|_______|\\_________\\",
"                                 \\|_________|"
}; //Example of a very awful ASCII art...

void foo(void* param) {
  write(1, param, 2);
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */


  
  /*int time0 = gettime();
  while (gettime() < time0 + 500); //Give time to user to press keys
  */

  screen_matrix mat = {0};
  screen_matrix_init(mat);
  
  screen_matrix_compose_char(mat, ascii_art, Yellow, Black);

  screen_matrix_clrscr(mat); //Call wrapper function (avoid using strange pointer maipulation, easier)
  //clrscr((char*) mat); //or call directly clrscr through casting... (not recommeded for the user)

  changeColor(Blue, White);

  //threadCreateWithStack(&foo, 1, (void*)"t1", &exit);
  //threadCreateWithStack(&foo, 2, (void*)"t2", &exit);
  //threadCreateWithStack(&foo, 4, (void*)"t3", &exit);

  //int pid = fork();

  while(1) {
    /*
    if (pid > 0) {
      char p = 'p';
      write(1, &p, 1);
    } else {
      char p = 'P';
      write(1, &p, 1);
    }*/
  }
}
