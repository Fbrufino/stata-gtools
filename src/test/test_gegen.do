capture program drop consistency_gegen
program consistency_gegen
    syntax, [tol(real 1e-6) NOIsily *]
    di _n(1) "{hline 80}" _n(1) "consistency_gegen, `options'" _n(1) "{hline 80}" _n(1)

    local stats total sum mean sd max min count median iqr
    local percentiles 1 10 30 50 70 90 99
    qui `noisily' sim, n(500000) nj(10000) njsub(4) string groupmiss outmiss

    cap drop g*_*
    cap drop c*_*
    di _n(1) "Checking full egen range"
    foreach fun of local stats {
        qui `noisily' gegen g_`fun' = `fun'(rnorm), by(groupstr groupsub) `options'
        qui `noisily'  egen c_`fun' = `fun'(rnorm), by(groupstr groupsub)
        cap noi assert (g_`fun' == c_`fun') | abs(g_`fun' - c_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen (passed): gegen `fun' results similar to egen (tol = `tol')"
            
    }

    foreach p in `percentiles' {
        qui  `noisily' gegen g_p`p' = pctile(rnorm), by(groupstr groupsub) p(`p') `options'
        qui  `noisily'  egen c_p`p' = pctile(rnorm), by(groupstr groupsub) p(`p')
        cap noi assert (g_p`p' == c_p`p') | abs(g_p`p' - c_p`p') < `tol'
        if ( _rc ) {
            di as err "    compare_egen (failed): gegen percentile `p' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen (passed): gegen percentile `p' results similar to egen (tol = `tol')"
    }

    local fun tag
    {
        qui  `noisily' gegen g_`fun' = `fun'(groupstr groupsub), v `options'
        qui  `noisily'  egen c_`fun' = `fun'(groupstr groupsub)
        cap noi assert (g_`fun' == c_`fun') | abs(g_`fun' - c_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen (passed): gegen `fun' results similar to egen (tol = `tol')"
    }

    local fun group
    {
        qui  `noisily' gegen g_`fun' = `fun'(groupstr groupsub), v `options'
        qui  `noisily'  egen c_`fun' = `fun'(groupstr groupsub)
        qui bys g_`fun' (c_`fun'): gen byte g_`fun'_check = c_`fun'[1] == c_`fun'[_N]
        qui bys c_`fun' (g_`fun'): gen byte c_`fun'_check = g_`fun'[1] == g_`fun'[_N]
        cap noi assert g_`fun'_check & c_`fun'_check
        if ( _rc ) {
            di as err "    compare_egen (failed): gegen `fun' IDs do not map to egen IDs"
            exit _rc
        }
        else di as txt "    compare_egen (passed): gegen `fun' IDs correctly map to egen IDs"
    }

    * ---------------------------------------------------------------------
    * ---------------------------------------------------------------------

    cap drop g*_*
    cap drop c*_*
    di "Checking egen if range"
    foreach fun of local stats {
        qui  `noisily' gegen gif_`fun' = `fun'(rnorm) if rsort > 0, by(groupstr groupsub) `options'
        qui  `noisily'  egen cif_`fun' = `fun'(rnorm) if rsort > 0, by(groupstr groupsub)
        cap noi assert (gif_`fun' == cif_`fun') | abs(gif_`fun' - cif_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_if (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_if (passed): gegen `fun' results similar to egen (tol = `tol')"
    }

    foreach p in `percentiles' {
        qui  `noisily' gegen g_p`p' = pctile(rnorm) if rsort > 0, by(groupstr groupsub) p(`p') `options'
        qui  `noisily'  egen c_p`p' = pctile(rnorm) if rsort > 0, by(groupstr groupsub) p(`p')
        cap noi assert (g_p`p' == c_p`p') | abs(g_p`p' - c_p`p') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_if (failed): gegen percentile `p' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_if (passed): gegen percentile `p' results similar to egen (tol = `tol')"
    }

    local fun tag
    {
        qui  `noisily' gegen gif_`fun' = `fun'(groupstr groupsub) if rsort > 0, v `options'
        qui  `noisily'  egen cif_`fun' = `fun'(groupstr groupsub) if rsort > 0
        cap noi assert (gif_`fun' == cif_`fun') | abs(gif_`fun' - cif_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_if (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_if (passed): gegen `fun' results similar to egen (tol = `tol')"
    }

    local fun group
    {
        qui  `noisily' gegen g_`fun' = `fun'(groupstr groupsub) if rsort > 0, v `options'
        qui  `noisily'  egen c_`fun' = `fun'(groupstr groupsub) if rsort > 0
        qui bys g_`fun' (c_`fun'): gen byte g_`fun'_check = c_`fun'[1] == c_`fun'[_N]
        qui bys c_`fun' (g_`fun'): gen byte c_`fun'_check = g_`fun'[1] == g_`fun'[_N]
        cap noi assert g_`fun'_check & c_`fun'_check
        if ( _rc ) {
            di as err "    compare_egen_if (failed): gegen `fun' IDs do not map to egen IDs"
            exit _rc
        }
        else di as txt "    compare_egen_if (passed): gegen `fun' IDs correctly map to egen IDs"
    }

    * ---------------------------------------------------------------------
    * ---------------------------------------------------------------------

    cap drop g*_*
    cap drop c*_*
    di "Checking egen in range"
    foreach fun of local stats {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen gin_`fun' = `fun'(rnorm) in `from' / `to', by(groupstr groupsub) `options'
        qui  `noisily'  egen cin_`fun' = `fun'(rnorm) in `from' / `to', by(groupstr groupsub)
        cap noi assert (gin_`fun' == cin_`fun') | abs(gin_`fun' - cin_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_in (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_in (passed): gegen `fun' results similar to egen (tol = `tol')"
    }

    foreach p in `percentiles' {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen g_p`p' = pctile(rnorm) in `from' / `to', by(groupstr groupsub) p(`p') `options'
        qui  `noisily'  egen c_p`p' = pctile(rnorm) in `from' / `to', by(groupstr groupsub) p(`p')
        cap noi assert (g_p`p' == c_p`p') | abs(g_p`p' - c_p`p') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_in (failed): gegen percentile `p' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_in (passed): gegen percentile `p' results similar to egen (tol = `tol')"
    }

    local fun tag
    {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen gin_`fun' = `fun'(groupstr groupsub) in `from' / `to', v b `options'
        qui  `noisily'  egen cin_`fun' = `fun'(groupstr groupsub) in `from' / `to'
        cap noi assert (gin_`fun' == cin_`fun') | abs(gin_`fun' - cin_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_in (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_in (passed): gegen `fun' results similar to egen (tol = `tol')"
    }

    local fun group
    {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen g_`fun' = `fun'(groupstr groupsub) in `from' / `to', v `options'
        qui  `noisily'  egen c_`fun' = `fun'(groupstr groupsub) in `from' / `to'
        qui bys g_`fun' (c_`fun'): gen byte g_`fun'_check = c_`fun'[1] == c_`fun'[_N]
        qui bys c_`fun' (g_`fun'): gen byte c_`fun'_check = g_`fun'[1] == g_`fun'[_N]
        cap noi assert g_`fun'_check & c_`fun'_check
        if ( _rc ) {
            di as err "    compare_egen_in (failed): gegen `fun' IDs do not map to egen IDs"
            exit _rc
        }
        else di as txt "    compare_egen_in (passed): gegen `fun' IDs correctly map to egen IDs"
    }

    * ---------------------------------------------------------------------
    * ---------------------------------------------------------------------

    cap drop g*_*
    cap drop c*_*
    di "Checking egen if in range"
    foreach fun of local stats {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen gifin_`fun' = `fun'(rnorm) if rsort < 0 in `from' / `to', by(groupstr groupsub) `options'
        qui  `noisily'  egen cifin_`fun' = `fun'(rnorm) if rsort < 0 in `from' / `to', by(groupstr groupsub)
        cap noi assert (gifin_`fun' == cifin_`fun') | abs(gifin_`fun' - cifin_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_ifin (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_ifin (passed): gegen `fun' results similar to egen (tol = `tol')"
    }

    foreach p in `percentiles' {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen g_p`p' = pctile(rnorm) if rsort < 0 in `from' / `to', by(groupstr groupsub) p(`p') `options'
        qui  `noisily'  egen c_p`p' = pctile(rnorm) if rsort < 0 in `from' / `to', by(groupstr groupsub) p(`p')
        cap noi assert (g_p`p' == c_p`p') | abs(g_p`p' - c_p`p') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_ifin (failed): gegen percentile `p' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_ifin (passed): gegen percentile `p' results similar to egen (tol = `tol')"
    }

    local fun tag
    {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen gifin_`fun' = `fun'(groupstr groupsub) if rsort < 0 in `from' / `to', v `options'
        qui  `noisily'  egen cifin_`fun' = `fun'(groupstr groupsub) if rsort < 0 in `from' / `to'
        cap noi assert (gifin_`fun' == cifin_`fun') | abs(gifin_`fun' - cifin_`fun') < `tol'
        if ( _rc ) {
            di as err "    compare_egen_ifin (failed): gegen `fun' not equal to egen (tol = `tol')"
            exit _rc
        }
        else di as txt "    compare_egen_ifin (passed): gegen `fun' results similar to egen (tol = `tol')"
    }

    local fun group
    {
        local in1 = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2 = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui  `noisily' gegen g_`fun' = `fun'(groupstr groupsub) if rsort < 0 in `from' / `to', v `options'
        qui  `noisily'  egen c_`fun' = `fun'(groupstr groupsub) if rsort < 0 in `from' / `to'
        qui bys g_`fun' (c_`fun'): gen byte g_`fun'_check = c_`fun'[1] == c_`fun'[_N]
        qui bys c_`fun' (g_`fun'): gen byte c_`fun'_check = g_`fun'[1] == g_`fun'[_N]
        cap noi assert g_`fun'_check & c_`fun'_check
        if ( _rc ) {
            di as err "    compare_egen_ifin (failed): gegen `fun' IDs do not map to egen IDs"
            exit _rc
        }
        else di as txt "    compare_egen_ifin (passed): gegen `fun' IDs correctly map to egen IDs"
    }
end

capture program drop consistency_gegen_gcollapse
program consistency_gegen_gcollapse
    syntax, [tol(real 1e-6) NOIsily *]
    di _n(1) "{hline 80}" _n(1) "consistency_gegen_gcollapse, `options'" _n(1) "{hline 80}" _n(1)

    qui `noisily' {
        sim, n(20000) nj(100) njsub(2) string outmiss
        gegen id = group(groupstr groupsub)
        gegen double mean    = mean   (rnorm),  by(groupstr groupsub) verbose benchmark `options'
        gegen double sum     = sum    (rnorm),  by(groupstr groupsub) `options'
        gegen double median  = median (rnorm),  by(groupstr groupsub) `options'
        gegen double sd      = sd     (rnorm),  by(groupstr groupsub) `options'
        gegen double iqr     = iqr    (rnorm),  by(groupstr groupsub) `options'
        gegen double first   = first  (rnorm),  by(groupstr groupsub) `options' v b
        gegen double last    = last   (rnorm),  by(groupstr groupsub) `options'
        gegen double firstnm = firstnm(rnorm),  by(groupstr groupsub) `options'
        gegen double lastnm  = lastnm (rnorm),  by(groupstr groupsub) `options'
        gegen double q10     = pctile (rnorm),  by(groupstr groupsub) `options' p(10.5)
        gegen double q30     = pctile (rnorm),  by(groupstr groupsub) `options' p(30)
        gegen double q70     = pctile (rnorm),  by(groupstr groupsub) `options' p(70)
        gegen double q90     = pctile (rnorm),  by(groupstr groupsub) `options' p(90.5)

        gcollapse (mean)    g_mean    = rnorm  ///
                  (sum)     g_sum     = rnorm  ///
                  (median)  g_median  = rnorm  ///
                  (sd)      g_sd      = rnorm  ///
                  (iqr)     g_iqr     = rnorm  ///
                  (first)   g_first   = rnorm  ///
                  (last)    g_last    = rnorm  ///
                  (firstnm) g_firstnm = rnorm  ///
                  (lastnm)  g_lastnm  = rnorm  ///
                  (p10.5)   g_q10     = rnorm  ///
                  (p30)     g_q30     = rnorm  ///
                  (p70)     g_q70     = rnorm  ///
                  (p90.5)   g_q90     = rnorm, by(id) benchmark verbose `options' merge double
    }

    di _n(1) "Checking gegen vs gcollapse full range"
    foreach fun in mean sum median sd iqr first last firstnm lastnm q10 q30 q70 q90 {
        cap noi assert (g_`fun' == `fun') | abs(g_`fun' - `fun') < `tol'
        if ( _rc ) {
            recast double g_`fun' `fun'
            cap noi assert (g_`fun' == `fun') | abs(g_`fun' - `fun') < `tol'
            if ( _rc ) {
                di as err "    compare_gegen_gcollapse (failed): `fun' yielded different results (tol = `tol')"
                exit _rc
            }
            else di as txt "    compare_gegen_gcollapse (passed): `fun' yielded same results (tol = `tol')"
        }
        else di as txt "    compare_gegen_gcollapse (passed): `fun' yielded same results (tol = `tol')"
    }

    qui `noisily' {
        sim, n(20000) nj(100) njsub(2) string outmiss

        local in1  = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2  = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui count if rsort < 0 in `from' / `to'
        if ( `r(N)' == 0 ) {
            local in1  = ceil(runiform() * 10)
            local in2  = ceil(`=_N' - runiform() * 10)
            local from = cond(`in1' < `in2', `in1', `in2')
            local to   = cond(`in1' > `in2', `in1', `in2')
        }

        gegen id = group(groupstr groupsub) in `from' / `to'
        gegen double mean    = mean   (rnorm) in `from' / `to',  by(groupstr groupsub) verbose benchmark `options'
        gegen double sum     = sum    (rnorm) in `from' / `to',  by(groupstr groupsub) `options'
        gegen double median  = median (rnorm) in `from' / `to',  by(groupstr groupsub) `options'
        gegen double sd      = sd     (rnorm) in `from' / `to',  by(groupstr groupsub) `options'
        gegen double iqr     = iqr    (rnorm) in `from' / `to',  by(groupstr groupsub) `options'
        gegen double first   = first  (rnorm) in `from' / `to',  by(groupstr groupsub) `options' v b
        gegen double last    = last   (rnorm) in `from' / `to',  by(groupstr groupsub) `options'
        gegen double firstnm = firstnm(rnorm) in `from' / `to',  by(groupstr groupsub) `options'
        gegen double lastnm  = lastnm (rnorm) in `from' / `to',  by(groupstr groupsub) `options'
        gegen double q10     = pctile (rnorm) in `from' / `to',  by(groupstr groupsub) `options' p(10.5)
        gegen double q30     = pctile (rnorm) in `from' / `to',  by(groupstr groupsub) `options' p(30)
        gegen double q70     = pctile (rnorm) in `from' / `to',  by(groupstr groupsub) `options' p(70)
        gegen double q90     = pctile (rnorm) in `from' / `to',  by(groupstr groupsub) `options' p(90.5)

        gcollapse (mean)    g_mean    = rnorm  ///
                  (sum)     g_sum     = rnorm  ///
                  (median)  g_median  = rnorm  ///
                  (sd)      g_sd      = rnorm  ///
                  (iqr)     g_iqr     = rnorm  ///
                  (first)   g_first   = rnorm  ///
                  (last)    g_last    = rnorm  ///
                  (firstnm) g_firstnm = rnorm  ///
                  (lastnm)  g_lastnm  = rnorm  ///
                  (p10.5)   g_q10     = rnorm  ///
                  (p30)     g_q30     = rnorm  ///
                  (p70)     g_q70     = rnorm  ///
                  (p90.5)   g_q90     = rnorm in `from' / `to', by(id) benchmark verbose `options' merge double
    }

    di _n(1) "Checking gegen vs gcollapse in range"
    foreach fun in mean sum median sd iqr first last firstnm lastnm q10 q30 q70 q90 {
        cap noi assert (g_`fun' == `fun') | abs(g_`fun' - `fun') < `tol'
        if ( _rc ) {
            recast double g_`fun' `fun'
            cap noi assert (g_`fun' == `fun') | abs(g_`fun' - `fun') < `tol'
            if ( _rc ) {
                di as err "    compare_gegen_gcollapse_in (failed): `fun' yielded different results (tol = `tol')"
                exit _rc
            }
            else di as txt "    compare_gegen_gcollapse_in (passed): `fun' yielded same results (tol = `tol')"
        }
        else di as txt "    compare_gegen_gcollapse_in (passed): `fun' yielded same results (tol = `tol')"
    }

    qui `noisily' {
        sim, n(20000) nj(100) njsub(2) string outmiss

        local in1  = ceil((0.00 + 0.25 * runiform()) * `=_N')
        local in2  = ceil((0.75 + 0.25 * runiform()) * `=_N')
        local from = cond(`in1' < `in2', `in1', `in2')
        local to   = cond(`in1' > `in2', `in1', `in2')
        qui count if rsort < 0 in `from' / `to'
        if ( `r(N)' == 0 ) {
            local in1  = ceil(runiform() * 10)
            local in2  = ceil(`=_N' - runiform() * 10)
            local from = cond(`in1' < `in2', `in1', `in2')
            local to   = cond(`in1' > `in2', `in1', `in2')
        }

        gegen id = group(groupstr groupsub)   if rsort < 0 in `from' / `to'
        gegen double mean    = mean   (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) verbose benchmark `options'
        gegen double sum     = sum    (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options'
        gegen double median  = median (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options'
        gegen double sd      = sd     (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options'
        gegen double iqr     = iqr    (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options'
        gegen double first   = first  (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options' v b
        gegen double last    = last   (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options'
        gegen double firstnm = firstnm(rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options'
        gegen double lastnm  = lastnm (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options'
        gegen double q10     = pctile (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options' p(10.5)
        gegen double q30     = pctile (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options' p(30)
        gegen double q70     = pctile (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options' p(70)
        gegen double q90     = pctile (rnorm) if rsort < 0 in `from' / `to',  by(groupstr groupsub) `options' p(90.5)

        keep if rsort < 0 in `from' / `to'
        gcollapse (mean)    g_mean    = rnorm  ///
                  (sum)     g_sum     = rnorm  ///
                  (median)  g_median  = rnorm  ///
                  (sd)      g_sd      = rnorm  ///
                  (iqr)     g_iqr     = rnorm  ///
                  (first)   g_first   = rnorm  ///
                  (last)    g_last    = rnorm  ///
                  (firstnm) g_firstnm = rnorm  ///
                  (lastnm)  g_lastnm  = rnorm  ///
                  (p10.5)   g_q10     = rnorm  ///
                  (p30)     g_q30     = rnorm  ///
                  (p70)     g_q70     = rnorm  ///
                  (p90.5)   g_q90     = rnorm, by(id) benchmark verbose `options' merge double
    }

    di _n(1) "Checking gegen vs gcollapse if in range"
    foreach fun in mean sum median sd iqr first last firstnm lastnm q10 q30 q70 q90 {
        cap noi assert (g_`fun' == `fun') | abs(g_`fun' - `fun') < `tol'
        if ( _rc ) {
            recast double g_`fun' `fun'
            cap noi assert (g_`fun' == `fun') | abs(g_`fun' - `fun') < `tol'
            if ( _rc ) {
                di as err "    compare_gegen_gcollapse_ifin (failed): `fun' yielded different results (tol = `tol')"
                exit _rc
            }
            else di as txt "    compare_gegen_gcollapse_ifin (passed): `fun' yielded same results (tol = `tol')"
        }
        else di as txt "    compare_gegen_gcollapse_ifin (passed): `fun' yielded same results (tol = `tol')"
    }
end
