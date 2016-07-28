require "torch"
require "nn"
require "math"

-- my open source tensor-to-c parser, feel free to use it
function print_tensor(t,D1,D2,D3,D4)
	str = "{"
	nD = t:nDimension() --the number of dimensions
	if (nD ==1) then
		for i=1,D1 do
			str = str .. t[i]
			if (i ~= D1) then
				str = str .. ","
			end
		end
	else
		for i=1,D1 do
			str = str .. print_tensor(t[i],D2,D3,D4)
			if(i ~= D1) then
				str = str .. ","
			end
		end
	end
	str = str .. "}"
	return str
end

--writes a .bin file
local fwrite = function(tensor, file)
  if not tensor then return false end
  local n = tensor:nElement()
  local s = tensor:storage()
  return assert(file:writeDouble(s) == n)
end

--writes a .bin file
local fread = function()
	local out = assert(io.open("output.txt", "rb"))
	lines = {}
	for line in out:lines() do
		lines[#lines +1] = line
	end
	out:close()
	
	result= torch.Tensor(10)
	for i=1,10 do
		result[i] = tonumber(lines[i])
	end
	return result
end

-- global variables



-- here we set up the architecture of the neural network
function create_network()

   local ann = nn.Sequential();  -- make a multi-layer structure
   
   -- 1x16x16                       
   ann:add(nn.SpatialConvolution(1,6,5,5))   -- becomes 6x12x12
	ann:add(nn.SpatialMaxPooling(2,2,2,2))   -- becomes 6x6x6
 
   ann:add(nn.Reshape(6*6*6)) --becomes 216
   ann:add(nn.Tanh())
   ann:add(nn.Linear(6*6*6,10)) --216 in, 10 out.
   ann:add(nn.LogSoftMax())
   
   return ann
end

-- train a Neural Network and writes its weights
function train_network( network)

	--I create some variables
	dataset, test_dataset, classes, classes_names = dofile('usps_dataset.lua')
   local learningRate = 0.01
	local maxIterations = 100000
   print( "Training the network" )
   local criterion = nn.ClassNLLCriterion()
   
   for iteration=1,maxIterations do
      local index = math.random(dataset:size()) -- pick example at random
      local input = dataset[index][1]               
      local output = dataset[index][2]
      
      criterion:forward(network:forward(input), output)
      
      network:zeroGradParameters()
      network:backward(input, criterion:backward(network.output, output))
      network:updateParameters(learningRate)
   end
	--here I start writing all the weights. It is hardcoded, yea I know
	file = io.open("convW.txt","w")
	file:write(print_tensor(network.modules[1].weight,6,1,5,5))
	file:close()

	file = io.open("convB.txt","w")
	file:write(print_tensor(network.modules[1].bias,6))
	file:close()

	file = io.open("lineW.txt","w")
	file:write(print_tensor(network.modules[5].weight,10,216))
	file:close()

	file = io.open("lineB.txt","w")
	file:write(print_tensor(network.modules[5].bias,10))
	file:close()

end


dataset, test_dataset, classes, classes_names = dofile('usps_dataset.lua')

function test_predictor()
		--I create some variables
        local mistakes = 0
        local tested_samples = 0
        
        local ffi = require("ffi")
		ffi.cdef([[
		void luaForward(void);
		]])
		clib = ffi.load('mycnn')
        
        print( "----------------------" )
        print( "Index Label Prediction" )
        for i=1,test_dataset:size() do

               local input  = test_dataset[i][1]
               local class_id = test_dataset[i][2]
				
				-- roba in c
				local file = torch.DiskFile("input.bin","w"):binary()
				fwrite(input, file)
				file:close()
				
				clib.luaForward()
				
				--I read the results from a file
				local responses_per_class = fread()
				-- fine roba in c
				
               --local responses_per_class  =  predictor:forward(input) 
               local probabilites_per_class = torch.exp(responses_per_class)
               local probability, prediction = torch.max(probabilites_per_class, 1) 

                      
               if prediction[1] ~= class_id then
                      mistakes = mistakes + 1
                      local label = classes_names[ classes[class_id] ]
                      local predicted_label = classes_names[ classes[prediction[1] ] ]
                      print(i , label , predicted_label )
               end

               tested_samples = tested_samples + 1
        end

        local test_err = mistakes/tested_samples
        print ( "Test error " .. test_err .. " ( " .. mistakes .. " out of " .. tested_samples .. " )")

end
function test_one_input(i)
		--I create some variables
        local mistakes = 0
        local tested_samples = 0
        
        local ffi = require("ffi")
		ffi.cdef([[
		void luaForward(void);
		]])
		clib = ffi.load('mycnn')
        
        print( "----------------------" )
        print( "Index Label Prediction" )

               local input  = test_dataset[i][1]
               local class_id = test_dataset[i][2]
        print("class id is "..class_id)
				
				-- roba in c
				local file = torch.DiskFile("input.bin","w"):binary()
				fwrite(input, file)
				file:close()
				
				clib.luaForward()
				
				--I read the results from a file
				local responses_per_class = fread()
				-- fine roba in c
				
               --local responses_per_class  =  predictor:forward(input) 
               local probabilites_per_class = torch.exp(responses_per_class)
               local probability, prediction = torch.max(probabilites_per_class, 1) 
		print("prediction is "..prediction[1])
		
                      
               if prediction[1] ~= class_id then
                      print( "WRONG!")
                      print(i , label , predicted_label )
               else 
					print("CORRECT!")
				end

end

function main()
	print("Usage:")
	print("createNetwork() creates a new cnn and returns it")
	print("train_network(net) takes a network, trains it and writes in some .bin files every weight")
	print("test_predictor() tests via C the neural network, with 1000 samples")
	print("test_one_input(index) tests via C the neural network, with 1 sample")
end

main()
