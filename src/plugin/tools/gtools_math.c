#define SQUARE(x) ( (x) * (x) )
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
#define MIN(a, b) ( (a) > (b) ? (b) : (a) )

#define MAX_MATCHES 1

#include "gtools_math.h"
#include "qselect.c"

/**
 * @brief Standard deviation entries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Standard deviation of the elements of @v from @start to @end
 */
double mf_array_dsd_range (const double v[], const size_t start, const size_t end) {
    size_t i;
    double vvar  = 0;
    double vmean = mf_array_dmean_range(v, start, end);
    for (i = start; i < end; i++)
        vvar += SQUARE(v[i] - vmean);
    return (sqrt(vvar / (end - start - 1)));
}

/**
 * @brief Mean of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Mean of the elements of @v from @start to @end
 */
double mf_array_dmean_range (const double v[], const size_t start, const size_t end) {
    return (mf_array_dsum_range(v, start, end) / (end - start));
}

/**
 * @brief Sum of entries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Mean of the elements of @v from @start to @end
 */
double mf_array_dsum_range (const double v[], const size_t start, const size_t end)
{
    size_t i;
    double vsum = 0;
    for (i = start; i < end; i++)
        vsum += v[i];
    return (vsum);
}

/**
 * @brief Min of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Min of the elements of @v from @start to @end
 */
double mf_array_dmin_range (const double v[], const size_t start, const size_t end)
{
    size_t i;
    double min = v[start];
    for (i = start + 1; i < end; ++i) {
        if (min > v[i]) min = v[i];
    }
    return (min);
}

/**
 * @brief Max of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Max of the elements of @v from @start to @end
 */
double mf_array_dmax_range (const double v[], const size_t start, const size_t end)
{
    size_t i;
    double max = v[start];
    for (i = start + 1; i < end; ++i) {
        if (max < v[i]) max = v[i];
    }
    return (max);
}

/**
 * @brief Quantile of enries in range of array
 *
 * This computes the (quantile)th quantile using qsort. When
 * computing multiple quantiles, the data will already be sorted for the
 * next iteration, so it's faster than sorting every time, but it it
 * still a VERY inefficient implementation.
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Quantile of the elements of @v from @start to @end
 */
double mf_array_dquantile_range (double v[], const size_t start, const size_t end, const double quantile)
{
    size_t N   = end - start;
    size_t qth = floor(quantile * N / 100);

    // Special cases
    // -------------

    if ( N == 1 ) {
        // If only 1 entry, can't take quantile
        return (v[start]);
    }
    else if ( N == 2 ) {
        // If 2 entries, only 3 options
        if ( quantile > 50 ) {
            return (MAX(v[start], v[end - 1]));
        }
        else if ( quantile < 50 ) {
            return (MIN(v[start], v[end - 1]));
        }
        else {
            return ( (v[start] + v[end - 1]) / 2 );
        }
    }
    else if ( qth == 0 ) {
        // 0th quantile is not a thing, so we can just take the min
        return (mf_array_dmin_range(v, start, end));
    }

    // Full selection algorithm
    // ------------------------

    size_t left = start, right = end;
    int dmax = ( qth == (N - 1) );
    double q = dmax? mf_array_dmax_range(v, left, right): mf_qselect_range (v, left, right, qth);
    if ( (double) qth == (quantile * N / 100) ) {
        q += mf_qselect_range (v, left, right, qth - 1);
        q /= 2;
    }
    return (q);
}

/**
 * @brief Median of enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Median of the elements of @v from @start to @end
 */
double mf_array_dmedian_range (double v[], const size_t start, const size_t end)
{
    return (mf_array_dquantile_range(v, start, end, 50));
}

/**
 * @brief IRQ for enries in range of array
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return IRQ for the elements of @v from @start to @end
 */
double mf_array_diqr_range (double v[], const size_t start, const size_t end)
{
    return (mf_array_dquantile_range(v, start, end, 75) - mf_array_dquantile_range(v, start, end, 25));
}

/**
 * @brief Wrapper to choose summary function using a string
 *
 * @param fname Character with name of funtion to apply to @v
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return @fname(@v[@start to @end])
 */
double mf_switch_fun (char * fname, double v[], const size_t start, const size_t end)
{
    if ( strcmp (fname, "sum")    == 0 ) return (mf_array_dsum_range    (v, start, end));
    if ( strcmp (fname, "mean")   == 0 ) return (mf_array_dmean_range   (v, start, end));
    if ( strcmp (fname, "sd")     == 0 ) return (mf_array_dsd_range     (v, start, end));
    if ( strcmp (fname, "max")    == 0 ) return (mf_array_dmax_range    (v, start, end));
    if ( strcmp (fname, "min")    == 0 ) return (mf_array_dmin_range    (v, start, end));
    if ( strcmp (fname, "median") == 0 ) return (mf_array_dmedian_range (v, start, end));
    if ( strcmp (fname, "iqr")    == 0 ) return (mf_array_diqr_range    (v, start, end));
    double q = (double) atof(fname);
    return (q > 0? mf_array_dquantile_range(v, start, end, q): 0);
}

/**
 * @brief Wrapper to encode summary function using an integer
 *
 * We use negative numbers so that we can return quantiles as is.
 *
 * @param fname Character with name of funtion
 * @return internal code for summary function
 */
double mf_code_fun (char * fname)
{
    if ( strcmp (fname, "sum")     == 0 ) return (-1);  // sum
    if ( strcmp (fname, "mean")    == 0 ) return (-2);  // mean
    if ( strcmp (fname, "sd")      == 0 ) return (-3);  // sd
    if ( strcmp (fname, "max")     == 0 ) return (-4);  // max
    if ( strcmp (fname, "min")     == 0 ) return (-5);  // min
    if ( strcmp (fname, "count")   == 0 ) return (-6);  // count
    if ( strcmp (fname, "percent") == 0 ) return (-7);  // percent
    if ( strcmp (fname, "median")  == 0 ) return (50); // median
    if ( strcmp (fname, "iqr")     == 0 ) return (-9);  // iqr
    if ( strcmp (fname, "first")   == 0 ) return (-10); // first
    if ( strcmp (fname, "firstnm") == 0 ) return (-11); // firstnm
    if ( strcmp (fname, "last")    == 0 ) return (-12); // last
    if ( strcmp (fname, "lastnm")  == 0 ) return (-13); // lastnm
    double q = (double) atof(fname);                    // quantile
    return (q > 0? q: 0);
}

/**
 * @brief Wrapper to choose summary function using internal code
 *
 * See mf_code_fun above
 *
 * @param fcode double with function code
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return @fname(@v[@start to @end])
 */
double mf_switch_fun_code (double fcode, double v[], const size_t start, const size_t end)
{
    if ( fcode == -1 )  return (mf_array_dsum_range  (v, start, end)); // sum
    if ( fcode == -2 )  return (mf_array_dmean_range (v, start, end)); // mean
    if ( fcode == -3 )  return (mf_array_dsd_range   (v, start, end)); // sd)
    if ( fcode == -4 )  return (mf_array_dmax_range  (v, start, end)); // max
    if ( fcode == -5 )  return (mf_array_dmin_range  (v, start, end)); // min
    if ( fcode == -9 )  return (mf_array_diqr_range  (v, start, end)); // iqr
    return (mf_array_dquantile_range(v, start, end, fcode));           // percentiles
}

/**
 * @brief Compare function for qsort
 *
 * @param a First element
 * @param b Second element
 * @return @a - @b
 */
int mf_qsort_compare (const void * a, const void * b)
{
    return ( (int) *(double*)a - *(double*)b );
}

/**
 * @brief Determine if of enries in range of array are sorted
 *
 * @param v vector of doubles containing the current group's variables
 * @param start summaryze starting at the @start-th entry
 * @param end summaryze until the (@end - 1)-th entry
 * @return Whether the elements of @v are sorted from @start to @end
 */
int mf_array_dsorted_range (const double v[], const size_t start, const size_t end) {
    size_t i;
    for (i = start + 1; i < end; i++) {
        if (v[i - 1] > v[i]) return (0);
    }
    return (1);
}
