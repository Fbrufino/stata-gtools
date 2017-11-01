#include "gtools_sort.h"

/**
 * @brief Counting or radix sort on 64-bit hash with index
 *
 * Perform a counting or radix sort on an array of 64-bit integers. The radix
 * sort performs 64 / d passes of the counting sort, where the set of integers
 * is sorted d-bits at a time. In order to achiave this, we sort
 * 
 *     kth bit chunk = (x[i] >> d * k) & 0xff
 *     
 * The 0th d-bit chunk, then the 1st, and so on. We sort 16 bits at a time.
 * An 8-bit at a time version is also available.
 *
 * @param hash hash to sort
 * @param index Stata index of sort
 * @param N number of elements
 * @param verbose Print sorting info to Stata
 * @param hash_level whether we used a bijection (0) or a 128-bit hash (1)
 * @return stable sorted @hash, with @index sorted as well
 */
ST_retcode gf_sort_hash (
    uint64_t *hash,
    GT_size *index,
    GT_size N,
    GT_bool verbose)
{
    GT_size i;
    ST_retcode rc = 0;

    GTOOLS_MIN (hash, N, min, i)
    GTOOLS_MAX (hash, N, max, i)

    uint64_t range = max - min + 1;
    uint64_t ctol  = pow(2, 24);

    if ( range < ctol ) {
        if ( (rc = gf_counting_sort (hash, index, N, min, max)) ) return(rc);
        if ( verbose ) {
            sf_printf("Counting sort on hash; min = "
                      GT_size_cfmt", max = "
                      GT_size_cfmt"\n", min, max);
        }
    }
    else {
        clock_t timer = clock();

        uint64_t *copy_hash  = calloc(N, sizeof *copy_hash);
        GT_size  *copy_index = calloc(N, sizeof *copy_index);

        for (i = 0; i < N; i++) {
            copy_hash[i]  = hash[i];
            copy_index[i] = index[i];
        }
        sf_running_timer (&timer, "Made copy of hash, index");

        if ( (rc = gf_radix_psort16 (copy_hash, copy_index, N)) ) return(rc);
        sf_running_timer (&timer, "Sorted in parallel");

        if ( (rc = gf_radix_sort16 (hash, index, N)) ) return(rc);
        sf_running_timer (&timer, "Single-threaded sort");

        if ( verbose ) {
            sf_printf("Radix sort on hash (16-bits at a time)\n");
        }
    }

    return (rc);
}

/**
 * @brief Radix sort with index (8-bit)
 *
 * Perform radix sort, additionally storing data shuffle in index
 *
 * @param hash hash to sort
 * @param index Hash sort index
 * @param N number of elements
 * @return Radix sort on hash array.
 */
ST_retcode gf_radix_sort8 (
    uint64_t *hash,
    GT_size *index,
    GT_size N)
{
	radixCounts8 counts;
	memset (&counts, 0, 256 * 8 * sizeof(uint32_t));

    GT_size i;
	uint64_t *hcopy  = (uint64_t *) calloc(N, sizeof(uint64_t));
	uint64_t *ixcopy = (uint64_t *) calloc(N, sizeof(uint64_t));

    if ( hcopy  == NULL ) return (sf_oom_error("radixSort", "hcopy"));
    if ( ixcopy == NULL ) return (sf_oom_error("radixSort", "ixcopy"));

	uint32_t byte8,
             byte7,
             byte6,
             byte5,
             byte4,
             byte3,
             byte2,
             byte1;

    uint32_t offset8 = 0,
             offset7 = 0,
             offset6 = 0,
             offset5 = 0,
             offset4 = 0,
             offset3 = 0,
             offset2 = 0,
             offset1 = 0;

	// calculate counts
	// ----------------

	for(i = 0; i < N; i++) {
		byte8 =  hash[i]        & 0xff;
		byte7 = (hash[i] >> 8)  & 0xff;
		byte6 = (hash[i] >> 16) & 0xff;
		byte5 = (hash[i] >> 24) & 0xff;
		byte4 = (hash[i] >> 32) & 0xff;
		byte3 = (hash[i] >> 40) & 0xff;
		byte2 = (hash[i] >> 48) & 0xff;
		byte1 = (hash[i] >> 56) & 0xff;

		counts.c8[byte8]++;
		counts.c7[byte7]++;
		counts.c6[byte6]++;
		counts.c5[byte5]++;
		counts.c4[byte4]++;
		counts.c3[byte3]++;
		counts.c2[byte2]++;
		counts.c1[byte1]++;
	}

	// convert counts to offsets
	// -------------------------

	for(i = 0; i < 256; i++) {
		byte8 = offset8 + counts.c8[i];
		byte7 = offset7 + counts.c7[i];
		byte6 = offset6 + counts.c6[i];
		byte5 = offset5 + counts.c5[i];
		byte4 = offset4 + counts.c4[i];
		byte3 = offset3 + counts.c3[i];
		byte2 = offset2 + counts.c2[i];
		byte1 = offset1 + counts.c1[i];

		counts.c8[i] = offset8;
		counts.c7[i] = offset7;
		counts.c6[i] = offset6;
		counts.c5[i] = offset5;
		counts.c4[i] = offset4;
		counts.c3[i] = offset3;
		counts.c2[i] = offset2;
		counts.c1[i] = offset1;

		offset8 = byte8;
		offset7 = byte7;
		offset6 = byte6;
		offset5 = byte5;
		offset4 = byte4;
		offset3 = byte3;
		offset2 = byte2;
		offset1 = byte1;
	}

	// radix
	// -----

	for(i = 0; i < N; i++) {
		byte8 = hash[i] & 0xff;
		hcopy[counts.c8[byte8]]  = hash[i];
		ixcopy[counts.c8[byte8]] = index[i];
		counts.c8[byte8]++;
	}

	for(i = 0; i < N; i++) {
		byte7 = (hcopy[i] >> 8) & 0xff;
		hash[counts.c7[byte7]]  = hcopy[i];
		index[counts.c7[byte7]] = ixcopy[i];
		counts.c7[byte7]++;
	}

	for(i = 0; i < N; i++) {
		byte6 = (hash[i] >> 16) & 0xff;
		hcopy[counts.c6[byte6]]  = hash[i];
		ixcopy[counts.c6[byte6]] = index[i];
		counts.c6[byte6]++;
	}

	for(i = 0; i < N; i++) {
		byte5 = (hcopy[i] >> 24) & 0xff;
		hash[counts.c5[byte5]]  = hcopy[i];
		index[counts.c5[byte5]] = ixcopy[i];
		counts.c5[byte5]++;
	}

	for(i = 0; i < N; i++) {
		byte4 = (hash[i] >> 32) & 0xff;
		hcopy[counts.c4[byte4]]  = hash[i];
		ixcopy[counts.c4[byte4]] = index[i];
		counts.c4[byte4]++;
	}

	for(i = 0; i < N; i++) {
		byte3 = (hcopy[i] >> 40) & 0xff;
		hash[counts.c3[byte3]]  = hcopy[i];
		index[counts.c3[byte3]] = ixcopy[i];
		counts.c3[byte3]++;
	}

	for(i = 0; i < N; i++) {
		byte2 = (hash[i] >> 48) & 0xff;
		hcopy[counts.c2[byte2]]  = hash[i];
		ixcopy[counts.c2[byte2]] = index[i];
		counts.c2[byte2]++;
	}

	for(i = 0; i < N; i++) {
		byte1 = (hcopy[i] >> 56) & 0xff;
		hash[counts.c1[byte1]]  = hcopy[i];
		index[counts.c1[byte1]] = ixcopy[i];
		counts.c1[byte1]++;
	}

	free(hcopy);
	free(ixcopy);

    return (0);
}

/**
 * @brief Radix sort with index (16-bit)
 *
 * Perform radix sort, additionally storing data shuffle in index
 *
 * @param hash hash to sort
 * @param index Hash sort index
 * @param N number of elements
 * @return Radix sort on hash array.
 */
ST_retcode gf_radix_sort16 (
    uint64_t *hash,
    GT_size *index,
    GT_size N)
{
    GT_size size = 65536;

    // Allocate space for index and hash copies
    // ----------------------------------------

    GT_size i;
	uint64_t *hcopy  = (uint64_t *) calloc(N, sizeof(uint64_t));
	uint64_t *ixcopy = (uint64_t *) calloc(N, sizeof(uint64_t));

    if ( hcopy  == NULL ) return (sf_oom_error("radixSort", "hcopy"));
    if ( ixcopy == NULL ) return (sf_oom_error("radixSort", "ixcopy"));

	uint32_t byte4,
             byte3,
             byte2,
             byte1;

    uint32_t offset4 = 0,
             offset3 = 0,
             offset2 = 0,
             offset1 = 0;

    // Initialize counts to 0
    // ----------------------

    struct radixCounts16 *counts = malloc(sizeof(*counts));
	counts->c4 = calloc(size, sizeof(uint32_t));
	counts->c3 = calloc(size, sizeof(uint32_t));
	counts->c2 = calloc(size, sizeof(uint32_t));
	counts->c1 = calloc(size, sizeof(uint32_t));

    for (i = 0; i < size; i++) {
        counts->c4[i] = 0;
        counts->c3[i] = 0;
        counts->c2[i] = 0;
        counts->c1[i] = 0;
    }

	// Calculate counts
	// ----------------

	for(i = 0; i < N; i++) {
		byte4 =  hash[i]        & 0xffff;
		byte3 = (hash[i] >> 16) & 0xffff;
		byte2 = (hash[i] >> 32) & 0xffff;
		byte1 = (hash[i] >> 48) & 0xffff;

		counts->c4[byte4]++;
		counts->c3[byte3]++;
		counts->c2[byte2]++;
		counts->c1[byte1]++;
	}

	// Convert counts to offsets
	// -------------------------

	for(i = 0; i < size; i++) {
		byte4 = offset4 + counts->c4[i];
		byte3 = offset3 + counts->c3[i];
		byte2 = offset2 + counts->c2[i];
		byte1 = offset1 + counts->c1[i];

		counts->c4[i] = offset4;
		counts->c3[i] = offset3;
		counts->c2[i] = offset2;
		counts->c1[i] = offset1;

		offset4 = byte4;
		offset3 = byte3;
		offset2 = byte2;
		offset1 = byte1;
	}

	// Radix bit
	// ---------

	for(i = 0; i < N; i++) {
		byte4 = hash[i] & 0xffff;
		hcopy[counts->c4[byte4]]  = hash[i];
		ixcopy[counts->c4[byte4]] = index[i];
		counts->c4[byte4]++;
	}

	for(i = 0; i < N; i++) {
		byte3 = (hcopy[i] >> 16) & 0xffff;
		hash[counts->c3[byte3]]  = hcopy[i];
		index[counts->c3[byte3]] = ixcopy[i];
		counts->c3[byte3]++;
	}

	for(i = 0; i < N; i++) {
		byte2 = (hash[i] >> 32) & 0xffff;
		hcopy[counts->c2[byte2]]  = hash[i];
		ixcopy[counts->c2[byte2]] = index[i];
		counts->c2[byte2]++;
	}

	for(i = 0; i < N; i++) {
		byte1 = (hcopy[i] >> 48) & 0xffff;
		hash[counts->c1[byte1]]  = hcopy[i];
		index[counts->c1[byte1]] = ixcopy[i];
		counts->c1[byte1]++;
	}

	free(counts->c1);
	free(counts->c2);
	free(counts->c3);
	free(counts->c4);
	free(counts);
	free(hcopy);
	free(ixcopy);

    return (0);
}

ST_retcode gf_radix_psort16 (
    uint64_t *hash,
    GT_size *index,
    GT_size N)
{
    GT_size size = 65536;

    int prc, __GTOOLS_THREADS = 4;
    uint32_t *pcounts4  = calloc(size, sizeof(uint32_t));
    uint32_t *pcounts3  = calloc(size, sizeof(uint32_t));
    uint32_t *pcounts2  = calloc(size, sizeof(uint32_t));
    uint32_t *pcounts1  = calloc(size, sizeof(uint32_t));
    struct pInfo *pinfo = calloc(__GTOOLS_THREADS, sizeof(*pinfo));

    // Allocate space for index and hash copies
    // ----------------------------------------

    GT_size i;
    uint32_t byte;
	uint64_t *hcopy  = (uint64_t *) calloc(N, sizeof(uint64_t));
	uint64_t *ixcopy = (uint64_t *) calloc(N, sizeof(uint64_t));

    if ( hcopy  == NULL ) return (sf_oom_error("radixSort", "hcopy"));
    if ( ixcopy == NULL ) return (sf_oom_error("radixSort", "ixcopy"));

	// In parallel
	// -----------

    pthread_t threads[__GTOOLS_THREADS];

    pinfo[0].counts = pcounts4;
    pinfo[1].counts = pcounts3;
    pinfo[2].counts = pcounts2;
    pinfo[3].counts = pcounts1;
    for (i = 0; i < __GTOOLS_THREADS; i++) {
        pinfo[i].hash   = hash;
        pinfo[i].size   = size;
        pinfo[i].N      = N;
    }

    for (i = 0; i < __GTOOLS_THREADS; i++) {
        pinfo[i].thread = i;
        prc = pthread_create(&threads[i], NULL, gf_radix_counts16, &pinfo[i]);
        assert (!prc);
    }

    for (i = 0; i < __GTOOLS_THREADS; i++) {
        prc = pthread_join(threads[i], NULL);
        assert(!prc);
        pinfo[i].counts = NULL;
        pinfo[i].hash   = NULL;
    }

	// Radix bit
	// ---------

	for(i = 0; i < N; i++) {
		byte = hash[i] & 0xffff;
		hcopy[pcounts4[byte]]  = hash[i];
		ixcopy[pcounts4[byte]] = index[i];
		pcounts4[byte]++;
	}

	for(i = 0; i < N; i++) {
		byte = (hcopy[i] >> 16) & 0xffff;
		hash[pcounts3[byte]]  = hcopy[i];
		index[pcounts3[byte]] = ixcopy[i];
		pcounts3[byte]++;
	}

	for(i = 0; i < N; i++) {
		byte = (hash[i] >> 32) & 0xffff;
		hcopy[pcounts2[byte]]  = hash[i];
		ixcopy[pcounts2[byte]] = index[i];
		pcounts2[byte]++;
	}

	for(i = 0; i < N; i++) {
		byte = (hcopy[i] >> 48) & 0xffff;
		hash[pcounts1[byte]]  = hcopy[i];
		index[pcounts1[byte]] = ixcopy[i];
		pcounts1[byte]++;
	}

    free(pinfo);
    free(pcounts4);
    free(pcounts3);
    free(pcounts2);
    free(pcounts1);
	free(ixcopy);
	free(hcopy);

    return (0);
}

void* gf_radix_counts16 (void *argument)
{
    struct pInfo *pinfo = ((struct pInfo *) argument);

    GT_size i;
    uint32_t byte;
    uint32_t coffset = 0;
    uint32_t bits    = 16 * pinfo->thread;

    for (i = 0; i < pinfo->size; i++) {
        pinfo->counts[i] = 0;
    }

    if ( bits == 0 ) {
        for(i = 0; i < pinfo->N; i++) {
            byte =  pinfo->hash[i] & 0xffff;
            pinfo->counts[byte]++;
        }
    }
    else {
        for(i = 0; i < pinfo->N; i++) {
            byte = (pinfo->hash[i] >> bits) & 0xffff;
            pinfo->counts[byte]++;
        }
    }

	for(i = 0; i < pinfo->size; i++) {
		byte = coffset + pinfo->counts[i];
		pinfo->counts[i] = coffset;
		coffset = byte;
    }

    return (NULL);
}

/**
 * @brief Counting sort with index
 *
 * Perform counting sort, additionally storing data shuffle
 * in index variable.
 *
 * @param hash hash to sort
 * @param index Hash sort index
 * @param N number of elements
 * @param min Smallest hash
 * @param max Largest hash
 * @return Counting sort on hash array.
 */
ST_retcode gf_counting_sort (
    uint64_t *hash,
    GT_size *index,
    GT_size N,
    uint64_t min,
    uint64_t max)
{

    GT_size i, s;
    uint64_t range = max - min + 1;

    // Allocate space for x, index copies and x mod
    uint64_t *xcopy = calloc(N, sizeof *xcopy);
    GT_size  *icopy = calloc(N, sizeof *icopy);
    GT_size  *count = calloc(range + 1, sizeof *count);

    if ( xcopy == NULL ) return(sf_oom_error("gf_counting_sort_hash", "xcopy"));
    if ( icopy == NULL ) return(sf_oom_error("gf_counting_sort_hash", "icopy"));
    if ( count == NULL ) return(sf_oom_error("gf_counting_sort_hash", "count"));

    // Initialize count as 0s
    for (i = 0; i < range + 1; i++)
        count[i] = 0;

    // Freq count of hash
    for (i = 0; i < N; i++) {
        count[ xcopy[i] = (hash[i] + 1 - min) ]++;
        icopy[i] = index[i];
    }

    // Cummulative freq count (position in output)
    for (i = 1; i < range; i++)
        count[i] += count[i - 1];

    // Copy back in stable sorted order
    // for (i = N - 1; i >= 0; i--) {
    for (i = 0; i < N; i++) {
        index[ s = count[xcopy[i] - 1]++ ] = icopy[i];
        hash[s] = xcopy[i] - 1 + min;
    }

    // Free space
    free (count);
    free (xcopy);
    free (icopy);

    return (0);
}
