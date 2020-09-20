#include <assert.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>

/*
output:
testing
done
end:
*/

int
main()
{
	struct lconv *lc;

	puts("testing");
	assert(setlocale(LC_ALL, "C"));
	assert((lc = localeconv()) != NULL);
	assert(!strcmp(lc->decimal_point, "."));
	assert(!strcmp(lc->thousands_sep, ""));
	assert(!strcmp(lc->grouping, ""));
	assert(!strcmp(lc->int_curr_symbol, ""));
	assert(!strcmp(lc->currency_symbol, ""));
	assert(!strcmp(lc->mon_decimal_point, ""));
	assert(!strcmp(lc->mon_thousands_sep, ""));
	assert(!strcmp(lc->mon_grouping, ""));
	assert(!strcmp(lc->positive_sign, ""));
	assert(!strcmp(lc->negative_sign, ""));
	assert(!strcmp(lc->currency_symbol, ""));
	assert(!strcmp(lc->int_curr_symbol, ""));
	assert(lc->int_frac_digits == CHAR_MAX);
	assert(lc->frac_digits == CHAR_MAX);
	assert(lc->p_cs_precedes == CHAR_MAX);
	assert(lc->n_cs_precedes == CHAR_MAX);
	assert(lc->p_sep_by_space == CHAR_MAX);
	assert(lc->n_sep_by_space == CHAR_MAX);
	assert(lc->p_sign_posn == CHAR_MAX);
	assert(lc->n_sign_posn == CHAR_MAX);
	puts("done");

	return 0;
}
