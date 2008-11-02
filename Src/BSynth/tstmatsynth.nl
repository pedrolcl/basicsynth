' Test Matrix Synth
' Daniel R. Mitchell
tempo 4, 60;

voice 0
begin      
	instr "Test";
	channel 0;
	volume 100;
	artic percent 80;
	'LFO frequency, wavetable, attack, level
	map "Test" 16, 17, 18, 19;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 2.0, 2.5, 3.0 3.5, 4.0 4.5, 5.0},
	{ 0, 0, 1, 1, 2, 2, 0},
	{ 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6},
	{ 0.05, 0.10, 0.2, 0.3, 0.4, 0.5, 0.8};
	R, %4;
	'OSC on3, vol3, on4, vol4
	map "Test" 2576, 2565, 2832, 2821;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 1, 1, 1, 0, 0, 0, 1 },
	{ 0.4, 0.6, 0.8, 0, 0, 0, 0.5 },
	{ 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 0, 0, 0.4, 0.6, 0.8, 0.5 };
	R, %4;
	' Amplitude EG attack, sustain, release
	map "Test" 4098, 4107, 4114;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 0.05, 0.10, 0.15, 0.20, 0.30, 0.40, 0.10},
	{ 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.00},
	{ 0.05, 0.10, 0.15, 0.20, 0.30, 0.40, 0.2};
	R, %4;
	' Modulator EG attack, sustain, release
	map "Test" 4354, 4363, 4370;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 0.05, 0.10, 0.15, 0.20, 0.30, 0.40, 0.10},
	{ 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.00},
	{ 0.05, 0.10, 0.15, 0.20, 0.30, 0.40, 0.2};
	R, %4;
	' Mod multiple (osc * 256) + 2048 + 3
	map "Test" 2307;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 5.0, 4.0, 3.0, 2.0, 1.667, 1.41, 1.0};
	R, %4;
	' Mod index (osc * 256) + 2048 + 4
	map "Test" 2308;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 8.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
	R, %4;
	' Panning
	map "Test" 2062, 2063;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ -1, -0.6, -0.2, 0.2, 0.6, 1.0, 0.0}, 1;
	R, %4;
	' FX1
	artic percent 40;
	map "Test" 2055, 2066;
	{C4, C4, C4, C4, C4, C4, C4}, %4, 100, 
	{ 0.01, 0.05, 0.10, 0.15, 0.20, 0.25, 0.0}, 1;
	R, %4;
end
