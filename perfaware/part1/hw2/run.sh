
g++ -std=c++2b main.cpp -o decoder
sudo mv decoder /usr/local/bin
decoder ./listing_0037_single_register_mov > single_mov.asm
decoder ./listing_0039_more_movs > many_mov.asm
#diff many_mov.asm listing_0038_many_register_mov.asm 
#diff single_mov.asm listing_0037_single_register_mov.asm
