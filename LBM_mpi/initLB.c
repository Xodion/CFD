#include "initLB.h"
#include "LBDefinitions.h"

int readParameters(int *xlength, double *tau, double *bddParams, int *iProc, int *jProc, int *kProc,  int *timesteps, int *timestepsPerPlotting, char *problem, char *pgmInput, int argc, char *argv[])
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
        read_int(argv[1], "kProc" , kProc);
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


void initialiseFields(double *collideField, double *streamField, int *flagField, int *xlength, char *problem, char* pgmInput, int rank, int iProc, int jProc, int kProc)
{
    int i, j, k, x, y, z;
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


    // independend of the problem first initialize all ghost cells as PARALLEL_BOUNDARY
    {
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
    }

    /** initialization of different scenarios */
    // only the drivenCavity scenario is adjusted for the parallelized version

    i = rank % iProc;
    j = ((rank - i) / iProc) % jProc;
    k = (rank - i - iProc * j) / (iProc * jProc);

    if (!strcmp(problem, "drivenCavity"))
    {
        /* top boundary */
        y = xlength[1] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                if (j == jProc - 1)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = MOVING_WALL;


        /* back boundary */
        z = 0;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                if (k == 0)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* bottom boundary */
        y = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (x = 0; x <= xlength[0] + 1; x++)
                if (j == 0)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;


        /* left boundary */
        x = 0;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                if (i == 0)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* right boundary */
        x = xlength[0] + 1;
        for (z = 0; z <= xlength[2] + 1; z++)
            for (y = 0; y <= xlength[1] + 1; y++)
                if (i == iProc - 1)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;

        /* front boundary, i.e. z = xlength + 1 */
        z = xlength[2] + 1;
        for (y = 0; y <= xlength[1] + 1; y++)
            for (x = 0; x <= xlength[0] + 1; x++)
                if (k == kProc - 1)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = NO_SLIP;
    }

    if (!strcmp(problem, "tiltedPlate"))
    {
        int** pgmImage;
        pgmImage = read_pgm(pgmInput);
        for (z = 1; z <= xlength[2]; z++)
            for (y = 1; y <= xlength[1]; y++)
                for (x = 1; x <= xlength[0]; x++)
                    flagField[z * (xlength[0] + 2) * (xlength[1] + 2) + y * (xlength[0] + 2) + x] = pgmImage[x][y];
        free_imatrix(pgmImage, 0, xlength[0] + 2, 0, xlength[1] + 2);

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

