/**************************************/
/*              analex.h              */
/**************************************/

typedef enum {
	T_WORD,  /* un mot */
	T_BAR,   /* | */
	T_SEMI,  /* ; */
	T_AMPER, /* & */
	T_LT,    /* < */
	T_GT,    /* > */
	T_GTGT,  /* >> */
	T_NL,    /* retour-chariot */
	T_EOF,   /* ctrl-d */
	T_EXIT,  /* commande exit */
	T_CD,			/* commande cd */
	T_VAR_DECLA, /* variable au moment de sa déclaration */
	T_VAR_VAL	/* variable quand on appelle sa valeur */
} TOKEN;

/* renvoie le prochain token de l'entree standard.
Si c'est un mot, le met dans la variable word.
*/
extern TOKEN getToken(char* word);
