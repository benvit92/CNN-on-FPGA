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

	local file = torch.DiskFile("net.bin", 			"w"):binary()
	fwrite(m.weight, file)
	fwrite(m.bias, file)
	
	return
end
