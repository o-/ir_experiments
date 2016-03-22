clang++ --std=c++11 -g3 ir.cpp main.cpp -o out/ir && out/ir | dot -Tps | ps2pdf  - out/ir.pdf && evince out/ir.pdf
