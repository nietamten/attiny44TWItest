local M = require 'posix.termio'
local F = require 'posix.fcntl'
local U = require 'posix.unistd'
local P = require 'posix.stdio'

local dev = arg[1] or '/dev/ttyACM0'
local rserial = assert( io.open( dev, "a+" ) )
local fd = assert( P.fileno( rserial ) )

M.tcsetattr(fd, 0, {
   cflag = M.B115200 + M.CS8 + M.CLOCAL + M.CREAD,
   iflag = M.IGNPAR,
   oflag = M.OPOST,
   cc = {
      [M.VTIME] = 0,
      [M.VMIN] = 1,
   }
})


fl = require("moonfltk")

chart = nil;

devAddr = 0x02;

regs = {}; 
regs[1] = {0,'charge enabled',fl.rgb_color(250,0,0),2};
regs[2] = {1,'discharge enabled',fl.rgb_color(150,0,0),2};
regs[3] = {2,'max charge current',fl.rgb_color(150,100,0),2};
regs[4] = {3,'max charge cell voltage',fl.rgb_color(150,100,1000),1};
regs[5] = {4,'max discharge current',fl.rgb_color(100,150,0),4};
regs[6] = {5,'min discharge cell voltage',fl.rgb_color(100,200,0),1};
regs[7] = {6,'current',fl.rgb_color(100,0,100),1};
regs[8] = {7,'auto balancer',fl.rgb_color(0,100,0),1};
regs[9] = {8,'balancer state',fl.rgb_color(0,100,100),1};

regs[10] = {9,'vc1',fl.rgb_color(250,100,0),3};
regs[11] = {10,'vc2',fl.rgb_color(250,0,100),3};
regs[12] = {11,'vc3',fl.rgb_color(250,200,0),3};
regs[13] = {12,'vc4',fl.rgb_color(250,0,200),3};
regs[14] = {13,'vc5',fl.rgb_color(250,100,100),3};
regs[15] = {14,'vc6',fl.rgb_color(250,200,200),3};
regs[16] = {15,'vc7',fl.rgb_color(250,50,100),3};
regs[17] = {16,'vc8',fl.rgb_color(250,100,50),3};

chartSize = 10;

function read(widget, value)
	msg = "r"..string.format('%02X', devAddr)..string.format('%02X', value[0].in1:value());
	print(">"..msg);
	rserial:write(msg);
	
	res = "";
	c = "";
	repeat
		res = res..c;
		c = rserial:read( 1 );
	until(c == "*" );
	print("<"..res);
	value[0].in2:value(res);
end;

function write(widget, value)
	msg = "w"..string.format('%02X', devAddr)..string.format('%02X', value[0].in1:value())..value[0].in2:value();
	print(">"..msg);
	rserial:write(msg);
	
	res = "";
	c = "";
	repeat
		res = res..c;
		c = rserial:read( 1 );
	until(c == "*" );
	print("<"..res);
end;

function readall(widget, valu)
	for index,value in ipairs(regs) do
		read(nil,value);
	end
end;

function writeall(widget, valu)
	for index,value in ipairs(regs) do
		write(nil,value);	
	end
end;

function timer()
	--nv = nv +1;
	--if nv > 99 then
	--	nv = 0;
	--end;
	for index,value in ipairs(regs) do
		if value[0].cb1:value() then
			
			read(nil,value);
			nv = tonumber(value[0].in2:value(),16);
			
			if nv == nil then
			    return;
			end

			minus = (nv>=512);
			if minus then
				nv = nv-1023;
			end
			
			update = false;
			lower, upper = chart:bounds( );
			if upper < nv then
				upper = nv;
				update = true;
			end;
			if lower > nv then
				lower = nv;
				update = true;
			end;
			if update then
				chart:bounds(lower,upper);
			end;

			if not value[-1][1] then
				for i=1,chartSize  do 
					chart:insert(i,0,0,value[3]);
					value[-1][i] = 0;
				end;
			end
			
			for i=1,chartSize  do 				
				if i == chartSize then
					chart:replace(i,nv,nv,value[3]);
					value[-1][i] = nv;
				else 
					chart:replace(i,value[-1][i+1],value[-1][i+1],value[3]);
					value[-1][i] = value[-1][i+1];
				end
			end
			
		end
	end;
end;

width = 530;
height = 640;

win = fl.window(width,height, arg[0])

	lblA = fl.box(25, 1, 30, 20, "reg");
	--lblA:align( fl.alignment('right'));
	
	lblB = fl.box(55, 1, 80, 20, "val (hex)");
	--lblB:align( fl.alignment('right'));	

	btn1 = fl.button(150, 1, 70, 20, "read all");
	btn1:callback(readall);
	btn2 = fl.button(225, 1, 70, 20, "write all");
	btn2:callback(writeall);
	
y=25;

for index,value in ipairs(regs) do
	ctrls = {};
	
	ctrls.cb1 = fl.check_button(5, y, 20, 20, "");
	ctrls.cb1:down_box('down box');
	--cb1:set(0);
		
	ctrls.in1 = fl.int_input(25, y, 30, 20, "");
	ctrls.in1:value(index);	

	ctrls.in2 = fl.input(55, y, 80, 20, "");
	ctrls.in2:value(value[1]);
	
	ctrls.btn1 = fl.button(150, y, 70, 20, "read");
	ctrls.btn1:callback(read,value);
	ctrls.btn2 = fl.button(225, y, 70, 20, "write");
	ctrls.btn2:callback(write,value);
	
	ctrls.lbl = fl.box(150, y, 165, 20, value[2]);
	ctrls.lbl:align( fl.alignment('right'));
	
	ctrls.lbl:labelcolor(value[3]);
	
	value[0] = ctrls;
	value[-1] = {};
	
	y=y+25;
end;

chart = fl.chart(0,y,width,height-y-5);
chart:type('line');
chart:maxsize(chartSize);
chart:bounds(0,10);
--chart:autosize(true);

fl.set_timeout(0.1, true, timer);

win:done() 
win:show(arg[0], arg)

fl.run();

rserial:close();


