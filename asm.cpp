// Name: Hariomkant sharma
// Roll No.- 2101CS31
// mini project course - CS210, Computer architechture


#include<bits/stdc++.h>	    
using namespace std;


//following functions are helping functions and not related to assembler

// function for checking valid label name
bool valid_label(string cur)
{
	for(auto to: cur)
    {
		if((to >= 'a' and to <= 'z') or (to >= 'A' and to <= 'Z') or (to >= '0' and to <= '9') or (to == '_'))
			continue;
		return false;
	}

	if((cur[0] >= 'a' and cur[0] <= 'z') or (cur[0] >= 'A' and cur[0] <= 'Z') or (cur[0] == '_'))
		return true;

	return false;
}

// this function is to check string is of form decimal
bool isDecimal(string s)
{
	bool ans = true;
	for(int i = 0; i < (int)s.size(); i++)
		ans &= (s[i] >= '0' and s[i] <= '9');

	return ans;
}

// this function is to check string is of form octal
bool isOctal(string s)
{
    if((int) s.size() < 2)
    	return false;
    bool ans = true;
    for(int i = 1; i < (int)s.size(); i++){
        ans &= (s[i] >= '0' and s[i] <= '7');
    }
    return ans & (s[0] == '0');
}

// this function is to check string is of form hexadecimal
bool isHexadecimal(string s)
{
    bool ans = true ;
    if((int)s.size() < 3)
    	return false;
    ans &= (s[0] == '0') & ((s[1] == 'x' or s[1] == 'X'));
    for(int i = 2; i < (int) s.size(); i++){
        bool st = (s[i] >= '0' and s[i] <= '9');
        st |= ((s[i] >= 'a' and s[i] <= 'f') or ((s[i] >= 'A' and s[i] <= 'F')));
        ans &= st;
    }
    return ans;
}


// This functions converts decimal number to Hexadecimal
string decToHex(int number, int add = 24)
{
	if(add == 32){
		unsigned int num = number;
		stringstream a; 
    	a << hex << num; 
    	return a.str();
	}
	if(number < 0)
		number += (1 << add);
	stringstream a; 
    a << hex << number; 
    return a.str();
}


///helping functions end///


// This table will be store asm code instructions in following formate-> Label | Mnemonic | Operand | Operand Type (Hex, Oct etc) | If label is present in that line or not
struct insttable
{ 
	string label,mnemonic,operand;
	int opertype;
	bool lable_present_or_not;
};

vector<pair<int, string>> errors;		//vector to store the errors

vector<insttable> data;					//Data vector in Table form
map<string, pair<string, int>> OPCode;	//vector to store mnemonic and OPCode
map<string, int> labels;				//vector to store the labels and their declaration line
vector<int> programCounter;				//vector to maintain Program counter on every line

vector<string> cleaned;					//Contains the code line in clean form w/o spaces

vector<pair<int, string>> machineCode;	//Stores the machine code and extra info for generating listCode

bool haltPresent = false;				// Check if HALT is present in the Code
string fileName;						// Name of the file


void Initialization(){
//Initialization of the Mnemonic, OPCode
// 0 = Takes no operands like add
// 1 = Takes 1 operand and is value, Ex: ldc 5
// 2 = Takes 1 operand and is offset, Ex: call return
// No OPCODE for data and SET
//SET is pseudo instruction
    OPCode["data"] = {"", 1}, OPCode["ldc"] = {"00", 1}, OPCode["adc"] = {"01", 1};
    OPCode["ldl"] = {"02", 2}, OPCode["stl"] = {"03", 2}, OPCode["ldnl"] = {"04", 2};
    OPCode["stnl"] = {"05", 2}, OPCode["add"] = {"06", 0}, OPCode["sub"] = {"07", 0};
    OPCode["shl"] = {"08", 0}, OPCode["shr"] = {"09", 0}, OPCode["adj"] = {"0A" , 1};
    OPCode["a2sp"] = {"0B" , 0}, OPCode["sp2a"] = {"0C" , 0}, OPCode["call"] = {"0D" , 2};
    OPCode["return"] = {"0E" , 0}, OPCode["brz"] = {"0F" , 2}, OPCode["brlz"] = {"10" , 2};
    OPCode["br"] = {"11", 2}, OPCode["HALT"] = {"12", 0}, OPCode["SET"] = {"" , 1};
}



void storeErrors(int line, string type)
{
// This functions stores the errors in errors vector
	errors.push_back({line + 1, "Error at line: " + to_string(line) + " -- Type: " + type});
}



string clean(string s, int line)
{
// This function remove unnecessary spaces
	
	for(int i = 0; i < 2; i++){
		reverse(s.begin(), s.end());
		while(s.back() == ' ' or s.back() == '\t'){
			s.pop_back();
		}
	}


	string temp;


	for(int i = 0; i < (int)s.size(); i++){
		if(s[i] == ';')
			break;
		if(s[i] == ':'){
			temp += ":";
			if(i == (int)s.size() - 1 or s[i + 1] != ' ')
				temp += " ";
			continue;
		}
		if(s[i] != ' ' and s[i] != '\t'){
			temp += s[i];
			continue;
		}
		temp += " ";
		int j = i;
		while(s[i] == s[j] and j < (int) s.size()) j++;
		i = j - 1;
	}


	while(!temp.empty() and (temp.back() == ' ' or temp.back() == '\t'))
		temp.pop_back();
	int spac = 0;
	for(auto to: temp)
		spac += (to == ' ');
	if(spac > 2)
		storeErrors(line + 1, "Invalid syntax");
	return temp;
}



void pushSETinstructions(vector<string> &temp, string token, string s, int j)
{
// Following mnemonic are used in implmenting SET mnemonic in assembler
	if(s.size() <= j + 5){
		return;
	}
	temp.push_back("adj 10000");			//stack pointer at random memory
	temp.push_back("stl -1");				//Here to load A
	temp.push_back("stl 0");				//Here B
	temp.push_back("ldc " + s.substr(j + 6, s.size() - (j + 6)));
	temp.push_back("ldc " + token.substr(0, j));
	temp.push_back("stnl 0");				//Load A, B
	temp.push_back("ldl 0");				
	temp.push_back("ldl -1");
	temp.push_back("adj -10000");			// Adjust stack ptr
}



void implementSET(){
// This function implements SET mnemonic given.
// Since SET is pseudo instruction, we implement it with other mnemonic
	vector<string> temp;
	for(int i = 0; i < (int) cleaned.size(); i++){
		string cur;
		bool state = false;
		for(int j = 0; j < (int) cleaned[i].size(); j++){
			cur += cleaned[i][j];
			if(cleaned[i][j] == ':'){
				cur.pop_back();
				if(cleaned[i].size() > j + 5 and cleaned[i].substr(j + 2, 3) == "SET"){
					state = true;
					if(abs(labels[cur]) == i){
						labels[cur] = (int)temp.size() - 1;
						temp.push_back(cleaned[i].substr(0, j + 1) + " data " + cleaned[i].substr(j + 6, (int)cleaned[i].size() - (j + 6)));
					}
					else{
						pushSETinstructions(temp, cur, cleaned[i], j);
					}
					break;
				}
			}
		}
		if(!state and !cleaned[i].empty())
			temp.push_back(cleaned[i]);
	}
	cleaned = temp;
}




void processLabel(){
// This function processes Lables and stores the labels map, with the postion they are declared
	for(int i = 0; i < (int) cleaned.size(); i++){
		string cur;
		for(int j = 0; j < (int) cleaned[i].size(); j++){
			if(cleaned[i][j] == ':'){
				bool is = valid_label(cur);
				if(!is){
					storeErrors(i + 1, "Invalid label name" );
					break;
				}
				if(labels.count(cur)){
					if(cleaned[i].size() > j + 4 and cleaned[i].substr(j + 2, 3) == "SET"){
						continue;
					}
					if(cleaned[i].size() > j + 5 and cleaned[i].substr(j + 2, 4) == "data" and labels[cur] < 0){
						labels[cur] = i;
						continue;
					}
					storeErrors(i + 1, "Multiple declaration of label: " + cur);
				}
				if(cleaned[i].size() > j + 4 and cleaned[i].substr(j + 2, 3) == "SET"){
					labels[cur] = -i;
					continue;
				}
				labels[cur] = i;
				break;
			}
			cur += cleaned[i][j];
		}
	}
}




void fillData(int i, string one, string two, string three, int type){
// Fills the data vector to reduce code size and make clean
	data[i].label = one;
	data[i].mnemonic = two;
	data[i].operand = three;
	data[i].opertype = type;
}




int calType(string s){
// This functions return whether the operand is label/ Hex value/ Decimal value/ Octal value
	if(s.empty()) return 0;
	if(s[0] == '+' or s[0] == '-'){
		reverse(s.begin(), s.end());
		s.pop_back();
		reverse(s.begin(), s.end());
	}
	if(s.empty())
		return -1;
	else if(isDecimal(s)) return 10;
	else if(isOctal(s)) return 8;
	else if(isHexadecimal(s)) return 16;
	else if(valid_label(s)) return 1;
	return -1;
}



void tableForm(){
// This functions process the data as:
// ----------------------------------------------------------
//  Label | Mnemonic | Operand | Operand Type (Hex, Oct etc)
// ----------------------------------------------------------
// Stores the data in aboe form in table vector
	int pc = 0;
	for(int i = 0; i < (int) cleaned.size(); i++){
		string ans[10] = {"", "", "", ""}, cur = "";
		int ptr = 1;
		for(int j = 0; j < (int) cleaned[i].size(); j++){
			if(cleaned[i][j] == ':'){
				ans[0] = cur;
				cur = "";
				j++;
				continue;
			}
			else if(cleaned[i][j] == ' '){
				ans[ptr++] = cur;
				cur = "";
				continue;
			}
			cur += cleaned[i][j];
			if(j == (int)cleaned[i].size() - 1)
				ans[ptr++] = cur;
		}
		if(!ans[1].empty()){
			data[i].lable_present_or_not = true;
		}
		else{
			data[i].lable_present_or_not = false;
		}
		if(ans[1] == "HALT")
			haltPresent = true;
		if(!ans[0].empty())
			labels[ans[0]] = pc;
		programCounter[i] = pc;
		if(ptr == 1){
			fillData(i, ans[0], "", "", 0);
			continue;
		}
		pc++;		
		if(!OPCode.count(ans[1])){
			storeErrors(i + 1, "Invalid Mnemonic");
			continue;
		}
		if(min(OPCode[ans[1]].second, 1) != ptr - 2){
			storeErrors(i + 1, "Invalid OPCode-Syntax combination");
			continue;
		}
		fillData(i, ans[0], ans[1], ans[2], calType(ans[2]));
		if(data[i].opertype == 1 and !labels.count(data[i].operand)){
			storeErrors(i + 1, "No such label / data variable");
		}
		else if(data[i].opertype == -1){
			storeErrors(i + 1, "Invalid number");
		}
	}
}



// This function, seprates the code in DATA segment, so the instructions are executed properly
void makeDataSegment()
{
	vector<string> instr, dataseg;
	for(int i = 0; i < (int)cleaned.size(); i++){
		bool state = false;
		for(int j = 0; j < cleaned[i].size(); j++){
			if(cleaned[i].substr(j, 4) == "data" and j + 4 < cleaned[i].size()){
				dataseg.push_back(cleaned[i]);
				state = true;
				break;
			}
			if(cleaned[i].back() == ':' and i + 1 < (int)cleaned.size() and cleaned[i + 1].substr(0, 4) == "data"){
				dataseg.push_back(cleaned[i]);
				state = true;
				break;
			}
		}
		if(!state)
			instr.push_back(cleaned[i]);
	}
	instr.insert(instr.end(), dataseg.begin(), dataseg.end());
	cleaned = instr;
}



void firstPass() {
// First pass of assembler which uses the functions declared above
	ifstream infile;
	cout << "Enter ASM file name to assemble:" << endl;
	cin >> fileName;
	infile.open(fileName);
	if(infile.fail()){
		cout << "Input file doesn't exist, please make sure file is in same directory as the code!" << endl;
		exit(0);
	}
	string s;
	while(getline(infile, s)) {
		string cur = clean(s, (int) cleaned.size());
		cleaned.push_back(cur);
	}
	Initialization();							// Initializing the mnemonics
	processLabel();					// Process labels in the code Ex: var1:
	if(errors.empty())
		implementSET();				// Implementing SET mnemonic
	data.resize((int) cleaned.size()); //Allocates memory for table
	programCounter.resize((int) cleaned.size());	//Allocates memory for programCounter array
	makeDataSegment();						//Seprates the code in data segment and code
	tableForm();						// Makes the code in table form as states in he problem
}




bool seeErrors() {
// Stores the errors or warnings in the file: logFile.log
	ofstream outErrors("logFile.log");
	outErrors << "Log code generated in: logFile.log" << endl;
	if(errors.empty()){
		cout << "No errors found!" << endl;
		if(haltPresent == false){
			cout << "1 warning detected" << endl;
			outErrors << "Warning: HALT not present!" << endl;
		}
		outErrors << "Machine code generated in: machineCode.o" << endl;
		outErrors << "Listing code generated in: listCode.l" << endl;
		outErrors.close();
		return true;
	}
	sort(errors.begin(), errors.end());
	cout << (int)errors.size() << " errors encountered! See logFile.log" << endl;
	for(auto to: errors){
		outErrors << to.second << endl;
	}
	outErrors.close();
	return false;
}




string appZero(string s, int sz = 6){
// Small function to append zero at end to make it of 24 bit
//Example: F changes to 00000F
	reverse(s.begin(), s.end());
	while((int) s.size() < sz)
		s += '0';
	reverse(s.begin(), s.end());	
	return s;
}




void secondPass() {
// Second pass of assembler
// Its converts the code to machine code and also generates Listing code
	for(int i = 0; i < (int) data.size(); i++){
		if(cleaned[i].empty()){
			continue;
		}
		string location = appZero(decToHex(programCounter[i]));		
		if(data[i].mnemonic == ""){
			string curMacCode = "        ";
			machineCode.push_back({i, curMacCode});
			continue;
		}
		if(data[i].opertype == 1){
			int decForm;
			if(OPCode[data[i].mnemonic].second == 2){
				int val = labels[data[i].operand];
				decForm = val - (programCounter[i] + 1);	
			}
			else{
				decForm = labels[data[i].operand];
			} 
			string curMacCode = appZero(decToHex(decForm)) + OPCode[data[i].mnemonic].first;
			machineCode.push_back({i, curMacCode});
		}
		else if(data[i].opertype == 0){
			string curMacCode = "000000" + OPCode[data[i].mnemonic].first;
			machineCode.push_back({i, curMacCode});
		}
		else{
			int sz = 6, add = 24;
			if(data[i].mnemonic == "data")
				sz = 8, add = 32;
			int decForm = stoi(data[i].operand, 0, data[i].opertype);
			string curMacCode = appZero(decToHex(decForm, add), sz) + OPCode[data[i].mnemonic].first;
			machineCode.push_back({i, curMacCode});
		}
	}
}




void writeToFile(){
// THis functions writes machine code and to the file:
// 1. Listing Code: listCode.txt
// 2. Machine code: machineCode.txt
	ofstream outList("listCode.l");
	for(auto to: machineCode){
		outList << appZero(decToHex(programCounter[to.first])) << " " << to.second << " " << cleaned[to.first] << endl;
	}
	outList.close();
	ofstream outMachineCode;
	outMachineCode.open("machineCode.o",ios::binary | ios::out);
	for(auto to: machineCode){ 
		unsigned int x;
		if(to.second.empty() or to.second == "        ")
			continue;
    	stringstream ss;
    	ss << hex << to.second;
    	ss >> x; // output it as a signed type
     	static_cast<int>(x);
 		outMachineCode.write((const char*)&x, sizeof(unsigned int));
	}
	outMachineCode.close();
	cout << "Log code generated in: logFile.log" << endl;
	cout << "Machine code generated in: machineCode.o" << endl;
	cout << "Listing code generated in: listCode.l" << endl;
}




int main() {	
	// Calling required functions
	firstPass();
	if(seeErrors()){
		secondPass();
		writeToFile();
	}
	system("pause");
	return 0;
}




