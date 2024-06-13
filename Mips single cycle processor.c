#include <stdio.h>
#include <stdint.h>

// Register file - 4 registers each 8 bits
uint8_t registers[4] = {0};

// Instruction Memory - 256 x 16 bits
uint16_t instruction_memory[256] = {0};

// Data Memory - 256 x 8 bits
uint8_t data_memory[256] = {0};

// ALU Control Codes
#define ALU_ADD 0
#define ALU_SUB 1
#define ALU_OR  2

// Opcodes
#define ADD  0x0
#define ADDI 0x1
#define OR   0x2
#define SUB  0x3
#define BEQ  0x4
#define LW   0x5
#define SW   0x6
#define J    0x7

// Control Signals
typedef struct {
    uint8_t alu_op;
    uint8_t reg_dst;
    uint8_t alu_src;
    uint8_t mem_to_reg;
    uint8_t reg_write;
    uint8_t mem_read;
    uint8_t mem_write;
    uint8_t branch;
    uint8_t jump;
} control_signals_t;

control_signals_t control;
typedef struct {
    uint8_t result;
    uint8_t carry_out;
    uint8_t zero;
} alu_output_t;

alu_output_t alu(uint8_t a, uint8_t b, uint8_t alu_control) {
    alu_output_t output;
    switch (alu_control) {
        case ALU_ADD:
            output.result = a + b;
            output.carry_out = (output.result < a);
            break;
        case ALU_SUB:
            output.result = a - b;
            output.carry_out = (a < b);
            break;
        case ALU_OR:
            output.result = a | b;
            break;
    }
    output.zero = (output.result == 0);
    return output;
}
void set_control_signals(uint8_t opcode) {
    switch (opcode) {
        case ADD:
            control = (control_signals_t){ALU_ADD, 1, 0, 0, 1, 0, 0, 0, 0};
            break;
        case ADDI:
            control = (control_signals_t){ALU_ADD, 0, 1, 0, 1, 0, 0, 0, 0};
            break;
        case OR:
            control = (control_signals_t){ALU_OR, 1, 0, 0, 1, 0, 0, 0, 0};
            break;
        case SUB:
            control = (control_signals_t){ALU_SUB, 1, 0, 0, 1, 0, 0, 0, 0};
            break;
        case BEQ:
            control = (control_signals_t){ALU_SUB, 0, 0, 0, 0, 0, 0, 1, 0};
            break;
        case LW:
            control = (control_signals_t){ALU_ADD, 0, 1, 1, 1, 1, 0, 0, 0};
            break;
        case SW:
            control = (control_signals_t){ALU_ADD, 0, 1, 0, 0, 0, 1, 0, 0};
            break;
        case J:
            control = (control_signals_t){0, 0, 0, 0, 0, 0, 0, 0, 1};
            break;
    }
}
void execute_instruction(uint16_t instruction) {
    uint8_t opcode = (instruction >> 12) & 0xF;
    uint8_t rs = (instruction >> 10) & 0x3;
    uint8_t rt = (instruction >> 8) & 0x3;
    uint8_t rd = (instruction >> 6) & 0x3;
    uint8_t immediate = instruction & 0xFF;

    set_control_signals(opcode);

    uint8_t read_data1 = registers[rs];
    uint8_t read_data2 = registers[rt];
    uint8_t alu_result;
    uint8_t write_data;

    if (control.alu_src) {
        alu_result = alu(read_data1, immediate, control.alu_op).result;
    } else {
        alu_result = alu(read_data1, read_data2, control.alu_op).result;
    }

    if (control.mem_read) {
        write_data = data_memory[alu_result];
    } else if (control.mem_write) {
        data_memory[alu_result] = read_data2;
        return;
    } else {
        write_data = alu_result;
    }

    if (control.reg_write) {
        if (control.mem_to_reg) {
            registers[rt] = write_data;
        } else {
            registers[control.reg_dst ? rd : rt] = write_data;
        }
    }

    if (control.branch && (read_data1 == read_data2)) {
        // Handle branch (not implemented in this simple simulation)
    }

    if (control.jump) {
        // Handle jump (not implemented in this simple simulation)
    }
}

void load_instructions() {
    int num_instructions;
    printf("Enter the number of instructions: ");
    scanf("%d", &num_instructions);

    for (int i = 0; i < num_instructions; i++) {
        printf("Enter instruction %d (in hex, e.g., 0x1234): ", i);
        scanf("%hx", &instruction_memory[i]);
    }
}

void load_data_memory() {
    int num_data;
    printf("Enter the number of data memory entries: ");
    scanf("%d", &num_data);

    for (int i = 0; i < num_data; i++) {
        int address, value;
        printf("Enter address (0-255) and value (0-255) for data entry %d: ", i);
        scanf("%d %d", &address, &value);
        data_memory[address] = value;
    }
}

int main() {
    // Load instructions and data memory from user input
    load_instructions();
    load_data_memory();

    // Execute each instruction in memory
    for (int pc = 0; instruction_memory[pc] != 0xFFFF; pc++) {
        execute_instruction(instruction_memory[pc]);
    }

    // Print the final state of the registers
    printf("Final state of registers:\n");
    for (int i = 0; i < 4; i++) {
        printf("R%d: %d\n", i, registers[i]);
    }

    return 0;
}
