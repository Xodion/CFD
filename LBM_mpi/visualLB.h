#ifndef _VISUALLB_H_
#define _VISUALLB_H_

/** writes the density and velocity field (derived from the distributions in collideField)
 *  to a file determined by 'filename' and timestep 't'. You can re-use parts of the code
 *  from visual.c (VTK output for Navier-Stokes solver) and modify it for 3D datasets.
 */
void writeVtkOutput(const double * const collideField, const int * const flagField, const char * filename, unsigned int t, int *xlength, int * local_xlength, int rank, int iCoord, int jCoord, int kCoord, int iProc, int jProc, int kProc);
void write_vtkPointCoordinates( FILE *fp, int *xlength, int * local_xlength, int iCoord, int jCoord, int kCoord, int iProc, int jProc, int kProc);
void write_vtkHeader( FILE *fp, int * local_xlength);


#endif

