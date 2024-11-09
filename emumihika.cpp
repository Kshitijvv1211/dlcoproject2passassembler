#include <bits/stdc++.h>
using namespace std;

int main_memory[1 << 24];
vector<string> machine_code_lines;
int accumulator_A, accumulator_B, program_counter, stack_pointer, index, total_instructions, ExecStatus;
array<int, 2> memory_change;
vector<string> mnemonics = {"ldc", "adc", "ldl", "stl", "ldnl", "stnl", "add", "sub",
                            "shl", "shr", "adj", "a2sp", "sp2a", "call", "return", "brz", "brlz", "br", "HALT"};

void load_constant(int value) { accumulator_B = accumulator_A; accumulator_A = value; }
void add_constant(int value) { accumulator_A = accumulator_A + value; }
void load_local(int offset) {
    accumulator_B = accumulator_A;
    accumulator_A = main_memory[stack_pointer + offset];
    memory_change = {stack_pointer + offset, 0};
    ExecStatus = 1;
}
void store_local(int offset) {
    memory_change = {main_memory[stack_pointer + offset], accumulator_A};
    main_memory[stack_pointer + offset] = accumulator_A;
    ExecStatus = 2;
    accumulator_A = accumulator_B;
}
void load_non_local(int offset) {
    accumulator_A = main_memory[accumulator_A + offset];
    memory_change = {stack_pointer + offset, 0};
    ExecStatus = 1;
}
void store_non_local(int offset) {
    memory_change = {main_memory[accumulator_A + offset], accumulator_B};
    main_memory[accumulator_A + offset] = accumulator_B;
    ExecStatus = 2;
}
void add(int unused = 0) { accumulator_A = accumulator_A + accumulator_B; }
void subtract(int unused = 0) { accumulator_A = accumulator_B - accumulator_A; }
void shift_left(int unused = 0) { accumulator_A = accumulator_B << accumulator_A; }
void shift_right(int unused = 0) { accumulator_A = accumulator_B >> accumulator_A; }
void adjust_stack_pointer(int value) { stack_pointer = stack_pointer + value; }
void accumulatorToSP(int unused = 0) { stack_pointer = accumulator_A; accumulator_A = accumulator_B; }
void spToAccumulator(int unused = 0) { accumulator_B = accumulator_A; accumulator_A = stack_pointer; }
void call_procedure(int offset) {
    accumulator_B = accumulator_A;
    accumulator_A = program_counter;
    program_counter = program_counter + offset;
}
void returnProcedure(int unused = 0) { program_counter = accumulator_A; accumulator_A = accumulator_B; }
void branchIfZero(int offset) { if (accumulator_A == 0) program_counter = program_counter + offset; }
void branchIfLessZero(int offset) { if (accumulator_A < 0) program_counter = program_counter + offset; }
void branch(int offset) { program_counter = program_counter + offset; }

void (*instructionFunctions[])(int) = {load_constant, add_constant, load_local, store_local, load_non_local,
                                       store_non_local, add, subtract, shift_left, shift_right,
                                       adjust_stack_pointer, accumulatorToSP, spToAccumulator, call_procedure,
                                       returnProcedure, branchIfZero, branchIfLessZero, branch};

string decimal_to_hexadecimal_conversion(unsigned int num) {
    stringstream ss;
    ss << hex << num;
    string hexRepresentation = ss.str();
    reverse(hexRepresentation.begin(), hexRepresentation.end());
    while ((int)hexRepresentation.size() < 8) {
        hexRepresentation += '0';
    }
    reverse(hexRepresentation.begin(), hexRepresentation.end());
    return hexRepresentation;
}

void loadMachineCode() {
    string fileName;
    cout << "Enter file name: Ex: machineCode.o" << endl;
    cin >> fileName;
    ifstream inputFile(fileName, ios::in | ios::binary);
    unsigned int instruction;
    int pos = 0;
    while (inputFile.read((char*)&instruction, sizeof(int))) {
        main_memory[pos++] = instruction;
        machine_code_lines.push_back(decimal_to_hexadecimal_conversion(instruction));
    }
}

void displayWelcomeMessage() {
    cout << "Welcome to Emulator" << endl;
    cout << "Following functions are implemented:" << endl;
    cout << "1. Memory Dump using instruction: -dump" << endl;
    cout << "2. Emulate code one line at time: using instruction: -t" << endl;
    cout << "3. Emulate till the end using -run" << endl;
    cout << "4. Show registers and SP values: -reg" << endl;
    cout << "5. Show instruction set using: -isa" << endl;
    cout << "6. Read operations using -read" << endl;
    cout << "7. Write operations using -write" << endl;
}

void displayMemoryDump() {
    for (int i = 0; i < (int)machine_code_lines.size(); i++) {
        cout << decimal_to_hexadecimal_conversion(i) << " ";
        for (int j = i; j < min(i + 4, (int)machine_code_lines.size()); j++) {
            cout << decimal_to_hexadecimal_conversion(main_memory[j]) << " ";
        }
        i = i + 3;
        cout << endl;
    }
}

void displayRegisters() {
    cout << "A: " << decimal_to_hexadecimal_conversion(accumulator_A) << "     B: " << decimal_to_hexadecimal_conversion(accumulator_B)
         << "     SP: " << decimal_to_hexadecimal_conversion(stack_pointer) << "     PC: " << decimal_to_hexadecimal_conversion(program_counter + 1)
         << "     " << mnemonics[program_counter] << endl;
}

void displayReadOperation() {
    cout << "Reading memory[" << decimal_to_hexadecimal_conversion(program_counter) << "], has value: " << decimal_to_hexadecimal_conversion(memory_change[0]) << endl;
}

void displayWriteOperation() {
    cout << "Writing memory[" << decimal_to_hexadecimal_conversion(program_counter) << "], from " << decimal_to_hexadecimal_conversion(memory_change[0])
         << " to " << decimal_to_hexadecimal_conversion(memory_change[1]) << endl;
}

void displayInstructionSet() {
    cout << "OpMachineCode Mnemonic Operand" << '\n';
    cout << "0      ldc      value " << endl;
    cout << "1      adc      value " << endl;
    cout << "2      ldl      value " << endl;
    cout << "3      stl      value " << endl;
    cout << "4      ldnl     value " << endl;
    cout << "5      stnl     value " << endl;
    cout << "6      add            " << endl;
    cout << "7      sub            " << endl;
    cout << "9      shr            " << endl;
    cout << "10     adj      value " << endl;
    cout << "11     a2sp           " << endl;
    cout << "12     sp2a           " << endl;
    cout << "13     call     offset" << endl;
    cout << "14     return         " << endl;
    cout << "15     brz      offset" << endl;
    cout << "16     brlz     offset" << endl;
    cout << "17     br       offset" << endl;
    cout << "18     HALT           " << endl;
}

bool executeInstructions(int operation, int maxExecutions = (1 << 25)) {
    while (maxExecutions-- && program_counter < machine_code_lines.size()) {
        total_instructions++;
        if (program_counter >= machine_code_lines.size() || total_instructions > (int)3e7) {
            cout << "Segmentation Fault" << endl;
            return false;
        }
        string currentInstruction = machine_code_lines[program_counter];
        int opcode = stoi(currentInstruction.substr(6, 2), 0, 16);
        if (opcode == 18) {
            cout << "HALT found" << endl;
            cout << total_instructions << " statements were executed in total" << endl;
            return true;
        }
        int operand = stoi(currentInstruction.substr(0, 6), 0, 16);
        if (operand >= (1 << 23)) {
            operand -= (1 << 24);
        }
        if (maxExecutions == 0)
            displayRegisters();
        (instructionFunctions[opcode])(operand);
        if (operation == 1 && ExecStatus == 1) {
            displayReadOperation();
            ExecStatus = 0;
        }
        else if (operation == 2 && ExecStatus == 2) {
            displayWriteOperation();
            ExecStatus = 0;
        }
        program_counter++;
        index++;
    }
    return true;
}

bool startEmulator() {
    cout << "Enter command or 0 to exit:" << endl;
    string command;
    cin >> command;
    if (command == "0") {
        exit(0);
    }
    else if (command == "-dump") {
        displayMemoryDump();
    }
    else if (command == "-reg") {
        displayRegisters();
    }
    else if (command == "-t") {
        return executeInstructions(0, 1);
    }
    else if (command == "-run") {
        return executeInstructions(0);
    }
    else if (command == "-isa") {
        displayInstructionSet();
    }
    else if (command == "-read") {
        return executeInstructions(1);
    }
    else if (command == "-write") {
        return executeInstructions(2);
    }
    else {
        cout << "Enter a valid command" << endl;
    }
    return true;
}

int main() {
    loadMachineCode();
    displayWelcomeMessage();
    while (true) {
       startEmulator();
    }
    return 0;
}
