/*---------------------------------------------
 Fichier: gabarit.c
 Description: Calculer la valeur de la résistance idéale puis
            tracer la courbe de la charge en fonction du temps.
-------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "gng1106plplot.h"
// Les constantes symboliques
#define EPSILON 1e-10
#define FICHIERBIN "ensembledonnees.bin"
#define VRAI 1
#define FAUX 0
#define TAILLE_MAX 300  // taille maximale des tableaux
// Définition de structure
typedef struct
{
    // Entrée de l'utilisateur
    double c;  // la conductance du condensateur
    double l;   // l'inductance de la bobine
    double pc;  // le pourcentage de dissipationb
    double vo;  // la tension initiale
    double td;  // le temps de dissipation
    // Valeurs calculées
    double qo;       // la charge initiale ou maximale
    double deltat;   // le pas de temps (incrementation)
    double q[TAILLE_MAX];    // tableau de valeurs de q
    double t[TAILLE_MAX];      // tableau de valeurs du temps
}  CIRCUIT;
// Prototypes de fonction
void OuvrirFichier(CIRCUIT *, CIRCUIT *);
void getEntrees(CIRCUIT *, CIRCUIT *, FILE *);
double getValeurPositive(char *);
double fct(CIRCUIT *, double);
double calculeR(CIRCUIT *, double);
void calculeQ(CIRCUIT *, double);
double getRmax(CIRCUIT *);
void plot(CIRCUIT *);
double qmax(double *);
double qmin(double *);
/*--------------------------------------------
Fonction: main
Description:  Contrôle principale du logiciel.
              Obtient l'entrée de l'utilisateur,
              calcule la valeur de Rmax, R et q(t)
              et trace la courbe de la charge en fonction
              du temps sur un graphe.
----------------------------------------------*/
int main()
{
    char reponse;
    double x;
    double r;
    CIRCUIT circuit;
    CIRCUIT tblsauve[5];
    do
    {
    OuvrirFichier(&circuit, tblsauve);
    // Calculs
    x = getRmax(&circuit);
    r = calculeR(&circuit, x);
    calculeQ(&circuit, r);
     // Graphe
    plot(&circuit);
    //demande a l'utilisateur s'il veut quitter
     do
       {
           printf("Voulez-vous quitter (o/n)? ");
           fflush(stdin);
           scanf("%c", &reponse);
       } while (reponse!='o' && reponse!='n');
   } while (reponse=='n');
   printf("Programme fini\n");
}
/*-----------------------------------------------------------------------
Fonction: ouvrirFichier
Paramètres:
   cPtr - pointeur à la variable structure CIRCUIT. Les membres utilisés sont:
        c - la conductance
            l - l'inductance
            td - temps de dissipation
            vo - la tension initiale
            pc - pourcentage de dissipation
    sauve - pointeur à une variable structure CIRCUIT où on sauvegarde l'entrée de l'utilisateur
Description:  Cette fonction va tenter d'ouvrir le fichier binaire.
Elle invitera l'utilisateur à choisir un ensemble de données ou d'en rentrer un nouveau.
S'il n'existe pas elle va le creer et sauvegarder le tableau de structure (initialisé à 0) et inviter l'utilisateur
à entrer des données.
------------------------------------------------------------------------*/
void OuvrirFichier(CIRCUIT *cPtr, CIRCUIT *sauve)
{
    FILE *fp; // pointeur à une structure FILE
    int ix;
    char reponse; //reponse de l'utilisateur o/n
    int ensemble; //l'ensemble de valeur choisit par l'utilisateur
    //tentative d'ouverture du fichier pour la lecture
    fp=fopen(FICHIERBIN, "rb");
    if (fp==NULL)//fichier n'existe pas, il va etre creer
    {
        printf("Le fichier est vide\n");
        for (ix = 0; ix < 5; ix = ix +1)
        {
            sauve[ix].c=0;
            sauve[ix].l=0;
            sauve[ix].pc=0;
            sauve[ix].vo=0;
            sauve[ix].td=0;
        }
        getEntrees(cPtr, sauve, fp);// appel à la fonction pour obtenir les valeurs de la structure
    }
    else
    {
    //Lecture du fichier pour afficher le contenu à l'utilisateur
       fread(sauve, sizeof(CIRCUIT),5,fp);
       printf("Conductance    Inductance    pourcentage de dissipation    tension initiale     temps de dissipation\n");
       printf("----------------------------------------------------------------------------\n");
       for (ix=0; ix<5; ix=ix+1)
       {
       printf("%10.8f,%10.8f, %10.8f, %10.8f, %10.8f\n", sauve[ix].c, sauve[ix].l, sauve[ix].pc, sauve[ix].vo, sauve[ix].td);
       }
       fclose(fp); //fermer le fichier
       do
       {
           printf("Voulez-vous prendre un de ses ensembles? (o/n)\n");// demande à l'utilisateur pour faire un choix
           fflush(stdin);
           scanf("%c", &reponse);
       }while (reponse!='o' && reponse!='n');
       if (reponse=='o')
       {

           printf("Veuillez selectionner parmis les 5 ensembles (chiffre entre 1 et 5)\n");
           fflush(stdin);
           scanf("%d", &ensemble);
           while(ensemble <1 ||ensemble>5)
           {
            printf("Faites une selection valide en entrant un nombre entre 1 et 5\n");
            fflush(stdin);
            scanf("%d", &ensemble);
           }
           cPtr->c=sauve[ensemble-1].c;
           cPtr->l=sauve[ensemble-1].l;
           cPtr->pc=sauve[ensemble-1].pc;
           cPtr->vo=sauve[ensemble-1].vo;
           cPtr->td=sauve[ensemble-1].td;

       }
       else getEntrees(cPtr, sauve, fp);//appel à la fonction pour obtenir les valeurs de la structure
    }
}
/*----------------------------------------------------------
Fonction: getEntrees
Paramètres:
    cPtr - référence à une variable structure CIRCUIT. Membres
           utilisés:
            c - la conductance
            l - l'inductance
            td - temps de dissipation
            vo - la tension initiale
            pc - pourcentage de dissipation
     savePtr - référence à une variable structure CIRCUIT où on sauvegarde l'entrée de l'utilisateur
Description: Obtient de l'utilisateur la conductance, l'inductance, le temps de dissipation,
              la tension initiale et le pourcentage de dissipation. Vérifie les contraintes
              sur l, c et pc.
              Sauvegarde les données dans le fichier binaire lorsque l'utilisateur le souhaite
-------------------------------------------------------------*/
void getEntrees(CIRCUIT *cPtr, CIRCUIT *savePtr, FILE *fp)
{
    // Déclarations des variables
    char reponse;//reponse de l'utilisateur
    int ensemble;//choix de l'ensemble
    //Obtient les entrees
    do
        {
        cPtr->c = getValeurPositive("la conductance C en Faraday ");
        if(cPtr->c < EPSILON || cPtr->c >0.1)
        printf("Les valeurs doivent être entre 1e-10 et 0.1\n");
        }while(cPtr->c < EPSILON || cPtr->c >0.1);
        do
        {
        cPtr->l = getValeurPositive("l'inductance en Henri ");
        if(cPtr->l < EPSILON || cPtr->l >0.1)
        printf("Les valeurs doivent être entre 1e-10 et 0.1\n");
        }while(cPtr->l>0.1 || cPtr->l<EPSILON);
        do
        {
        cPtr->pc = getValeurPositive("pourcentage de dissipation ");
        if(cPtr->pc < 0.1 || cPtr->pc > 0.9)
        printf("Les valeurs doivent être entre 0.1 et 0.9\n.3-");
        }while(cPtr->pc < 0.1 || cPtr->pc > 0.9);
        cPtr->vo = getValeurPositive("la tension initiale en Volt ");
        cPtr->td = getValeurPositive("temps de dissipation en secondes ");
        printf("Voulez-vous enregistrer vos donnees (o/n)?\n"); // demande à l'utilisateur pour sauvegarder ses données
        fflush(stdin);
        scanf("%c", &reponse);
        while (reponse!='o'&&reponse!='n')
        {
         printf("Veuillez donner une réponse valide (o/n)\n");
         fflush(stdin);
         scanf("%c", &reponse);
        }
        if (reponse=='o')
        {
        do
        {
        printf("Choisissez un emplacement parmi les 5 ensembles de donnees");
        scanf("%d", &ensemble);
        while (ensemble<1 || ensemble>5);
        printf("Voulez-vous remplacer cet ensemble de donnees par votre nouvel ensemble (o/n)? \n");
        fflush(stdin);
        scanf("%c",&reponse);
        while (reponse!='o'&&reponse!='n')
        {
          printf("Veuillez entrer une reponse valide\n");
          fflush(stdin);
          scanf("%c",&reponse);
        }
       }while(reponse=='n'); //sauvegarde les données de l'utilisateur
               savePtr[ensemble-1].c=cPtr->c;
               savePtr[ensemble-1].l=cPtr->l;
               savePtr[ensemble-1].pc=cPtr->pc;
               savePtr[ensemble-1].vo=cPtr->vo;
               savePtr[ensemble-1].td=cPtr->td;
               fp=fopen(FICHIERBIN,"wb");
               fwrite(savePtr, sizeof(CIRCUIT),5,fp);
               fclose(fp);

   }
}
/*----------------------------------------------------------
Fonction: getValeurPositive
Paramètres:
    invitation - reference à la chaîne à inclure dans le message
                 d'invitation à l'utilisateur
Valeur de retour: valeur positive (>0) obtenu de l'utilisateur
Description: Demande à l'utilisateur une valeur (avec la chaine
             d'invitation donnée) et vérifie qu'elle soit
             positive.
-------------------------------------------------------------*/

double getValeurPositive(char *invitation)
{
    double valeur; // valeur donnée par l'utilisateur.
    do
    {
        printf("S.V.P. donnez la valeur pour %s: ", invitation);
        scanf("%lf",&valeur);
        if(valeur <= 0.0)
            printf("La valeur doit etre plus grand que zero.\n");
    }
    while(valeur <= 0.0);
    return(valeur);
}


/*----------------------------------------------------------
Fonction: getRmax
Paramètres:
    cPtr - référence à une variable structure CIRCUIT. Membres
           utilisés:
            l - inductance
            c - conductance
Description: Trouver la valeur maximale de la résistance grace à
             l'équation:  Rmax = (4*l/c)^1/2.
-------------------------------------------------------------*/
double getRmax(CIRCUIT *cPtr)
{

     double r_max;
     r_max = sqrt(4*cPtr->l/cPtr->c);
     return (r_max);
}
/*----------------------------------------------------------
Fonction: calculeR
Paramètres:
    cPtr - reference à la structure CIRCUIT
    r_max - valeur maximale de la resistance
Description: utilise l'algorithme de bissection pour trouver la valeur exacte de
            R la resistance utilisée pour tracer q en fonction du temps.
-------------------------------------------------------------*/
double calculeR(CIRCUIT *cPtr, double r_max)
{
     double ax;
     double bas;
     double haut;
     bas = 0.0;
     haut = r_max;
    if ((fct(cPtr, haut)*fct(cPtr, bas))<0)
    {
     do
      {
        ax = (bas+haut)/2;
        if ((fct(cPtr, ax)*fct(cPtr, bas))<0.0)
        {
            haut = ax;
        }
        else if ((fct(cPtr, ax)*fct(cPtr, bas))>0.0)
        {
            bas = ax;
        }
      }while ((haut-bas)>EPSILON);
    }
    return(ax);
}
/*----------------------------------------------------------
Fonction: fct
Paramètres:
    valeur - abscisse de la fonction
Valeur de retour: Valeur f(x)
Description: Calcule la valeur f(x) pour la valeur donnée d'x.
             La fonction est défini comme étant:

             f(x) = e^(x*td/(2*l))-pc
-------------------------------------------------------------*/
double fct(CIRCUIT *cPtr, double valeur)
{
    double fonct;
    fonct = valeur*cPtr->td;
    fonct = -(fonct/(cPtr->l*2));
    fonct = exp(fonct)- cPtr->pc;
    return (fonct);
}
/*----------------------------------------------------------
Fonction: calculeQ
Paramètres:
    cPtr - référence à une variable structure CIRCUIT. Membres
           utilisés:
            qo - charge initiale
            vo - tension initiale
            c - conductance
            td - temps de dissipation
            l - inductance
            t - tableau pour sauver le temps
            q - tableau pour sauver la charge
    racine -  la valeur de la resistance trouvée
Description: Remplit les tableaux avec 300 points de valeurs
              q/t calculer avec l'equation dans le projet.
-------------------------------------------------------------*/
void calculeQ(CIRCUIT *cPtr, double racine)
{
    double temps = 0;//initialiser le temps
    int ix; // index du tableau
    double q; // valeur pour stocker les valeurs de la fonction
    double f; // fréquence pour la distorsion
    cPtr->qo = cPtr->c*cPtr->vo; // calcul de qo
    cPtr->deltat = cPtr->td/TAILLE_MAX; // calcul du pas
    for (ix=0; ix < TAILLE_MAX; ix = ix +1)
    {
     // calcul de la fonction par accumulation
     q = cPtr->c*cPtr->vo*exp((-racine*temps)/(2*cPtr->l));
     q = q*cos((sqrt(1/(cPtr->c*cPtr->l)-pow(racine/(2*cPtr->l),2))*temps));
     cPtr->q[ix]= q;
     cPtr->t[ix]= temps;
     temps = temps + cPtr->deltat; // incrementation du temps
    }
    // evaluer la distorsion
    printf("\n la valeur de la resistance est : %f \n", racine);
    f = sqrt(1/(cPtr->c*cPtr->l)-pow(racine/(2*cPtr->l),2))/(2*M_PI);
    if (50/f < cPtr->td) printf("La frequence est trop elevee, de la distorsion peut exister dans le graphe\n");

}
/*----------------------------------------------------------
Fonction: plot
Paramètres:
     cPtr - référence à une variable structure CIRCUIT. Membres
           utilisés:
            q - tableau des valeurs de q en fonction du temps
            td - fin de l'étendu de valeurs du temps
            t - tableau des valeurs du temps
Description: Initialise l'appareil de graphique, la largeur de plume,
             et trace la courbe de la charge en fonction du temps (q(t) vs t) .
----------------------------------------------------------------*/
void plot(CIRCUIT *cPtr)
{
    double minfx, maxfx;// valeur maxiamle et minimale
    double verticale;
    //Trouve la valeur maximale et minimale de la charge q
    minfx = qmin(cPtr->q);
    maxfx = qmax(cPtr->q);
    verticale = maxfx- minfx;
    maxfx = maxfx + (0.1*verticale);
    minfx = minfx - (0.1*verticale);
    // Initiliaise la page PLplot
    plsdev("wingcc"); // L'appareil est wingcc - compilateur CodeBlocks
    // Initialise le graphique
    plinit();
    plwidth(3);// largeur de la plume
    plenv(0, cPtr->td, minfx, maxfx, 0, 1);
    plcol0(GREEN);      // couleur des étiquettes
    pllab("temps", "q(t)", "la charge");
    // Trace la courbe
    plcol0(BLUE);    // Couleur pour tracer la courbe
    plline(TAILLE_MAX, cPtr->t, cPtr->q);
    plend();
}


/*-------------------------------------------------
 Fonction: qmin
 Paramètres:
    tbl: reference a un tableau de valeurs double.
 Valeur retournée: la plus petite valeur trouvée dans le tableau.
 Description: Trouve la plus petite valeur trouvée dans le tableau.
              Utilise une boucle à répétition déterminée pour
	      traverser le tableau afin d'examiner chaque
	      valeur dans le tableau.
-------------------------------------------------*/
double qmin(double tbl[])
{
    // Déclaration des variables
    double min;  // pour stocker la valeur minimale
    int ix;      // pour indexer dans le tableau
    // Instructions
    min = DBL_MAX;  // valeur positive la plus grande pour le double
    for(ix = 1; ix < TAILLE_MAX; ix = ix + 1)
    {
        if(min >tbl[ix]) min = tbl[ix];
    }
    return(min);
}

/*-------------------------------------------------
 Fonction: qmax
 Paramètres:
    tbl: reference a un tableau de valeurs double.
 Valeur retournée: la plus grande valeur trouvée dans le tableau.
 Description: Trouve la plus grande valeur trouvée dans le tableau.
              Utilise une boucle à répétition déterminée pour
	      traverser le tableau afin d'examiner chaque
	      valeur dans le tableau.
-------------------------------------------------*/
double qmax(double tbl[])
{
    // Déclaration des variables
    double max;  // pour stocker la valeur maximale
    int ix;      // pour indexer dans le tableau
    // Instructions
    max = -DBL_MAX;  // valeur négative la plus petite pour le double
    for(ix = 1; ix < TAILLE_MAX; ix = ix + 1)
    {
        if(max < tbl[ix]) max = tbl[ix];
    }
    return(max);
}
