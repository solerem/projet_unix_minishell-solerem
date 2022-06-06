/**************************************/
/*            minishell.c             */
/**************************************/

#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include "analex.h"

#define TAILLE_MAX 100 /* taille max d'un mot */
#define ARGS_MAX 10    /* nombre maximum d'arguments a une commande */

/* Execute la commande donnee par le tableau argv[] dans un nouveau processus ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande,
 * a rediriger eventuellement.
 * Renvoie le numero du processus executant la commande
 * Le dernier argument est non nul si on est dans un pipe */
pid_t execute(int entree, int sortie, char* argv[], int pipe){
  if(argv[0] == NULL)
    return getpid();
  int x = fork();
  if (! x){
    if(entree)
      dup2(entree, 0);
    if(sortie - 1)
      dup2(sortie, 1);
    execvp(argv[0],argv);
    perror(NULL);
    exit(1);
  }
  else{
    if(pipe)
      wait(NULL);
    if(pipe || sortie - 1)
      close(sortie);
    if(entree)
      close (entree);
    return x;
  }
  return 0;
}

// Renvoie la copie d'une chaine
char* copie(char* o){
  int i;
  char* p=malloc(100);
  for(i = 0; o[i] != '\0'; i++)
    p[i] = o[i];
  p[i] = '\0';
  return p;
}

/* Lit et execute une commande en recuperant ses tokens ;
 * les deux premiers parametres indiquent l'entree et la sortie de la commande ;
 * pid contient le numero du processus executant la commande et
 * background est non nul si la commande est lancee en arriere-plan
 * id_var et val_var sont l'identificateur et la valeur d'une variable éventuelle, NULL si n'existe pas
 * tabArgs_var est le tableau des arguments si on appelle une variable, NULL sinon */

TOKEN commande(int entree, int sortie, pid_t* pid, int* background, char* id_var, char *val_var, char *tabArgs_var[]) {
  TOKEN t;
  char word[TAILLE_MAX], *tabArgs[TAILLE_MAX*ARGS_MAX];
  int i = 0;
  while(1){
    t = getToken(word);
    if(t == T_VAR_VAL){
      *tabArgs_var = *tabArgs;
      getToken(word);
      tabArgs_var[i++] = copie(word);
      tabArgs_var[i] = NULL;
      strcpy(id_var, copie(word));
      return t;
    }
    if(t == T_VAR_DECLA){
      strcpy(id_var, copie(word));
      getToken(word);
      strcpy(val_var, copie(word));
      return t;
    }
    if(t == T_CD){
      t = getToken(word);
      if((t != T_WORD) && (t != T_CD))
        chdir(getenv("HOME"));
      else{
        *pid = getpid();
        if(chdir(word))
          printf("cd: %s: No such file or directory\n", word);
        return T_SEMI;
      }
    }
    if(t == T_EXIT)
      return t;
    if(t == T_WORD)
      tabArgs[i++] = copie(word);
    if(t == T_GT){
      getToken(word);
      sortie = open(word, O_RDWR | O_CREAT | O_TRUNC, 0666);
    }
    if(t == T_GTGT){
      getToken(word);
      sortie = open(word, O_RDWR | O_CREAT | O_APPEND, 0666);
    }
    if(t == T_LT){
      getToken(word);
      entree = open(word, O_RDWR, 0666);
    }
    if (t == T_BAR){
      int tube[2];
      int pipe(int[2]);
      pipe(tube);
      tabArgs[i] = NULL;
      *pid = execute(entree,tube[1],tabArgs,1);
      return commande(tube[0], sortie, pid, background, NULL, NULL, NULL);;
    }
    if(t == T_AMPER){
      tabArgs[i] = NULL;
      *background = 1;
      printf("%d\n",*pid = execute(entree, sortie, tabArgs,0));
      return t;
    }
    if (t == T_SEMI){
      tabArgs[i] = NULL;
      *pid = execute(entree, sortie, tabArgs,0);
      return t;
    }
    if(t == T_NL){
      tabArgs[i] = NULL;
      *pid = execute(entree, sortie, tabArgs,0);
      return t;
    }
    if (t == T_EOF)
      return t;
  }
  return t;
}

/* Retourne une valeur non-nulle si minishell est en train de s'exécuter en mode interactif,
 * c'est à dire, si l'affichage de minishell n'est pas redirigé vers un fichier.*/
int is_interactive_shell(){
  return isatty(1);
}

// Affiche le prompt "minishell>" uniquement si l'affichage n'est pas redirigé vers un fichier.
void print_prompt(){
  if(is_interactive_shell()){
    printf("mini-shell>");
    fflush(stdout);
  }
}

void rien(int signal){}

// Cherche une chaine dans un tableau de chaine, renvoie sa position ou -1 si elle n'apparait pas
int look_str(char *chaine, char* tabChaine[]){
  int i;
  for(i = 0; tabChaine[i] != NULL; i++){
    if(! strcmp(tabChaine[i],chaine))
      return i;
  }
  return -1;
}

// Fonction main
int main(int argc, char* argv[]) {
  signal(2,rien);
  char *tabvars[TAILLE_MAX*ARGS_MAX*100], *tabvals[TAILLE_MAX*ARGS_MAX*100], var[TAILLE_MAX], val[TAILLE_MAX], *tabArgs_var[TAILLE_MAX*ARGS_MAX];
	TOKEN t;
	pid_t pid;
	int background = 0, status = 0, i, j, nb_var = 0;
	print_prompt();
	while (((t = commande(0, 1, &pid, &background, var, val, tabArgs_var)) != T_EOF) && (t != T_EXIT)){
    if(t == T_VAR_VAL){
      i = look_str(var,tabvars);
      j = look_str(var, tabArgs_var);
      if(i + 1)
        tabArgs_var[j++] = copie(tabvals[i]);
      else
        tabArgs_var[j++] = "";
      tabArgs_var[j] = NULL;
      waitpid(execute(0,1,tabArgs_var,0), &status, 0);
    }
    if(t == T_VAR_DECLA){
      if((i = look_str(var,tabvars)) + 1)
        tabvals[i] = val;
      else{
        tabvars[nb_var] = copie(var);
        tabvals[nb_var++] = copie(val);
        tabvars[nb_var] = NULL;
        tabvals[nb_var] = NULL;
      }
    }
	  if (t == T_NL){
      waitpid(pid, &status, 0);
	    print_prompt();
	  }
    if(t == T_SEMI)
      waitpid(pid, &status, 0);
	}
	if(is_interactive_shell() && t != T_EXIT)
	  printf("\n");
	return WEXITSTATUS(status);
}
