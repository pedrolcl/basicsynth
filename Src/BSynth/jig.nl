tempo 4, 148;

map "Fidl" 31;

voice 0
begin      
	instr "Fidl";   
	channel 0;            
	artic percent, param;  
	transpose -12;   
	R, %1;  
	R, %1;   
	R, %1;
	loop 2
	begin
		loop 4
		begin
			{C5, C5, C5, G5, C5, C5}, %12, {100, 80, 80, 100, 80, 100}, {60, 100, 100, 60, 100, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07};
		end
		loop 2
		begin
			{C5, G5, A5, F5, E5, G5}, %12, {100, 80, 80, 100, 80, 100}, {100, 80, 80, 100, 80, 80}, {0.09, 0.04, 0.4, 0.09, 0.09, 0.07};
		end        
		{F5, E5, D5, G4}, {%8, %8, %6, %12}, 100, {60, 60, 80, 100};
		loop 2
		begin
			{C5, C5, C5, G5, C5, C5}, %12, {100, 80, 80, 100, 80, 100}, {80, 100, 80, 80, 100, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07};
		end
		{C5, G5, G5, F5, E5, G5}, %12, {100, 80, 80, 100, 80, 100}, {80, 100, 80, 80, 100, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07};
		{F5, E5, D5, G4}, {%8, %8, %6, %12}, 100, {60, 60, 80, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07};
		{C5, G5, A5, F5, E5, G5}, %12, 100, {80, 100, 80, 80, 100, 100}, {0.04, 0.09, 0.09, 0.04, 0.09, 0.07}; 
		transpose 0;
	end               
	[C5, G5], %2, {80, 80}, 100, 0.02;
end          

voice 1
begin      
	instr "Fidl";   
	channel 3;            
	artic percent, param;  
	C1, %1, 80, 100;
	C2, %1, 80, 100;
	C1, %1, 80, 100;
	loop 6
	begin  
		C2, %1, 50, 100;
		C1, %1, 30, 100;
	end       
	C1, %2, 30, 100;
end

voice 2
begin      
	instr "Bass";   
	channel 1;     
	artic fixed, 0.3;
	loop 6
	begin
		[C3, G3], %4, 80;
		G2, %4, 100;
	end
	loop 2
	begin
		loop 11
		begin                 
			[C3, G3], %4, 60;
			G2, %4, 80;
		end
		{G2, A2, B2}, %6, 100;  
	end       
	C3, %2, 80;
end

voice 3
begin      
	instr "Bass";  
	channel 2;     
	artic fixed, 0.4;   
	R, %1;   
	loop 4
	begin           
		R, %6;
		{C4, G4, C4}, %12, 90;
		R, %12;
	end
	loop 2
	begin
		loop 11
		begin                 
			R, %6;
			C4, %12, 80;
			G4, %12, 100;
			C4, %12, 80;
			R, %12;
		end
		{G4, R, R, D4}, {%6, %6, %12, %12}, 100;  
	end       
	C4, %2, 80;
end
