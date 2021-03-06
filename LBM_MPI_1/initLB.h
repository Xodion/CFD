#ifndef _INITLB_H_
#define _INITLB_H_
#include "helper.h"
#include <mpi.h>

/* reads the parameters for the lid driven cavity scenario from a config file */
int readParameters(
    int *xlength,                       /* reads domain size. Parameter name: "xlength" */
    double *tau,                        /* relaxation parameter tau. Parameter name: "tau" */
    double *velocityWall,               /* velocity of the lid. Parameter name: "characteristicvelocity" */
    int *timesteps,                     /* number of timesteps. Parameter name: "timesteps" */
    int *timestepsPerPlotting,          /* timesteps between subsequent VTK plots. Parameter name: "vtkoutput" */
    char *problem,                      /* name of the scenario to simulate*/
    char *pgmInput,                      /* pgm File for the scenario used in the tilted plate case */
    int argc,                           /* number of arguments. Should equal 2 (program + name of config file */
    char *argv[],                      /* argv[1] shall contain the path to the config file */
    int * iProc, int * jProc, int * kProc
);


/* initialises the particle distribution functions and the flagfield */
void initialiseFields(double *collideField, double *streamField, int *flagField, int *xlength, char *problem, char* pgmInput,int rank,int number_of_ranks,int iProc,int jProc,int kProc);

void initialiseBuffers(double*sendBuffer[],double**readBuffer,int*xlength);

void extract(double*(sendBuffer[]),double*collideField,int*xlength);

void decideneighbours(int*il,int*ir,int*jb,int*jt,int*kf,int*kb,int iProc,int jProc,int kProc,int rank,int *xlength);

void swap_send_read( double**sendBuffer ,double**readBuffer ,int*xlength,int il,int ir,int jb,int jt,int kf,int kb,int myrank,MPI_Status *status);

void  inject(double**readBuffer,double*collideField,int*xlength);

#endif

