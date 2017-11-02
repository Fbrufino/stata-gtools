#define BaseCompareNum(a, b) ( ( (a) > (b) ) - ( (a) < (b) ) )
#define BaseCompareChar(a, b) ( strcmp(a, b) )

/*********************************************************************
 *                              Doubles                              *
 *********************************************************************/

int MultiCompareNum2 (const void *a, const void *b, void *thunk);
int MultiCompareNum2 (const void *a, const void *b, void *thunk)
{
    GT_size kstart = *(GT_size *)thunk;
    ST_double aa   = *((ST_double *)a + kstart);
    ST_double bb   = *((ST_double *)b + kstart);
    return BaseCompareNum(aa, bb);
}

int MultiCompareNum2Invert (const void *a, const void *b, void *thunk);
int MultiCompareNum2Invert (const void *a, const void *b, void *thunk)
{
    GT_size kstart = *(GT_size *)thunk;
    ST_double aa   = *((ST_double *)a + kstart);
    ST_double bb   = *((ST_double *)b + kstart);
    return BaseCompareNum(bb, aa);
}

/*********************************************************************
 *                       Mixed Character Array                       *
 *********************************************************************/

int AltCompareChar (const void *a, const void *b, void *thunk);
int AltCompareChar (const void *a, const void *b, void *thunk)
{
    GT_size kstart = *(GT_size *)thunk;
    char *aa = (char *)(a + kstart);
    char *bb = (char *)(b + kstart);
// printf("\tcmp(%s, %s) = %d\n", aa, bb, BaseCompareChar(aa, bb));
    return BaseCompareChar(aa, bb);
}

int AltCompareCharInvert (const void *a, const void *b, void *thunk);
int AltCompareCharInvert (const void *a, const void *b, void *thunk)
{
    GT_size kstart = *(GT_size *)thunk;
    char *aa = (char *)(a + kstart);
    char *bb = (char *)(b + kstart);
// printf("cmp(%s, %s) = %d\n", bb, aa, BaseCompareChar(bb, aa));
    return BaseCompareChar(bb, aa);
}

int AltCompareNum (const void *a, const void *b, void *thunk);
int AltCompareNum (const void *a, const void *b, void *thunk)
{
    GT_size kstart = *(GT_size *)thunk;
    ST_double aa   = *(ST_double *)(a + kstart);
    ST_double bb   = *(ST_double *)(b + kstart);
// printf("\tcmp(%.4f, %.4f) = %d\n", aa, bb, BaseCompareNum(aa, bb));
    return BaseCompareNum(aa, bb);
}

int AltCompareNumInvert (const void *a, const void *b, void *thunk);
int AltCompareNumInvert (const void *a, const void *b, void *thunk)
{
    GT_size kstart = *(GT_size *)thunk;
    ST_double aa   = *(ST_double *)(a + kstart);
    ST_double bb   = *(ST_double *)(b + kstart);
// printf("\tcmp(%.4f, %.4f) = %d\n", bb, aa, BaseCompareNum(bb, aa));
    return BaseCompareNum(bb, aa);
}

/*********************************************************************
 *                  Hashed 64-bit array with index                   *
 *********************************************************************/

int CompareSpooky (const void *a, const void *b, void *thunk);
int CompareSpooky (const void *a, const void *b, void *thunk)
{
    GT_size kstart = *(GT_size *)thunk;
    uint64_t aa = *(uint64_t *)(a + kstart);
    uint64_t bb = *(uint64_t *)(b + kstart);
    return BaseCompareNum(aa, bb);
    // return BaseCompareNum(*(uint64_t *)a, *(uint64_t *)b);
}
