#include <bits/stdc++.h>
using namespace std;

int MainMemory[1 << 24];
vector<string> MachineCodeLines;
int AccumulatorA, AccumulatorB, ProgramCounter, StackPointer, Index, TotalInstructions, ExecStatus;
array<int, 2> MemoryChange;
vector<string> Mnemonics = {"ldc", "adc", "ldl", "stl", "ldnl", "stnl", "add", "sub",
                            "shl", "shr", "adj", "a2sp", "sp2a", "call", "return", "brz", "brlz", "br", "HALT"};

void loadConstant(int value) { AccumulatorB = AccumulatorA; AccumulatorA = value; }
void addConstant(int value) { AccumulatorA += value; }
void loadLocal(int offset) {
    AccumulatorB = AccumulatorA;
    AccumulatorA = MainMemory[StackPointer + offset];
    MemoryChange = {StackPointer + offset, 0};
    ExecStatus = 1;
}
void storeLocal(int offset) {
    MemoryChange = {MainMemory[StackPointer + offset], AccumulatorA};
    MainMemory[StackPointer + offset] = AccumulatorA;
    ExecStatus = 2;
    AccumulatorA = AccumulatorB;
}
void loadNonLocal(int offset) {
    AccumulatorA = MainMemory[AccumulatorA + offset];
    MemoryChange = {StackPointer + offset, 0};
    ExecStatus = 1;
}
void storeNonLocal(int offset) {
    MemoryChange = {MainMemory[AccumulatorA + offset], AccumulatorB};
    MainMemory[AccumulatorA + offset] = AccumulatorB;
    ExecStatus = 2;
}
void add(int unused = 0) { AccumulatorA += AccumulatorB; }
void subtract(int unused = 0) { AccumulatorA = AccumulatorB - AccumulatorA; }
void shiftLeft(int unused = 0) { AccumulatorA = AccumulatorB << AccumulatorA; }
void shiftRight(int unused = 0) { AccumulatorA = AccumulatorB >> AccumulatorA; }
void adjustSP(int value) { StackPointer += value; }
void accumulatorToSP(int unused = 0) { StackPointer = AccumulatorA; AccumulatorA = AccumulatorB; }
void spToAccumulator(int unused = 0) { AccumulatorB = AccumulatorA; AccumulatorA = StackPointer; }
void callProcedure(int offset) {
    AccumulatorB = AccumulatorA;
    AccumulatorA = ProgramCounter;
    ProgramCounter += offset;
}
void returnProcedure(int unused = 0) { ProgramCounter = AccumulatorA; AccumulatorA = AccumulatorB; }
void branchIfZero(int offset) { if (AccumulatorA == 0) ProgramCounter += offset; }
void branchIfLessZero(int offset) { if (AccumulatorA < 0) ProgramCounter += offset; }
void branch(int offset) { ProgramCounter += offset; }

void (*instructionFunctions[])(int) = {loadConstant, addConstant, loadLocal, storeLocal, loadNonLocal,
                                       storeNonLocal, add, subtract, shiftLeft, shiftRight,
                                       adjustSP, accumulatorToSP, spToAccumulator, callProcedure,
                                       returnProcedure, branchIfZero, branchIfLessZero, branch};

string decimalToHex(unsigned int num) {
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
        MainMemory[pos++] = instruction;
        MachineCodeLines.push_back(decimalToHex(instruction));
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
    for (int i = 0; i < (int)MachineCodeLines.size(); i++) {
        cout << decimalToHex(i) << " ";
        for (int j = i; j < min(i + 4, (int)MachineCodeLines.size()); j++) {
            cout << decimalToHex(MainMemory[j]) << " ";
        }
        i += 3;
        cout << endl;
    }
}

void displayRegisters() {
    cout << "A: " << decimalToHex(AccumulatorA) << "     B: " << decimalToHex(AccumulatorB)
         << "     SP: " << decimalToHex(StackPointer) << "     PC: " << decimalToHex(ProgramCounter + 1)
         << "     " << Mnemonics[ProgramCounter] << endl;
}

void displayReadOperation() {
    cout << "Reading memory[" << decimalToHex(ProgramCounter) << "], has value: " << decimalToHex(MemoryChange[0]) << endl;
}

void displayWriteOperation() {
    cout << "Writing memory[" << decimalToHex(ProgramCounter) << "], from " << decimalToHex(MemoryChange[0])
         << " to " << decimalToHex(MemoryChange[1]) << endl;
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
    while (maxExecutions-- && ProgramCounter < MachineCodeLines.size()) {
        TotalInstructions++;
        if (ProgramCounter >= MachineCodeLines.size() || TotalInstructions > (int)3e7) {
            cout << "Segmentation Fault" << endl;
            return false;
        }
        string currentInstruction = MachineCodeLines[ProgramCounter];
        int opcode = stoi(currentInstruction.substr(6, 2), 0, 16);
        if (opcode == 18) {
            cout << "HALT found" << endl;
            cout << TotalInstructions << " statements were executed in total" << endl;
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
        ProgramCounter++;
        Index++;
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
