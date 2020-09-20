#!/bin/sh

set -e
cpu=${1?}
exec >>test.log
exec 2>&1

tmp1=`mktemp`
tmp2=`mktemp`
file=$cpu.s

trap "rm -f a.out $tmp1 $tmp2; exit" 0 2 3

as-$cpu $file

sed -n '/^\#/ ! {
	s%.*#%%
	s%^[ 	]*%%
	s%[ 	]*$%%
	/^$/d
	s%[ 	][ 	]*%\
%g
	p
}' $file |
nl -b a > $tmp1


objdump |
sed -n '/^data:/,$ {
	/^data:/ ! {
		s%.*:%%
		s%^[ 	]*%%
		s%[ 	]*$%%
		/^$/d
		s%[ 	][ 	]*%\
%g
		p
	}
}' |
nl -b a > $tmp2

echo diff
diff -u $tmp1 $tmp2
