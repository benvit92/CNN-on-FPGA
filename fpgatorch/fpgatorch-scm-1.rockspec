package = "fpgatorch"
version = "scm-1"

source = {
   url = "127.0.0.1",
   tag = "master"
}

description = {
   summary = "A FPGA Torch back end",
   detailed = [[
   	    A stissa cusa
   ]]
}

dependencies = {
   "torch >= 7.0"
}

build = {
   type = "command",
   build_command = [[
cmake -E make_directory build;
cd build;
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(LUA_BINDIR)/.." -DCMAKE_INSTALL_PREFIX="$(PREFIX)"; 
$(MAKE)
   ]],
   install_command = "cd build && $(MAKE) install"
}
