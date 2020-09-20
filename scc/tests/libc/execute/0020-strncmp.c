#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
output:
testing
done
end:
*/

int
main()
{
        puts("testing");

        assert(strncmp("abc", "abc", 3) == 0);
        assert(strncmp("abcd", "abce", 3) == 0);
        assert(strncmp("abc", "abc", 4) == 0);
        assert(strncmp("abcd", "abef", 4) < 0);
        assert(strncmp("abcf", "abcd", 4) > 0);
        assert(strncmp("abc", "abe", 0) == 0);
        assert(strncmp("", "", 1) == 0);
        assert(strncmp("abc", "", 3) > 0);
        assert(strncmp("", "abc", 3) < 0);

        puts("done");

        return 0;
}
