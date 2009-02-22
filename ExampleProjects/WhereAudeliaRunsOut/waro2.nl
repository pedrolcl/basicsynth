tempo 4, 104;

middlec 8;

' modulation index, attack rate, decay rate
map "OH" {"g4.mnx":0.0005, "g5.mnx":0.0008}, "e0s0.rt", "e0s2.rt";

voice 0
begin
	instr "OH";
	channel 0;
	volume 100;

! INTRO
	R, %1*5;
	G8, %1*4, 60, 800, 1, 0.8;
	F#8, %2, 60, 400, 0.1, 0.4;
	R, %1*2;
	G8, %1*2, 60, 600, 0.8, 0.6;
	F#, %1, 60, 400, 0.1, 0.4;
	R, %1;
	G8, %1*2, 65, 600, 0.8, 0.4;
	R, %1;
	B8, %1, 65, 200, 0.1, 0.4;
	F#8, %1*2, 65, 400, 1, 0.4;
	G8, %2, 66, 400, 0.1, 0.4;
	F#8, %4, 67, 200, 0.08, 0.4;
	R, %2;
	G8, %2,  67, 400, 0.08, 0.4;
	B8, %1,  68, 400, 0.08, 0.4;
	F#8, %4, 68, 200, 0.05, 0.4;
	G8, %2,  69, 400, 0.08, 0.4;
	F#8, %4,  69, 200, 0.05, 0.4;
	R, %4;
	G8, %1, 70, 600, 0.05, 0.4;
	R, (%2*3);
	! C: 28 !
	! 21 * %1 + 15 * %2 + 6 * %3 + 6 * %4 = 32 !
	R, %2*3;
	G8, %2, 60, 200, 0.1, 0.6;
	E9, %1*3, 61, 400, 0.8, 0.4;
	D9, %2, 62, 200, 0.1, 0.4;
	A8, %1*2, 63, 600, 0.8, 0.4;
	R, %2;
	{A8, B8, C9}, %3, 64, {100, 120, 140}, 0.08, 0.4;
	C9, %1, 65, 200, 0.05, 0.4;
	B8, %2, 65, 300, 0.1, 0.4;
	D8, %1*2, 66, 400, 0.8, 0.4;
	R, %2;
	{D8, E8, B7}, %3, 66, 100, 0.08, 0.4;
	A7, %1, 67, 400, 0.1, 0.4;
	G8, %1*2, 68, 600, 1, 0.6;
	R, %2;
	G8, %1, 69, 400, 0.1, 0.4;
	F8, %4, 69, 200, 0.1, 0.4;
	E8, %4*3, 69, 400, 0.1, 0.4;
	D8, %2, 69, 400, 0.1, 0.4;
	C8, %1*2, 70, 600, 0.1, 0.6;
	B7, %2, 70, 400, 0.08, 0.4;
	R, %2;
	C8, %1, 71, 400, 0.08, 0.4;
	B7, %2, 72, 400, 0.08, 0.4;
	R, %4;
	C8, %4, 72, 400, 0.05, 0.4;
	E8, %1, 73, 400, 0.05, 0.4;
	B7, %2, 73, 400, 0.05, 0.4;
	C8, %1*4, 74, 600, 1, 0.6;
	R, %2+%1*2;
	! C: 30 !
	! 23 * %1 + 6 * %2 + 8 * %4 = 28 !
	G8, %1*4, 70, 600, 1, 0.6;
	F#8, %2, 72, 400, 0.1, 0.4;
	R, %1*2;
	G8, %1*2, 73, 600, 0.8, 0.6;
	F#8, %1, 74, 400, 0.1, 0.4;
	R, %1;
	G8, %1*2, 74, 600, 0.1, 0.4;
	R, %1;
	B8, %1, 75, 400, 0.1, 0.4;
	F#8, %1*2, 75, 600, 1, 0.4;
	G8, %2, 76, 400, 0.1, 0.4;
	F#8, %4, 76, 200, 0.1, 0.4;
	R, %2;
	G8, %2, 77, 200, 0.1, 0.4;
	B8, %1, 77, 400, 0.1, 0.4;
	F#8, %4, 78, 200, 0.08, 0.4;
	G8, %2, 78, 200, 0.08, 0.4;
	F#8, %4, 79, 200, 0.08, 0.4;
	R, %4;
	G8, %1, 80, 400, 0.08, 0.4;
	F, %4, 80, 200, 0.08, 0.4;
	E, %4*3, 80, 400, 0.05, 0.4;
	D, %2, 80, 400, 0.05, 0.4;
	C, %1*5, 80, 800, 0.05, 0.8;
	R, (%1*2);
	! C: 9 !
	G8, %2, 80, 400, 0.1, 0.4;
	B8, %2, 80, 400, 0.1, 0.4;
	F#8, %4, 80, 200, 0.1, 0.4;
	G8, %2, 80, 400, 0.1, 0.4;
	E9, %1+%2+%4, 80, 600, 0.1, 0.4;
	D9, %2, 80, 400, 0.1, 0.4;
	! F: 9 !
	A8, %1*2,  80, 600, 0.1, 0.4;
	R, %1;
	{A8, B8, C9 }, %3, 81, {100, 120, 140}, 0.08, 0.4;
	C9, %1, 82, 200, 0.08, 0.4;
	B8, %2, 82, 400, 0.08, 0.4;
	D, %1+%2, 83, 400, 0.08, 0.4;
	{D, E, B7}, %3, 83, {100, 120, 140}, 0.08, 0.4;
	A, %1, 84, 200, 0.1, 0.4;
	! C: 6 !
	G8, %1*2, 85, 600, 0.8, 0.6;
	R, %2;
	G8, %1, 86, 400, 0.08, 0.4;
	F, %2, 86, 400, 0.06, 0.4;
	E, %1, 87, 400, 0.05, 0.4;
	D, %1, 88, 400, 0.05, 0.4;
	!F: 14 !
	C8, %1*2, 89, 600, 0.05, 0.6;
	B7, %2, 90, 400, 0.1, 0.4;
	R, %2;
	C8, %1, 91, 400, 0.1, 0.4;
	B7, %2, 91, 400, 0.1, 0.4;
	R, %4;
	C8, %4, 92, 200, 0.08, 0.4;
	E8, %1, 92, 400, 0.08, 0.4;
	B7, %2, 93, 400, 0.08, 0.4;
	C8, %1*6, 94, 600, 0.1, 0.4;
	R, %2;
	{D8, E, B7}, %6, 95, 200, 0.05, 0.4;
	A, %2, 96, 300, 0.05, 0.4;
	! C: 8 !
	G8, %1*2, 97, 400, 0.05, 0.6;
	R, %2;
	F, %2, 98, 400, 0.05, 0.4;
	E, %1, 99, 500, 0.05, 0.4;
	D, %1, 99, 600, 0.05, 0.4;
	C, (%1*7), 100, 800, 0.05, 1;
	R, (%1*3);

! EXPO 1
	R, (%4*189);
	! arp repeat !
	! C: 36 !
	R, %1;
	G9, %1, 100, 600, 0.2, 0.4, 0.1;
	B,  %2, 100, 400, 0.1, 0.4;
	F#, %4, 100, 200, 0.08, 0.4;
	G,  %1*2, 100,  600, 0.2, 0.4;
	F#, %2, 100, 400, 0.1, 0.4;
	G9, %2, 100, 300, 0.1, 0.4;
	F#, %4, 100, 200, 0.08, 0.4;
	G,  %4, 100, 200, 0.08, 0.4;
	E10, %1*2, 100, 600, 2, 0.4;
	D,  %2+%4, 100, 400, 0.1, 0.4;
	! F: 18 !
	A9, %1+%4, 100, 400, 0.1, 0.4;
	R,  %4;
	{A, B, C10},  %6, 100, {100, 110, 120}, 0.08, 0.4;
	{C, B9}, %4,  100, {200, 300}, 0.1, 0.4;

	D9, %2, 100, 400, 0.1, 0.4;
	D,  %4+%6, 100, 400, 0.1, 0.4;
	{E, B8}, %6, 100, 200, 0.05, 0.4;
	A, %4, 100, 400, 0.1, 0.4;
	{D9, E, B8, A9}, {%8, %8, %8, %8}, 100, 100, 0.05, 0.4;
	! C: 18 !
	G9, %1, 100, 200, 0.05, 0.4;
	R, %2;
	G, %1, 100, 400, 0.2, 0.4;
	F, %4, 100, 200, 0.1, 0.4;
	E, %2+%4, 100, 300, 0.1, 0.4;
	D, %1, 100, 400, 0.1, 0.4;
	! F: 18 !
	C, %1, 100, 400, 0.1, 0.4;
	B8, %2, 100, 400, 0.1, 0.4;
	C9, %4, 100, 200, 0.1, 0.4;
	E,  %2, 100, 300, 0.1, 0.4;
	B8, %4, 100, 400, 0.1, 0.4;
	C9, %1, 100, 500, 0.1, 0.4;
	E, %2, 100, 400, 0.1, 0.4;
	C, %2, 100, 400, 0.1, 0.4;
	! Bb: 18 !
	D, %1, 100, 600, 0.2, 0.4;
	R, (%4*14);
	! Eb: 9 !
	R, %4;
	G, %4*3,  100, 400, 0.1, 0.4;
	{ G, A, Bb}, %6, 100, {100, 110, 120}, 0.08, 0.4;
	Bb, %2, 100, 400, 0.1, 0.4;
	A, %4, 100, 200, 0.1, 0.4;
	! D: 9 !
	D, %1, 100, 400, 0.2, 0.4;
	{D, E, B8}, %4, 100, {100, 110, 120}, 0.08, 0.4;
	A, %2, 100, 400, 0.1, 0.4;
	! A6: 9 !
	E, %1, 100, 600, 0.2, 0.4;
	R, (%4*5);
	! Bb: 18 !
	R, (%4*18);
	! A: 9 !
	C9, %1, 100, 400, 0.1, 0.4;
	B8, %4, 100, 200, 0.1, 0.4;
	{E9, F, G}, {%6, %6, %6}, 100, 100, 0.08, 0.4;
	{A, F, E, F}, {%8, %8, %8, %8}, 100, 100, 0.05, 0.4;
	! E: 9 !
	G9, %4, 100, 200, 0.1, 0.4;
	E10, %4*3, 100, 400, 0.1, 0.4;
	D,  %4, 100, 200, 0.1, 0.4;
	B9, %2, 100, 200, 0.1, 0.4;
	E10, %4, 100, 200, 0.1, 0.4;
	D,  %4, 100, 200, 0.1, 0.4;
	! F: 18 !
	A9, %1, 100, 600, 0.1, 0.4;
	R, %4*7;
	{A, B, C10}, %3, 100, 300, 0.08, 0.4;
	{C, B9, D}, {%4, %4, %4}, 100, 200, 0.09, 0.4;
	! C: 9 !
	G, %1, 100, 600, 0.1, 0.4;
	F, %4, 100, 100, 0.1, 0.4;
	E, %2, 100, 300, 0.1, 0.4;
	D, %2, 100, 300, 0.1, 0.4;
	! F: 9 !
	C, %1+%2, 100, 400, 0.1, 0.4;
	{A8, B, C9}, {%4, %4, %4}, 100, 200, 0.05, 0.4;
	! Bb: 9 !
	{D, Bb8, C9}, {%3, %3, %3}, 100, 300, 0.05, 0.4;
	D, %2+%4, 100, 200, 0.05, 0.4;
	{C, D}, %4, 100, 200, 0.05, 0.4;
	! Eb: 9 !
	Eb, %2+%4, 100, 250, 0.05, 0.4;
	F, %2+%4, 100, 300, 0.05, 0.4;
	G,  %2+%4, 100, 350, 0.05, 0.4;
	! D: 9 !
	A, %1, 100, 400, 0.05, 0.4;
	C10, %2, 100, 450, 0.05, 0.4;
	A9, %2, 100, 500, 0.05, 0.4;
	D10, %4, 100, 600, 0.05, 0.4;
	! A: 9 !
	E10, (%4*9), 100, 800, 0.02, 0.4;
	! D: 9 !
	R, (%4*9);

! EXPO 2
	loop (2)
	begin
		vol 100;
		loop (2)
		begin
			! A: 18 !
			{ A8, C9, A8, B, C9 }, {(%4+%8), (%4+%8), %8, %8, %4}, 100, {400, 400, 100, 200, 400}, {0.1, 0.1, 0.05, 0.05, 0.1}, 0.4, 0.1, 0.5;
			{ A8, C9, C9, A8 }, {%8, %8, %8, %8}, 100, 100, 0.05, 0.4;
			{ C9, G#8, A8, C9, G#8 }, { %8, %8, %4, (%4+%8), (%4+%8) }, 100, {100, 200, 400, 400, 400}, {0.05, 0.05, 0.1, 0.1, 0.1}, 0.4;
			{ A8, C9, G#8, A, C9 }, {%4, %8, %8, %8, %8 }, 100, {400, 100}, {0.1, 0.05}, 0.4;
			{ G#8, A, C9, G#8, A8 }, { %8, %4, %8, %8, %8 };
			! D: 18 !
			{ F9, E, F, E }, {(%4+%8), (%4+%8), %4, %4}, 100, {400, 200, 400, 200}, 0.1, 0.4;
			{ F, E, E, F}, { %8, %8, %8, %8}, 100, 100, 0.05, 0.4;
			{ F, E, B8, F9, E, E}, {%8, %8, (%4+%8), %8, %8, %8}, 100, {100, 100, 200, 100, 100, 100}, 0.05, 0.4;
			{ F, E, B8, D9},  {%8, %8, (%4+%8), (%4+%8)}, 100, {100, 150, 200, 300}, 0.05, 0.4;
			{ C, E8, E, F, D}, {%8, %4, %8, %8, %8}, 100, {100, 400, 100, 100, 100}, 0.05, 0.4;
		end
		! G: 18 !
		G, %1, 100, 600, 0.1, 0.6;
		R, %1;
		vol 130;
		{G, A, B, F# }, {%2, %2, (%4+%8), %8}, 100, {200, 300, 400, 100}, {0.1, 0.1, 0.1, 0.05}, 0.4;
		{G, A, B, F#, G}, {%8, %8, (%4+%8), %4, %8}, 100, {100, 100, 400, 400, 100}, {0.05, 0.05, 0.1, 0.1, 0.05}, 0.4;
		! E: 9 !
		{ E, G, E, G}, {%1, %2, %2, %4}, 100, {400, 300, 400, 200}, 0.1, 0.4;
		! F: 9 !
		{ F, E, E, F, R}, {%8,%8,%8,%8,%4}, 100, 100, 0.05, 0.4;
		{ F, E, E, F, R, F}, {%8,%8,%8,%8,%8,%8}, 100, 150, 0.05, 0.4;
		{ E, E, F, R, E, F}, {%8,%8,%8,%8,%8,%8}, 100, 200, 0.05, 0.4;
		! D: 9 !
		D, (%4*6), 100, 400, 0.1, 0.4;
		{ C, E8, E, F, D}, {%8, %4, %8, %8, %8}, 100, {100, 200, 100}, 0.05, 0.4;
	end
	! A: 36 !
	{A8, C9, G#8, A, C9, G#8}, {%1, %2, (%4+%8), (%4+%8), %2, %1}, 100, 400, 0.1, 0.4;
	{A, C9, A8}, {%1, %2, (%1*2)}, 100, {600, 400, 600}, 0.1, 0.6;
	R, (%4*7);

	mark "develop";

! DEVEL
	A7, (%4*9), 100, 400, 0.1, 1;
	init 0 LINE 400, 200, 3;
	vol 80;
	loop (4)
	begin
		R, (%4*9);
		{ A6, A6, E7, C8, G, B, F#9 }, {%1, %24, %24, %24, %24, %24, %24 }, 100, fgen(0,1), 0.05, 2.5;
		R, %1;
	end
	loop (2)
	begin
		loop (2)
		begin
			{ R, B8, A, A, B, R }, { %2, %24, %24, %24, %24, %12 }, 100, 200, 0.01, 0.1;
			{ R, B, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
			{ R, B, B, A, A, B, R}, {%6, %24, %24, %24, %24, %12, %12};
			{ A, B, C9 }, {%24, %24, (%6+%4)};

			{ R, B8, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
			{ R, B, B, A, A, B, R }, { %12, %24, %24, %24, %24, %24, %12 };
			{ R, B, A, A, B, A, B }, { %4, %24, %24, %24, %24, %24, %24 };
			{ R, B, A, A, B, B, A, B }, { %8, %24, %24, %24, %8, %24, %24, %24 };
			{ R, B, A, R }, { %4, %24, %12, %4 };
			{ A, B, C9 }, { %24, %24, (%24+%8+%4+%8) };

			{ B8, A, B, C9 }, { %24, %24, %24, %4 };
			{ C9, B8, A, A, B, C9, B8 }, {%24, %24, %24, %24, %24, (%24+%6), %12 };
			{ F#, A, G#, B7 }, { %4, %6, %12, %4 };
			{ R, B, B, C8, G#7, G }, { %8, %24, %24, %24, %6, %12 };
			{ E8, D, C#, C, B7, A, A, R }, { %8, %8, %6, %6, %24, %24, %24, %24 };
			{ A7, A8, E7, C8, G, B, F#9 }, {%2, %24, %24, %24, %24, %24, %24 }, 100, 200, 0.05, 2;
		end
! - !
		{ E8, G, D#, E, C9 }, { %4, %2, %2, %8, (%1+%8) }, 100, {200, 400, 400, 100, 600}, 0.05, 0.4;
		{ B8, F#, A, G# }, { %8, %2, %4, %8 }, 100, {100, 400, 200, 100}, 0.05, 0.4;
		{ B7, B, B, C8 }, { %2, %8, %8, %8 }, 100, 200, {0.1, 0.01, 0.01, 0.05}, 0.4;
		{ G#7, G, E8, D, C#, C }, {%6, %12, %6, %12, %2, (%4+%8) }, 100, 0.2, 100, 0.05, 0.2;
! - !
		{ B7, A, A, B, R }, { %24, %24, %24, %8, %4 }, 100, 100, 0.01, 0.1;
		{ B, A, R }, { %24, (%24+%6), %4 };
		{ B, A, A, B }, { %24, %24, %24, %8 };
		{ R, B, A, A, A }, { %4, %24, %24, %24, (%8+%2+%1) };
		{ A6, A6, E7, C8, G, B, F#9 }, {%2, %24, %24, %24, %24, %24, %24 }, 100, 400, 0.05, 2;
	end
! D !
transpose 5;
	loop (2)
	begin
		{ R, B8, A, A, B, R }, { %2, %24, %24, %24, %24, %12 }, 100, 200, 0.01, 0.1;
		{ R, B, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
		{ R, B, B, A, A, B, R}, {%6, %24, %24, %24, %24, %12, %12};
		{ A, B, C9 }, {%24, %24, (%6+%4)};

		{ R, B8, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
		{ R, B, B, A, A, B, R }, { %12, %24, %24, %24, %24, %24, %12 };
		{ R, B, A, A, B, A, B }, { %4, %24, %24, %24, %24, %24, %24 };
		{ R, B, A, A, B, B, A, B }, { %8, %24, %24, %24, %8, %24, %24, %24 };
		{ R, B, A }, { %4, %24, %12 };
	end
! G !
transpose -2;
	{ B8, A, A, B, R }, { %24, %24, %24, %24, %12 };
	{ R, B8, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
	{ R, B, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
	{ R, B, B, A, A, B, R}, {%6, %24, %24, %24, %24, %12, %12};
	{ A, B, C9 }, {%24, %24, (%6+%4)};
! - !
	{ R, B8, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
	{ R, B, B, A, A, B, R }, { %12, %24, %24, %24, %24, %24, %12 };
	{ R, B, A, A, B, A, B }, { %4, %24, %24, %24, %24, %24, %24 };
	{ R, B, A, A, B, B, A, B }, { %8, %24, %24, %24, %8, %24, %24, %24 };
	{ R, B, A }, { %4, %24, %12 };
! C !
transpose 3;
	{ B8, A, A, B, R }, { %24, %24, %24, %24, %12 };
	{ R, B8, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
	{ R, B, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
	{ R, B, B, A, A, B, R}, {%6, %24, %24, %24, %24, %12, %12};
	{ A, B, C#9 }, {%24, %24, %6};
transpose 12;
	{C8, D, E, B7 }, {%2, %2, (%4+%8), %8}, 100, 400, 0.05, 0.4;
	{C8, D, E, B7, C8}, {%8, %8, (%4+%8), %4, %8};
! F: 9 !
	{ A7, C8, A7, C8}, {%1, %2, %2, %4}, 100, 400, 0.05, 0.4;
! Bb: 9 !
	{ Bb7, D8, Bb7, D8}, {%1, %2, %2, %4};
! Eb: 9 !
	{ Eb, D, D, Eb, R}, {%8,%8,%8,%8,%4};
	{ Eb, D, D, Eb, R, Eb}, {%8,%8,%8,%8,%8,%8};
	{ D, D, Eb, R, D, Eb}, {%8,%8,%8,%8,%8,%8};
! C: 9 !
	C, (%4*6);
	{ Bb7, D8, A7, Eb8, C}, {%8, %4, %8, %8, %8};
! G: 18 !
	{ G8, Bb, F#, G, R }, { (%4+%8), (%4+%8), %4, %1, %4 }, 100, 400, 0.1, 0.4;
	R, (%4*9);
! C: 9 !
	R, (%4*9);
! G: 18 !
	{ G, Bb, F#, G, R }, { %1, %1, %1, %1, %2 };
transpose 0;

! INTERLUDE
	R, %1*103;

! RECAP
	! C: 36 !
	transpose 24;
	double 12, 80;
	R, %1;
	{G6, B, F#, G, F#}, {%1, %2, %4, (%1*2), %2}, 70, 200, 0.1, 0.4;
	{ G6, F#}, {%2, %4}, 72, 210;
	{G, E7, D}, {%4, (%1*2), (%2+%4)}, 74, 220;
	! F: 18 !
	{A6, R, A, B, C7, C, B6}, {(%1+%4), %4, %6, %6, %6, %4, %4}, 76, 230, 0.1, 0.3;
	{D6, D, E, B5, A}, {%2, (%4+%6), %6, %6, %4}, 78, 240, 0.05, 0.3;
	{D6, E, B5, A6}, {%8, %8, %8, %8}, 80, 250, 0.05, 0.2;
	! C: 18 !
	{G6, R}, {%1, %2}, 82, 260, 0.1, 0.4;
	{G, F, E, D}, {%1, %4, (%2+%4), %1}, 84, 300, 0.1, 0.4;
	! F: 18 !
	{C, B5}, {%1, %2}, 86, 340;
	{C6, E, B5, C6}, {%4, %2, %4, %1}, 88, 360;
	{E, C}, {%2, %2}, 90, 380;
	loop (2)
	begin
		! C: 18 !
		{ C6, E6, C6, D, E6 }, {(%4+%8), (%4+%8), %8, %8, %4}, 100, 400, 0.05, 0.2;
		{ C6, E6, E6, C6 }, {%8, %8, %8, %8};
		{ E6, B5, C6, E6, B5 }, { %8, %8, %4, (%4+%8), (%4+%8) };
		{ C6, E, B5, C6, E6 }, {%4, %8, %8, %8, %8 };
		{ B5, C6, E6, B5, C6 }, { %8, %4, %8, %8, %8 };
		! F: 18 !
		{ A6, G, A, G }, {(%4+%8), (%4+%8), %4, %4};
		{ A, G, G, A}, { %8, %8, %8, %8};
		{ A, G, D, A6, G, G}, {%8, %8, (%4+%8), %8, %8, %8};
		{ A, G, D, F6},  {%8, %8, (%4+%8), (%4+%8)};
		{ E, G, G, A, F}, {%8, %4, %8, %8, %8};
	end
	! Bb: 18 !
	Bb, %1;
	R, %1;
	'transpose 24;
	{Bb, C7, D, A6 }, {%2, %2, (%4+%8), %8}, 100, 400, 0.1, 0.4;
	{Bb, C7, D, A6, Bb}, {%8, %8, (%4+%8), %4, %8}, 100, 400, 0.05, 0.2;
	! Eb: 9 !
	{ G, Bb, G, Bb}, {%1, %2, %2, %4}, 100, 400, 0.1, 0.4;
	! D: 9 !
	{ A, G, G, A, R}, {%8,%8,%8,%8,%4}, 100, 400, 0.05, 0.2;
	{ A, G, G, A, R, A}, {%8,%8,%8,%8,%8,%8};
	{ G, G, A, R, G, A}, {%8,%8,%8,%8,%8,%8};
	! A(6): 9 !
	E, (%4*6);
	{ C, E, E, C, E}, {%8, %4, %8, %8, %8};
	! Bb: 18 !
	D, %1, 100, 400, 0.1, 0.4;
	R, (%4*14);
	transpose 36;
	! A: 9 !
	{ C5, B4, E5, F, G, A }, { %1, %4, %6, %6, %6, %2 };
	! E: 9 !
	{ G, E6, D, B5, E6, D }, {%4, (%2+%4), %4, %2, %4, %4};
	! F: 18 !
	{ A5, R, A, B, C6, C, B5, D}, {%1, (%4*7), %3, %3, %3, %4, %4, %4 };
	! C: 9 !
	{G5, F, E, D}, { %1, %4, %2, %2 };
	! F: 9 !
	{C5, B4, C5, E, B4}, {%1, %4, %4, %2, %4};
	! C: 36 !
	{C5, E, B4, C5}, {(%1*2), %1, %4, (%1*4)};

end


voice 8
begin
	instr "OH";
	channel 0;
	volume 100;

! DEVELOP
	sync "develop";

	R, (%4*81);
	R, (%4*36);
	R, (%4*36);
	{ E9, G, D#, E, C10}, { %4, %2, %2, %8, (%1+%8) }, 100, 400, 0.05, 0.2;
	{ B9, F#, A, G# }, { %8, %2, %4, %8 };
	{ B8, B, B, C9 }, { %2, %8, %8, %8 };
	{ G#8, G, E9, D, C#, C }, {%6, %12, %6, %12, %2, (%4+%8) };
! - !
	{ B8, A, A, B, R }, { %24, %24, %24, %8, %4 };
	{ B, A, R }, { %24, (%24+%6), %4 };
	{ B, A, A, B }, { %24, %24, %24, %8 };
	{ R, B, A, A, A }, { %4, %24, %24, %24, (%8+%2+%1) };
	R, (%4*3);
	loop (2)
	begin
		{ R, R, B7, A, A, B, R }, { %2, %6, %24, %24, %24, %24, %6 }, 100, 200, 0.01, 0.1;
		{ R, B, A, A, B, R }, { %6, %24, %24, %24, %24, %6 };
		{ R, B, B, A, A, B, R }, { %8, %24, %24, %24, %24, %24, (%6+%4) };
		{ R, A, B, C8, B7, R }, { %8, %24, %24, %24, %24, %12 };
		! - !
		{ R, B, A, A, B, R }, { %24, %24, %24, %24, %24, %6 };
		{ R, B, B, A, A, B }, { %24, %24, %24, %24, %24, %24 };
		{ R, B, A, A, B, R }, { (%4+%8), %24, %24, %24, %12, %6 };
		{ R, B, A, A, B, B, A, B, R }, { %8, %24, %24, %24, %24, %24, %24, %24, %12};
		{ R, B, A, R, A, B, C8 }, { %2, %24, %24, %6, %24, %24, %24 };
		{ R, B7, A, B, C8, R }, { %4, %24, %24, %24, %24, %12 };
		{ R, R, R, B7, A, A, B }, { %8, %4, %12, %24, %24, %24, %24 };
		{ C8, B7, F#8, R }, { %6, %12, %4, %4 };
		{ R, B7, B, C8, G#7 }, { %8, %24, %24, %24, %4 };
		{ A, R }, { %4, %4 };
		R, (%4*3);
	end
	{ E9, G, D#, E, C8 }, { %4, %2, %2, %8, (%1+%8) }, 100, {200, 400, 400, 100, 600}, 0.05, 0.4;
	{ B9, F#, A, G# }, { %8, %2, %4, %8 }, 100, {100, 400, 200, 100}, 0.05, 0.4;
	{ B8, B, B, C9 }, { %2, %8, %8, %8 }, 100, 100, 0.05, 0.4;
	{ G#8, G, E9, D, C#, C }, {%6, %12, %6, %12, %2, (%4+%8) }, 100, 100, 0.05, 0.2;
	! - !
	{ B8, A, A, B, R }, { %24, %24, %24, %8, %4 }, 100, 200, 0.01, 0.1;
	{ B, A, R }, { %24, (%24+%6), %4 };
	{ B, A, A, B }, { %24, %24, %24, %8 };
	{ R, B, A, A, A }, { %4, %24, %24, %24, (%8+%2+%1) };
	R, (%4*3);
! D !
	transpose 5;
	loop (2)
	begin
		{ R, R, B7, A, A, B, R }, { %2, %6, %24, %24, %24, %24, %6 };
		{ R, B, A, A, B, R }, { %6, %24, %24, %24, %24, %6 };
		{ R, B, B, A, A, B, R }, { %8, %24, %24, %24, %24, %24, (%6+%4) };
		{ R, A, B, C8, B7, R }, { %8, %24, %24, %24, %24, %12 };
! - !
		{ R, B, A, A, B, R }, { %24, %24, %24, %24, %24, %6 };
		{ R, B, B, A, A, B }, { %24, %24, %24, %24, %24, %24 };
		{ R, B, A, A, B, R }, { (%4+%8), %24, %24, %24, %12, %6 };
		{ R, B, A, A, B, B, A, B, R }, { %8, %24, %24, %24, %24, %24, %24, %24, %12};
	end
! G !
	transpose -2;
	{ R, B7, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
	{ R, B7, A, A, B, R }, { %6, %24, %24, %24, %24, %6 };
	{ R, B, A, A, B, R }, { %6, %24, %24, %24, %24, %6 };
	{ R, B, B, A, A, B, R }, { %8, %24, %24, %24, %24, %24, (%6+%4) };
	{ R, A, B, C8, B7, R }, { %8, %24, %24, %24, %24, %12 };
	! - !
	{ R, B, A, A, B, R }, { %24, %24, %24, %24, %24, %6 };
	{ R, B, B, A, A, B }, { %24, %24, %24, %24, %24, %24 };
	{ R, B, A, A, B, R }, { (%4+%8), %24, %24, %24, %12, %6 };
	{ R, B, A, A, B, B, A, B, R }, { %8, %24, %24, %24, %24, %24, %24, %24, %12};
! C !
	transpose 3;
	{ R, B7, A, A, B, R }, { %4, %24, %24, %24, %24, %12 };
	{ R, B7, A, A, B, R }, { %6, %24, %24, %24, %24, %6 };
	{ R, B, A, A, B, R }, { %6, %24, %24, %24, %24, %6 };
	{ R, B, B, A, A, B, R }, { %8, %24, %24, %24, %24, %24, %6 };
	transpose 0;
	{C8, D, E, B7 }, {%2, %2, (%4+%8), %8},100, 400, 0.05, 0.4;
	{C8, D, E, B7, C8}, {%8, %8, (%4+%8), %4, %8};
! F: 9 !
	{ A7, C8, A7, C8}, {%1, %2, %2, %4};
! Bb: 9 !
	{ Bb7, D8, Bb7, D8}, {%1, %2, %2, %4};
! Eb: 9 !
	{ Eb, D, D, Eb, R}, {%8,%8,%8,%8,%4};
	{ Eb, D, D, Eb, R, Eb}, {%8,%8,%8,%8,%8,%8};
	{ D, D, Eb, R, D, Eb}, {%8,%8,%8,%8,%8,%8};
! C: 9 !
	C, (%4*6);
	{ Bb7, D8, A7, Eb8, C}, {%8, %4, %8, %8, %8};
! G: 18 !
	G, %4;
	R, (%4*14);
	transpose -2;
	{ A6, A6, E7, C8, G, B, F#9 }, {%2, %24, %24, %24, %24, %24, %24 },100, 400, 0.05, 2;
! C: 9 !
	transpose 3;
	R, (%4*6);
	{ A6, A6, E7, C8, G, B, F#9 }, {%2, %24, %24, %24, %24, %24, %24 }, 100, 300, 0.05, 2;
! G: 18 !
	transpose -2;
	R, (%4*15);
	{ A6, A6, E7, C8, G, B, F#9 }, {%2, %24, %24, %24, %24, %24, %24 }, 100, 200, 0.05, 2;
	transpose 0;
end