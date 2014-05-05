#include "visualLB.h"
#include "computeCellValues.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void writeVtkOutput(const double * const collideField, const int * const flagField, const char * filename, unsigned int t, int xlength) {
  int i,j,k,index;
  char szFileName[80];
  double velocity[3], density[xlength][xlength][xlength];
  int dim = xlength + 2;
  int dim_pow2 = dim * dim;
  FILE *fp=NULL;
  sprintf( szFileName, "%s.%i.vtk", filename, t );
  fp = fopen( szFileName, "w");
  if( fp == NULL )		       
  {
    char szBuff[80];
    sprintf( szBuff, "Failed to open %s", szFileName );
    perror( szBuff );
    return;
  }

  write_vtkHeader( fp, xlength );
  write_vtkPointCoordinates(fp, xlength );

  fprintf(fp,"POINT_DATA %i \n", xlength * xlength * xlength );


  fprintf(fp,"\n");
  fprintf(fp, "VECTORS velocity float\n");
  for(k = 1; k < xlength+1; k++)
	  for(j = 1; j < xlength+1; j++) {
	     for(i = 1; i < xlength+1; i++) {
	     	index = 19 * (k*dim_pow2 + j*dim + i);
	     	computeDensity(&collideField[index], &density[i-1][j-1][k-1] );
	     	computeVelocity(&collideField[index], &density[i-1][j-1][k-1], velocity);	

	        fprintf(fp, "%f %f %f\n", velocity[0], velocity[1], velocity[2] );
	     }
  }

  fprintf(fp,"\n");
  fprintf(fp,"CELL_DATA %i \n", xlength * xlength * xlength );
  fprintf(fp, "SCALARS density float 1 \n"); 
  fprintf(fp, "LOOKUP_TABLE default \n");
  for(k = 0; k < xlength; k++ )
	  for(j = 0; j < xlength; j++) {
	    for(i = 0; i < xlength; i++) {
	      fprintf(fp, "%f\n", density[i][j][k] );
	    }
  }

  if( fclose(fp) )
  {
    char szBuff[80];
    sprintf( szBuff, "Failed to close %s", szFileName );
    perror( szBuff );
  }
}

void write_vtkHeader( FILE *fp, int xlength) {
  if( fp == NULL )		       
  {
    char szBuff[80];
    sprintf( szBuff, "Null pointer in write_vtkHeader" );
    perror( szBuff );
    return;
  }

  fprintf(fp,"# vtk DataFile Version 2.0\n");
  fprintf(fp,"generated by CFD-lab course output (written by Ye) \n");
  fprintf(fp,"ASCII\n");
  fprintf(fp,"\n");	
  fprintf(fp,"DATASET STRUCTURED_GRID\n");
  fprintf(fp,"DIMENSIONS  %i %i %i \n", xlength, xlength, xlength);
  fprintf(fp,"POINTS %i float\n", xlength * xlength * xlength );
  fprintf(fp,"\n");
}


void write_vtkPointCoordinates( FILE *fp, int xlength) {
  double originX = 0.0;  
  double originY = 0.0;
  double originZ = 0.0;

  int i = 0;
  int j = 0;
  int k = 0;

for(k = 0; k < xlength; k++)
  for(j = 0; j < xlength; j++) {
    for(i = 0; i < xlength; i++){
      fprintf(fp, "%f %f %f\n", originX+i, originY+j, originZ+k );
    }
  }
}

