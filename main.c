/*---------------------------------------------
 Fichier: gabarit.c
 Description: Calculer la valeur de la r�sistance id�ale puis
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
// D�finition de structure
typedef struct
{
    // Entr�e de l'utilisateur
    double c;  // la conductance du condensateur
    double l;   // l'inductance de la bobine
    double pc;  // le pourcentage de dissipationb
    double vo;  // la tension initiale
    double td;  // le temps de dissipation
    // Valeurs calcul�es
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
Description:  Contr�le principale du logiciel.
              Obtient l'entr�e de l'utilisateur,
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
Param�tres:
   cPtr - pointeur � la variable structure CIRCUIT. Les membres utilis�s sont:
        c - la conductance
            l - l'inductance
            td - temps de dissipation
            vo - la tension initiale
            pc - pourcentage de dissipation
    sauve - pointeur � une variable structure CIRCUIT o� on sauvegarde l'entr�e de l'utilisateur
Description:  Cette fonction va tenter d'ouvrir le fichier binaire.
Elle invitera l'utilisateur � choisir un ensemble de donn�es ou d'en rentrer un nouveau.
S'il n'existe pas elle va le creer et sauvegarder le tableau de structure (initialis� � 0) et inviter l'utilisateur
� entrer des donn�es.
------------------------------------------------------------------------*/
void OuvrirFichier(CIRCUIT *cPtr, CIRCUIT *sauve)
{
    FILE *fp; // pointeur � une structure FILE
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
        getEntrees(cPtr, sauve, fp);// appel � la fonction pour obtenir les valeurs de la structure
    }
    else
    {
    //Lecture du fichier pour afficher le contenu � l'utilisateur
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
           printf("Voulez-vous prendre un de ses ensembles? (o/n)\n");// demande � l'utilisateur pour faire un choix
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
       else getEntrees(cPtr, sauve, fp);//appel � la fonction pour obtenir les valeurs de la structure
    }
}
/*----------------------------------------------------------
Fonction: getEntrees
Param�tres:
    cPtr - r�f�rence � une variable structure CIRCUIT. Membres
           utilis�s:
            c - la conductance
            l - l'inductance
            td - temps de dissipation
            vo - la tension initiale
            pc - pourcentage de dissipation
     savePtr - r�f�rence � une variable structure CIRCUIT o� on sauvegarde l'entr�e de l'utilisateur
Description: Obtient de l'utilisateur la conductance, l'inductance, le temps de dissipation,
              la tension initiale et le pourcentage de dissipation. V�rifie les contraintes
              sur l, c et pc.
              Sauvegarde les donn�es dans le fichier binaire lorsque l'utilisateur le souhaite
-------------------------------------------------------------*/
void getEntrees(CIRCUIT *cPtr, CIRCUIT *savePtr, FILE *fp)
{
    // D�clarations des variables
    char reponse;//reponse de l'utilisateur
    int ensemble;//choix de l'ensemble
    //Obtient les entrees
    do
        {
        cPtr->c = getValeurPositive("la conductance C en Faraday ");
        if(cPtr->c < EPSILON || cPtr->c >0.1)
        printf("Les valeurs doivent �tre entre 1e-10 et 0.1\n");
        }while(cPtr->c < EPSILON || cPtr->c >0.1);
        do
        {
        cPtr->l = getValeurPositive("l'inductance en Henri ");
        if(cPtr->l < EPSILON || cPtr->l >0.1)
        printf("Les valeurs doivent �tre entre 1e-10 et 0.1\n");
        }while(cPtr->l>0.1 || cPtr->l<EPSILON);
        do
        {
        cPtr->pc = getValeurPositive("pourcentage de dissipation ");
        if(cPtr->pc < 0.1 || cPtr->pc > 0.9)
        printf("Les valeurs doivent �tre entre 0.1 et 0.9\n.3-");
        }while(cPtr->pc < 0.1 || cPtr->pc > 0.9);
        cPtr->vo = getValeurPositive("la tension initiale en Volt ");
        cPtr->td = getValeurPositive("temps de dissipation en secondes ");
        printf("Voulez-vous enregistrer vos donnees (o/n)?\n"); // demande � l'utilisateur pour sauvegarder ses donn�es
        fflush(stdin);
        scanf("%c", &reponse);
        while (reponse!='o'&&reponse!='n')
        {
         printf("Veuillez donner une r�ponse valide (o/n)\n");
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
       }while(reponse=='n'); //sauvegarde les donn�es de l'utilisateur
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
Param�tres:
    invitation - reference � la cha�ne � inclure dans le message
                 d'invitation � l'utilisateur
Valeur de retour: valeur positive (>0) obtenu de l'utilisateur
Description: Demande � l'utilisateur une valeur (avec la chaine
             d'invitation donn�e) et v�rifie qu'elle soit
             positive.
-------------------------------------------------------------*/

double getValeurPositive(char *invitation)
{
    double valeur; // valeur donn�e par l'utilisateur.
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
Param�tres:
    cPtr - r�f�rence � une variable structure CIRCUIT. Membres
           utilis�s:
            l - inductance
            c - conductance
Description: Trouver la valeur maximale de la r�sistance grace �
             l'�quation:  Rmax = (4*l/c)^1/2.
-------------------------------------------------------------*/
double getRmax(CIRCUIT *cPtr)
{

     double r_max;
     r_max = sqrt(4*cPtr->l/cPtr->c);
     return (r_max);
}
/*----------------------------------------------------------
Fonction: calculeR
Param�tres:
    cPtr - reference � la structure CIRCUIT
    r_max - valeur maximale de la resistance
Description: utilise l'algorithme de bissection pour trouver la valeur exacte de
            R la resistance utilis�e pour tracer q en fonction du temps.
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
Param�tres:
    valeur - abscisse de la fonction
Valeur de retour: Valeur f(x)
Description: Calcule la valeur f(x) pour la valeur donn�e d'x.
             La fonction est d�fini comme �tant:

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
Param�tres:
    cPtr - r�f�rence � une variable structure CIRCUIT. Membres
           utilis�s:
            qo - charge initiale
            vo - tension initiale
            c - conductance
            td - temps de dissipation
            l - inductance
            t - tableau pour sauver le temps
            q - tableau pour sauver la charge
    racine -  la valeur de la resistance trouv�e
Description: Remplit les tableaux avec 300 points de valeurs
              q/t calculer avec l'equation dans le projet.
-------------------------------------------------------------*/
void calculeQ(CIRCUIT *cPtr, double racine)
{
    double temps = 0;//initialiser le temps
    int ix; // index du tableau
    double q; // valeur pour stocker les valeurs de la fonction
    double f; // fr�quence pour la distorsion
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
Param�tres:
     cPtr - r�f�rence � une variable structure CIRCUIT. Membres
           utilis�s:
            q - tableau des valeurs de q en fonction du temps
            td - fin de l'�tendu de valeurs du temps
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
    plcol0(GREEN);      // couleur des �tiquettes
    pllab("temps", "q(t)", "la charge");
    // Trace la courbe
    plcol0(BLUE);    // Couleur pour tracer la courbe
    plline(TAILLE_MAX, cPtr->t, cPtr->q);
    plend();
}


/*-------------------------------------------------
 Fonction: qmin
 Param�tres:
    tbl: reference a un tableau de valeurs double.
 Valeur retourn�e: la plus petite valeur trouv�e dans le tableau.
 Description: Trouve la plus petite valeur trouv�e dans le tableau.
              Utilise une boucle � r�p�tition d�termin�e pour
	      traverser le tableau afin d'examiner chaque
	      valeur dans le tableau.
-------------------------------------------------*/
double qmin(double tbl[])
{
    // D�claration des variables
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
 Param�tres:
    tbl: reference a un tableau de valeurs double.
 Valeur retourn�e: la plus grande valeur trouv�e dans le tableau.
 Description: Trouve la plus grande valeur trouv�e dans le tableau.
              Utilise une boucle � r�p�tition d�termin�e pour
	      traverser le tableau afin d'examiner chaque
	      valeur dans le tableau.
-------------------------------------------------*/
double qmax(double tbl[])
{
    // D�claration des variables
    double max;  // pour stocker la valeur maximale
    int ix;      // pour indexer dans le tableau
    // Instructions
    max = -DBL_MAX;  // valeur n�gative la plus petite pour le double
    for(ix = 1; ix < TAILLE_MAX; ix = ix + 1)
    {
        if(max < tbl[ix]) max = tbl[ix];
    }
    return(max);
}
