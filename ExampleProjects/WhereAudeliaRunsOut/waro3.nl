tempo 4, 104;

middlec 8;

map "Drone" {"g2.mnx":0.8, "g3.mnx":0.8}, "e0s0.rt", "e0s2.rt", "e1s0.rt", "e1.s2.rt", "g3.mul";
voice 1
begin
	instr "Drone";
	channel 1;
	volume 80;

! INTRO
	R, (%1*2);
! 57.69s
	[C6, G7, C9], (%1*26), {100, 80, 60}, 1.0,  30, 2.5, 50, 1, {2.005, 2.002, 1.001};
	[C6, G7, C9], (%1*28), {100, 80, 60}, 1.1,  30, 2.5, 50, 1;
	[C6, G7, C10], (%1*30), {100, 80, 60}, 1.0,  30, 2.5, 50, 1;
	[C6, G7, C10], (%1*9), {100, 80, 60}, 1.0,  4, 2.5, 10, 1;
	[F6, C8, F10], (%1*9),  {100, 80, 60}, 1.1,  4, 2.5, 10, 1;
	[C6, G7, C10], (%1*6), {100, 80, 60}, 1.0,  4, 2.5, 8, 1;
	[F6, C8, F10], (%1*14), {100, 80, 60}, 1.1,  4, 2.5, 20, 1;
	[C6, G7, C9], (%1*15), {100, 80, 60}, 1.0,  4, 12, 8, 1;

! EXPO 1
	volume 50;
	R, (%4*171);
	[F6, C8], (%4*18), {100, 80}, 1.0, 2, 1, 6, 1, 2.001;
	[C6, C7], (%4*36), {100, 80}, 1.0, 4, 2, 1, 10, 1;
	[F6, F7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 6, 1;
	[C6, C7], (%4*18);
	[F6, F7], (%4*18);
	[Bb6, Bb7], (%4*18);
	[Eb7, Eb8], (%4*9), {100, 80}, 1.0, 4, 2, 1, 2, 1;
	[D7, D8], (%4*9);
	[C7, C8], (%4*9);
	[Bb6, Bb7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 6, 1;
	[A6, A8], (%4*9), {100, 80}, 1.0, 4, 2, 1, 2, 1;
	[E6, E7], (%4*9);
	[F6, F7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 6, 1;
	[C6, C7], (%4*9), {100, 80}, 1.0, 4, 2, 1, 2, 1;
	[F6, F7], (%4*9);
	[Bb6, Bb7], (%4*9);
	[Eb6, Eb7], (%4*9);
	vol 55;
	[D6, D7], (%4*9);
	vol 60;
	[A6, A7], (%4*9);
	vol 70;
	[D6, D7], (%4*9);

! EXPO 2
	vol 50;
	loop (2)
	begin
		[A6, A7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 8, 1;
		[D6, D7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 8, 1;
		[A6, A7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 8, 1;
		[D6, D7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 8, 1;
		[G6, G7], (%4*18), {100, 80}, 1.0, 4, 2, 1, 8, 1;
		[E6, E7], (%4*9), {100, 80}, 1.0, 4, 2, 1, 2, 1;
		[F6, F7], (%4*9), {100, 80}, 1.0, 4, 2, 1, 2, 1;
		[D6, D7], (%4*9), {100, 80}, 1.0, 4, 2, 1, 2, 1;
	end
	[A6, A7], (%4*36), {100, 80}, 1.0, 4, 2, 4, 10, 4;

! DEVEL
	R, %4*9;
	loop (4)
	begin
		R, %4*14;
		[E6, C7], %1, 100, 1.0, 4, 1, 1, 1;
	end
	R, %4*36;
	[E6, C7], %4*9, 75, 1.0, 4, 2, 2, 3;
	R, %4*27;
	[E6, C7], %4*9;
	R, %4*30;
	R, %4*36;
	[E6, C7], %4*9;
	R, %4*27;
	[E6, C7], %4*39;
	! - !
	[F6, D7], %1*2;
	R, %4*28;
	[G6, G7], %1*2;
	R, %4*10;
	[C6, C7], %1*2;
	R, %4*10;
	[F6,F7], %1*2;
	R, %4;
	[Bb6,Bb7], %1*2;
	R, %4;
	[Eb6,Eb7], %1*2;
	R, %4;
	[C6,C7], %1*2;
	R, %4;
	[G6,G7], %1*2;
	R, %4*10;
	[C6,C7], %1*2;
	R, %4;
	[G6,G7], %1*2;
	R, %4*10;

! INTERLUDE
	mark "interlude";
	vol 100;
	G6, %1*14, 75, 1.0, 1, 20, 1, 20, 2.000;

	G5, %1*3,  40, 0.5, 4, 1, 1,  4;
	G6, %1*12, 44, 0.5, 4, 8, 1, 20;

	C5, %1*3,  54, 0.5, 4, 2, 1,  4;
	C6, %1*10+%2, 58, 0.5, 4, 8, 1, 12;
	R,  %1;

	D5, %1*5+%2,58, 0.5, 4, 4, 1,  8;
	Eb5,%1*2+%2,60, 0.5, 4, 2, 1,  4;
	F5, %1*6,  62,  0.5, 4, 2, 1,  6;
	 
	G5, %1*15+%2,68, 0.5, 4, 20, 1, 30;  
	G6, %1*11, 72, 0.6,  4, 12, 1, 20;
	C6, %1*10, 76, 0.7,  4, 10, 1, 16;
	C5, %1*4,   80, 0.8,  4, 1,  1, 8;

! RECAP
	vol 80;
	R, (%1*5);
	! C: 36 !
	vol 50;
	[ C5, C7 ], (%4*36), {100, 80}, 1.0, 4, 2, 2, 6, 2.001;
	! F: 18 !
	[ F5, F7 ], (%4*18);
	! C: 18 !
	[ C5, C7 ], (%4*18);
	! F: 18 !
	[ F5, F7 ], (%4*18);
	! C: 18 !
	[ C5, C7 ], (%4*18);
	! F: 18 !
	[ F5, F7 ], (%4*18);
	! C: 18 !
	[ C5, C7 ], (%4*18);
	! F: 18 !
	[ F5, F7 ], (%4*18);
	! Bb: 18 !
	[ Bb5, Bb7 ], (%4*18);
	! Eb: 9 !
	[Eb5, Eb7 ], (%4*9);
	! D: 9 !
	[D5, D7 ], (%4*9);
	! A(6): 9 !
	[C5, C7 ], (%4*9);
	! Bb: 18 !
	[ Bb5, Bb7 ], (%4*18);
	! A: 9 !
	[ A5, A7 ], (%4*9);
	! E: 9 !
	[E5, E7 ], (%4*9);
	! F: 18 !
	[ F5, F7 ], (%4*18);
	! C: 9 !
	[ C5, C7 ], (%4*9);
	! F: 9 !
	[ F5, F7 ], (%4*9);
	! C: 36 !
	[ C5, C7 ], (%4*36);
end

voice 2
begin
	instr "Drone";
	volume 100;
	channel 2;

! INTRO
	R, (%1*2);
	R, (%1*26);
	R, (%1*28);
	R, (%1*30);
	R, (%1*9);
	R, (%1*9);
	R, (%1*6);
	R, (%1*14);
	R, (%1*12);
	R, (%1*3);

! EXPO 1
	volume 80;
	loop (2)
	begin
		R, (%4*36);
		! 36 !
		R, (%4*5); 
		[C9, C10], (%4*8), 100, {0.3, 0.25}, 2, 2, 4, 1, 1.001; 
		R, (%4*10);
		[C9, C10], (%4*18), 100, {0.5, 0.25}, 6, 2, 8, 1;
		[A9, A10], (%4*13), 100, {0.5, 0.25}, 4, 2, 6, 1;
		! 54 !
		R, (%4*5); 
		[D9, D10],   (%4*9), 100, {0.6, 0.25}, 2, 2, 4, 1;
		[Bb9, Bb10], (%4*7), 100, {0.6, 0.25}, 2, 2, 3, 1;
		[G9, G10],   (%4*3), 100, {0.65, 0.25}, 1, 0.5, 1, 0.5; 
		[F9, G10],   (%4*8), 100, {0.7, 0.25}, 1, 1, 4, 1;
		[A9, A10],   (%4*13), 100, {0.75, 0.25}, 4, 2, 6, 1;
		! 45 !
		R, (%4*23);
		[E9, E10],   (%4*13), 100, {0.75, 0.25}, 4, 2, 6, 1;
		! 36 !
		R, (%4*5);
		[C9, C10],   (%4*13), 100, {1.0, 0.25}, 4, 2, 6, 1;
		volume 60;
	end
	[C9, C10],  (%4*18), 100, {1.0, 0.25}, 4, 4, 8, 1;
	[Bb9, Bb10], (%4*18), 100, {1.0, 0.5}, 4, 4, 8, 1;
	[A9, A10],   (%4*27), 100, {1.0, 0.5}, 4, 6, 8, 6;

! EXPO 2

	vol 70;
	loop (2)
	begin
		[A9, A10], (%4*(18*4)), 100, {1.5, 1.0}, 10, 1, 40, 1;
		[B9, B10], (%4*27), 100, {1.5, 1.0}, 4, 1, 14, 1;
		[C10, C11], (%4*18), 100, {1.5, 1.0}, 2, 1, 8, 1;
	end
	[A9, A10], (%4*36), 100, {1.5, 1.0}, 4, 20, 20, 20;

! DEVELOP

! INTERLUDE
	sync "interlude";
	vol 130;
	R, %1*45;
	D9, %2,  60, 0.5, 4, 0.5, 1, 1, 1.0001;
	A9, %1*6, 61, 0.5, 4, 0.5, 1, 6;
	B9, %2,  62, 0.5, 4, 0.5, 1, 1;
	Bb9, %2, 63, 0.5, 4, 0.5, 1, 1;
	G9, %2,  64, 0.5, 4, 0.5, 1, 1;
	F9, %2,  65, 0.5, 4, 0.5, 1, 1;
	E9, %1*2,66, 0.6, 4, 1, 1, 2;
	Eb9,%1*2,68, 0.65, 4, 1, 1, 2;
	R, %1*7+%2;
	D8, %1*10, 80, 0.7, 4, 1, 1, 20;
	R, %4*12;
! RECAP

end

voice 9
begin
	instr "Drone";
	volume 130;
	channel 8;

	sync "interlude";

	R, (%1*8);
	
	Bb8, %1*7,   40, 0.5, 4,  4,  1, 10, 1.0002;
	C9, %1*2,    41, 0.5, 4,  2,   1,  3;
	B8, %2,        42, 0.5, 4, 0.5, 1,  1;
	D8, %1*2+%2, 43, 0.5, 4,  1, 1, 4;
	Eb8, %2,     43, 0.5, 4, 0.5, 1,  1;

	B7,  %1,     44, 0.5, 4, 1,   1,  1;
	Bb7, %1*2,   45, 0.6, 4, 1,   1,  3;
	G8,  %1*3+%2, 46, 0.65, 4, 4,   1,  6;

	F8,  %2,     46, 0.5, 4, 0.5, 1,  1, 0.5;
	E8,  %2,     47, 0.5, 4, 0.5, 1,  1, 0.5;
	Eb8, %2,    48, 0.5, 4, 0.5, 1,  1, 0.5;
	D8,  %2,    48, 0.5, 4, 0.5, 1,  1, 0.5;

	C8,  %1*2,   49, 0.5, 4, 0.5, 1,  3;
	R,   %1;

	Eb9, %1,     50, 0.5, 4, 1,   1,  1;
	D9,  %2,     51, 0.5, 4, 0.5, 1,  1;
	A8,  %1*2,  52, 0.5, 4, 0.5, 1,  3;

	Eb8, %2,     53, 0.5, 4, 0.5, 1,  1;
	B8,  %2,     54, 0.5, 4, 0.5, 1,  1;
	Bb8, %2,     55, 0.5, 4, 0.5, 1,  1;
	G8,  %1+%2,  56, 0.5, 4, 0.5, 1,  2;
	Bb8, %1*2,   57, 0.5, 4, 0.5, 1,  3;
	F#8, %1,     58, 0.5, 4, 0.5, 1,  1;
	!!
	G8,  %1*16,  60, 0.5, 4, 10,  1, 20;
	R,   %2;
	G8,  %1*6,   64, 0.5, 4, 4,   1, 8,  1;

	Ab8, %2,     66, 0.5, 4, 0.5, 1, 1,  1;
	G8,  %1*3+%2, 68, 0.5, 4, 0.5, 4, 6,  1;
	R,   %1*9;

	Bb8, %1*6+%2, 72, 0.5, 4, 4,  1, 8;
	F#8, %2,     73, 0.5, 0.5, 1, 1,  1; 
	G8,  %1*6,   74, 0.5, 0.5, 1, 6,  1;

	R,   %1;
	F8,  %2,     75, 0.5, 0.5, 1, 1,  1;
	E8,  %2,     76, 0.5, 0.5, 1, 1,  1;
	Eb8, %1,    77, 0.5, 0.5, 1, 1,  1;
	D8,  %1,    78, 0.5, 0.5, 1, 1,  1;
	C8, %1*4,  80, 0.5, 0.5, 1, 6,  1;

end

voice 10
begin
	instr "Drone";
	volume 120;
	channel 1;

	sync "interlude";
	R, %1*47;
	C10, %1*2, 64, 0.5, 0.5, 1, 2, 1, 1.0001;
	B9, %2, 64, 0.5, 4, 0.5, 1, 1;
	D10, %1, 64, 0.5, 4, 0.5, 1, 1;
	Eb10, %1*5, 64, 0.5, 4, 0.5, 1, 8;

end

voice 11
begin
	instr "Drone";
	volume 130;
	channel 9;

	sync "interlude";

	G8,  %1*2, 80, 0.5, 1, 1, 2, 1, 1.0001;
	Bb8, %1*2, 76, 0.5, 1, 1, 2, 1;
	F#8, %1*2, 72, 0.5, 1, 1, 2, 1;

	G8,  %1*4, 56, 0.5, 1, 1, 4, 1;
	F#8, %1*2, 54, 0.5, 1, 1, 2, 1;
	Eb9, %1,   52, 0.5, 1, 1, 1, 1;
	D9,  %2,   48, 0.5, 0.5, 1, 1, 1;
	A8,  %1*10,44, 0.5, 1, 1, 20, 1;
	R, %2;

	F8, %2, 44, 0.5, 0.5, 1, 1, 1, 1;
	E8, %2, 45, 0.5, 0.5, 1, 1, 1, 1;
	Eb8, %2, 46, 0.5, 0.5, 1, 1, 1, 1;
	D8, %2, 47, 0.5, 0.5, 1, 1, 1, 1;
	C8, %1*2, 48, 0.5, 1, 1, 3, 1, 1;
	R, %1*4;
	C8, %1*2, 50, 0.5, 1, 1, 2, 1;
	C9, %2, 52, 0.5, 0.5, 1, 1, 1;
	B8, %2, 54, 0.5, 0.5, 1, 1, 1;
	D8, %1*7+%2, 56, 0.5, 1, 1, 12, 1;
	Bb8, %2, 58, 0.5, 0.5, 1, 1, 1;
	F#8, %2, 59, 0.5, 0.5, 1, 1, 1;

	Eb9, %1*8, 60, 0.5, 1, 1,10, 1; 
	D9,  %1*4, 60, 0.5, 1, 1, 6, 1; 
	R, %1*2;

	D9, %2, 64, 0.5, 0.5, 1, 1, 1;
	Eb9, %1*2,  65, 0.5, 0.5, 1, 3, 1;
	B8, %1*2, 66, 0.5, 0.5, 1, 3, 1;
	C9, %2,  67, 0.5, 0.5, 1, 1, 1;
	Eb9, %2, 68, 0.5, 0.5, 1, 1, 1;
	B8, %1*4, 68, 0.5, 0.5, 1, 6, 1;
	F8, %2, 69, 0.5, 0.5, 1, 1, 1;
	E8, %2, 69, 0.5, 0.5, 1, 1, 1;

	C9, %1*2, 69, 0.5, 0.5, 1, 3, 1;
	B8, %2,   70, 0.5, 0.5, 1, 1, 1;
	Bb8, %2,  71, 0.5, 0.5, 1, 1, 1;
	A8, %1*2, 72, 0.5, 0.5, 1, 3, 1;

	G8, %1*6, 73, 0.5, 1, 1, 8, 1;
	F#8, %1*2,  74, 0.5, 1, 1, 2, 1;
	G8, %1*2, 75, 0.5, 1, 1, 2, 1;
	R, %1;

	Eb8, %2, 76, 0.5, 0.5, 0.5, 0.5, 0.5;
	D8,  %2, 77, 0.5, 0.5, 0.5, 0.5, 0.5;
	A7,  %2, 77, 0.5, 0.5, 0.5, 0.5, 0.5;
	C8,  %2, 77, 0.5, 0.5, 0.5, 0.5, 0.5;
	B7,  %2, 78, 0.5, 0.5, 0.5, 0.5, 0.5;
	D8, %1*2, 78, 0.5, 0.5, 1, 3, 1;
	F8, %2, 78, 0.5, 0.5, 1, 1, 1;
	B7, %2, 79, 0.5, 0.5, 1, 1, 1;
	Bb7, %1, 79, 0.5, 1, 1, 3, 1;
	G7, %1*7+%2, 80, 0.5, 1, 1, 12, 1;
end