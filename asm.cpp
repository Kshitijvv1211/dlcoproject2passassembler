#include <bits/stdc++.h>
using namespace std;

struct AsmTable { 
    string label;
    string mnemonic;
    string operand;
    int operand_type;
    bool label_present;
};

vector<AsmTable> asm_data;
map<string, pair<string, int>> opcode_map;
vector<pair<int, string>> error_list;
vector<string> cleaned_code;
map<string, int> label_map;
vector<pair<int, string>> machine_code_list;
vector<int> program_counters;
bool halt_present = false;
string asm_file_name;

bool is_octal(string s);
bool is_hexadecimal(string s);
bool is_decimal(string s);
bool is_valid_label(string label);
string decimal_to_hex(int number, int bits = 24);

void initialize_opcodes();
void generate_error(int line, string type);
string clean_line(string s, int line);
void push_set_instructions(vector<string> &temp, string token, string s, int j);
void implement_set_instruction();
void process_labels();
void fill_asm_data(int i, string label, string mnemonic, string operand, int type);
int get_operand_type(string s);
void create_asm_table();
void separate_data_segment();
void first_pass();
bool display_errors();
string pad_zeroes(string s, int length = 6);
void second_pass();
void write_output_files();

int main() {
    first_pass();
    if (display_errors()) {
        second_pass();
        write_output_files();
    }
    system("pause");
    return 0;
}

string decimal_to_hex(int number, int bits) {
    if (bits == 32) {
        unsigned int num = number;
        stringstream ss;
        ss << hex << num;
        return ss.str();
    }
    if (number < 0)
        number += (1 << bits);
    stringstream ss;
    ss << hex << number;
    return ss.str();
}

void initialize_opcodes() {
    opcode_map["data"] = {"", 1};
    opcode_map["ldc"] = {"00", 1};
    opcode_map["adc"] = {"01", 1};
    opcode_map["ldl"] = {"02", 2};
    opcode_map["stl"] = {"03", 2};
    opcode_map["ldnl"] = {"04", 2};
    opcode_map["stnl"] = {"05", 2};
    opcode_map["add"] = {"06", 0};
    opcode_map["sub"] = {"07", 0};
    opcode_map["shl"] = {"08", 0};
    opcode_map["shr"] = {"09", 0};
    opcode_map["adj"] = {"0A", 1};
    opcode_map["a2sp"] = {"0B", 0};
    opcode_map["sp2a"] = {"0C", 0};
    opcode_map["call"] = {"0D", 2};
    opcode_map["return"] = {"0E", 0};
    opcode_map["brz"] = {"0F", 2};
    opcode_map["brlz"] = {"10", 2};
    opcode_map["br"] = {"11", 2};
    opcode_map["HALT"] = {"12", 0};
    opcode_map["SET"] = {"", 1};
}

void generate_error(int line, string type) {
    error_list.push_back({line + 1, "Error at line: " + to_string(line) + " -- Type: " + type});
}

string clean_line(string s, int line) {
    for (int i = 0; i < 2; i++) {
        reverse(s.begin(), s.end());
        while (s.back() == ' ' || s.back() == '\t') {
            s.pop_back();
        }
    }
    string temp;
    for (int i = 0; i < (int)s.size(); i++) {
        if (s[i] == ';')
            break;
        if (s[i] == ':') {
            temp += ":";
            if (i == (int)s.size() - 1 || s[i + 1] != ' ')
                temp += " ";
            continue;
        }
        if (s[i] != ' ' && s[i] != '\t') {
            temp += s[i];
            continue;
        }
        temp += " ";
        int j = i;
        while (s[i] == s[j] && j < (int) s.size()) j++;
        i = j - 1;
    }
    while (!temp.empty() && (temp.back() == ' ' || temp.back() == '\t'))
        temp.pop_back();
    int space_count = 0;
    for (auto ch : temp)
        space_count += (ch == ' ');
    if (space_count > 2)
        generate_error(line + 1, "Invalid syntax");
    return temp;
}

void push_set_instructions(vector<string> &temp, string token, string s, int j) {
    if (s.size() <= j + 5) {
        return;
    }
    temp.push_back("adj 10000");
    temp.push_back("stl -1");
    temp.push_back("stl 0");
    temp.push_back("ldc " + s.substr(j + 6, s.size() - (j + 6)));
    temp.push_back("ldc " + token.substr(0, j));
    temp.push_back("stnl 0");
    temp.push_back("ldl 0");
    temp.push_back("ldl -1");
    temp.push_back("adj -10000");
}

void implement_set_instruction() {
    vector<string> temp;
    for (int i = 0; i < (int) cleaned_code.size(); i++) {
        string cur;
        bool is_set_instruction = false;
        for (int j = 0; j < (int) cleaned_code[i].size(); j++) {
            cur += cleaned_code[i][j];
            if (cleaned_code[i][j] == ':') {
                cur.pop_back();
                if (cleaned_code[i].size() > j + 5 && cleaned_code[i].substr(j + 2, 3) == "SET") {
                    is_set_instruction = true;
                    if (abs(label_map[cur]) == i) {
                        label_map[cur] = (int)temp.size() - 1;
                        temp.push_back(cleaned_code[i].substr(0, j + 1) + " data " + cleaned_code[i].substr(j + 6, (int)cleaned_code[i].size() - (j + 6)));
                    } else {
                        push_set_instructions(temp, cur, cleaned_code[i], j);
                    }
                    break;
                }
            }
        }
        if (!is_set_instruction && !cleaned_code[i].empty())
            temp.push_back(cleaned_code[i]);
    }
    cleaned_code = temp;
}

void process_labels() {
    for (int i = 0; i < (int) cleaned_code.size(); i++) {
        string cur_label;
        for (int j = 0; j < (int) cleaned_code[i].size(); j++) {
            if (cleaned_code[i][j] == ':') {
                if (!is_valid_label(cur_label)) {
                    generate_error(i + 1, "Invalid label name");
                    break;
                }
                if (label_map.count(cur_label)) {
                    if (cleaned_code[i].size() > j + 4 && cleaned_code[i].substr(j + 2, 3) == "SET") {
                        continue;
                    }
                    if (cleaned_code[i].size() > j + 5 && cleaned_code[i].substr(j + 2, 4) == "data" && label_map[cur_label] < 0) {
                        label_map[cur_label] = i;
                        continue;
                    }
                    generate_error(i + 1, "Multiple declaration of label: " + cur_label);
                }
                if (cleaned_code[i].size() > j + 4 && cleaned_code[i].substr(j + 2, 3) == "SET") {
                    label_map[cur_label] = -i;
                    continue;
                }
                label_map[cur_label] = i;
                break;
            }
            cur_label += cleaned_code[i][j];
        }
    }
}

void fill_asm_data(int i, string label, string mnemonic, string operand, int type) {
    asm_data[i].label = label;
    asm_data[i].mnemonic = mnemonic;
    asm_data[i].operand = operand;
    asm_data[i].operand_type = type;
}

int get_operand_type(string s) {
    if (s.empty()) return 0;
    if (s[0] == '+' || s[0] == '-') {
        reverse(s.begin(), s.end());
        s.pop_back();
        reverse(s.begin(), s.end());
    }
    if (s.empty())
        return -1;
    else if (is_decimal(s)) return 10;
    else if (is_octal(s)) return 8;
    else if (is_hexadecimal(s)) return 16;
    else if (is_valid_label(s)) return 1;
    return -1;
}

void create_asm_table() {
    int pc = 0;
    for (int i = 0; i < (int) cleaned_code.size(); i++) {
        string line_parts[10] = {"", "", "", ""}, current_part = "";
        int part_index = 1;
        for (int j = 0; j < (int) cleaned_code[i].size(); j++) {
            if (cleaned_code[i][j] == ':') {
                line_parts[0] = current_part;
                current_part = "";
                j++;
                continue;
            } else if (cleaned_code[i][j] == ' ') {
                line_parts[part_index++] = current_part;
                current_part = "";
                continue;
            }
            current_part += cleaned_code[i][j];
            if (j == (int)cleaned_code[i].size() - 1)
                line_parts[part_index++] = current_part;
        }
        if (!line_parts[1].empty()) {
            asm_data[i].label_present = true;
        } else {
            asm_data[i].label_present = false;
        }
        if (line_parts[1] == "HALT")
            halt_present = true;
        if (!line_parts[0].empty())
            label_map[line_parts[0]] = pc;
        program_counters[i] = pc;
        if (part_index == 1) {
            fill_asm_data(i, line_parts[0], "", "", 0);
            continue;
        }
        pc++;
        if (!opcode_map.count(line_parts[1])) {
            generate_error(i + 1, "Invalid Mnemonic");
            continue;
        }
        if (min(opcode_map[line_parts[1]].second, 1) != part_index - 2) {
            generate_error(i + 1, "Invalid OPCode-Syntax combination");
            continue;
        }
        fill_asm_data(i, line_parts[0], line_parts[1], line_parts[2], get_operand_type(line_parts[2]));
        if (asm_data[i].operand_type == 1 && !label_map.count(asm_data[i].operand)) {
            generate_error(i + 1, "No such label / data variable");
        } else if (asm_data[i].operand_type == -1) {
            generate_error(i + 1, "Invalid number");
        }
    }
}

void separate_data_segment() {
    vector<string> instructions, data_segment;
    for (int i = 0; i < (int)cleaned_code.size(); i++) {
        bool is_data = false;
        for (int j = 0; j < cleaned_code[i].size(); j++) {
            if (cleaned_code[i].substr(j, 4) == "data" && j + 4 < cleaned_code[i].size()) {
                data_segment.push_back(cleaned_code[i]);
                is_data = true;
                break;
            }
            if (cleaned_code[i].back() == ':' && i + 1 < (int)cleaned_code.size() && cleaned_code[i + 1].substr(0, 4) == "data") {
                data_segment.push_back(cleaned_code[i]);
                is_data = true;
                break;
            }
        }
        if (!is_data)
            instructions.push_back(cleaned_code[i]);
    }
    instructions.insert(instructions.end(), data_segment.begin(), data_segment.end());
    cleaned_code = instructions;
}

void first_pass() {
    ifstream infile;
    cout << "Enter ASM file name to assemble:" << endl;
    cin >> asm_file_name;
    infile.open(asm_file_name);
    if (infile.fail()) {
        cout << "Input file doesn't exist, please make sure file is in the same directory as the code!" << endl;
        exit(0);
    }
    string line;
    while (getline(infile, line)) {
        string cleaned_line = clean_line(line, (int) cleaned_code.size());
        cleaned_code.push_back(cleaned_line);
    }
    initialize_opcodes();
    process_labels();
    if (error_list.empty())
        implement_set_instruction();
    asm_data.resize((int) cleaned_code.size());
    program_counters.resize((int) cleaned_code.size());
    separate_data_segment();
    create_asm_table();
}

bool display_errors() {
    ofstream error_file("logFile.log");
    error_file << "Log code generated in: logFile.log" << endl;
    if (error_list.empty()) {
        cout << "No errors found!" << endl;
        if (!halt_present) {
            cout << "1 warning detected" << endl;
            error_file << "Warning: HALT not present!" << endl;
        }
        error_file << "Machine code generated in: machineCode.o" << endl;
        error_file << "Listing code generated in: listCode.l" << endl;
        error_file.close();
        return true;
    }
    sort(error_list.begin(), error_list.end());
    cout << (int)error_list.size() << " errors encountered! See logFile.log" << endl;
    for (auto &error : error_list) {
        error_file << error.second << endl;
    }
    error_file.close();
    return false;
}

string pad_zeroes(string s, int length) {
    reverse(s.begin(), s.end());
    while ((int) s.size() < length)
        s += '0';
    reverse(s.begin(), s.end());
    return s;
}

void second_pass() {
    for (int i = 0; i < (int) asm_data.size(); i++) {
        if (cleaned_code[i].empty()) {
            continue;
        }
        string location = pad_zeroes(decimal_to_hex(program_counters[i]));
        if (asm_data[i].mnemonic == "") {
            string machine_code = "        ";
            machine_code_list.push_back({i, machine_code});
            continue;
        }
        if (asm_data[i].operand_type == 1) {
            int decimal_value;
            if (opcode_map[asm_data[i].mnemonic].second == 2) {
                int label_address = label_map[asm_data[i].operand];
                decimal_value = label_address - (program_counters[i] + 1);
            } else {
                decimal_value = label_map[asm_data[i].operand];
            }
            string machine_code = pad_zeroes(decimal_to_hex(decimal_value)) + opcode_map[asm_data[i].mnemonic].first;
            machine_code_list.push_back({i, machine_code});
        } else if (asm_data[i].operand_type == 0) {
            string machine_code = "000000" + opcode_map[asm_data[i].mnemonic].first;
            machine_code_list.push_back({i, machine_code});
        } else {
            int hex_length = 6, bit_length = 24;
            if (asm_data[i].mnemonic == "data")
                hex_length = 8, bit_length = 32;
            int decimal_value = stoi(asm_data[i].operand, nullptr, asm_data[i].operand_type);
            string machine_code = pad_zeroes(decimal_to_hex(decimal_value, bit_length), hex_length) + opcode_map[asm_data[i].mnemonic].first;
            machine_code_list.push_back({i, machine_code});
        }
    }
}

void write_output_files() {
    ofstream listing_code_file("listCode.l");
    for (auto &code : machine_code_list) {
        listing_code_file << pad_zeroes(decimal_to_hex(program_counters[code.first])) << " " << code.second << " " << cleaned_code[code.first] << endl;
    }
    listing_code_file.close();
    ofstream machine_code_file;
    machine_code_file.open("machineCode.o", ios::binary | ios::out);
    for (auto &code : machine_code_list) {
        unsigned int code_value;
        if (code.second.empty() || code.second == "        ")
            continue;
        stringstream ss;
        ss << hex << code.second;
        ss >> code_value;
        machine_code_file.write((const char *)&code_value, sizeof(unsigned int));
    }
    machine_code_file.close();
    cout << "Log code generated in: logFile.log" << endl;
    cout << "Machine code generated in: machineCode.o" << endl;
    cout << "Listing code generated in: listCode.l" << endl;
}

bool is_octal(string s) {
    if ((int) s.size() < 2)
        return false;
    bool valid_octal = true;
    for (int i = 1; i < (int)s.size(); i++) {
        valid_octal &= (s[i] >= '0' && s[i] <= '7');
    }
    return valid_octal & (s[0] == '0');
}

bool is_hexadecimal(string s) {
    bool is_hex = true;
    if ((int)s.size() < 3)
        return false;
    is_hex &= (s[0] == '0') & ((s[1] == 'x' || s[1] == 'X'));
    for (int i = 2; i < (int) s.size(); i++) {
        bool hex_char = (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >= 'A' && s[i] <= 'F');
        is_hex &= hex_char;
    }
    return is_hex;
}

bool is_decimal(string s) {
    bool is_decimal = true;
    for (int i = 0; i < (int)s.size(); i++)
        is_decimal &= (s[i] >= '0' && s[i] <= '9');
    return is_decimal;
}

bool is_valid_label(string label) {
    for (auto ch : label) {
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == '_'))
            continue;
        return false;
    }
    if ((label[0] >= 'a' && label[0] <= 'z') || (label[0] >= 'A' && label[0] <= 'Z') || (label[0] == '_'))
        return true;
    return false;
}
