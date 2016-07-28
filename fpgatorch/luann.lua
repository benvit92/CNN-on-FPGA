require 'nn'

local luann = {}

local fwrite = function(tensor, file)
  if not tensor then return false end
  local n = tensor:nElement()
  local s = tensor:storage()
  return assert(file:writeDouble(s) == n)
end

function luann.train(lenght)
	local m = nn.Linear(lenght, lenght)

	print(m.weight)
	print(m.bias)

	local file = torch.DiskFile("net.bin","w"):binary()
	fwrite(m.weight, file)
	fwrite(m.bias, file)

	file:close()
	
	return
end

function luann.showInC(lenght)
	local ffi = require("ffi")
	ffi.cdef([[
	void readTensor(const int N);
	]])
	clib = ffi.load('fpgatorch')
	clib.readTensor(lenght)
end

function luann.testPipe(lenght)
	local ffi = require("ffi")
	ffi.cdef([[
	void testPipe(const int N);
	]])
	clib = ffi.load('fpgatorch')

	clib.testPipe(lenght)
	return
end

function luann.testTwoWayPipe(lenght)
	local ffi = require("ffi")
	ffi.cdef([[
	void testTwoWayPipe(const int N);
	]])
	clib = ffi.load('fpgatorch')

	clib.testTwoWayPipe(lenght)
	return
end
return luann
