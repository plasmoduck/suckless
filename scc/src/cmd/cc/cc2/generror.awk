/^enum nerrors \{/     {print "char *errlist[] = {"; inhome = 1}

inhome && /E[A-Z]*, /  {sub(/,/, "", $1)
                        printf("\t[%s] = \"", $1)
                        for (i = 3; i <= NF-1; ++i)
				printf("%s%s", $i, (i == NF-1) ? "\"" : " ")
			print ","}

inhome && /^}/          {print "};" ; inhome = 0}
