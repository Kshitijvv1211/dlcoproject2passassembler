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
    const char hexDigits[] = "0123456789abcdef";
    string hexRepresentation(8, '0');  // Start with a string of 8 '0's

    for (int i = 7; i >= 0; --i) {           // Fill the string from right to left
        hexRepresentation[i] = hexDigits[num % 16];  // Get the last hex digit
        num /= 16;                          // Move to the next hex digit
    }

    return hexRepresentation;
}

void loadMachineCode() {
    cout << "Enter file name (e.g., machineCode.o): ";
    string fileName;
    getline(cin, fileName);  // Use getline to handle spaces

    ifstream inputFile(fileName, ios::binary);
    if (!inputFile) {
        cerr << "Error: Unable to open file " << fileName << endl;
        return;
    }

    unsigned int instruction;
    int position = 0;

    while (inputFile.read(reinterpret_cast<char*>(&instruction), sizeof(instruction))) {
        if (position < sizeof(main_memory) / sizeof(main_memory[0])) {
            main_memory[position++] = instruction;
            machine_code_lines.emplace_back(decimal_to_hexadecimal_conversion(instruction));
        } else {
            cerr << "Error: Exceeded memory limits." << endl;
            break;
        }
    }

    if (inputFile.eof()) {
        cout << "Machine code loaded successfully from " << fileName << endl;
    } else {
        cerr << "Error: Reading interrupted before reaching end of file." << endl;
    }
}

#include <iostream>
#include <iomanip>
using namespace std;

void displayWelcomeMessage() {
    cout << "Welcome to the Emulator!" << endl;
    cout << "\nAvailable Commands:\n" << endl;

    cout << left << setw(7) << "Sr.No." << setw(10) << "Command" << setw(40) << "Description" << endl;
    cout << "---------------------------------------------------------------" << endl;
    cout << left << setw(7) << "1." << setw(10) << "-dump"    << setw(40) << "Display a memory dump" << endl;
    cout << left << setw(7) << "2." << setw(10) << "-t"       << setw(40) << "Step through code one line at a time" << endl;
    cout << left << setw(7) << "3." << setw(10) << "-run"     << setw(40) << "Run the code until completion" << endl;
    cout << left << setw(7) << "4." << setw(10) << "-reg"     << setw(40) << "Display register and stack pointer (SP) values" << endl;
    cout << left << setw(7) << "5." << setw(10) << "-isa"     << setw(40) << "Show the instruction set" << endl;
    cout << left << setw(7) << "6." << setw(10) << "-read"    << setw(40) << "Read from memory" << endl;
    cout << left << setw(7) << "7." << setw(10) << "-write"   << setw(40) << "Write to memory" << endl;

    cout << "\nEnter a command to proceed." << endl;
}


void displayMemoryDump() {
    int memorySize = static_cast<int>(machine_code_lines.size());

    for (int i = 0; i < memorySize; i += 4) {
        cout << decimal_to_hexadecimal_conversion(i) << ": ";  // Print memory address in hex

        // Display up to 4 consecutive memory contents at this address
        for (int j = i; j < i + 4 && j < memorySize; j++) {
            cout << machine_code_lines[j] << " ";
        }
        
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
    cout << "Instruction Set:\n" << endl;

    cout << left << setw(10) << "Sr. No."
         << setw(18) << "| OpMachineCode"
         << setw(20) << "| Mnemonic"
         << "Operand" << endl;

    cout << "----------------------------------------------------------" << endl;

    cout << left << setw(10) << "0" << setw(18) << "| ldc"   << setw(20) << "| value" << endl;
    cout << left << setw(10) << "1" << setw(18) << "| adc"   << setw(20) << "| value" << endl;
    cout << left << setw(10) << "2" << setw(18) << "| ldl"   << setw(20) << "| value" << endl;
    cout << left << setw(10) << "3" << setw(18) << "| stl"   << setw(20) << "| value" << endl;
    cout << left << setw(10) << "4" << setw(18) << "| ldnl"  << setw(20) << "| value" << endl;
    cout << left << setw(10) << "5" << setw(18) << "| stnl"  << setw(20) << "| value" << endl;
    cout << left << setw(10) << "6" << setw(18) << "| add"   << setw(20) << "|" << endl;
    cout << left << setw(10) << "7" << setw(18) << "| sub"   << setw(20) << "|" << endl;
    cout << left << setw(10) << "9" << setw(18) << "| shr"   << setw(20) << "|" << endl;
    cout << left << setw(10) << "10" << setw(18) << "| adj"   << setw(20) << "| value" << endl;
    cout << left << setw(10) << "11" << setw(18) << "| a2sp"  << setw(20) << "|" << endl;
    cout << left << setw(10) << "12" << setw(18) << "| sp2a"  << setw(20) << "|" << endl;
    cout << left << setw(10) << "13" << setw(18) << "| call"  << setw(20) << "| offset" << endl;
    cout << left << setw(10) << "14" << setw(18) << "| return" << setw(20) << "|" << endl;
    cout << left << setw(10) << "15" << setw(18) << "| brz"   << setw(20) << "| offset" << endl;
    cout << left << setw(10) << "16" << setw(18) << "| brlz"  << setw(20) << "| offset" << endl;
    cout << left << setw(10) << "17" << setw(18) << "| br"    << setw(20) << "| offset" << endl;
    cout << left << setw(10) << "18" << setw(18) << "| HALT"  << setw(20) << "|" << endl;
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
