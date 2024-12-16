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
};

void inf() {
  while(1);
}

void foo(void* param) {
  char p = 'c';
  write(1, &p, 1);
}

void foo2(void* param) {
  while(1) {
    char p = 'z';
    write(1, &p, 1);
  }
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

  clrscr(mat);

  changeColor(Blue, White);

  threadCreateWithStack(&foo, 1, (void*)3, &exit);
  threadCreateWithStack(&foo2, 1, (void*)3, &inf);

  while(1) {
    char p = 'p';
    write(1, &p, 1);
    /*char *buffer = "_getKey test;\n";
    write(1, buffer, 15);
    char c = ' ';
    int error = getKey(&c, 500);
    if (error != -1) { 
      buffer = "_got this character:";
      write(1, buffer, 21);
      write(1, &c, 1);
      buffer = "\n_got outside getKey\n";
      write(1, buffer, 22);
    }*/
  }
}
