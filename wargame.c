

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define NB_LIGNES 10
#define NB_COLONNES 10
#define INFINI 10000
#define PROFONDEUR_MAX 3

/*#define DEBUG*/

// Structures IA
typedef struct pion_s
{
	int couleur;
	int valeur;
} Pion;

typedef struct cell
{
	Pion *plateau;
	struct cell *suivant;
} * Liste;

Pion *plateauDeJeu;

Pion *plateauAJouer;

// Variables de statistiques

int nbToursIA;			   // nb de tours du jeu avant que l'IA gagne ou perde
int nbAppelsMinMAX;		   // nb appels de fmin ou fmax
float moyenneFilsAnalyses; // nb moyen de fils analyses pendant le jeu
int nbCoupures;			   // nb total coupures
float moyenneNbCoupures;   // moyenne du nb de coupures par tours de l'IA
float vMoyExecIA;		   // Vitesse moyenne d'execution de l'IA
float *tabVExecIA;		   // tableau des vitesses pour chaque tour d'IA
int nbFilsAnalyses;		   // nb total de fils analysés
float totalTpsIA;		   // somme des temps
struct timeval time1;
struct timeval time2;

void f_affiche_plateau(Pion *plateau);
int f_convert_char2int(char c);
char f_convert_int2char(int i);
int max(int val, int resultat);
int min(int val, int resultat);
Liste genererFils(Pion *noeud, int joueur);
int getDistance(int x, int joueur);
void freeList(Liste list);
int evaluation(Pion* plateau, int joueur);

int f_max(Pion *noeud, int profondeur, int joueur);
int f_min(Pion *noeud, int profondeur, int joueur);
int f_max_stats(Pion *noeud, int profondeur, int joueur);
int f_min_stats(Pion *noeud, int profondeur, int joueur);
int f_max_stats_elagage(Pion *noeud, int profondeur, int joueur, int alpha, int beta);
int f_min_stats_elagage(Pion *noeud, int profondeur, int joueur, int alpha, int beta);

void initStats();
void afficheStats();
void getTime(int a);
float getSec();

int f_convert_char2int(char c)
{
#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
	if (c >= 'A' && c <= 'Z')
		return (int)(c - 'A');
	if (c >= 'a' && c <= 'z')
		return (int)(c - 'a');
	return -1;
}

char f_convert_int2char(int i)
{
#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

	return (char)i + 'A';
}

Pion *f_init_plateau()
{
	int i, j;
	Pion *plateau = NULL;

#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

	plateau = (Pion *)malloc(NB_LIGNES * NB_COLONNES * sizeof(Pion));
	if (plateau == NULL)
	{
		printf("error: unable to allocate memory\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < NB_LIGNES; i++)
	{
		for (j = 0; j < NB_COLONNES; j++)
		{
			plateau[i * NB_COLONNES + j].couleur = 0;
			plateau[i * NB_COLONNES + j].valeur = 0;
		}
	}

	plateau[9 * NB_COLONNES + 5].couleur = 1;
	plateau[9 * NB_COLONNES + 5].valeur = 1;

	plateau[9 * NB_COLONNES + 6].couleur = 1;
	plateau[9 * NB_COLONNES + 6].valeur = 2;

	plateau[9 * NB_COLONNES + 7].couleur = 1;
	plateau[9 * NB_COLONNES + 7].valeur = 3;

	plateau[9 * NB_COLONNES + 8].couleur = 1;
	plateau[9 * NB_COLONNES + 8].valeur = 2;

	plateau[9 * NB_COLONNES + 9].couleur = 1;
	plateau[9 * NB_COLONNES + 9].valeur = 1;

	plateau[8 * NB_COLONNES + 0].couleur = 1;
	plateau[8 * NB_COLONNES + 0].valeur = 1;

	plateau[8 * NB_COLONNES + 1].couleur = 1;
	plateau[8 * NB_COLONNES + 1].valeur = 3;

	plateau[8 * NB_COLONNES + 2].couleur = 1;
	plateau[8 * NB_COLONNES + 2].valeur = 3;

	plateau[8 * NB_COLONNES + 3].couleur = 1;
	plateau[8 * NB_COLONNES + 3].valeur = 1;

	plateau[8 * NB_COLONNES + 6].couleur = 1;
	plateau[8 * NB_COLONNES + 6].valeur = 1;

	plateau[8 * NB_COLONNES + 7].couleur = 1;
	plateau[8 * NB_COLONNES + 7].valeur = 1;

	plateau[8 * NB_COLONNES + 8].couleur = 1;
	plateau[8 * NB_COLONNES + 8].valeur = 1;

	plateau[7 * NB_COLONNES + 1].couleur = 1;
	plateau[7 * NB_COLONNES + 1].valeur = 1;

	plateau[7 * NB_COLONNES + 2].couleur = 1;
	plateau[7 * NB_COLONNES + 2].valeur = 1;

	plateau[2 * NB_COLONNES + 7].couleur = -1;
	plateau[2 * NB_COLONNES + 7].valeur = 1;

	plateau[2 * NB_COLONNES + 8].couleur = -1;
	plateau[2 * NB_COLONNES + 8].valeur = 1;

	plateau[1 * NB_COLONNES + 1].couleur = -1;
	plateau[1 * NB_COLONNES + 1].valeur = 1;

	plateau[1 * NB_COLONNES + 2].couleur = -1;
	plateau[1 * NB_COLONNES + 2].valeur = 1;

	plateau[1 * NB_COLONNES + 3].couleur = -1;
	plateau[1 * NB_COLONNES + 3].valeur = 1;

	plateau[1 * NB_COLONNES + 6].couleur = -1;
	plateau[1 * NB_COLONNES + 6].valeur = 1;

	plateau[1 * NB_COLONNES + 7].couleur = -1;
	plateau[1 * NB_COLONNES + 7].valeur = 3;

	plateau[1 * NB_COLONNES + 8].couleur = -1;
	plateau[1 * NB_COLONNES + 8].valeur = 3;

	plateau[1 * NB_COLONNES + 9].couleur = -1;
	plateau[1 * NB_COLONNES + 9].valeur = 1;

	plateau[0 * NB_COLONNES + 0].couleur = -1;
	plateau[0 * NB_COLONNES + 0].valeur = 1;

	plateau[0 * NB_COLONNES + 1].couleur = -1;
	plateau[0 * NB_COLONNES + 1].valeur = 2;

	plateau[0 * NB_COLONNES + 2].couleur = -1;
	plateau[0 * NB_COLONNES + 2].valeur = 3;

	plateau[0 * NB_COLONNES + 3].couleur = -1;
	plateau[0 * NB_COLONNES + 3].valeur = 2;

	plateau[0 * NB_COLONNES + 4].couleur = -1;
	plateau[0 * NB_COLONNES + 4].valeur = 1;

#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif

	return plateau;
}

void f_affiche_plateau(Pion *plateau)
{
	int i, j, k;

#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

	printf("\n    ");
	for (k = 0; k < NB_COLONNES; k++)
		printf("%2c ", f_convert_int2char(k));
	printf("\n    ");
	for (k = 0; k < NB_COLONNES; k++)
		printf("-- ");
	printf("\n");
	for (i = NB_LIGNES - 1; i >= 0; i--)
	{
		printf("%2d ", i);
		for (j = 0; j < NB_COLONNES; j++)
		{
			printf("|");
			switch (plateau[i * NB_COLONNES + j].couleur)
			{
			case -1:
				printf("%do", plateau[i * NB_COLONNES + j].valeur);
				break;
			case 1:
				printf("%dx", plateau[i * NB_COLONNES + j].valeur);
				break;
			default:
				printf("  ");
			}
		}
		printf("|\n    ");
		for (k = 0; k < NB_COLONNES; k++)
			printf("-- ");
		printf("\n");
	}
	printf("    ");

#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

int f_gagnant(Pion* plateau)
{
	int i, j, somme1 = 0, somme2 = 0;

#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

	//Quelqu'un est-il arrive sur la ligne de l'autre
	for (i = 0; i < NB_COLONNES; i++)
	{
		if (plateau[i].couleur == 1)
			return 1;
		if (plateau[(NB_LIGNES - 1) * NB_COLONNES + i].couleur == -1)
			return -1;
	}

	//taille des armees
	for (i = 0; i < NB_LIGNES; i++)
	{
		for (j = 0; j < NB_COLONNES; j++)
		{
			if (plateau[i * NB_COLONNES + j].couleur == 1)
				somme1++;
			if (plateau[i * NB_COLONNES + j].couleur == -1)
				somme2++;
		}
	}
	if (somme1 == 0)
		return -1;
	if (somme2 == 0)
		return 1;

#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
	return 0;
}

/**
 * Prend comme argument la ligne et la colonne de la case
 * 	pour laquelle la bataille a lieu
 * Renvoie le couleur du gagnant
 * */
int f_bataille(int l, int c)
{
	int i, j, mini, maxi, minj, maxj;
	int somme = 0;

#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
	mini = l - 1 < 0 ? 0 : l - 1;
	maxi = l + 1 > NB_LIGNES - 1 ? NB_LIGNES - 1 : l + 1;
	minj = c - 1 < 0 ? 0 : c - 1;
	maxj = c + 1 > NB_COLONNES - 1 ? NB_COLONNES - 1 : c + 1;

	for (i = mini; i <= maxi; i++)
	{
		for (j = minj; j <= maxj; j++)
		{
			somme += plateauDeJeu[i * NB_COLONNES + j].couleur * plateauDeJeu[i * NB_COLONNES + j].valeur;
		}
	}
	somme -= plateauDeJeu[l * NB_COLONNES + c].couleur * plateauDeJeu[l * NB_COLONNES + c].valeur;

#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
	if (somme < 0)
		return -1;
	if (somme > 0)
		return 1;

	return plateauDeJeu[l * NB_COLONNES + c].couleur;
}

/**
 * Prend la ligne et colonne de la case d'origine
 * 	et la ligne et colonne de la case de destination
 * Renvoie 1 en cas d'erreur
 * Renvoie 0 sinon
 * */
int f_test_mouvement(Pion *plateau, int l1, int c1, int l2, int c2, int couleur)
{
#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
	printf("de (%d,%d) vers (%d,%d)\n", l1, c1, l2, c2);
#endif
	/* Erreur, hors du plateau */
	if (l1 < 0 || l1 >= NB_LIGNES || l2 < 0 || l2 >= NB_LIGNES ||
		c1 < 0 || c1 >= NB_COLONNES || c2 < 0 || c2 >= NB_COLONNES)
		return 1;
	/* Erreur, il n'y a pas de pion a deplacer ou le pion n'appartient pas au joueur*/
	if (plateau[l1 * NB_COLONNES + c1].valeur == 0 || plateau[l1 * NB_COLONNES + c1].couleur != couleur)
		return 1;
	/* Erreur, tentative de tir fratricide */
	if (plateau[l2 * NB_COLONNES + c2].couleur == plateau[l1 * NB_COLONNES + c1].couleur)
		return 1;

	if (l1 - l2 > 1 || l2 - l1 > 1 || c1 - c2 > 1 || c2 - c1 > 1 || (l1 == l2 && c1 == c2))
		return 1;
#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
	return 0;
}

/**
 * Prend la ligne et colonne de la case d'origine
 * 	et la ligne et colonne de la case de destination
 *  et effectue le trantement de l'operation demandée
 * Renvoie 1 en cas d'erreur
 * Renvoie 0 sinon
 * */
int f_bouge_piece(Pion *plateau, int l1, int c1, int l2, int c2, int couleur)
{
	int gagnant = 0;

#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

	if (f_test_mouvement(plateau, l1, c1, l2, c2, couleur) != 0)
		return 1;

	/* Cas ou il n'y a personne a l'arrivee */
	if (plateau[l2 * NB_COLONNES + c2].valeur == 0)
	{
		plateau[l2 * NB_COLONNES + c2].couleur = plateau[l1 * NB_COLONNES + c1].couleur;
		plateau[l2 * NB_COLONNES + c2].valeur = plateau[l1 * NB_COLONNES + c1].valeur;
		plateau[l1 * NB_COLONNES + c1].couleur = 0;
		plateau[l1 * NB_COLONNES + c1].valeur = 0;
	}
	else
	{
		gagnant = f_bataille(l2, c2);
		/* victoire */
		if (gagnant == couleur)
		{
			plateau[l2 * NB_COLONNES + c2].couleur = plateau[l1 * NB_COLONNES + c1].couleur;
			plateau[l2 * NB_COLONNES + c2].valeur = plateau[l1 * NB_COLONNES + c1].valeur;
			plateau[l1 * NB_COLONNES + c1].couleur = 0;
			plateau[l1 * NB_COLONNES + c1].valeur = 0;
		}
		/* defaite */
		else if (gagnant != 0)
		{
			plateau[l1 * NB_COLONNES + c1].couleur = 0;
			plateau[l1 * NB_COLONNES + c1].valeur = 0;
		}
	}

#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
	return 0;
}

//Calcul du nombre de pions sur le plateau du joueur
int f_nbPions(Pion *jeu, int joueur)
{
	int nbPion = 0;
	int i, j;
	for (i = 0; i < NB_COLONNES; ++i)
	{
		for (j = 0; j < NB_LIGNES; ++j)
		{
			if (jeu[i * NB_COLONNES + j].couleur == joueur)
			{
				++nbPion;
			}
		}
	}
	return nbPion;
}

//Calcul de la valeur de tous les pions du joueur
int f_valeur(Pion *jeu, int joueur)
{
	int i, j;
	int valeur = 0;
	for (i = 0; i < NB_COLONNES; ++i)
	{
		for (j = 0; j < NB_LIGNES; ++j)
		{
			if (jeu[i * NB_COLONNES + j].couleur == joueur)
			{
				valeur += jeu[i * NB_COLONNES + j].valeur;
			}
		}
	}
	return valeur;
}

//fonction d'évaluation
int f_eval(Pion *jeu, int joueur)
{
	
	// check victoire ou defaite
	if (f_gagnant(jeu) == joueur)
	{
		return joueur * INFINI;
	}
	return evaluation(jeu, joueur) - evaluation(jeu, -joueur);
	
}

int evaluation(Pion* plateau, int joueur){
    int avancementPions = 0;	// score d'avancement des pions
	int nbPions = 0;
	int sommeScore = 0;
	for (int i = 0; i < NB_LIGNES; i++)
	{
		for (int j = 0; j < NB_COLONNES; j++)
		{
			if(plateau[i * NB_LIGNES + j].couleur == joueur){
				avancementPions += getDistance(i, joueur);
				nbPions++;
				sommeScore += plateau[i * NB_LIGNES + j].valeur;
			}
		}
	}
	return avancementPions + (int)rand()%5;
	//return avancementPions + 5*nbPions + sommeScore + (int)rand()%5;
}

int getDistance(int x, int joueur)
{
	if (joueur == -1)
	{
		return NB_LIGNES - x;
	}
	else
	{
		return x;
	}
}

//copie du plateau
void f_copie_plateau(Pion *source, Pion *destination)
{
	int i, j;
	for (i = 0; i < NB_LIGNES; i++)
	{
		for (j = 0; j < NB_COLONNES; j++)
		{
			destination[i * NB_COLONNES + j].couleur = source[i * NB_COLONNES + j].couleur;
			destination[i * NB_COLONNES + j].valeur = source[i * NB_COLONNES + j].valeur;
		}
	}
}

//mise a zero du plateau
Pion *f_raz_plateau()
{
	Pion *jeu = NULL;
	int i, j;
	jeu = (Pion *)malloc(NB_LIGNES * NB_COLONNES * sizeof(Pion));
	for (i = 0; i < NB_LIGNES; i++)
	{
		for (j = 0; j < NB_COLONNES; j++)
		{
			jeu[i * NB_COLONNES + j].couleur = 0;
			jeu[i * NB_COLONNES + j].valeur = 0;
		}
	}
	return jeu;
}

//Fonction min trouve le minimum des noeuds fils
int f_min(Pion *noeud, int profondeur, int joueur)
{
	Pion *tempNoeud = f_init_plateau();
	f_copie_plateau(noeud, tempNoeud);

	int value = INFINI;
	if (profondeur == 0)
	{
		return f_eval(noeud, joueur);
	}
	else
	{
		Liste list = genererFils(noeud, joueur);
		while (list != NULL)
		{
			int oldvalue = value;
			value = min(value, f_max(list->plateau, profondeur - 1, -joueur));
			if (value < oldvalue)
			{
				f_copie_plateau(list->plateau, tempNoeud);
			}
			list = list->suivant;
		}
		f_copie_plateau(tempNoeud, plateauAJouer);

		return value;
	}
}

int f_min_stats(Pion *noeud, int profondeur, int joueur)
{
	nbAppelsMinMAX++;

	Pion *tempNoeud = f_init_plateau();
	f_copie_plateau(noeud, tempNoeud);

	int value = INFINI;
	if (profondeur == 0)
	{
		return f_eval(noeud, joueur);
	}
	else
	{
		Liste list = genererFils(noeud, joueur);
		Liste p = list;
		while (list != NULL)
		{
			nbFilsAnalyses++;
			int oldvalue = value;
			value = min(value, f_max_stats(list->plateau, profondeur - 1, -joueur));
			if (value < oldvalue)
			{
				f_copie_plateau(list->plateau, tempNoeud);
			}
			list = list->suivant;
		}
		f_copie_plateau(tempNoeud, plateauAJouer);
		free(tempNoeud);
		freeList(p);
		return value;
	}
}

int f_min_stats_elagage(Pion *noeud, int profondeur, int joueur, int alpha, int beta)
{
	nbAppelsMinMAX++;
	Pion *tempNoeud = f_init_plateau();
	f_copie_plateau(noeud, tempNoeud);
	int coupure = 0;
	int value = INFINI;
	if (profondeur == 0)
	{
		return f_eval(noeud, joueur);
	}
	else
	{
		Liste list = genererFils(noeud, joueur);
		Liste p = list;
		while (list != NULL && !coupure)
		{
			nbFilsAnalyses++;
			int oldvalue = value;
			value = min(value, f_max_stats_elagage(list->plateau, profondeur - 1, -joueur, alpha, beta));
			if (oldvalue != value)
			{
				f_copie_plateau(list->plateau, tempNoeud);
			}
			if (alpha >= beta) // coupure alpha
			{
				nbCoupures++;
				coupure = 1;
			}
			if (!coupure)
			{
				beta = min(beta, value);
			}
			list = list->suivant;
		}
		f_copie_plateau(tempNoeud, plateauAJouer);
		freeList(p);
		return value;
	}
}

//Fonction max trouve le maximum des noeuds fils
int f_max(Pion *noeud, int profondeur, int joueur)
{
	int value = -INFINI;
	Pion *tempNoeud = f_init_plateau();
	f_copie_plateau(noeud, tempNoeud);

	if (profondeur == 0)
	{
		return f_eval(noeud, joueur);
	}
	else
	{
		Liste list = genererFils(noeud, joueur);
		while (list != NULL)
		{
			int oldvalue = value;
			value = max(value, f_min(list->plateau, profondeur - 1, -joueur));
			if (value > oldvalue)
			{
				f_copie_plateau(list->plateau, tempNoeud);
			}
			list = list->suivant;
		}

		f_copie_plateau(tempNoeud, plateauAJouer);
		return value;
	}
}

//Fonction max trouve le maximum des noeuds fils
int f_max_stats(Pion *noeud, int profondeur, int joueur)
{
	nbAppelsMinMAX++;
	int value = -INFINI;
	Pion *tempNoeud = f_init_plateau();
	f_copie_plateau(noeud, tempNoeud);

	if (profondeur == 0)
	{
		return f_eval(noeud, joueur);
	}
	else
	{

		Liste list = genererFils(noeud, joueur);
		Liste p = list;
		while (list != NULL)
		{
			nbFilsAnalyses++;
			int oldvalue = value;
			value = max(value, f_min_stats(list->plateau, profondeur - 1, -joueur));
			if (value > oldvalue)
			{
				f_copie_plateau(list->plateau, tempNoeud);
			}
			list = list->suivant;
		}

		f_copie_plateau(tempNoeud, plateauAJouer);
		free(tempNoeud);
		freeList(p);
		return value;
	}
}

int f_max_stats_elagage(Pion *noeud, int profondeur, int joueur, int alpha, int beta)
{
	nbAppelsMinMAX++;
	int value = -INFINI;
	Pion *tempNoeud = f_init_plateau();
	f_copie_plateau(noeud, tempNoeud);
	int coupure = 0;
	if (profondeur == 0)
	{
		return f_eval(noeud, joueur);
	}
	else
	{
		Liste list = genererFils(noeud, joueur);
		Liste p = list;
		while (list != NULL && !coupure)
		{
			nbFilsAnalyses++;
			int oldvalue = value;
			value = max(value, f_min_stats_elagage(list->plateau, profondeur - 1, -joueur, alpha, beta));
			if (oldvalue != value)
			{
				f_copie_plateau(list->plateau, tempNoeud);
			}
			if (alpha >= beta) // coupure beta
			{
				nbCoupures++;
				coupure = 1;
			}
			if (!coupure)
			{
				alpha = max(alpha, value);
			}
			list = list->suivant;
		}

		f_copie_plateau(tempNoeud, plateauAJouer);
		freeList(p);
		free(tempNoeud);
		return value;
	}
}

Liste genererFils(Pion *noeud, int joueur)
{
	// allocation de la liste de plateau
	Liste list = NULL;

	// parcours complet du plateau
	for (int i = 0; i < NB_LIGNES; i++)
	{
		for (int j = 0; j < NB_COLONNES; j++)
		{

			// Test si le pion au point i,j nous appartient
			Pion p = noeud[i * NB_LIGNES + j];
			// printf("ui");
			if (p.couleur == joueur)
			{
				// generer liste de mouvements
				int deplAbs[8] = {i + 1, i + 1, i, i - 1, i - 1, i - 1, i, i + 1};
				int deplOrd[8] = {j, j + 1, j + 1, j + 1, j, j - 1, j - 1, j - 1};
				// tester tous les mouvements
				for (int a = 0; a < 8; a++)
				{
					if (!f_test_mouvement(noeud, i, j, deplAbs[a], deplOrd[a], joueur))
					{
						// on ajoute le tableau a la liste de tableau
						Pion *plateau = f_init_plateau();

						f_copie_plateau(noeud, plateau);

						//f_affiche_plateau(plateau);
						f_bouge_piece(plateau, i, j, deplAbs[a], deplOrd[a], joueur);

						Liste elt = NULL;
						elt = malloc(sizeof(Liste));

						elt->plateau = plateau;
						elt->suivant = list;
						list = elt;

					}
				}
			}
		}
	}

	return list;
}

void freeList(Liste list)
{
	while (list != NULL)
	{
		Liste p = list;
		list = list->suivant;
		free(p->plateau);
		free(p);
	}
}
/**
 * Return minimum of two values
 */
int min(int val, int resultat)
{
	if (val < resultat)
	{
		return val;
	}
	return resultat;
}

/**
 * Return maximum of two values
 */
int max(int val, int resultat)
{
	if (val > resultat)
	{
		return val;
	}
	return resultat;
}

void initStats()
{
	nbToursIA = 0;
	nbAppelsMinMAX = 0;
	nbCoupures = 0;
	nbToursIA = 0;
	nbFilsAnalyses = 0;
	totalTpsIA = 0;
}

void afficheStats()
{
	printf(" -------------- STATS DE L'IA -------------- \n");
	printf("\t Nombre de tours de l'IA : %d\n", nbToursIA);
	printf("\t Nombre d'appels de fmin et fmax: %d\n", nbAppelsMinMAX);
	printf("\t Nombre de fils analyses: %d\n", nbFilsAnalyses);
	printf("\t Nombre de coupures : %d\n\n", nbCoupures);

	moyenneNbCoupures = nbCoupures / nbToursIA;
	moyenneFilsAnalyses = nbFilsAnalyses / nbToursIA;
	vMoyExecIA = totalTpsIA / nbToursIA;

	printf("\t Nombre moyen de coupures : %f\n", moyenneNbCoupures);
	printf("\t Nombre moyen de fils analyses : %f\n", moyenneFilsAnalyses);
	printf("\t Vitesse moyenne d'execution d'un tour d'IA : %f\n", vMoyExecIA);
}

void getTime(int a)
{
	if (a == 0)
	{
		gettimeofday(&time1, NULL);
	}
	else if (a == 1)
	{
		gettimeofday(&time2, NULL);
	}
}

float getSec()
{
	return time2.tv_sec - time1.tv_sec;
}

/**
 * Calcule et joue le meilleur cout
 * */
void f_IA(int joueur)
{
#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
	printf("joueur ");
	switch (joueur)
	{
	case -1:
		printf("o ");
		printf("joue:\n");
		break;
	case 1:
		printf("x ");
		printf("joue:\n");
		break;
	default:
		printf("inconnu ");
	}

	getTime(0);

	//f_max(plateauDeJeu, PROFONDEUR_MAX, joueur);
	//f_max_stats(plateauDeJeu, PROFONDEUR_MAX, joueur);
	int retval = f_max_stats_elagage(plateauDeJeu, PROFONDEUR_MAX, joueur, -INFINI, INFINI);
	printf("joueur %d -> %d\n", joueur, retval);
	f_copie_plateau(plateauAJouer, plateauDeJeu);

	getTime(1);
	totalTpsIA += getSec();
	nbToursIA++;

#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

/**
 * Demande le choix du joueur humain et calcule le coup demande
 * */
void f_humain(int joueur)
{
	char c1, c2;
	char buffer[32];
	int l1, l2;

#ifdef DEBUG
	printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

	printf("joueur ");
	switch (joueur)
	{
	case -1:
		printf("o ");
		break;
	case 1:
		printf("x ");
		break;
	default:
		printf("inconnu ");
	}
	printf("joue:\n");
	while (1)
	{
		fgets(buffer, 32, stdin);
		if (sscanf(buffer, "%c%i%c%i\n", &c1, &l1, &c2, &l2) == 4)
		{
			if (f_bouge_piece(plateauDeJeu, l1, f_convert_char2int(c1), l2, f_convert_char2int(c2), joueur) == 0)
				break;
		}
		fflush(stdin);
		printf("mauvais choix\n");
	}

#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

int main(int argv, char *argc[])
{
	int fin = 0, mode = 0, ret, joueur = 1;
	int nbtours = 0;
	srand(time(NULL));
	printf("1 humain vs IA\n2 humain vs humain\n3 IA vs IA\n");
	scanf("%d", &mode);
	initStats();
	plateauAJouer = f_init_plateau();
	plateauDeJeu = f_init_plateau();
	while (!fin)
	{
		f_affiche_plateau(plateauDeJeu);
		if (mode == 1)
		{
			if (joueur > 0)
				f_humain(joueur);
			else
				f_IA(joueur);
		}
		else if (mode == 2)
		{
			f_humain(joueur);
		}
		else
		{
			printf("%d", joueur);
			f_IA(joueur);
			char choice;
			nbtours++;
		}

		if ((ret = f_gagnant(plateauDeJeu)) != 0)
		{
			switch (ret)
			{
			case 1:
				f_affiche_plateau(plateauDeJeu);
				printf("joueur x gagne!\n");
				fin = 1;
				break;
			case -1:
				f_affiche_plateau(plateauDeJeu);
				printf("joueur o gagne!\n");
				fin = 1;
				break;
			}
		}
		joueur = -joueur;
	}
	afficheStats();
#ifdef DEBUG
	printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif

	return 0;
}
