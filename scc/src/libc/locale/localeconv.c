#include <locale.h>
#include <limits.h>
#undef localeconv

struct lconv *
localeconv(void)
{
	static struct lconv lc = {
		.decimal_point = ".",
		.thousands_sep = "",
		.grouping = "",
		.mon_decimal_point = "",
		.mon_thousands_sep = "",
		.mon_grouping = "",
		.positive_sign = "",
		.negative_sign = "",
		.currency_symbol = "",
		.int_curr_symbol = "",
		.frac_digits = CHAR_MAX,
		.p_cs_precedes = CHAR_MAX,
		.n_cs_precedes = CHAR_MAX,
		.p_sep_by_space = CHAR_MAX,
		.p_sign_posn = CHAR_MAX,
		.n_sep_by_space = CHAR_MAX,
		.n_sign_posn = CHAR_MAX,
		.int_frac_digits = CHAR_MAX,
	};
	return &lc;
}
