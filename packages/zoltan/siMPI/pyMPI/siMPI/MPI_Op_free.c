/*****************************************************************************
 * CVS File Information :
 *    $RCSfile$
 *    Author: patmiller $
 *    Date: 2007/06/11 14:12:51 $
 *    Revision: 1.2 $
 ****************************************************************************/
/****************************************************************************/
/* FILE  ******************     MPI_Op_free.c        ************************/
/****************************************************************************/
/* Author : Lisa Alano July 8 2002                                          */
/* Copyright (c) 2002 University of California Regents                      */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

/*==========================================================================*/
int MPI_Op_free( MPI_Op *op )
{
  _MPI_COVERAGE();
  return PMPI_Op_free(op);
}
/*==========================================================================*/
