#define _CRT_SECURE_NO_WARNINGS
#include <unordered_map>
#include <bitset>
#include <iostream>
//#include <unistd.h>
#include <sstream>
#include <io.h>
#include <fcntl.h>
#include <iomanip>
using namespace std;

struct instruction {
	int v, rs, rd, rt, shamt, func, opcode, imm, target, intVal, addr, jTarget,
		dest = -1, src1 = -1, src2 = -1;
	string out, bitstr, istr;
	bool isBreak;
	instruction() {}
	instruction(int i, int addr, bool doneBreak) : isBreak(false)
	{
		unsigned int ui = (unsigned int)i;
		this->addr = addr;
		intVal = i;
		v = ui >> 31;
		opcode = ui >> 26;
		rs = ui << 6 >> 27;
		rt = ui << 11 >> 27;
		rd = ui << 16 >> 27;
		imm = i << 16 >> 16;
		target = (imm << 2);
		jTarget = ui << 6 >> 4;
		shamt = ui << 21 >> 27;
		func = ui << 26 >> 26;
		if (doneBreak) {
			stringstream ss;
			ss << "\t" << addr << "\t" << i;
			out = bitset<32>(i).to_string() + ss.str();
		}
		else {
			bitstr = bitset<32>(i).to_string();
			bitstr = bitstr.substr(0, 1) + " " + bitstr.substr(1, 5) + " " + bitstr.substr(6, 5) + " "
				+ bitstr.substr(11, 5) + " " + bitstr.substr(16, 5) + " "
				+ bitstr.substr(21, 5) + " " + bitstr.substr(26, 6);
			{
				stringstream ss;
				ss << addr;
				out = bitstr + "\t" + ss.str() + "\t";
			}

			if (v == 0) {
				out = out + "Invalid Instruction";
				istr = "Invalid Instruction";
			}
			else if (ui == 2147483648) {
				stringstream ss;
				ss << "NOP";
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 40) {
				stringstream ss;
				ss << "ADDI\tR" << rt << ", R" << rs << ", #" << imm;
				out = out + ss.str();
				istr = ss.str();
				dest = rt;
				src1 = rs;
				src2 = rs;
			}
			else if (opcode == 43) {
				stringstream ss;
				ss << "SW\tR" << rt << ", " << imm << "(R" << rs << ")";
				out = out + ss.str();
				istr = ss.str();

				src1 = rs;
				src2 = rt;
			}
			else if (opcode == 35) {
				stringstream ss;
				ss << "LW\tR" << rt << ", " << imm << "(R" << rs << ")";
				out = out + ss.str();
				istr = ss.str();
				dest = rt;
				src1 = rs;
				src2 = rs;
			}
			else if (opcode == 33) {
				stringstream ss;
				ss << "BLTZ\tR" << rs << ", #" << target;
				out = out + ss.str();
				istr = ss.str();
				//dest = rt;
				src1 = rs;
				src2 = rs;
			}
			else if (opcode == 32 && func == 0) {
				stringstream ss;
				ss << "SLL\tR" << rd << ", R" << rt << ", #" << shamt;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 32 && func == 2) {
				stringstream ss;
				ss << "SRL\tR" << rd << ", R" << rt << ", #" << shamt;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 32 && func == 34) {
				stringstream ss;
				ss << "SUB\tR" << rd << ", R" << rs << ", R" << rt;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 32 && func == 32) {
				stringstream ss;
				ss << "ADD\tR" << rd << ", R" << rs << ", R" << rt;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 32 && func == 10) {
				stringstream ss;
				ss << "MOVZ\tR" << rd << ", R" << rs << ", R" << rt;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 60 && func == 2) {
				stringstream ss;
			    ss << "MUL\tR" << rd << ", R" << rs << ", R" << rt;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 32 && func == 8) {
				stringstream ss;
				ss << "JR\tR" << rs;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 34) {
				stringstream ss;
				ss << "J\t#" << jTarget;
				out = out + ss.str();
				istr = ss.str();
			}
			else if (opcode == 32 && func == 13) {
				stringstream ss;
				ss << "BREAK";
				istr = ss.str();
				out = out + ss.str();
				isBreak = true;
			}
			else {
				printf("Opcode %i func %i not implemented\n", opcode, func);
				print();
				exit(0);
			}
		}


	}

	void print() {
		printf("%s\n%s v %i; rs %i, rt %i, rd %i, shamt %i, func %i, opcode %i, func %i, imm %i, target %i, intVal %i, addr %i\n",
			out.c_str(), bitstr.c_str(), v, rs, rt, rd, shamt, func, opcode, func, imm, target, intVal, addr);
	}
};

string printState(const int R[], const int PC, const int cycle, unordered_map< int, instruction>& MEM,
	const int breakAddr, const int lastAddr) {
	std::ios oldState(nullptr);
	stringstream ss1;
	oldState.copyfmt(ss1);
	instruction I = MEM[PC];
	ss1 << "====================\ncycle:" << cycle << " " << PC << "\t" << I.istr << "\n\nregisters:";
	for (int i = 0; i < 32; i++) {
		if (i % 8 == 0) {
			ss1 << "\nr" << std::setfill('0') << std::setw(2) << i;
			std::cout.copyfmt(oldState);
		}
		ss1 << "\t" << R[i];
	}
	ss1 << "\n\ndata:";
	for (int i = breakAddr + 4; i < lastAddr; i += 4) {
		if (((i - breakAddr - 4) / 4) % 8 == 0)
			ss1 << "\n" << i << ":";
		ss1 << "\t" << MEM[i].intVal;
	}
	ss1 << "\n\n";
	return ss1.str();

}

//int main(int argc, char* argv[])
int main()
{
	unordered_map< int, instruction> MEM;
	bool doneBreak = false;
	int breakAddr = 0;
	int lastAddr = 0;
	char buffer[4];
	int i;
	char* iPtr;
	iPtr = (char*)(void*)&i;
	int addr = 96;
	//int FD = open(argv[2] , O_RDONLY);
	int FD = _open("t1.bin", O_RDONLY);
	printf("filename: %s", "t1.bin\n");
	int amt = 4;
	while (amt != 0)
	{
		amt = _read(FD, buffer, 4);
		//amt = read(FD, buffer, 4);
		if (amt == 4)
		{
			iPtr[0] = buffer[3];
			iPtr[1] = buffer[2];
			iPtr[2] = buffer[1];
			iPtr[3] = buffer[0];
			//cout << "i = " <<hex<< i << endl;
		}
		instruction I(i, addr, doneBreak);
		MEM[addr] = I;
		if (I.isBreak) {
			doneBreak = true;
			breakAddr = addr;
		}
		addr += 4;
	}
	lastAddr = addr - 4;

	for (int i = 96; i < lastAddr; i += 4)
		cout << MEM[i].out << endl;

	// make a register file and processor state elements
	struct processorState {
		int R[32] = { 0 };
		int PC = 96;
		int cycle = 1;
		unordered_map< int, instruction> MEM;
		int preIssue[4] = { 0 };
		int preALU[2] = { 0 };
		int preMEM[2] = { 0 };
		
		struct postThings {
			int instr = 0, value = 0;
		};
		postThings postALU, postMEM;

		bool XBWcheck(int reg, int issuePos) {
			if (reg < 0) return false;
			for (int i = issuePos - 1; i >= 0; i--) {
				if (reg == MEM[preIssue[i]].dest) return true;
			}
			for (int i = 0; i < 2; i++) {
				if (reg == MEM[preALU[i]].dest) return true;
				if (reg == MEM[preMEM[i]].dest) return true;
			}
			if (reg == MEM[postALU.instr].dest) return true;
			if (reg == MEM[postMEM.instr].dest) return true;
			return false;
		}
		void WB() {
			if (postMEM.instr != 0) {
				//R[MEM[postMEM[instr]].dest] = postMEM.value;
				postMEM = postThings();
			}
			if (postALU.instr != 0) {
				//R[MEM[postALU[instr]].dest] = postALU.value;
				postALU = postThings();
			}
		}

		void IF() {
			for( int i = 0; i <2 ; i++) {
				
				// chekc if room in preissue
				if( preIssue[3] != 0 )
					break;
				// is it J,JR,NOP,BLTZ, BREAK
				//Is it J?(Jump)
				else if () {

				}
				//Is it JR?
				/*else if () {
				}*/
				//Is it NOP?
				/*else if () {
				}*/
				//Is it BLTZ?
				/*else if () {
				}*/
				//Is it BREAK
				else if () {
				}
				for (int j = 5; j > 0; j++) {
					if (preIssue[j - 1] == 0) {
						preIssue[j - 1] = j;
						break;
					}
				}
				
				// find an open spot in preIssueBuffer
				// copy the instruction to that spot 
				PC +=4;
			}	
		}
		void MEMU() {

		}

		void ISSUE() {

		}
	};
	//Instruction = opcode + 32, fcode, what chips get used.
	//	NOP = 0                    Only use IF
	//	SLL = 32, 0                ALU Pipeline, IF, Issue, ALU, WB
	//	SRL = 32, 2                ALU Pipeline, IF, Issue, ALU, WB
	//	JR = 32, 8                Only use IF
	//	MOVZ = 32, 10            ALU Pipeline, IF, Issue, ALU, WB
	//	BREAK = 32, 13            Only use IF
	//	MUL = 32, 24            ALU Pipeline, IF, Issue, ALU, WB
	//	ADD = 32, 32            ALU Pipeline, IF, Issue, ALU, WB
	//	SUB = 32, 34            ALU Pipeline, IF, Issue, ALU, WB
	//	AND = 32, 36            ALU Pipeline, IF, Issue, ALU, WB
	//	OR = 32, 37                ALU Pipeline, IF, Issue, ALU, WB
	//	BLTZ = 33                Only use IF
	//	J = 34                    Only use IF
	//	LW = 35                    MEM Pipeline, IF, Issue, MEM, WB
	//	BEQ = 36                Only use IF
	//	ADDI = 40                ALU Pipeline, IF, Issue, ALU, WB
	//	SW = 43                    MEM Pipeline, but does not go past MEM.Instead goes directly to Memory.
	processorState state;
	state.MEM = MEM;
	//MEM[PC] = PC;
	while (true) {

		state.WB();
		//state.ALU();
		//state.MEM();
		//state.ISSUE();
		state.IF();
		
		state.cycle ++;
		if (state.PC == breakAddr + 4) break;
		//instruction I = MEM[state.PC];
		//while( I.v == 0 ){
		//	state.PC += 4;
		//	I = MEM[ state.PC ];
		//}
		//state.PC += 4;
		/*nextpc = pc;

		cout << printstate(  r, pc-4,  cycle, mem,  breakaddr,  lastaddr ) << endl;*/
		//state.PC = nextpc;
		//if( cycle >= 145) break;
		
	}

}
