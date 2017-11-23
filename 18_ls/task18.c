#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>

void printType(struct stat *sb) {
  switch(sb->st_mode & S_IFMT) {
    case S_IFDIR:
      printf("d");
      break;
    case S_IFREG:
      printf("-");
      break;
    default:
      printf("?");
  }
}

void printRights(struct stat *sb) {
  static const mode_t Rights[] = {
    S_IRUSR, S_IWUSR, S_IXUSR,
    S_IRGRP, S_IWGRP, S_IXGRP,
    S_IROTH, S_IWOTH, S_IXOTH
  };
  static const char Symbols[] = {
    'r', 'w', 'x'
  };
  char rightsStr[] = "---------";
  for (int i = 0; i < 9; ++i) {
    if (sb->st_mode & Rights[i]) {
      rightsStr[i] = Symbols[i % 3];
    }
  }
  printf("%s", rightsStr);
}

void printNLinks(struct stat *sb) {
  printf("%2d", sb->st_nlink);
}

int printOwnerAndGroup(struct stat *sb) {
  struct passwd *user = getpwuid(sb->st_uid);
  if (!user) {
    perror("getpwuid failed");
    return -1;
  }
  printf("%15s ", user->pw_name);

  struct group *group = getgrgid(sb->st_gid);
  if (!group) {
    perror("getgrgid failed");
    return -1;
  }
  printf("%15s", group->gr_name);

  return 0;
}

void printSize(struct stat *sb) {
  if ((sb->st_mode & S_IFMT) == S_IFREG) {
    printf("%10d", sb->st_size);
  } else {
    printf("%10s", "");
  }
}

int printModificationTime(struct stat *sb) {
  char *time = ctime(&sb->st_mtime);
  if (!time) {
    perror("ctime failed");
    return -1;
  }
  printf("%.24s", time);
  return 0;
}

void printFileName(char *file) {
  printf("%s", basename(file));
}

int printInfo(char *file) {
  struct stat sb;
  if (stat(file, &sb) == -1) {
    perror("stat failed");
    return -1;
  }
  printType(&sb);
  printRights(&sb);
  printf(" ");
  printNLinks(&sb);
  printf(" ");
  if (printOwnerAndGroup(&sb) == -1) {
    return -1;
  }
  printf(" ");
  printSize(&sb);
  printf(" ");
  printModificationTime(&sb);
  printf(" ");
  printFileName(file);
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    if (printInfo(".") == -1) {
      exit(1);
    }
  } else {
    for (int i = 1; i < argc; ++i) {
      if (printInfo(argv[i]) == -1) {
        exit(1);
      }
    }
  }
  return 0;
}
