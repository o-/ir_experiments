clang++ --std=c++11 -g3 ir.cpp main.cpp && ./a.out | dot -Tps | ps2pdf  - a.pdf && evince a.pdf
