#include "headers.h"

char ** parse_args( char * line ){

    char ** pointers = malloc (1000);

    int i = 0;

    char * entry = malloc(100);

    while ((entry = strsep(&line, " "))){
        pointers[i] = entry;
        i++;
        //printf("pointer %d: %s\n", i, entry);
    }
    pointers[i] = NULL;
    return pointers;

}

void execute(char * command) {
  if (strstr(command, ">") != NULL) {//if uses redirect
    stdout_to_file(command);
  }
  if (strstr(command, "<") != NULL) {
    file_to_stdin(command);
  }
  char ** args = parse_args(command);
  int status;
  if (!strcmp(args[0], "exit")) {//if exit command is called
    exit(0);
  }
  if (!strcmp(args[0], "cd")) {//if cd is called
    char nwd[512];
    getcwd(nwd, sizeof(nwd));
    strcat(nwd, "/");
    strcat(nwd, args[1]);
    //printf("Changing to: %s\n", nwd);
    chdir(args[1]);
  }
  else {
    if (fork()) {//parent
      wait(&status);
    }
    else {//child that executes command
      execvp(args[0], args);
      exit(0);
    }
  }
}

void stdout_to_file(char * line) {
  char * command = strsep(&line, ">");
  char * file = line;
  command = trim(command); //input
  file = trim(file); //output
  //printf("command: %s\n", command);
  //printf("file: %s\n", file);
  int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  //printf("fd: %d\n", fd);
  //printf("Errno: %s\n", strerror(errno));
  int fout = fileno(stdout);
  int newout = dup(fout);//file no of stdout
  dup2(fd, fout);
  execute(command);
  dup2(newout, fout);
  close(newout);
}

void file_to_stdin(char * line) {
  char * file = strsep(&line, "<");
  char * command = line;
  command = trim(command);
  file = trim(file);
  int fd = open(file, O_RDONLY);
  int newin = dup(0);//file no of stdin
  dup2(fd, 0);
  execute(command);
  dup2(newin, 0);
  close(newin);
}


void execute_all(char * line){
    int i = 0;
    char * command = malloc(100);

    while ((command = strsep(&line, ";"))){
        //printf("command: %s\n", command);
        command = trim(command);
        execute(command);
        i++;

    }
}


void piping(char * line){

  char ** pointers = malloc (1000);
    int i = 0;
    char * entry = malloc(100);
    while ((entry = strsep(&line, "|"))){
      pointers[i] = trim(entry);
        i++;
    }
    pointers[i] = NULL;

    FILE *fp = popen(pointers[0], "r");

    char * info = malloc(99999 + 1);
    fgets(info, sizeof(info), fp);

    char ** args = malloc(100);
    args[1] = info;
    args[2] = NULL;

    execvp(pointers[1], args);
    pclose(fp);

}



char * trim(char * raw){
  while (isspace(*raw)) {
    raw++;
  }

  char * end = raw + strlen(raw);
  while (isspace(*(end - 1))) {
    end--;
  }
  *end = 0;
  return raw;
}

char * last_cwd(char cwd[]) {
  char * cwad = cwd;
  // printf("%s\n", cwad);
  while (strstr(cwad, "/")){
    cwad++;
    // printf("%s\n", cwad);
  }
  return cwad;
}



int main(){
  while (1) {//terminal keeps running

//===========Prompt=======================
    char * userin = malloc(500);
    char hostname[512];
    hostname[511] = '\0';//end of file character
    gethostname(hostname, 511);
    char cwd[512];
    getcwd(cwd, sizeof(cwd));
    char * cwad = last_cwd(cwd);
    printf("%s:%s$ ", hostname, cwad); //print prompt

//===========Shell=========================
    fgets(userin, 500, stdin);
    strtok(userin, "\n"); //remove newline
    userin = trim(userin);
    execute_all(userin);
  }
  return 0;
}
