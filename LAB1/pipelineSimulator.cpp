#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000
int cycle=0;
// memory size, in reality, the memory size should be 2^32, but for this lab, for the space reason, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem;
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;
    bool        nop;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem;
    bool        wrt_enable;
    bool        nop;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
public:
    bitset<32> Reg_data;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32> (0);
    }

    bitset<32> readRF(bitset<5> Reg_addr)
    {
        Reg_data = Registers[Reg_addr.to_ulong()];
        return Reg_data;
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
    {
        Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
    }

    void outputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt",std::ios_base::app);
        if (rfout.is_open())
        {
            rfout<<"State of RF:"<<endl;
            for (int j = 0; j<32; j++)
            {
                rfout << Registers[j]<<endl;
            }
        }
        else cout<<"Unable to open file";
        rfout.close();
    }

private:
    vector<bitset<32> >Registers;
};

class INSMem
{
public:
    bitset<32> Instruction;
    INSMem()
    {
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i=0;
        imem.open("imem.txt");
        if (imem.is_open())
        {
            while (getline(imem,line))
            {
                IMem[i] = bitset<8>(line);
                i++;
            }
        }
        else cout<<"Unable to open file";
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress)
    {
        string insmem;
        insmem.append(IMem[ReadAddress.to_ulong()].to_string());
        insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
        insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
        insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
        Instruction = bitset<32>(insmem);		//read instruction memory
        return Instruction;
    }

private:
    vector<bitset<8> > IMem;
};

class DataMem
{
public:
    bitset<32> ReadData;
    DataMem()
    {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i=0;
        dmem.open("dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem,line))
            {
                DMem[i] = bitset<8>(line);
                i++;
            }
        }
        else cout<<"Unable to open file";
        dmem.close();
    }

    bitset<32> readDataMem(bitset<32> Address)
    {
        string datamem;
        datamem.append(DMem[Address.to_ulong()].to_string());
        datamem.append(DMem[Address.to_ulong()+1].to_string());
        datamem.append(DMem[Address.to_ulong()+2].to_string());
        datamem.append(DMem[Address.to_ulong()+3].to_string());
        ReadData = bitset<32>(datamem);		//read data memory
        return ReadData;
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData)
    {
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
        DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
        DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
        DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));
    }

    void outputDataMem()
    {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open())
        {
            for (int j = 0; j< 1000; j++)
            {
                dmemout << DMem[j]<<endl;
            }

        }
        else cout<<"Unable to open file";
        dmemout.close();
    }

private:
    vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl;

        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl;

        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl;
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl;
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl;
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl;
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}

unsigned long shiftbits(bitset<32> inst, int start)
{
    unsigned long ulonginst;
    return ((inst.to_ulong())>>start);

}

bitset<32> signextend (bitset<16> imm)
{
    string sestring;
    if (imm[15]==0){
        sestring = "0000000000000000"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    else{
        sestring = "1111111111111111"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    return (bitset<32> (sestring));
}

bitset<32> beq_extend (bitset<16> imm)
{
    string str;
    if (imm[15]==0){
        str = "00000000000000"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }
    else{
        str = "11111111111111"+imm.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }

    return (bitset<32> (str));
}

int main()
{

    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    stateStruct state,newState;
    state.IF.PC=0;
    newState.IF.nop=0;
    newState.ID.nop=1;
    newState.EX.nop=1;
    newState.MEM.nop=1;
    newState.WB.nop=1;
    state.IF.nop=0;
    state.ID.nop=1;
    state.EX.nop=1;
    state.MEM.nop=1;
    state.WB.nop=1;

    state.EX.Rs=0;
    state.EX.Rt=0;
    state.EX.alu_op=1;
    state.EX.wrt_enable=0;
    state.EX.rd_mem=0;
    state.EX.wrt_mem=0;
    state.EX.is_I_type=0;
    state.MEM.wrt_enable=0;
    state.MEM.rd_mem=0;
    state.MEM.wrt_mem=0;
    state.WB.wrt_enable=0;

    newState.EX.Rs=0;
    newState.EX.Rt=0;
    newState.EX.alu_op=1;
    newState.EX.wrt_enable=0;
    newState.EX.rd_mem=0;
    newState.EX.wrt_mem=0;
    newState.EX.is_I_type=0;
    newState.MEM.wrt_enable=0;
    newState.MEM.rd_mem=0;
    newState.MEM.wrt_mem=0;
    newState.WB.wrt_enable=0;

    bitset<32> op1,op2,WBdata;
    bitset<32> immaddress;
    bool Stall=0,beq_taken=0;


    while (1) {

        /* --------------------- WB stage --------------------- */
        newState.WB.nop=state.MEM.nop;
        if(state.WB.nop==0)
        {

            if(state.WB.wrt_enable==1){
                WBdata=state.WB.Wrt_data;
                myRF.writeRF(state.WB.Wrt_reg_addr,WBdata);
            }
        }



        /* --------------------- MEM stage --------------------- */
        newState.MEM.nop=state.EX.nop;
        if(state.MEM.nop==0)
        {
            newState.WB.Rs=state.MEM.Rs;
            newState.WB.Rt=state.MEM.Rt;
            newState.WB.Wrt_reg_addr=state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable=state.MEM.wrt_enable;
            newState.WB.Wrt_data=state.MEM.ALUresult;

            if (state.MEM.rd_mem||Stall){                                   //MEM-MEM
                if(state.WB.Wrt_reg_addr==state.MEM.Rt) {
                    myDataMem.writeDataMem(state.MEM.ALUresult,state.WB.Wrt_data);
                }
            }

            if(state.MEM.rd_mem==1 && state.MEM.wrt_mem==0){                         //lw
                newState.MEM.Wrt_data=myDataMem.readDataMem(state.MEM.ALUresult);
                newState.WB.Wrt_data=myDataMem.readDataMem(state.MEM.ALUresult);
            }
            if(state.MEM.rd_mem==0 && state.MEM.wrt_mem==1){                        //sw
                myDataMem.writeDataMem(state.MEM.ALUresult,state.MEM.Store_data);
            }
        }



        /* --------------------- EX stage --------------------- */
        newState.EX.nop=state.ID.nop;
        if(state.EX.nop==0)
        {

            newState.MEM.Rs=state.EX.Rs;
            newState.MEM.Rt=state.EX.Rt;
            newState.MEM.Wrt_reg_addr=state.EX.Wrt_reg_addr;
            newState.MEM.wrt_enable=state.EX.wrt_enable;
            newState.MEM.rd_mem=state.EX.rd_mem;
            newState.MEM.wrt_mem=state.EX.wrt_mem;

            op1=state.EX.Read_data1;
            op2=state.EX.Read_data2;

            if(state.WB.wrt_enable||Stall){                                             //MEM-EX
                if(state.WB.Wrt_reg_addr==state.EX.Rs)
                {
                    op1=state.WB.Wrt_data;
                }
                if(state.WB.Wrt_reg_addr==state.EX.Rt)
                {
                    op2=state.WB.Wrt_data;
                }
            }

            if(state.MEM.wrt_enable||Stall){                                  //EX-EX
                if(state.MEM.Wrt_reg_addr==state.EX.Rs)
                {
                    op1=state.MEM.ALUresult;
                }
                if(state.MEM.Wrt_reg_addr==state.EX.Rt)
                {
                    op2=state.MEM.ALUresult;
                }
            }

            if(state.EX.is_I_type==0){
                if(state.EX.alu_op==1){
                    unsigned long a = op1.to_ulong()+ op2.to_ulong();
                    newState.MEM.ALUresult = bitset<32>(a);
                }
                else {
                    unsigned long b = op1.to_ulong() - op2.to_ulong();
                    newState.MEM.ALUresult = bitset<32>(b);
                }
            }
            else{
                if(state.EX.wrt_mem)
                    newState.MEM.Store_data = op2;
                bitset<32> signextendimm = signextend (state.EX.Imm);
                unsigned long c = op1.to_ulong() + signextendimm.to_ulong();
                newState.MEM.ALUresult = bitset<32>(c);

            }
        }



        /* --------------------- ID stage --------------------- */
        if(Stall==0)
        {
            newState.ID.nop = state.IF.nop;
            if (state.ID.nop == 0) {

                bitset<32> instruction = state.ID.Instr;
                bitset<6> Opcode = bitset<6>(shiftbits(instruction, 26));
                bitset<6> Rfunction = bitset<6>(shiftbits(instruction, 0));

                if (Opcode.to_string() == "000000")                     //R-type
                    newState.EX.is_I_type = 0;
                else newState.EX.is_I_type = 1;                        //I-type

                if(newState.EX.is_I_type == 1) {
                    if(Opcode.to_string() == "000100") {
                        newState.EX.wrt_enable=0;
                        newState.EX.rd_mem=0;
                        newState.EX.wrt_mem=0;
                        newState.EX.Rs = bitset<5>(shiftbits(instruction, 21));
                        newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                        newState.EX.Rt = bitset<5>(shiftbits(instruction, 16));
                        newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
                        newState.EX.Imm = bitset<16>(shiftbits(instruction, 0));
                        //immaddress = beq_extend(newState.EX.Imm);
                        if (newState.EX.Read_data1 != newState.EX.Read_data2)
                            beq_taken = 1;

                    }
                }

                if (newState.EX.is_I_type == 0) {
                    if (Rfunction.to_string() == "100001") {                                        //addu
                        newState.EX.alu_op = 1;
                        newState.EX.Rs = bitset<5>(shiftbits(instruction, 21));
                        newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                        newState.EX.Rt = bitset<5>(shiftbits(instruction, 16));
                        newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
                        newState.EX.Wrt_reg_addr = (shiftbits(instruction, 11));
                        newState.EX.wrt_mem = 0;
                        newState.EX.rd_mem = 0;
                        newState.EX.wrt_enable = 1;
                    }
                    if (Rfunction.to_string() == "100011") {                                 //subu
                        newState.EX.alu_op = 0;
                        newState.EX.Rs = bitset<5>(shiftbits(instruction, 21));
                        newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                        newState.EX.Rt = bitset<5>(shiftbits(instruction, 16));
                        newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
                        newState.EX.Wrt_reg_addr = (shiftbits(instruction, 11));
                        newState.EX.wrt_mem = 0;
                        newState.EX.rd_mem = 0;
                        newState.EX.wrt_enable = 1;
                    }
                } else {
                    if (Opcode.to_string() == "100011") {                                    //lw
                        newState.EX.alu_op = 1;
                        newState.EX.wrt_enable = 1;
                        newState.EX.wrt_mem = 0;
                        newState.EX.rd_mem = 1;
                        newState.EX.Rs = bitset<5>(shiftbits(instruction, 21));
                        newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                        newState.EX.Rt = bitset<5>(shiftbits(instruction, 16));
                        newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
                        newState.EX.Wrt_reg_addr = bitset<5>(shiftbits(instruction, 16));
                        newState.EX.Imm = bitset<16>(shiftbits(instruction, 0));
                    }
                    if (Opcode.to_string() == "101011") {                                    //sw
                        newState.EX.alu_op = 1;
                        newState.EX.wrt_enable = 0;
                        newState.EX.wrt_mem = 1;
                        newState.EX.rd_mem = 0;
                        newState.EX.Rs = bitset<5>(shiftbits(instruction, 21));
                        newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
                        newState.EX.Rt = bitset<5>(shiftbits(instruction, 16));
                        newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
                        newState.EX.Wrt_reg_addr = bitset<5>(shiftbits(instruction, 16));
                        newState.EX.Imm = bitset<16>(shiftbits(instruction, 0));
                    }
                }

                if (state.EX.is_I_type == 1 && state.EX.wrt_enable == 1)                                //stall
                {
                    if (state.MEM.Wrt_reg_addr == newState.EX.Rs) {
                        Stall = 1;
                        //state.IF.nop=1;
                        newState.IF = state.IF;
                        newState.ID = state.ID;
                        newState.EX.nop = 1;
                    }
                    if (state.MEM.Wrt_reg_addr == newState.EX.Rt) {
                        Stall = 1;
                        //state.IF.nop=1;
                        newState.IF = state.IF;
                        newState.ID = state.ID;
                        newState.EX.nop = 1;
                    }
                } else Stall=0;

            }
        }


        /* --------------------- IF stage --------------------- */
        if(Stall==1){
            newState.EX.nop=0;
            newState.IF=state.IF;
            Stall=0;
        }
        else {
            if (state.IF.nop == 0) {
                newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
                bitset<6> bk = bitset<6> (shiftbits(newState.ID.Instr,26));
                if(bk.to_string()=="111111")
                    newState.IF.nop=1;
                else{
                    if(beq_taken==1) {
                        immaddress = beq_extend(newState.EX.Imm);
                        newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + immaddress.to_ulong());
                        newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
                        newState.ID.nop = 1;
                        beq_taken = 0;
                    }
                    else{
                        newState.IF.PC = bitset<32>(state.IF.PC.to_ulong()+4);
                    }
                }

                /*newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
               if (beq_taken == 1) {
                   immaddress = beq_extend(newState.EX.Imm);
                   newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + immaddress.to_ulong());
                   newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
                   newState.ID.nop = 1;
                   beq_taken = 0;
               }
               bitset<6> bk = bitset<6>(shiftbits(newState.ID.Instr, 26));
               if (bk.to_string() == "111111")
                   newState.IF.nop = 1;
               else newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);*/
                }


        }




        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;

        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...
        cycle++;

        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

    }

    myRF.outputRF(); // dump RF;
    myDataMem.outputDataMem(); // dump data mem

    return 0;
}