PWD=`pwd`
pushd ${PWD}
rm -f dynstr dynsym dynstrAscii global_offset so_func_offset libso.hex cpu0.hex *~
cd ../cpu0_verilog
rm -f dynstr dynsym dynstrAscii global_offset so_func_offset libso.hex cpu0.hex *~
popd


