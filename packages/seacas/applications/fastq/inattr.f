C    Copyright (c) 2014, Sandia Corporation.
C    Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
C    the U.S. Government retains certain rights in this software.
C    
C    Redistribution and use in source and binary forms, with or without
C    modification, are permitted provided that the following conditions are
C    met:
C    
C        * Redistributions of source code must retain the above copyright
C          notice, this list of conditions and the following disclaimer.
C    
C        * Redistributions in binary form must reproduce the above
C          copyright notice, this list of conditions and the following
C          disclaimer in the documentation and/or other materials provided
C          with the distribution.
C    
C        * Neither the name of Sandia Corporation nor the names of its
C          contributors may be used to endorse or promote products derived
C          from this software without specific prior written permission.
C    
C    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
C    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
C    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
C    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
C    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
C    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
C    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
C    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
C    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
C    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
C    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
C    

C $Id: inattr.f,v 1.1 1990/11/30 11:09:14 gdsjaar Exp $
C $Log: inattr.f,v $
C Revision 1.1  1990/11/30 11:09:14  gdsjaar
C Initial revision
C
C
CC* FILE: [.MAIN]INATTR.FOR
CC* MODIFIED BY: TED BLACKER
CC* MODIFICATION DATE: 7/6/90
CC* MODIFICATION: COMPLETED HEADER INFORMATION
C
      SUBROUTINE INATTR (MS, MR, MA, N17, N23, JJ, RIN, IFOUND, ATTRIB,
     &   LINKM, NOROOM)
C***********************************************************************
C
C  SUBROUTINE INATTR = INPUTS MATERIAL ATTRIBUTES INTO THE DATABASE
C
C***********************************************************************
C
      DIMENSION ATTRIB (MA, MR+MS), LINKM (2,  (MS+MR))
C
      LOGICAL NOROOM, ADDLNK
C
      NOROOM = .TRUE.
      ADDLNK = .FALSE.
C
C  UPDATE THE COUNTER IF NEEDED
C
      IF (JJ.GT.N23)N23 = JJ
C
C  ADD THE ATTRIBUTES INTO THE DATABASE
C
      N17 = N17 + 1
      J = N17
      IF (J .GT. (MS + MR))RETURN
      CALL LTSORT (MS + MR, LINKM, JJ, IPNTR, ADDLNK)
      ADDLNK = .TRUE.
      IF (IPNTR .LE. 0)THEN
         J = -J
         MINUSJ  =  -J
         CALL LTSORT (MS + MR, LINKM, JJ, MINUSJ, ADDLNK)
         J = IABS (MINUSJ)
      ELSE
         CALL LTSORT (MS + MR, LINKM, JJ, J, ADDLNK)
      ENDIF
      IF (IFOUND .GT. MA)THEN
         IEND = MA
         WRITE (*, 10000)IFOUND, MA
      ELSE
         IEND = IFOUND
      ENDIF
      DO 100 I = 1, IEND
         ATTRIB (J, I) = RIN (I)
  100 CONTINUE
C
      NOROOM = .FALSE.
C
      RETURN
C
10000 FORMAT (' FOR MATERIAL NUMBER:', I5,
     &   ' NUMBER OF ATTRIBUTES READ:',  I5, /,
     &   '                               EXCEEDS MAX ALLOWED OF:',
     &   I5)
C
      END
