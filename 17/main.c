#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define LINESIZE 40
#define BACKSPACE write(STDOUT_FILENO, "\b \b", 3);

int main() {
  if (!isatty(STDIN_FILENO)) {
    fprintf(stderr, "isatty failed");
    exit(1);
  }
  struct termios oldattr, newattr;
  if (tcgetattr(STDIN_FILENO, &oldattr) == -1) {
    perror("tcgetattr failed");
    exit(2);
  }
  newattr = oldattr;
  newattr.c_lflag = ~(ISIG | ICANON | ECHO) & ICRNL;
  newattr.c_cc[VMIN] = 1;
  newattr.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSANOW, &newattr) == -1) {
    perror("tcsetattr failed");
    exit(3);
  }

  int pos = 0;
  char ch;
  char line[LINESIZE];

  while(read(STDIN_FILENO, &ch, 1) > 0) {
    if (ch == CEOT && pos == 0) {
      break;
    } else if (ch == newattr.c_cc[VERASE] && pos > 0) {
      BACKSPACE;
      --pos;
    } else if (ch == newattr.c_cc[VKILL]) {
      while (pos > 0) {
        BACKSPACE;
        --pos;
      }
    } else if (ch == CWERASE) {
      while (pos > 0 && isspace(line[pos - 1])) {
        BACKSPACE;
        --pos;
      }
      while (pos > 0 && !isspace(line[pos - 1])) {
        BACKSPACE;
        --pos;
      }
    } else if (ch == '\n') {
      write(STDOUT_FILENO, &ch, 1);
      pos = 0;
    } else if (!isprint(ch)) {
      write(STDOUT_FILENO, "\a", 1);
    } else {
      write(STDOUT_FILENO, &ch, 1);
      line[pos++] = ch;
    }

    if (pos == LINESIZE) {
      if (!isspace(ch)) {
        int saved = pos;
        while (pos > 0 && !isspace(line[pos - 1])) {
          --pos;
        }
        if (pos > 0) {
          int newpos = 0;
          for (int i = pos; i < saved; ++i) {
            BACKSPACE;
            line[newpos++] = line[i];
          }
          pos = newpos;
          write(STDOUT_FILENO, "\n", 1);
          for (int i = 0; i < pos; ++i) {
            write(STDOUT_FILENO, line + i, 1);
          }
        } else {
          write(STDOUT_FILENO, "\n", 1);
        }
      } else {
        write(STDOUT_FILENO, "\n", 1);
        pos = 0;
      }
    }
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return 0;
}
