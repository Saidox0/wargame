#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>           // Nouveau include pour srand() et time()
#include <sys/time.h>       // Nouveau include pour temps d'exécution de l'IA

#define NB_LIGNES 10
#define NB_COLONNES 10
#define INFINI 1000000
#define PROFONDEUR_MAX 3    // Profondeur fix 3

/*#define DEBUG*/

typedef struct pion_s
{
    int couleur;
    int valeur;
}Pion;

Pion *plateauDeJeu;

// Variable globale pour l'interrupteur Alpha-Beta
int g_use_alpha_beta = 1; // 1 par défaut (Alpha-Beta actif)

// Statistiques algorithme Minimax
// Analyse performances
typedef struct stats_s {
    long nbAppelsMinMax;        // Nombre d'appels à f_min et f_max
    long nbFilsAnalysesTotal;   // Nombre de nœuds explorés dans l'arbre
    long nbCoupures;            // Nombre de coupures alpha-beta effectuées
    double totalTpsIA;          // Temps d'exécution de l'IA
    int nbToursIA;              // Nombre de tours joués par l'IA
} Stats;

Stats g_stats = {0, 0, 0, 0.0, 0}; // Variable globale pour stocker les stats

// Oordonnancement des coups (Move Ordering)
// Trie les coups par pertinence avant de les explorer
// Améliore l'efficacité de l'élagage alpha-beta
typedef struct move_s {
    int l1, c1;         // Position de départ (ligne, colonne)
    int l2, c2;         // Position d'arrivée (ligne, colonne)
    int order_score;    // Score heuristique pour tri
} Move;

// Fonctions pour gestion du temps et statistiques
void afficheStats();
void getTime(struct timeval *tv);
double getSec(struct timeval *tv1, struct timeval *tv2);

void f_affiche_plateau(Pion *plateau);
int f_convert_char2int(char c);
char f_convert_int2char(int i);
int f_gagnant(Pion* plateau);           //  Plateau en paramètre (pour f min f max)
int f_bataille(Pion* plateau, int l, int c); // Plateau en paramètre
int f_test_mouvement(Pion *plateau, int l1, int c1, int l2, int c2, int couleur);
int f_bouge_piece(Pion *plateau, int l1, int c1, int l2, int c2, int couleur);
int f_nbPions(Pion* jeu, int joueur);
int f_valeur(Pion* jeu, int joueur);
void f_copie_plateau(Pion* source, Pion* destination);
Pion* f_raz_plateau();
int f_eval(Pion* jeu, int joueur);

// Déclaration fonctions minimax avec alpha-beta
int f_min(Pion* jeu, int profondeur, int joueur, int alpha, int beta);
int f_max(Pion* jeu, int profondeur, int joueur, int alpha, int beta);

// Fonction ordonnancement coups
// Calcule un score heuristique pour chercher les coups les plus prometteurs
// Explore les meilleurs coups pour maximiser les coupures alpha-beta
int f_get_order_score(Pion* plateau, int l1, int c1, int l2, int c2, int joueur) {
    int score = 0;
    
    // PRIORITÉ 1: Victoire immédiate (atteindre la ligne arrivée)
    if ((joueur == 1 && l2 == 0) || (joueur == -1 && l2 == NB_LIGNES - 1)) {
        return 1000; // Score maximal pour une victoire
    }
    
    // PRIORITÉ 2: Captures (attaquer un pion ennemi)
    if (plateau[l2 * NB_COLONNES + c2].couleur == -joueur) {
        Pion* copie = f_raz_plateau();
        f_copie_plateau(plateau, copie);
        f_bouge_piece(copie, l1, c1, l2, c2, joueur);
        
        // Bonus si on gagne la bataille
        if (copie[l2 * NB_COLONNES + c2].couleur == joueur) {
             score += 500; // Capture réussie
        } else {
             score += 100; // Attaque (même si on perd)
        }
        free(copie);
    }
    
    // PRIORITÉ 3: Avancement vers la ligne adverse
    if ((joueur == 1 && l2 < l1) || (joueur == -1 && l2 > l1)) {
        score += 20 + abs(l1 - l2) * 5; 
    }
    
    // PRIORITÉ 4: Contrôle du centre du plateau
    if (l2 >= 3 && l2 <= 6 && c2 >= 2 && c2 <= 6) {
        score += 5;
    }
    
    return score;
}

// Fonction de comparaison qsort()
// Trie les coups par ordre décroissant de score
int compareMoves(const void *a, const void *b) {
    const Move *moveA = (const Move *)a;
    const Move *moveB = (const Move *)b;
    return moveB->order_score - moveA->order_score;
}


int f_convert_char2int(char c)
{
#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    if(c>='A' && c<='Z')
        return (int)(c-'A');
    if(c>='a' && c<='z')
        return (int)(c-'a');
    return -1;
}

char f_convert_int2char(int i)
{
#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    return (char)i+'A';
}

Pion *f_init_plateau()
{
    int i, j;
    Pion *plateau=NULL;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    plateau = (Pion *)malloc(NB_LIGNES*NB_COLONNES*sizeof(Pion));
    if(plateau == NULL)
    {
        printf("error: unable to allocate memory\n");
        exit(EXIT_FAILURE);
    }

    for(i=0; i<NB_LIGNES; i++)
    {
        for(j=0; j<NB_COLONNES; j++)
        {
            plateau[i*NB_COLONNES+j].couleur = 0;
            plateau[i*NB_COLONNES+j].valeur = 0;
        }
    }

    /* les pieces de l'ordinateur (joueur 1) 'x' */
    plateau[9*NB_COLONNES+5].couleur = 1;
    plateau[9*NB_COLONNES+5].valeur = 1;

    plateau[9*NB_COLONNES+6].couleur = 1;
    plateau[9*NB_COLONNES+6].valeur = 2;

    plateau[9*NB_COLONNES+7].couleur = 1;
    plateau[9*NB_COLONNES+7].valeur = 3;

    plateau[9*NB_COLONNES+8].couleur = 1;
    plateau[9*NB_COLONNES+8].valeur = 2;

    plateau[9*NB_COLONNES+9].couleur = 1;
    plateau[9*NB_COLONNES+9].valeur = 1;

    plateau[8*NB_COLONNES+0].couleur = 1;
    plateau[8*NB_COLONNES+0].valeur = 1;

    plateau[8*NB_COLONNES+1].couleur = 1;
    plateau[8*NB_COLONNES+1].valeur = 3;

    plateau[8*NB_COLONNES+2].couleur = 1;
    plateau[8*NB_COLONNES+2].valeur = 3;

    plateau[8*NB_COLONNES+3].couleur = 1;
    plateau[8*NB_COLONNES+3].valeur = 1;

    plateau[8*NB_COLONNES+6].couleur = 1;
    plateau[8*NB_COLONNES+6].valeur = 1;

    plateau[8*NB_COLONNES+7].couleur = 1;
    plateau[8*NB_COLONNES+7].valeur = 1;

    plateau[8*NB_COLONNES+8].couleur = 1;
    plateau[8*NB_COLONNES+8].valeur = 1;

    plateau[7*NB_COLONNES+1].couleur = 1;
    plateau[7*NB_COLONNES+1].valeur = 1;

    plateau[7*NB_COLONNES+2].couleur = 1;
    plateau[7*NB_COLONNES+2].valeur = 1;

    /*les pieces de l'ennemi (joueur -1) 'o' */
    plateau[2*NB_COLONNES+7].couleur = -1;
    plateau[2*NB_COLONNES+7].valeur = 1;

    plateau[2*NB_COLONNES+8].couleur = -1;
    plateau[2*NB_COLONNES+8].valeur = 1;

    plateau[1*NB_COLONNES+1].couleur = -1;
    plateau[1*NB_COLONNES+1].valeur = 1;

    plateau[1*NB_COLONNES+2].couleur = -1;
    plateau[1*NB_COLONNES+2].valeur = 1;

    plateau[1*NB_COLONNES+3].couleur = -1;
    plateau[1*NB_COLONNES+3].valeur = 1;

    plateau[1*NB_COLONNES+6].couleur = -1;
    plateau[1*NB_COLONNES+6].valeur = 1;

    plateau[1*NB_COLONNES+7].couleur = -1;
    plateau[1*NB_COLONNES+7].valeur = 3;

    plateau[1*NB_COLONNES+8].couleur = -1;
    plateau[1*NB_COLONNES+8].valeur = 3;

    plateau[1*NB_COLONNES+9].couleur = -1;
    plateau[1*NB_COLONNES+9].valeur = 1;

    plateau[0*NB_COLONNES+0].couleur = -1;
    plateau[0*NB_COLONNES+0].valeur = 1;

    plateau[0*NB_COLONNES+1].couleur = -1;
    plateau[0*NB_COLONNES+1].valeur = 2;

    plateau[0*NB_COLONNES+2].couleur = -1;
    plateau[0*NB_COLONNES+2].valeur = 3;

    plateau[0*NB_COLONNES+3].couleur = -1;
    plateau[0*NB_COLONNES+3].valeur = 2;

    plateau[0*NB_COLONNES+4].couleur = -1;
    plateau[0*NB_COLONNES+4].valeur = 1;

#ifdef DEBUG
printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif

return plateau;
}

void f_affiche_plateau(Pion *plateau)
{
    int i,j,k;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    printf("\n    ");
    for(k=0; k<NB_COLONNES; k++)
        printf("%2c ",f_convert_int2char(k));
    printf("\n    ");
    for(k=0; k<NB_COLONNES; k++)
        printf("-- ");
    printf("\n");
    for(i=NB_LIGNES-1; i>=0; i--)
    {
        printf("%2d ", i);
        for(j=0; j<NB_COLONNES; j++)
        {
            printf("|");
            switch(plateau[i*NB_COLONNES+j].couleur)
            {
            case -1:
                printf("%do",plateau[i*NB_COLONNES+j].valeur);
                break;
            case 1:
                printf("%dx",plateau[i*NB_COLONNES+j].valeur);
                break;
            default:
                printf("  ");
            }
        }
        printf("|\n    ");
        for(k=0; k<NB_COLONNES; k++)
            printf("-- ");
        printf("\n");
    }
    printf("    ");

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

// Prend le plateau en paramètre au lieu d'utiliser la variable globale
// Permet de tester différents plateaux dans minimax
int f_gagnant(Pion* plateau)
{
    int i, j, somme1=0, somme2=0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    for(i=0; i<NB_COLONNES; i++)
    {
        if(plateau[i].couleur == 1)                               return  1; 
        if(plateau[(NB_LIGNES-1)*NB_COLONNES+i].couleur == -1)    return -1;
    }

    for(i=0; i<NB_LIGNES; i++)
    {
        for(j=0; j<NB_COLONNES; j++)
        {
            if(plateau[i*NB_COLONNES+j].couleur == 1)      somme1++;
            if(plateau[i*NB_COLONNES+j].couleur == -1)     somme2++;
        }
    }
    if(somme1==0)  return -1; 
    if(somme2==0)  return  1;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

// Prend le plateau en paramètre au lieu d'utiliser la variable globale
// Pour simuler les batailles sur copies du plateau
int f_bataille(Pion* plateau, int l, int c)
{
    int i, j, mini, maxi, minj, maxj;
    int somme=0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    mini = l-1<0?0:l-1;
    maxi = l+1>NB_LIGNES-1?NB_LIGNES-1:l+1;
    minj = c-1<0?0:c-1;
    maxj = c+1>NB_COLONNES-1?NB_COLONNES-1:c+1;

    for(i=mini; i<=maxi; i++)
    {
        for(j=minj; j<=maxj; j++)
        {
            somme += plateau[i*NB_COLONNES+j].couleur * plateau[i*NB_COLONNES+j].valeur;
        }
    }
    somme -= plateau[l*NB_COLONNES+c].couleur * plateau[l*NB_COLONNES+c].valeur;

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif

    if(somme < 0)   return -1;
    if(somme > 0)   return 1;

    return -plateau[l*NB_COLONNES+c].couleur;
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
    if(l1 < 0 || l1 >= NB_LIGNES || l2 < 0 || l2 >= NB_LIGNES || c1 < 0 || c1 >= NB_COLONNES || c2 < 0 || c2 >= NB_COLONNES)    return 1;
    /* Erreur, il n'y a pas de pion a deplacer ou le pion n'appartient pas au joueur*/
    if(plateau[l1*NB_COLONNES+c1].valeur == 0 || plateau[l1*NB_COLONNES+c1].couleur != couleur)                                 return 1;
    /* Erreur, tentative de tir fratricide */
    if(plateau[l2*NB_COLONNES+c2].couleur == plateau[l1*NB_COLONNES+c1].couleur)                                                return 1;

    if(abs(l1-l2) > 1 || abs(c1-c2) > 1 || (l1==l2 && c1==c2))                                                  return 1;
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
    int gagnant=0;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    if(f_test_mouvement(plateau, l1, c1, l2, c2, couleur) != 0)
    {
        return 1;
    }


    /* Cas ou il n'y a personne a l'arrivee */
    if(plateau[l2*NB_COLONNES+c2].valeur == 0)
    {
        plateau[l2*NB_COLONNES+c2].couleur = plateau[l1*NB_COLONNES+c1].couleur;
        plateau[l2*NB_COLONNES+c2].valeur = plateau[l1*NB_COLONNES+c1].valeur;
        plateau[l1*NB_COLONNES+c1].couleur = 0;
        plateau[l1*NB_COLONNES+c1].valeur = 0;
    }
    else
    {
        gagnant = f_bataille(plateau, l2, c2); 
        /* victoire */
        if(gagnant == couleur)
        {
            plateau[l2*NB_COLONNES+c2].couleur = plateau[l1*NB_COLONNES+c1].couleur;
            plateau[l2*NB_COLONNES+c2].valeur = plateau[l1*NB_COLONNES+c1].valeur;
            plateau[l1*NB_COLONNES+c1].couleur = 0;
            plateau[l1*NB_COLONNES+c1].valeur = 0;
        }
        /* defaite */
        else if(gagnant != 0)
        {
            plateau[l1*NB_COLONNES+c1].couleur = 0;
            plateau[l1*NB_COLONNES+c1].valeur = 0;
        }
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
    return 0;
}

//Calcul du nombre de pions sur le plateau du joueur
int f_nbPions(Pion* jeu, int joueur)
{
    int nbPion=0;
    int i, j;
    for (i = 0; i < NB_LIGNES; ++i)
    {
        for (j = 0; j < NB_COLONNES; ++j)
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
int f_valeur(Pion* jeu, int joueur)
{
    int i, j;
    int valeur=0;
    for (i = 0; i < NB_LIGNES; ++i)
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

void f_copie_plateau(Pion* source, Pion* destination)
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
Pion* f_raz_plateau()
{
    Pion* jeu = NULL;
    int i, j;
    jeu = (Pion *) malloc(NB_LIGNES * NB_COLONNES * sizeof (Pion));
    if(jeu == NULL) { // Vérification d'erreur d'allocation
        printf("Erreur d'allocation mémoire dans f_raz_plateau\n");
        exit(EXIT_FAILURE);
    }
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

// Fonction d'évaluation heuristique
// Evalue à quel point une position est favorable
// Plusieurs critères sont combinés avec des coefficients
int f_eval(Pion* jeu, int joueur)
{
    int i, j;
    int score = 0;
    
    // CRITÈRE 1: Victoire/Défaite (priorité absolue)
    int gagnant = f_gagnant(jeu);
    if(gagnant == joueur) return INFINI;    // Victoire = score maximal
    if(gagnant == -joueur) return -INFINI;  // Défaite = score minimal
   
    // Comptage des pions pour adapter la stratégie
    int nb_pions_allies = f_nbPions(jeu, joueur);
    int nb_pions_ennemis = f_nbPions(jeu, -joueur);
    
    // STRATÉGIE ADAPTATIVE: En fin de partie, privilégier l'avancement
    int mode_poussee_finale = (nb_pions_ennemis <= 5) && (nb_pions_allies >= 5);
    int facteur_avancement = mode_poussee_finale ? 30 : 10; // x3 en fin de partie
    int bonus_pre_victoire = 1000; // Gros bonus pour être près de la victoire
    int bonus_centre = 5;

    // CRITÈRE 2: Valeur totale des pions 
    score += f_valeur(jeu, joueur) * 15;
    score -= f_valeur(jeu, -joueur) * 15;

    // CRITÈRE 3: Nombre de pions (quantité d'unités)
    score += nb_pions_allies * 5;
    score -= nb_pions_ennemis * 5;

    // CRITÈRE 4: Positionnement tactique
    for(i = 0; i < NB_LIGNES; i++)
    {
        for(j = 0; j < NB_COLONNES; j++)
        {
            if(jeu[i * NB_COLONNES + j].couleur == joueur)
            {
                // Bonus avancement vers la ligne adverse
                if(joueur == 1) // Joueur x (va vers ligne 0)
                {
                    score += (NB_LIGNES - 1 - i) * facteur_avancement;
                    if (i == 1) score += bonus_pre_victoire; // Avant dernière case de la victoire
                }
                else // Joueur o (va vers ligne 9)
                {
                    score += i * facteur_avancement;
                    if (i == 8) score += bonus_pre_victoire; // Avant dernière 1 case de la victoire
                }
                
                // Bonus pour contrôle du centre (lignes 3-6, colonnes C-G)
                if (i >= 3 && i <= 6 && j >= 2 && j <= 6)
                {
                    score += bonus_centre;
                }

                // En fin de partie: bonus pour les pions faibles bien placés
                if (mode_poussee_finale && jeu[i * NB_COLONNES + j].valeur == 1) 
                {
                    if (joueur == 1 && i >= 8) score += 20; // x en lignes 8-9
                    if (joueur == -1 && i <= 1) score += 20; // o en lignes 0-1
                }
            }
            else if(jeu[i * NB_COLONNES + j].couleur == -joueur)
            {
                // Malus pour l'avancement ennemi
                if(joueur == 1)
                {
                    score -= i * facteur_avancement;
                    if (i == 8) score -= bonus_pre_victoire;
                }
                else
                {
                    score -= (NB_LIGNES - 1 - i) * facteur_avancement;
                    if (i == 1) score -= bonus_pre_victoire;
                }
                
                // Malus si l'ennemi contrôle le centre
                if (i >= 3 && i <= 6 && j >= 2 && j <= 6)
                {
                    score -= bonus_centre;
                }
            }
        }
    }
    
    return score;
}

// Fonction MIN de l'algorithme Minimax avec Alpha-Beta
// Simule les coups de l'adversaire (minimise le score)
// Alpha-Beta: évite d'explorer des branches qui ne changeront pas le résultat
int f_min(Pion* jeu, int profondeur, int joueur, int alpha, int beta)
{
    int i, j, k, l, m;
    Pion* copie;
    int valeur;
    
    g_stats.nbAppelsMinMax++; // STATISTIQUE: Comptage des appels

    // Condition d'arrêt: profondeur max ou fin de partie
    if(profondeur == 0 || f_gagnant(jeu) != 0)
        return f_eval(jeu, joueur);
    
    // ORDONNANCEMENT DES COUPS (OPTIMISATION) 
    // 1. Génération de tous les coups possibles pour l'adversaire
    Move *moves = (Move*)malloc(256 * sizeof(Move)); // Allocation dynamique
   int num_moves = 0;

    for(i = 0; i < NB_LIGNES; i++) {
        for(j = 0; j < NB_COLONNES; j++) {
            if(jeu[i * NB_COLONNES + j].couleur == -joueur) {
                // Pour chaque pion adverse, tester les 8 directions
                for(k = -1; k <= 1; k++) {
                    for(l = -1; l <= 1; l++) {
                        if(f_test_mouvement(jeu, i, j, i+k, j+l, -joueur) == 0) {
                            moves[num_moves].l1 = i;
                            moves[num_moves].c1 = j;
                            moves[num_moves].l2 = i + k;
                            moves[num_moves].c2 = j + l;
                            // 2. Calcul du score d'ordonnancement
                            moves[num_moves].order_score = f_get_order_score(jeu, i, j, i+k, j+l, -joueur);
                            num_moves++;
                        }
                    }
                }
            }
        }
    }
    
    // 3. Tri des coups par score décroissant 
    qsort(moves, num_moves, sizeof(Move), compareMoves);

    // 4. Exploration des coups triés
    for (m = 0; m < num_moves; m++) {
        int i_start = moves[m].l1;
        int j_start = moves[m].c1;
        int i_end = moves[m].l2;
        int j_end = moves[m].c2;
        
        // Simulation du coup sur une copie du plateau
        copie = f_raz_plateau();
        f_copie_plateau(jeu, copie);
        
        f_bouge_piece(copie, i_start, j_start, i_end, j_end, -joueur);

        g_stats.nbFilsAnalysesTotal++; // STATISTIQUE: Comptage des nœuds explorés
        
        // Appel récursif à MAX pour le joueur
        valeur = f_max(copie, profondeur - 1, joueur, alpha, beta);
        
        // Mise à jour de beta (le MIN cherche à minimiser)
        if (valeur < beta)
            beta = valeur;
        
        free(copie);
        
        // COUPURE ALPHA-BETA: Si beta <= alpha, pas besoin d'explorer plus (si activé ajout de choix)
        if(g_use_alpha_beta && beta <= alpha)
        {
            g_stats.nbCoupures++; // STATISTIQUE: Comptage des coupures
            free(moves);
            return alpha; 
        }
    }
    
    free(moves);
    return beta;
}

// Fonction MAX de l'algorithme Minimax avec Alpha-Beta
// Simule les coups du joueur (maximise le score)
// Identique à f_min mais pour le joueur au lieu de l'adversaire
int f_max(Pion* jeu, int profondeur, int joueur, int alpha, int beta)
{
    int i, j, k, l, m;
    Pion* copie;
    int valeur;
    
    g_stats.nbAppelsMinMax++; // STATISTIQUE: Comptage des appels

    // Condition d'arrêt: profondeur max ou fin de partie
    if(profondeur == 0 || f_gagnant(jeu) != 0)
        return f_eval(jeu, joueur);
    
    // ORDONNANCEMENT DES COUPS (OPTIMISATION)
    // 1. Génération de tous les coups possibles pour le joueur
    Move *moves = (Move*)malloc(256 * sizeof(Move)); // Allocation dynamique
    int num_moves = 0;

    for(i = 0; i < NB_LIGNES; i++) {
        for(j = 0; j < NB_COLONNES; j++) {
            if(jeu[i * NB_COLONNES + j].couleur == joueur) {
                // Pour chaque pion du joueur, tester les 8 directions
                for(k = -1; k <= 1; k++) {
                    for(l = -1; l <= 1; l++) {
                        if(f_test_mouvement(jeu, i, j, i+k, j+l, joueur) == 0) {
                            moves[num_moves].l1 = i;
                            moves[num_moves].c1 = j;
                            moves[num_moves].l2 = i + k;
                            moves[num_moves].c2 = j + l;
                            // 2. Calcul du score d'ordonnancement
                            moves[num_moves].order_score = f_get_order_score(jeu, i, j, i+k, j+l, joueur);
                            num_moves++;
                        }
                    }
                }
            }
        }
    }
    
    // 3. Tri des coups par score décroissant
    qsort(moves, num_moves, sizeof(Move), compareMoves);

    // 4. Exploration des coups triés
    for (m = 0; m < num_moves; m++) {
        int i_start = moves[m].l1;
        int j_start = moves[m].c1;
        int i_end = moves[m].l2;
        int j_end = moves[m].c2;
        
        // Simulation du coup sur une copie du plateau
        copie = f_raz_plateau();
        f_copie_plateau(jeu, copie);
        
        f_bouge_piece(copie, i_start, j_start, i_end, j_end, joueur);

        g_stats.nbFilsAnalysesTotal++; // STATISTIQUE: Comptage des nœuds explorés
        
        // Appel récursif à MIN pour l'adversaire
        valeur = f_min(copie, profondeur - 1, joueur, alpha, beta);
        
        // Mise à jour d'alpha (le MAX cherche à maximiser)
        if (valeur > alpha)
            alpha = valeur;
        
        free(copie);
        
        // COUPURE ALPHA-BETA: Si beta <= alpha, pas besoin d'explorer plus (si activé ajout de choix)
        if(g_use_alpha_beta && alpha >= beta)
        {
            g_stats.nbCoupures++; // STATISTIQUE: Comptage des coupures
            free(moves);
            return beta;
        }
    }
    
    free(moves);
    return alpha;
}

// Fonction IA avec Minimax, statistiques et aléatoire
// C'est ici que l'IA choisit le "meilleur coup"
void f_IA(int joueur)
{
    int i, j, k, l, m;
    int meilleur_score = -INFINI - 1;
    int score;
    int meilleur_i = -1, meilleur_j = -1, meilleur_k = -1, meilleur_l = -1;
    Pion* copie;
    struct timeval start, end;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif
    
    // STATISTIQUES
    long fils_analyses_avant_tour = g_stats.nbFilsAnalysesTotal;
    long coupures_avant_tour = g_stats.nbCoupures;
    
    g_stats.nbToursIA++; // nombre de tours joués
    getTime(&start); // chronomètre
    printf("IA joue pour le joueur %c...\n", (joueur == 1) ? 'x' : 'o');

    // ORDONNANCEMENT DES COUPS AU NIVEAU RACINE
    // 1. Génération de tous les coups possibles
    Move *moves = (Move*)malloc(256 * sizeof(Move));
    int num_moves = 0;

    for(i = 0; i < NB_LIGNES; i++) {
        for(j = 0; j < NB_COLONNES; j++) {
            if(plateauDeJeu[i * NB_COLONNES + j].couleur == joueur) {
                for(k = -1; k <= 1; k++) {
                    for(l = -1; l <= 1; l++) {
                        if(f_test_mouvement(plateauDeJeu, i, j, i+k, j+l, joueur) == 0) {
                            moves[num_moves].l1 = i;
                            moves[num_moves].c1 = j;
                            moves[num_moves].l2 = i + k;
                            moves[num_moves].c2 = j + l;
                            // 2. Calcul du score d'ordonnancement
                            moves[num_moves].order_score = f_get_order_score(plateauDeJeu, i, j, i+k, j+l, joueur);
                            num_moves++;
                        }
                    }
                }
            }
        }
    }
    
    // 3. Tri des coups (explore les meilleurs en premier)
    qsort(moves, num_moves, sizeof(Move), compareMoves);
    
    // 4. Évaluation de chaque coup trié avec Minimax
    for(m = 0; m < num_moves; m++)
    {
        int i_start = moves[m].l1;
        int j_start = moves[m].c1;
        int i_end = moves[m].l2;
        int j_end = moves[m].c2;
        
        // Simulation du coup
        copie = f_raz_plateau();
        f_copie_plateau(plateauDeJeu, copie);
        
        if(f_bouge_piece(copie, i_start, j_start, i_end, j_end, joueur) == 0)
        {
            // Appel à MIN car c'est le tour de l'adversaire après
            score = f_min(copie, PROFONDEUR_MAX - 1, joueur, -INFINI, INFINI); 

            // Mouvement aléatoire 5 premiers tours pour varier les ouvertures
            // Évite que toutes les parties IA vs IA soient identiques
            if (g_stats.nbToursIA <= 5)
            {
                score += (rand() % 51) - 25; // Ajout de -25 à +25 au score
            }
            
            // Mémorisation du meilleur coup
            if(score > meilleur_score)
            {
                meilleur_score = score;
                meilleur_i = i_start;
                meilleur_j = j_start;
                meilleur_k = i_end;
                meilleur_l = j_end;
            }
        }
        free(copie);
    }
    
    free(moves);
    
    // Mesure du temps d'exécution
    getTime(&end);
    g_stats.totalTpsIA += getSec(&start, &end);
    
    // Exécution du meilleur coup trouvé
    if (meilleur_i != -1)
    {
        f_bouge_piece(plateauDeJeu, meilleur_i, meilleur_j, meilleur_k, meilleur_l, joueur);
        printf("IA deplace de %c%d vers %c%d (Score Minimax: %d)\n", 
               f_convert_int2char(meilleur_j), meilleur_i,
               f_convert_int2char(meilleur_l), meilleur_k,
               meilleur_score);
               
        // Affichage stat tour
        printf("Stats Tour: Fils analysés: %ld | Coupures Alpha-Beta: %ld\n", 
               g_stats.nbFilsAnalysesTotal - fils_analyses_avant_tour,
               g_stats.nbCoupures - coupures_avant_tour);

    }
    else
    {
        printf("IA n'a pas trouve de mouvement valide. La partie pourrait etre terminee.\n");
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

// Fonctions pour mesure du temps
void getTime(struct timeval *tv)
{
    gettimeofday(tv, NULL);
}

double getSec(struct timeval *tv1, struct timeval *tv2)
{
    return (double)(tv2->tv_sec - tv1->tv_sec) + (double)(tv2->tv_usec - tv1->tv_usec) / 1000000.0;
}

// Affichage statistiques
void afficheStats()
{
    if (g_stats.nbToursIA == 0) return;
    
    printf("\n--- Statistiques Minimax %s ---\n", g_use_alpha_beta ? "(avec Alpha-Beta)" : "(sans Alpha-Beta)");
    printf("Profondeur de recherche: %d\n", PROFONDEUR_MAX);
    printf("Nombre de tours d'IA joués: %d\n", g_stats.nbToursIA);
    printf("Total des appels f_min/f_max: %ld\n", g_stats.nbAppelsMinMax);
    printf("Total des noeuds (fils) analysés: %ld\n", g_stats.nbFilsAnalysesTotal);
    printf("Total des coupures Alpha-Beta: %ld\n", g_stats.nbCoupures);
    printf("Temps total d'exécution IA: %.4f secondes\n", g_stats.totalTpsIA);
    printf("Temps moyen par tour: %.4f secondes\n", g_stats.totalTpsIA / g_stats.nbToursIA);
    printf("Moyenne des coupures par tour: %.2f\n", (float)g_stats.nbCoupures / g_stats.nbToursIA);
    printf("---------------------------------------\n");
}

void f_humain(int joueur)
{
    char c1, c2;
    char buffer[32];
    int l1, l2;

#ifdef DEBUG
    printf("dbg: entering %s %d\n", __FUNCTION__, __LINE__);
#endif

    printf("joueur ");
    switch(joueur)
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
    printf("joue (format: A0B1): ");
    
    while(1)
    {
        if(fgets(buffer, 32, stdin) != NULL)
        {
            if(sscanf(buffer, " %c%d%c%d", &c1, &l1, &c2, &l2) == 4)
            {
                if(f_bouge_piece(plateauDeJeu, l1, f_convert_char2int(c1), l2, f_convert_char2int(c2), joueur) == 0)
                    break;
            }
        }
        printf("Mauvais choix. Format attendu: A0B1 (colonne-ligne colonne-ligne)\n");
        printf("Reessayez: ");
    }

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif
}

int main(int argv, char *argc[])
{
    int fin = 0, mode=0, ret, joueur = 1;
    char buffer[32];
    int choix_alpha_beta; // Pour choisir d'activer ou non alpha beta 

    srand(time(NULL)); 
    
    // Choix activation Alpha-Beta
    printf("Voulez-vous utiliser l'élagage Alpha-Beta (coupures) ?\n");
    printf("1 - OUI (recommandé)\n0 - NON (exploration complète)\nChoix Alpha-Beta: ");
    if(fgets(buffer, 32, stdin) != NULL) {
        if (sscanf(buffer, "%d", &choix_alpha_beta) == 1) {
            g_use_alpha_beta = (choix_alpha_beta == 1) ? 1 : 0;
        } else {
            g_use_alpha_beta = 1; // Par défaut si mauvaise entrée
        }
    } else {
        g_use_alpha_beta = 1; // Par défaut si erreur de lecture
    }
    
    printf("\nMode de jeu :\n");
    printf("1 - Humain vs IA\n2 - Humain vs Humain\n3 - IA vs IA\nChoix: ");
    
    if(fgets(buffer, 32, stdin) != NULL)
        sscanf(buffer, "%d", &mode);

    plateauDeJeu = f_init_plateau();
    
    while (!fin)
    {
        f_affiche_plateau(plateauDeJeu);
        
        if(mode==1)
        {
            if(joueur>0)
                f_humain(joueur);
            else
                f_IA(joueur);
        }
        else if(mode==2)
        {
            f_humain(joueur);
        }
        else if(mode==3)
        {
            f_IA(joueur);
        } else {
            // Sortie si le mode n'est pas valide
            printf("Mode de jeu non valide. Fin de la partie.\n");
            break;
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
    
    free(plateauDeJeu);

#ifdef DEBUG
    printf("dbg: exiting %s %d\n", __FUNCTION__, __LINE__);
#endif

    return 0;
}