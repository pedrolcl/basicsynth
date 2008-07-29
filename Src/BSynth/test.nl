tempo 4, 120

! attack in ms, release
map "Pulse"  18=0.001, 22=(1/1000);
map "SineWave" 18, 22

maxparam 2;
! xxx

voice 0
begin
	instr "Flut";
	vol 100;
	channel 2;
	loop 2
	begin
		C4, %4, 100;
		D4, %4, 100;
		E4, %4, 100;
		F4, %4, 100;
		G4, %4, 100;
		A4, %4, 100;
		B4, %4, 100;
	end
	C5, %1, 100;
end

voice 1
begin
	instr "Pulse";
	vol 100;
	channel 3;
	loop 2
	begin
		C3, %1, 100, 1, 20;
		G2, %4*3, 100, 100, 200;
	end
	C2, %1, 100, 500, 500;
end
