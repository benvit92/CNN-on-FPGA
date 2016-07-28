local helloworld = {}

function helloworld.sayhello()
	print('Hello World from lua!')
	local ffi = require("ffi")
	ffi.cdef[[
	int printf(const char *fmt, ...);
	]]
	ffi.C.printf("Hello World from c using starndard library!")
	ffi.cdef([[
	void helloWorld();
	]])
	clib = ffi.load('fpgatorch')
	clib.helloWorld()


	
end

function helloworld.sum(a,b)
	local ffi = require("ffi")
	ffi.cdef[[
	int sumInC(int x,int y);
	]]
	clib = ffi.load('fpgatorch')
	return clib.sumInC(a,b)
	
end


return helloworld
