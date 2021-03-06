#include <stdlib.h>
#include <stdio.h>
#include "initLB.h"
#include "LBDefinitions.h"
#include <mpi.h>

int readParameters(int *xlength, double *tau, double *bddParams, int *timesteps, int *timestepsPerPlotting, char *problem, char *pgmInput, int argc, char *argv[], int * iProc, int * jProc, int * kProc)
{
    /**
    *  The array bddParams has the following structure
    *  bddParams[0]   Inflow velocity in x direction
    *  bddParams[1]   Inflow velocity in y direction
    *  bddParams[2]   Inflow velocity in z direction
    *  bddParams[3]   Pressure surplus for PRESSURE_IN cells
    *  bddParams[4]   Moving wall velocity in x direction
    *  bddParams[5]   Moving wall velocity in y direction
    *  bddParams[6]   Moving wall velocity in z direction
    */
    if (argc == 2)
    {
        read_string(argv[1], "problem", problem);
        read_int(argv[1], "xlength", xlength);
        read_int(argv[1], "ylength", xlength + 1);
        read_int(argv[1], "zlength", xlength + 2);
        read_int(argv[1], "iProc", iProc);
        read_int(argv[1], "jProc", jProc);
        read_int(argv[1], "kProc", kProc);
        READ_DOUBLE(argv[1], *tau);

        if(!strcmp(problem, "drivenCavity"))
        {
            read_double(argv[1], "velocityWallX", bddParams + 4);
            read_double(argv[1], "velocityWallY", bddParams + 5);
            read_double(argv[1], "velocityWallZ", bddParams + 6);
        }
        else
        {
            bddParams[4] = 0.0;
            bddParams[5] = 0.0;
            bddParams[6] = 0.0;
        }

        if(!strcmp(problem, "tiltedPlate"))
        {
            read_double(argv[1], "velocityInflowX", bddParams);
            read_double(argv[1], "velocityInflowY", bddParams + 1);
            read_double(argv[1], "velocityInflowZ", bddParams + 2);
            read_string(argv[1], "pgmInput", pgmInput);
        }
        else
        {
            bddParams[0] = 0.0;
            bddParams[1] = 0.0;
            bddParams[2] = 0.0;
            strcpy(pgmInput, "");
        }

        if (!strcmp(problem, "flowStep"))
        {
            read_double(argv[1], "velocityInflowX", bddParams);
            read_double(argv[1], "velocityInflowY", bddParams + 1);
            read_double(argv[1], "velocityInflowZ", bddParams + 2);
        }

        if(!strcmp(problem, "planeShearFlow"))
        {
            read_double(argv[1], "pressureIn", bddParams + 3);
        }
        else
        {
            bddParams[3] = 0.0;
        }
        READ_INT(argv[1], *timesteps);
        READ_INT(argv[1], *timestepsPerPlotting);
    }
    else

    {

        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return -1;
    }

    return 0;
}


void initialiseFields(double *collideField, double *streamField, int *flagField, int *xlength, char *problem, char* pgmInput,int rank,int number_of_ranks,int iProc,int jProc,int kProc)
{
    int i, x, y, z;

    for (z = 0; z <= xlength[2] + 1; z++)
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
            {
                for (i = 0; i < PARAMQ; i++)
                {
                    collideField[PARAMQ * (z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x) + i] = LATTICEWEIGHTS[i];
                    streamField[PARAMQ * (z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x) + i] = LATTICEWEIGHTS[i];
                }
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = FLUID;
            }


    if (!strcmp(problem, "drivenCavity"))
    {
        /* Now , let us focus on the initialization of the fields . First we mark all domains boundary cells as PARALLEL_BOUNDARY . then we go for moving wall , and no slip */
        /* top boundary */
        y = xlength[1] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = PARALLEL_BOUNDARY;
        /* back boundary */
        z = 0;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = PARALLEL_BOUNDARY;

        /* bottom boundary */
        y = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = PARALLEL_BOUNDARY;

        /* left boundary */
        x = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = PARALLEL_BOUNDARY;

        /* right boundary */
        x = xlength[0] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = PARALLEL_BOUNDARY;

        /* front boundary, i.e. z = xlength + 1 */
        z = xlength[2] + 1;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = PARALLEL_BOUNDARY;
       

       /* 
       If the domain is at the top boundary , then moving wall at the top.
       */
   
   if((rank%(iProc*jProc))>(iProc*jProc - iProc - 1))
   {    
        y = xlength[1] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
               flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = MOVING_WALL;
   }

/* 
If the domain is at the bottom boundary , then no slip at the bottom.
*/
   if(rank%(iProc*jProc)<iProc)
   {
       y = 0;
       for (z = 0; z <= xlength[2] + 1; z++)
          for (x = 0; x <= xlength[0] + 1; x++)
              flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
   }

/* 
If the domain is at the left boundary , then no slip at the left.
*/
     if(rank%iProc == 0)
     {
          x = 0;
          for (z = 0; z <= xlength[2] + 1; z++)
              for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
     }

/* 
If the domain is at the right boundary , then no slip at the back.
*/
     if(rank%iProc == iProc-1)
     {
          x = xlength[0] + 1;
          for (z = 0; z <= xlength[2] + 1; z++)
              for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
     }

/* 
If the domain is at the back boundary , then no slip at the right.
*/
     if((rank%(iProc*jProc*kProc)<(iProc*jProc)))
     {
          z = 0;
          for (y = 0; y <= xlength[1] + 1; y++)
              for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
     }

/* 
If the domain is at the front boundary , then no slip at the front.
*/
   if(((rank%(iProc*jProc*kProc))>(iProc*jProc*kProc - (iProc*jProc) - 1)))
   {
         z = xlength[2] + 1;
         for (y = 0; y <= xlength[1] + 1; y++)
             for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
    }

    /* top boundary */
/*
    y = xlength[1] + 1;
    for (z = 0; z <= xlength[2] + 1; z++)
        for (x = 0; x <= xlength[0] + 1; x++)
            flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = MOVING_WALL;
     back boundary 
    z = 0;
    for (y = 0; y <= xlength[1] + 1; y++)
        for (x = 0; x <= xlength[0] + 1; x++)
            flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

     bottom boundary 
    y = 0;
    for (z = 0; z <= xlength[2] + 1; z++)
        for (x = 0; x <= xlength[0] + 1; x++)
            flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;


     right boundary 
    x = xlength[0] + 1;
    for (z = 0; z <= xlength[2] + 1; z++)
        for (y = 0; y <= xlength[1] + 1; y++)
            flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

     front boundary, i.e. z = xlength + 1 
    z = xlength[2] + 1;
    for (y = 0; y <= xlength[1] + 1; y++)
        for (x = 0; x <= xlength[0] + 1; x++)
            flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
*/
    }

    if (!strcmp(problem, "tiltedPlate"))
    {
        int** pgmImage;
        pgmImage = read_pgm(pgmInput);
        //write_imatrix( "testing.txt", pgmImage, 0, xlength[0]+1,0,xlength[1]+1,1,1,0);               

        for (z = 1; z <= xlength[2]; z++)
            for (y = 1; y <= xlength[1]; y++)
                for (x = 1; x <= xlength[0]; x++)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = pgmImage[x][y];
        //free_imatrix(pgmImage, 0, xlength[0] + 2, 0, xlength[1] + 2);

         /* front boundary, i.e. z = xlength + 1 */
        z = xlength[2] + 1;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = FREE_SLIP;

         /* back boundary */
        z = 0;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = FREE_SLIP;


        /* left boundary */
        x = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = INFLOW;

        /* right boundary */
        x = xlength[0] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = OUTFLOW;


        /* top boundary */
        y = xlength[1] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* bottom boundary */
        y = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

    }
    if (!strcmp(problem, "flowStep"))
    {


        /* front boundary, i.e. z = xlength + 1 */
        z = xlength[2] + 1;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* back boundary */
        z = 0;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* left boundary */
        x = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = INFLOW;

        /* right boundary */
        x = xlength[0] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = OUTFLOW;


         /* top boundary */
        y = xlength[1] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* bottom boundary */
        y = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;


        /* step */
        for (z = 1; z <= xlength[2]; z++)
            for (y = 1; y <= xlength[1]/2; y++) /* integer division on purpose, half of the channel is blocked by step */
                for (x = 1; x <= xlength[1]/2; x++)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

    }
    if (!strcmp(problem, "planeShearFlow"))
    {

         /* front boundary, i.e. z = xlength + 1 */
        z = xlength[2] + 1;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = FREE_SLIP;

        /* back boundary */
        z = 0;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = FREE_SLIP;



        /* left boundary */
        x = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = PRESSURE_IN;

        /* right boundary */
        x = xlength[0] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = OUTFLOW;


        /* top boundary */
        y = xlength[1] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* bottom boundary */
        y = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
    }
}

void initialiseBuffers(double*(sendBuffer[]),double**readBuffer,int*xlength)
{
   int size_x = 5 * ((xlength[1]+2)*(xlength[2]+2)),size_y=5*((xlength[0]+2)*(xlength[2]+2)),size_z=5*((xlength[0]+2)*(xlength[1]+2)) ;

   (sendBuffer[0]) = (double*) calloc(size_x,sizeof(double)) ;
   (sendBuffer[1]) = (double*) calloc(size_x,sizeof(double)) ;
   (sendBuffer[2]) = (double*) calloc(size_y,sizeof(double)) ;
   (sendBuffer[3]) = (double*) calloc(size_y,sizeof(double)) ;
   (sendBuffer[4]) = (double*) calloc(size_z,sizeof(double)) ;
   (sendBuffer[5]) = (double*) calloc(size_z,sizeof(double)) ;
   (readBuffer[0]) = (double*) calloc(size_x,sizeof(double)) ;
   (readBuffer[1]) = (double*) calloc(size_x,sizeof(double)) ;
   (readBuffer[2]) = (double*) calloc(size_y,sizeof(double)) ;
   (readBuffer[3]) = (double*) calloc(size_y,sizeof(double)) ;
   (readBuffer[4]) = (double*) calloc(size_z,sizeof(double)) ;
   (readBuffer[5]) = (double*) calloc(size_z,sizeof(double)) ;

}

void extract(double*(sendBuffer[]),double*collideField,int*xlength)
{
            /* x right */
            int x , y , z , cellindex;
            x = xlength[0] ;
            for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
            {
                cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
                sendBuffer[0][5*(z*(xlength[1]+2)+y)+0] = collideField[PARAMQ*cellindex + 13] ;
                sendBuffer[0][5*(z*(xlength[1]+2)+y)+1] = collideField[PARAMQ*cellindex + 3] ;
                sendBuffer[0][5*(z*(xlength[1]+2)+y)+2] = collideField[PARAMQ*cellindex + 10] ;
                sendBuffer[0][5*(z*(xlength[1]+2)+y)+3] = collideField[PARAMQ*cellindex + 17] ;
                sendBuffer[0][5*(z*(xlength[1]+2)+y)+4] = collideField[PARAMQ*cellindex + 7] ;
            }

            /* x left */
            x = 1 ;
            for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
            {
                cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
                sendBuffer[1][5*(z*(xlength[1]+2)+y)+0] = collideField[PARAMQ*cellindex + 11] ;
                sendBuffer[1][5*(z*(xlength[1]+2)+y)+1] = collideField[PARAMQ*cellindex + 1] ;
                sendBuffer[1][5*(z*(xlength[1]+2)+y)+2] = collideField[PARAMQ*cellindex + 8] ;
                sendBuffer[1][5*(z*(xlength[1]+2)+y)+3] = collideField[PARAMQ*cellindex + 15] ;
                sendBuffer[1][5*(z*(xlength[1]+2)+y)+4] = collideField[PARAMQ*cellindex + 5] ;
            }

            /* y top */
            y = xlength[1] ;
            for (x = 0; x <= xlength[0] + 1; x++)
            for (z = 0; z <= xlength[2] + 1; z++)
            {
                cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
                sendBuffer[2][5*(x*(xlength[2]+2)+z)+0] = collideField[PARAMQ*cellindex + 4] ;
                sendBuffer[2][5*(x*(xlength[2]+2)+z)+1] = collideField[PARAMQ*cellindex + 11] ;
                sendBuffer[2][5*(x*(xlength[2]+2)+z)+2] = collideField[PARAMQ*cellindex + 12] ;
                sendBuffer[2][5*(x*(xlength[2]+2)+z)+3] = collideField[PARAMQ*cellindex + 13] ;
                sendBuffer[2][5*(x*(xlength[2]+2)+z)+4] = collideField[PARAMQ*cellindex + 18] ;
            }

            /* y bottom */
            y = 1 ;
            for (x = 0; x <= xlength[0] + 1; x++)
            for (z = 0; z <= xlength[2] + 1; z++)
            {
                cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
                sendBuffer[3][5*(x*(xlength[2]+2)+z)+0] = collideField[PARAMQ*cellindex + 0] ;
                sendBuffer[3][5*(x*(xlength[2]+2)+z)+1] = collideField[PARAMQ*cellindex + 5] ;
                sendBuffer[3][5*(x*(xlength[2]+2)+z)+2] = collideField[PARAMQ*cellindex + 6] ;
                sendBuffer[3][5*(x*(xlength[2]+2)+z)+3] = collideField[PARAMQ*cellindex + 7] ;
                sendBuffer[3][5*(x*(xlength[2]+2)+z)+4] = collideField[PARAMQ*cellindex + 14] ;
            }

            /* z back */
            z = 1 ;
            for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
            {
                cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
                sendBuffer[4][5*(y*(xlength[0]+2)+x)+0] = collideField[PARAMQ*cellindex + 4] ;
                sendBuffer[4][5*(y*(xlength[0]+2)+x)+1] = collideField[PARAMQ*cellindex + 1] ;
                sendBuffer[4][5*(y*(xlength[0]+2)+x)+2] = collideField[PARAMQ*cellindex + 2] ;
                sendBuffer[4][5*(y*(xlength[0]+2)+x)+3] = collideField[PARAMQ*cellindex + 3] ;
                sendBuffer[4][5*(y*(xlength[0]+2)+x)+4] = collideField[PARAMQ*cellindex + 0] ;
            }

            /* z front */
            z = xlength[2] ;
            for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
            {
                cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
                sendBuffer[5][5*(y*(xlength[0]+2)+x)+0] = collideField[PARAMQ*cellindex + 18] ;
                sendBuffer[5][5*(y*(xlength[0]+2)+x)+1] = collideField[PARAMQ*cellindex + 15] ;
                sendBuffer[5][5*(y*(xlength[0]+2)+x)+2] = collideField[PARAMQ*cellindex + 16] ;
                sendBuffer[5][5*(y*(xlength[0]+2)+x)+3] = collideField[PARAMQ*cellindex + 17] ;
                sendBuffer[5][5*(y*(xlength[0]+2)+x)+4] = collideField[PARAMQ*cellindex + 14] ;
            }
}

void decideneighbours(int*il,int*ir,int*jb,int*jt,int*kf,int*kb,int iProc,int jProc,int kProc,int rank,int *xlength)
{

   // left and right neighbour
   *il = rank - 1 ; *ir = rank + 1 ;
   if(rank%iProc==0)
   {
       *il = MPI_PROC_NULL ;
   }
   if(rank%iProc==iProc - 1)
   {
       *ir = MPI_PROC_NULL ;
   }
   
   // bottom and top neighbour
   *jb = -iProc + rank ; *jt = iProc+rank ;
   if(rank%(iProc*jProc)<iProc)
   {
       *jb = MPI_PROC_NULL ;
   }
   if((rank%(iProc*jProc))>(iProc*jProc - iProc - 1))
   {
       *jt = MPI_PROC_NULL ;
   }

   // back and front neighbour
   *kb = -(iProc*jProc) + rank ; *kf = (iProc*jProc) + rank ;
   if((rank%(iProc*jProc*kProc)<(iProc*jProc)))
   {
       *kb = MPI_PROC_NULL ;
   }
   if(((rank%(iProc*jProc*kProc))>(iProc*jProc*kProc - (iProc*jProc) - 1)))
   {
       *kf = MPI_PROC_NULL ;
   }
}

void swap_send_read( double**sendBuffer ,double**readBuffer ,int*xlength,int il,int ir,int jb,int jt,int kf,int kb,int myrank,MPI_Status *status)
{
   int size_x = 5 * ((xlength[1]+2)*(xlength[2]+2)),size_y=5*((xlength[0]+2)*(xlength[2]+2)),size_z=5*((xlength[0]+2)*(xlength[1]+2)) ;
   
   MPI_Send( sendBuffer[0], size_x, MPI_DOUBLE, ir, 1, MPI_COMM_WORLD );
   MPI_Recv( readBuffer[1], size_x, MPI_DOUBLE, il, 1, MPI_COMM_WORLD, status );
   MPI_Send( sendBuffer[1], size_x, MPI_DOUBLE, il, 1, MPI_COMM_WORLD );
   MPI_Recv( readBuffer[0], size_x, MPI_DOUBLE, ir, 1, MPI_COMM_WORLD, status );
   
   MPI_Send( sendBuffer[2], size_y, MPI_DOUBLE, jt, 1, MPI_COMM_WORLD );
   MPI_Recv( readBuffer[3], size_y, MPI_DOUBLE, jb, 1, MPI_COMM_WORLD, status );
   MPI_Send( sendBuffer[3], size_y, MPI_DOUBLE, jt, 1, MPI_COMM_WORLD );
   MPI_Recv( readBuffer[2], size_y, MPI_DOUBLE, jb, 1, MPI_COMM_WORLD, status );
   
   MPI_Send( sendBuffer[4], size_z, MPI_DOUBLE, kf, 1, MPI_COMM_WORLD );
   MPI_Recv( readBuffer[5], size_z, MPI_DOUBLE, kb, 1, MPI_COMM_WORLD, status );
   MPI_Send( sendBuffer[5], size_z, MPI_DOUBLE, kf, 1, MPI_COMM_WORLD );
   MPI_Recv( readBuffer[4], size_z, MPI_DOUBLE, kb, 1, MPI_COMM_WORLD, status );
}

void  inject(double**readBuffer,double*collideField,int*xlength)
{
    /* x right */
    int x , y , z , cellindex;
    x = xlength[0] + 1;
    for (z = 0; z <= xlength[2] + 1; z++)
    for (y = 0; y <= xlength[1] + 1; y++)
    {
        cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
        collideField[PARAMQ*cellindex + 13] = readBuffer[0][5*(z*(xlength[1]+2)+y)+0] ;
        collideField[PARAMQ*cellindex + 3] = readBuffer[0][5*(z*(xlength[1]+2)+y)+1] ;
        collideField[PARAMQ*cellindex + 10] = readBuffer[0][5*(z*(xlength[1]+2)+y)+2] ;
        collideField[PARAMQ*cellindex + 17] = readBuffer[0][5*(z*(xlength[1]+2)+y)+3] ;
        collideField[PARAMQ*cellindex + 7] = readBuffer[0][5*(z*(xlength[1]+2)+y)+4] ;
    }

    /* x left */
    x = 0;
    for (z = 0; z <= xlength[2] + 1; z++)
    for (y = 0; y <= xlength[1] + 1; y++)
    {
        cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
        collideField[PARAMQ*cellindex + 11] = readBuffer[1][5*(z*(xlength[1]+2)+y)+0] ;
        collideField[PARAMQ*cellindex + 1] = readBuffer[1][5*(z*(xlength[1]+2)+y)+1] ;
        collideField[PARAMQ*cellindex + 8] = readBuffer[1][5*(z*(xlength[1]+2)+y)+2] ;
        collideField[PARAMQ*cellindex + 15] = readBuffer[1][5*(z*(xlength[1]+2)+y)+3] ;
        collideField[PARAMQ*cellindex + 5] = readBuffer[1][5*(z*(xlength[1]+2)+y)+4] ;
    }

    /* y top */
    y = xlength[1] + 1;
    for (x = 0; x <= xlength[0] + 1; x++)
    for (z = 0; z <= xlength[2] + 1; z++)
    {
        cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
        collideField[PARAMQ*cellindex + 4] = readBuffer[2][5*(x*(xlength[2]+1)+z)+0] ;
        collideField[PARAMQ*cellindex + 11] = readBuffer[2][5*(x*(xlength[2]+1)+z)+1] ;
        collideField[PARAMQ*cellindex + 12] = readBuffer[2][5*(x*(xlength[2]+1)+z)+2] ;
        collideField[PARAMQ*cellindex + 13] = readBuffer[2][5*(x*(xlength[2]+1)+z)+3] ;
        collideField[PARAMQ*cellindex + 18] = readBuffer[2][5*(x*(xlength[2]+1)+z)+4] ;
    }

    /* y bottom */
    y = 0;
    for (x = 0; x <= xlength[0] + 1; x++)
    for (z = 0; z <= xlength[2] + 1; z++)
    {
        cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
        collideField[PARAMQ*cellindex + 0] = readBuffer[3][5*(x*(xlength[2]+2)+z)+0] ;
        collideField[PARAMQ*cellindex + 5] = readBuffer[3][5*(x*(xlength[2]+2)+z)+1] ;
        collideField[PARAMQ*cellindex + 6] = readBuffer[3][5*(x*(xlength[2]+2)+z)+2] ;
        collideField[PARAMQ*cellindex + 7] = readBuffer[3][5*(x*(xlength[2]+2)+z)+3] ;
        collideField[PARAMQ*cellindex + 14] = readBuffer[3][5*(x*(xlength[2]+2)+z)+4] ;
    }

    /* z back */
    z = 0;
    for (y = 0; y <= xlength[1] + 1; y++)
    for (x = 0; x <= xlength[0] + 1; x++)
    {
        cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
        collideField[PARAMQ*cellindex + 4] = readBuffer[4][5*(y*(xlength[0]+2)+x)+0] ;
        collideField[PARAMQ*cellindex + 1] = readBuffer[4][5*(y*(xlength[0]+2)+x)+1] ;
        collideField[PARAMQ*cellindex + 2] = readBuffer[4][5*(y*(xlength[0]+2)+x)+2] ;
        collideField[PARAMQ*cellindex + 3] = readBuffer[4][5*(y*(xlength[0]+2)+x)+3] ;
        collideField[PARAMQ*cellindex + 0] = readBuffer[4][5*(y*(xlength[0]+2)+x)+4] ;
    }

    /* z front */
    z = xlength[2] + 1;
    for (y = 0; y <= xlength[1] + 1; y++)
    for (x = 0; x <= xlength[0] + 1; x++)
    {
        cellindex = z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x ;
        collideField[PARAMQ*cellindex + 18] = readBuffer[5][5*(y*(xlength[0]+2)+x)+0] ;
        collideField[PARAMQ*cellindex + 15] = readBuffer[5][5*(y*(xlength[0]+2)+x)+1] ;
        collideField[PARAMQ*cellindex + 16] = readBuffer[5][5*(y*(xlength[0]+2)+x)+2] ;
        collideField[PARAMQ*cellindex + 17] = readBuffer[5][5*(y*(xlength[0]+2)+x)+3] ;
        collideField[PARAMQ*cellindex + 14] = readBuffer[5][5*(y*(xlength[0]+2)+x)+4] ;
    }
}
