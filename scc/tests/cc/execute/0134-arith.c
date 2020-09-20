int
main()
{
        int x;

        x = 0;
        if ((x = x + 2) != 2)        // 2
		return 1;
        if ((x = x - 1) != 1)        // 1
		return 1;
        if ((x = x * 6) != 6)        // 6
		return 1;
        if ((x = x / 2) != 3)        // 3
		return 1;
        if ((x = x % 2) != 1)        // 1
		return 1;
        if ((x = x << 2) != 4)       // 4
		return 1;
        if ((x = x >> 1) != 2)       // 2
		return 1;
        if ((x = x | 255) != 255)    // 255
		return 1;
        if ((x = x & 3) != 3)        // 3
		return 1;
        if ((x = x ^ 1) != 2)        // 2
		return 1;
        if ((x = x + (x > 1)) != 2)  // 2
		return 1;
        if ((x = x + (x < 3)) != 2)  // 2
		return 1;
        if ((x = x + (x > 1)) != 3)  // 3
		return 1;
        if ((x = x + (x < 4)) != 4)  // 4
		return 1;
        return 0;
}
