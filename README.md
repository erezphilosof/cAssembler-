# cAssembler

To build the assembler on Ubuntu:

```sh
sudo apt-get update && sudo apt-get install build-essential
make
```

This compiles all `.c` files into the `assembler` executable. Run `make clean` to remove object files and the binary.

## Opcode Table

| Mnemonic | Opcode |
|----------|--------|
| MOV      | 0      |
| CMP      | 1      |
| ADD      | 2      |
| SUB      | 3      |
| LEA      | 4      |
| CLR      | 5      |
| NOT      | 6      |
| INC      | 7      |
| DEC      | 8      |
| JMP      | 9      |
| BNE      | 10     |
| JSR      | 11     |
| RED      | 12     |
| PRN      | 13     |
| RTS      | 14     |
| STOP     | 15     |
