
g++ -std=c++2b main.cpp -o decoder
sudo mv decoder /usr/local/bin
decoder ./single_mov > single_mov.asm
decoder ./listing_0038_many_register_mov > many_mov.asm
#diff many_mov.asm listing_0038_many_register_mov.asm 
#diff single_mov.asm listing_0037_single_register_mov.asm
