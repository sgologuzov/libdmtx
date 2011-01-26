/*
libdmtx - Data Matrix Encoding/Decoding Library

Copyright (C) 2008, 2009, 2010 Mike Laughton

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------------
Portions of this file were derived from the Reed-Solomon encoder/decoder
released by Simon Rockliff in June 1991. It has been modified to include
only Data Matrix conditions and to integrate seamlessly with libdmtx.
--------------------------------------------------------------------------

Contact: mike@dragonflylogic.com
*/

/* $Id$ */

#define NN                      255
#define MAX_ERROR_WORD_COUNT     68
#define MAX_SYNDROME_COUNT       69

/* GF(256) log values using primitive polynomial 301 */
static unsigned char log301[] =
   { 255,   0,   1, 240,   2, 225, 241,  53,   3,  38, 226, 133, 242,  43,  54, 210,
       4, 195,  39, 114, 227, 106, 134,  28, 243, 140,  44,  23,  55, 118, 211, 234,
       5, 219, 196,  96,  40, 222, 115, 103, 228,  78, 107, 125, 135,   8,  29, 162,
     244, 186, 141, 180,  45,  99,  24,  49,  56,  13, 119, 153, 212, 199, 235,  91,
       6,  76, 220, 217, 197,  11,  97, 184,  41,  36, 223, 253, 116, 138, 104, 193,
     229,  86,  79, 171, 108, 165, 126, 145, 136,  34,   9,  74,  30,  32, 163,  84,
     245, 173, 187, 204, 142,  81, 181, 190,  46,  88, 100, 159,  25, 231,  50, 207,
      57, 147,  14,  67, 120, 128, 154, 248, 213, 167, 200,  63, 236, 110,  92, 176,
       7, 161,  77, 124, 221, 102, 218,  95, 198,  90,  12, 152,  98,  48, 185, 179,
      42, 209,  37, 132, 224,  52, 254, 239, 117, 233, 139,  22, 105,  27, 194, 113,
     230, 206,  87, 158,  80, 189, 172, 203, 109, 175, 166,  62, 127, 247, 146,  66,
     137, 192,  35, 252,  10, 183,  75, 216,  31,  83,  33,  73, 164, 144,  85, 170,
     246,  65, 174,  61, 188, 202, 205, 157, 143, 169,  82,  72, 182, 215, 191, 251,
      47, 178,  89, 151, 101,  94, 160, 123,  26, 112, 232,  21,  51, 238, 208, 131,
      58,  69, 148,  18,  15,  16,  68,  17, 121, 149, 129,  19, 155,  59, 249,  70,
     214, 250, 168,  71, 201, 156,  64,  60, 237, 130, 111,  20,  93, 122, 177, 150 };

/* GF(256) antilog values using primitive polynomial 301 */
static unsigned char antilog301[] =
   {   1,   2,   4,   8,  16,  32,  64, 128,  45,  90, 180,  69, 138,  57, 114, 228,
     229, 231, 227, 235, 251, 219, 155,  27,  54, 108, 216, 157,  23,  46,  92, 184,
      93, 186,  89, 178,  73, 146,   9,  18,  36,  72, 144,  13,  26,  52, 104, 208,
     141,  55, 110, 220, 149,   7,  14,  28,  56, 112, 224, 237, 247, 195, 171, 123,
     246, 193, 175, 115, 230, 225, 239, 243, 203, 187,  91, 182,  65, 130,  41,  82,
     164, 101, 202, 185,  95, 190,  81, 162, 105, 210, 137,  63, 126, 252, 213, 135,
      35,  70, 140,  53, 106, 212, 133,  39,  78, 156,  21,  42,  84, 168, 125, 250,
     217, 159,  19,  38,  76, 152,  29,  58, 116, 232, 253, 215, 131,  43,  86, 172,
     117, 234, 249, 223, 147,  11,  22,  44,  88, 176,  77, 154,  25,  50, 100, 200,
     189,  87, 174, 113, 226, 233, 255, 211, 139,  59, 118, 236, 245, 199, 163, 107,
     214, 129,  47,  94, 188,  85, 170, 121, 242, 201, 191,  83, 166,  97, 194, 169,
     127, 254, 209, 143,  51, 102, 204, 181,  71, 142,  49,  98, 196, 165, 103, 206,
     177,  79, 158,  17,  34,  68, 136,  61, 122, 244, 197, 167,  99, 198, 161, 111,
     222, 145,  15,  30,  60, 120, 240, 205, 183,  67, 134,  33,  66, 132,  37,  74,
     148,   5,  10,  20,  40,  80, 160, 109, 218, 153,  31,  62, 124, 248, 221, 151,
       3,   6,  12,  24,  48,  96, 192, 173, 119, 238, 241, 207, 179,  75, 150,   0 };

/* GF add (a + b) */
#define GfAdd(a,b) \
   ((a) ^ (b))

/* GF multiply (a * b) */
#define GfMult(a,b) \
   (((a) == 0 || (b) == 0) ? 0 : antilog301[(log301[(a)] + log301[(b)]) % NN])

/* GF multiply by antilog (a * alpha**b) */
#define GfMultAntilog(a,b) \
   (((a) == 0) ? 0 : antilog301[(log301[(a)] + (b)) % NN])

/**
 *
 *
 */
static DmtxPassFail
RsEncode(DmtxMessage *message, int sizeIdx)
{
   int i, j;
   int blockStride, blockIdx;
   int blockErrorWords, symbolDataWords, symbolErrorWords, symbolTotalWords;
   DmtxByte val, *eccPtr;
   DmtxByte genStorage[MAX_ERROR_WORD_COUNT];
   DmtxByte eccStorage[MAX_ERROR_WORD_COUNT];
   DmtxByteList gen, ecc;

   gen = dmtxByteListBuild(genStorage, sizeof(genStorage));
   ecc = dmtxByteListBuild(eccStorage, sizeof(eccStorage));

   blockStride = dmtxGetSymbolAttribute(DmtxSymAttribInterleavedBlocks, sizeIdx);
   blockErrorWords = dmtxGetSymbolAttribute(DmtxSymAttribBlockErrorWords, sizeIdx);
   symbolDataWords = dmtxGetSymbolAttribute(DmtxSymAttribSymbolDataWords, sizeIdx);
   symbolErrorWords = dmtxGetSymbolAttribute(DmtxSymAttribSymbolErrorWords, sizeIdx);
   symbolTotalWords = symbolDataWords + symbolErrorWords;

   /* Populate the generator polynomial */
   RsGenPoly(&gen, blockErrorWords);

   /* Generate error codewords for all interleaved blocks */
   for(blockIdx = 0; blockIdx < blockStride; blockIdx++)
   {
      /* Generate */
      dmtxByteListInit(&ecc, blockErrorWords, 0);
      for(i = blockIdx; i < symbolDataWords; i += blockStride)
      {
         val = GfAdd(ecc.b[blockErrorWords-1], message->code[i]);

         for(j = blockErrorWords - 1; j > 0; j--)
         {
            DMTX_CHECK_BOUNDS(&ecc, j); DMTX_CHECK_BOUNDS(&ecc, j-1); DMTX_CHECK_BOUNDS(&gen, j);
            ecc.b[j] = GfAdd(ecc.b[j-1], GfMult(gen.b[j], val));
         }

         ecc.b[0] = GfMult(gen.b[0], val);
      }

      /* Copy to output message */
      eccPtr = ecc.b + blockErrorWords;
      for(i = blockIdx + symbolDataWords; i < symbolTotalWords; i += blockStride)
         message->code[i] = *(--eccPtr);

      assert(ecc.b == eccPtr);
   }

   return DmtxPass;
}

/**
 *
 *
 */
static DmtxPassFail
RsDecode(unsigned char *code, int sizeIdx, int fix)
{
   int i;
   int blockStride, blockIdx;
   int blockErrorWords, blockTotalWords, blockMaxCorrectable;
   DmtxBoolean error, solvable;
   DmtxByteList rec, syn, elp, loc;
   DmtxByte elpStorage[MAX_ERROR_WORD_COUNT];
   DmtxByte synStorage[MAX_SYNDROME_COUNT];
   DmtxByte recStorage[NN];
   DmtxByte locStorage[NN];

   rec = dmtxByteListBuild(recStorage, sizeof(recStorage));
   syn = dmtxByteListBuild(synStorage, sizeof(synStorage));
   elp = dmtxByteListBuild(elpStorage, sizeof(elpStorage));
   loc = dmtxByteListBuild(locStorage, sizeof(locStorage));

   blockStride = dmtxGetSymbolAttribute(DmtxSymAttribInterleavedBlocks, sizeIdx);
   blockErrorWords = dmtxGetSymbolAttribute(DmtxSymAttribBlockErrorWords, sizeIdx);
   blockMaxCorrectable = dmtxGetSymbolAttribute(DmtxSymAttribBlockMaxCorrectable, sizeIdx);

   for(blockIdx = 0; blockIdx < blockStride; blockIdx++)
   {
      /* Need to query at block level because of special case at 144x144 */
      blockTotalWords = blockErrorWords + dmtxGetBlockDataSize(sizeIdx, blockIdx);

      /* Populate rec with data and error codewords */
      dmtxByteListInit(&rec, blockTotalWords, 0);
      for(i = 0; i < rec.length; i++)
         rec.b[i] = code[blockIdx + blockStride * (blockTotalWords - 1 - i)];

      /* Calculate syndrome (syn) */
      error = RsCalcSyndrome(&syn, &rec, blockErrorWords);

      /* Error(s) detected: Attempt to repair */
      if(error)
      {
         /* Populates elp, lam */
         solvable = RsFindErrorLocatorPoly(&elp, &syn, blockErrorWords, blockMaxCorrectable);
         if(solvable == DmtxFalse)
            return DmtxFail;

         /* Populates loc */
         solvable = RsFindErrorPositions(&loc, &elp, 0 /* lam[iNext] */, blockMaxCorrectable);
         if(solvable == DmtxFalse)
            return DmtxFail;

         /* Repairs rec */
         RsFindErrorValues(&rec, &loc, &elp, &syn);
      }

      /* Write correct/corrected values to output */
      /* for() ... */
   }

   return DmtxPass;
}

/**
 *
 *
 */
static DmtxPassFail
RsGenPoly(DmtxByteList *gen, int errorWordCount)
{
   int i, j;

   /* Initialize all coefficients to 1 */
   dmtxByteListInit(gen, errorWordCount, 1);

   /* Generate polynomial */
   for(i = 0; i < gen->length; i++)
   {
      for(j = i; j >= 0; j--)
      {
         gen->b[j] = GfMultAntilog(gen->b[j], i+1);
         if(j > 0)
            gen->b[j] = GfAdd(gen->b[j], gen->b[j-1]);
      }
   }

   return DmtxPass;
}

/**
 * Assume we have received bits grouped into mm-bit symbols in rec[i],
 * i=0..(nn-1),  and rec[i] is index form (ie as powers of alpha). We first
 * compute the 2*tt syndromes by substituting alpha**i into rec(X) and
 * evaluating, storing the syndromes in syn[i], i=1..2tt (leave syn[0] zero).
 */
static DmtxBoolean
RsCalcSyndrome(DmtxByteList *syn, const DmtxByteList *rec, int blockErrorWords)
{
   int i, j;
   DmtxBoolean error = DmtxFalse;

   /* Initialize all coefficients to 0 */
   dmtxByteListInit(syn, blockErrorWords + 1, 0);

   for(i = 1; i < syn->length; i++) /* blockErrorWords + 1 */
   {
      /* Calculate syndrome at i */
      for(j = 0; j < rec->length; j++) /* blockTotalWords */
         syn->b[i] = GfAdd(syn->b[i], GfMultAntilog(rec->b[j], i*j));

      /* Non-zero syndrome indicates error */
      if(syn->b[i] != 0)
         error = DmtxTrue;
   }

   return error;
}

/**
 * @brief Find the error location polynomial using Berlekamp-Massey.
 * @return DmtxPass if degree of elp is <= maxCorrectable errors. Otherwise
 *         return DmtxFail because uncorrectable errors are present.
 */
static DmtxBoolean
RsFindErrorLocatorPoly(DmtxByteList *elp, const DmtxByteList *syn, int errorWordCount, int maxCorrectable)
{
   int i, iNext, j;
   int m, mCmp;
   int dis[MAX_ERROR_WORD_COUNT] = { 0 };
   int lam[MAX_ERROR_WORD_COUNT+2] = { 0 };
   unsigned char elpX[MAX_ERROR_WORD_COUNT+2][MAX_ERROR_WORD_COUNT] = {{ 0 }};

   /* iNext = 0 */
   elpX[0][0] = 1;
   lam[0] = 0;
   dis[0] = 1;

   /* iNext = 1 */
   elpX[1][0] = 1;
   lam[1] = 0;
   dis[1] = syn->b[1];

   for(iNext = 2, i = 1; /* explicit break below */ ; i = iNext++)
   {
      if(dis[i] == 0)
      {
         /* Simple case: Copy directly from previous iteration */
         /* dmtxByteListCopy() */
         memcpy(elpX[iNext], elpX[i], sizeof(unsigned char) * MAX_ERROR_WORD_COUNT);
         lam[iNext] = lam[i];
      }
      else
      {
         /* Find earlier iteration (m) that provides maximal (m - lam[m]) */
         for(m = 0, mCmp = 1; mCmp < i; mCmp++)
            if(dis[mCmp] != 0 && (mCmp - lam[mCmp]) > (m - lam[m]))
               m = mCmp;

         /* Calculate error location polynomial elpX[i] (set 1st term) */
         for(j = 0; j < lam[m]; j++)
            elpX[iNext][j+i-m] = antilog301[(log301[dis[i]] - log301[dis[m]] + NN + elpX[m][j]) % NN];

         /* Calculate error location polynomial elpX[i] (add 2nd term) */
         for(j = 0; j < lam[i]; j++)
            elpX[iNext][j] = GfAdd(elpX[iNext][j], elpX[i][j]);

         /* Record lambda: lam[m] + iEqn - mEqn = lam[m] + (i-1) - (m-1) */
         lam[iNext] = max(lam[i], lam[m] + i - m);
      }

      if(i == errorWordCount || i >= lam[iNext] + maxCorrectable)
         break;

      /* Calculate discrepancy dis[i] */
      for(j = 0, dis[iNext] = syn->b[iNext]; j < lam[iNext]; j++)
         dis[iNext] = GfAdd(dis[iNext], GfMult(syn->b[iNext-j-1], elpX[iNext][j]));
   }

   return (lam[iNext] > maxCorrectable) ? DmtxFalse : DmtxTrue;
}

/**
 * Find roots of the error locator polynomial (Chien Search)
 *
 * If the degree of elp is <= tt, we substitute alpha**i, i=1..n into the elp
 * to get the roots, hence the inverse roots, the error location numbers.
 *
 * If the number of errors located does not equal the degree of the elp, we
 * have more than tt errors and cannot correct them.
 */
static DmtxBoolean
RsFindErrorPositions(DmtxByteList *loc, const DmtxByteList *elp, int lam, int maxCorrectable)
{
   int i, j, count = 0;
   unsigned char q, reg[MAX_ERROR_WORD_COUNT];

   dmtxByteListInit(loc, 0, 0);

   /* XXX use dmtxByteListCopy() instead */
   memcpy(reg, elp, sizeof(unsigned char) * MAX_ERROR_WORD_COUNT);

   for(i = 1; i <= NN; i++)
   {
      for(q = 1, j = 1; j <= lam; j++)
         q = GfAdd(q, GfMultAntilog(reg[j], j)); /* XXX reg is just a local copy of rec[] ... name it better */

      if(q == 0)
         dmtxByteListPush(loc, NN - i);
   }

   /* Number of roots != degree of elp => >tt errors and cannot solve */
   return (count != lam) ? DmtxFalse : DmtxTrue;
}

/**
 * Find the error values
 *
 * Solve for the error value at the error location and correct the error. The
 * procedure is that found in Lin and Costello.
 *
 * For the cases where the number of errors is known to be too large to
 * correct, the information symbols as received are output (the advantage of
 * systematic encoding is that hopefully some of the information symbols will
 * be okay and that if we are in luck, the errors are in the parity part of
 * the transmitted codeword).
 */
static DmtxBoolean
RsFindErrorValues(DmtxByteList *rec, const DmtxByteList *loc, const DmtxByteList *elp, const DmtxByteList *syn)
{
   int i, j, q;
   int lam;
   DmtxByte z[1]; /* XXX replace with DmtxByteList? */
   DmtxByte root, err;

   /* Form polynomial z(x) */
   for(z[0] = 1, i = 1; i <= lam; i++)
      for(z[i] = GfAdd(syn->b[i], elp->b[i]), j = 1; j < i; j++)
         z[i] = GfAdd(z[i], GfMult(elp->b[i-j], syn->b[j]));

   for(i = 0; i < lam; i++)
   {
      /* Calculate numerator of error term */
      root = NN - loc->b[i];

      for(err = 1, j = 1; j <= lam; j++)
         err = GfAdd(err, GfMultAntilog(z[j], j * root));

      if(err == 0)
         continue;

      /* Calculate denominator of error term */
      for(q = 0, j = 0; j < lam; j++)
      {
         if(j != i)
            q += log301[1 ^ antilog301[(loc->b[j] + root) % NN]];
      }
      q %= NN;

      err = GfMult(err, NN - q);
      rec->b[loc->b[i]] = GfAdd(rec->b[loc->b[i]], err);
   }

   return DmtxTrue;
}
