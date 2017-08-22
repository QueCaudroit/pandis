require 'mkmf'
#find_header("ruby.h","/usr/include/ruby-2.3.0")
find_header("matmul.h",".")
find_header("matmul_old.h",".")
find_header("pandis_types.h",".")
dir_config('pandis')
create_makefile('pandis')

