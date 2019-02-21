/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:

#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss


struct config{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/* you can define the cache class here, or design your own data structure for L1 and L2 cache
class cache {
      
      }
*/

struct cache{
    bitset<32> tag;
    int valid;
    int time;
};

int main(int argc, char* argv[]){

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while(!cache_params.eof())  // read config file
    {
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L1blocksize;
        cache_params>>cacheconfig.L1setsize;
        cache_params>>cacheconfig.L1size;
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L2blocksize;
        cache_params>>cacheconfig.L2setsize;
        cache_params>>cacheconfig.L2size;
    }

    // Implement by you:
    // initialize the hierarchical cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like


    int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";

    traces.open(argv[2]);
    tracesout.open(outname.c_str());

    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;        
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;

    int max,min,L1hitway,L2hitway;
    int L1updateway,L2updateway;
    bool L1hit=false,L2hit= false;

    int L1taglength,L1indexlength,L1offsetlength,cache1length,L1way;
    int L2taglength,L2indexlength,L2offsetlength,cache2length,L2way;

    //calculate cache L1&L2 parameter
    if(cacheconfig.L1setsize==0)
        cacheconfig.L1setsize==1;
    L1offsetlength = log2(cacheconfig.L1blocksize);
    L1indexlength = log2(cacheconfig.L1size*1024/(cacheconfig.L1setsize*cacheconfig.L1blocksize));
    L1taglength = 32-L1offsetlength-L1indexlength;
    cache1length = cacheconfig.L1size*1024/(cacheconfig.L1setsize*cacheconfig.L1blocksize);
    L1way = cacheconfig.L1setsize;

    if(cacheconfig.L2setsize==0)
        cacheconfig.L2setsize==1;
    L2offsetlength = log2(cacheconfig.L2blocksize);
    L2indexlength = log2(cacheconfig.L2size*1024/(cacheconfig.L2setsize*cacheconfig.L2blocksize));
    L2taglength = 32-L2offsetlength-L2indexlength;
    cache2length = cacheconfig.L2size*1024/(cacheconfig.L2setsize*cacheconfig.L2blocksize);
    L2way = cacheconfig.L2setsize;

    //define cache L1&L2
    cache **cache_L1 = new cache *[cache1length],**cache_L2 = new cache *[cache2length];
    for(int i=0;i<cache1length;i++){
        cache_L1[i] = new cache[L1way];
    }
    for(int i=0;i<cache2length;i++){
        cache_L2[i] = new cache[L2way];
    }

    //initialization
    for(int i=0;i<cache1length;i++){
        for(int j=0;j<L1way;j++){
            cache_L1[i][j].time = 0;
            cache_L1[i][j].valid = 0;
        }
    }
    for(int i=0;i<cache2length;i++){
        for(int j=0;j<L2way;j++){
            cache_L2[i][j].time = 0;
            cache_L2[i][j].valid = 0;
        }
    }

    if (traces.is_open()&&tracesout.is_open()){
        while (getline (traces,line)){   // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);

            //tag,index,offset
            string str1=accessaddr.to_string();
            bitset<32> L1Tag = bitset<32>(str1.substr(0,L1taglength));
            bitset<32> L1Index = bitset<32>(str1.substr(L1taglength,L1indexlength));
            bitset<32> L2Tag = bitset<32>(str1.substr(0,L2taglength));
            bitset<32> L2Index = bitset<32>(str1.substr(L2taglength,L2indexlength));

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R")==0) {
                //Implement by you:
                // read access to the L1 Cache,
                //  and then L2 (if required),
                //  update the L1 and L2 access state variable;
                //L1 hit?
                L1hitway = -1;
                for (int i = 0; i < L1way; i++) {
                    if (cache_L1[L1Index.to_ulong()][i].tag.to_string() == L1Tag.to_string()) {
                        L1hitway = i;
                        L1hit = true;
                        break;
                    }
                    else L1hit = false;
                }
                //L1 hit finish
                if (L1hit && cache_L1[L1Index.to_ulong()][L1hitway].valid == 1) {
                    L1AcceState = RH;
                    //L1 LRU update
                    max = -1;
                    for(int i=0;i<L1way;i++){
                        if(cache_L1[L1Index.to_ulong()][i].time > max ){
                            max = cache_L1[L1Index.to_ulong()][i].time;
                        }
                    }
                    cache_L1[L1Index.to_ulong()][L1hitway].time = max+1;
                    //L1 LRU update finish
                    L2AcceState = NA;
                } else {
                    L1AcceState = RM;
                    //update L1
                    max = -1;
                    min = 40000;
                    L1updateway = -1;
                    for(int i=0;i<L1way;i++){
                        if(cache_L1[L1Index.to_ulong()][i].time > max ){
                            max = cache_L1[L1Index.to_ulong()][i].time;
                        }
                        if(cache_L1[L1Index.to_ulong()][i].time < min){
                            min = cache_L1[L1Index.to_ulong()][i].time;
                            L1updateway = i;
                        }
                    }
                    cache_L1[L1Index.to_ulong()][L1updateway].tag = L1Tag;
                    cache_L1[L1Index.to_ulong()][L1updateway].valid = 1;
                    cache_L1[L1Index.to_ulong()][L1updateway].time = max+1;
                    //update L1 finish
                    //L2 hit?
                    L2hitway = -1;
                    for (int i = 0; i < L2way; i++) {
                        if (cache_L2[L2Index.to_ulong()][i].tag.to_string() == L2Tag.to_string()) {
                            L2hitway = i;
                            L2hit = true;
                            break;
                        }
                        else L2hit = false;
                    }
                    //L2 hit finish
                    if (L2hit && cache_L2[L2Index.to_ulong()][L2hitway].valid == 1) {
                        L2AcceState = RH;
                        //L2 LRU update
                        max = -1;
                        for(int i=0;i<L2way;i++){
                            if(cache_L2[L2Index.to_ulong()][i].time > max){
                                max = cache_L2[L2Index.to_ulong()][i].time;
                            }
                        }
                        cache_L2[L2Index.to_ulong()][L2hitway].time = max+1;
                        //L2 LRU update finish
                    } else {
                        L2AcceState = RM;
                        //update L2
                        max = -1;
                        min = 40000;
                        L2updateway = -1;
                        for (int i = 0; i < L2way; i++){
                            if(cache_L2[L2Index.to_ulong()][i].time > max){
                                max = cache_L2[L2Index.to_ulong()][i].time;
                            }
                            if (cache_L2[L2Index.to_ulong()][i].time < min) {
                                min = cache_L2[L2Index.to_ulong()][i].time;
                                L2updateway = i;
                            }
                        }
                        cache_L2[L2Index.to_ulong()][L2updateway].tag = L2Tag;
                        cache_L2[L2Index.to_ulong()][L2updateway].valid = 1;
                        cache_L2[L2Index.to_ulong()][L2updateway].time = max+1;
                        //update L2 finish
                    }
                }
            }

            else
            {
                //Implement by you:
                // write access to the L1 Cache,
                //and then L2 (if required),
                //update the L1 and L2 access state variable;
                //L1 hit?
                L1hitway = -1;
                for (int i=0;i<L1way;i++) {
                    if(cache_L1[L1Index.to_ulong()][i].tag.to_string() == L1Tag.to_string()){
                        L1hitway = i;
                        L1hit = true;
                        break;
                    }
                    else L1hit = false;
                }
                //L1 hit finish
                if (L1hit && cache_L1[L1Index.to_ulong()][L1hitway].valid == 1) {
                    L1AcceState = WH;
                    //L1 LRU update
                    max = -1;
                    for(int i=0;i<L1way;i++){
                        if(cache_L1[L1Index.to_ulong()][i].time > max ){
                            max = cache_L1[L1Index.to_ulong()][i].time;
                        }
                    }
                    cache_L1[L1Index.to_ulong()][L1hitway].time = max+1;
                    //L1 LRU update finish
                    //L2 hit?
                    L2hitway = -1;
                    for (int i = 0; i < L2way; i++) {
                        if (cache_L2[L2Index.to_ulong()][i].tag.to_string() == L2Tag.to_string()) {
                            L2hitway = i;
                            L2hit = true;
                            break;
                        }
                        else L2hit = false;
                    }
                    //L2 hit finish
                    if (L2hit && cache_L2[L2Index.to_ulong()][L2hitway].valid == 1) {
                        L2AcceState = WH;
                        //L2 LRU update
                        max = -1;
                        for(int i=0;i<L2way;i++){
                            if(cache_L2[L2Index.to_ulong()][i].time > max){
                                max = cache_L2[L2Index.to_ulong()][i].time;
                            }
                        }
                        cache_L2[L2Index.to_ulong()][L2hitway].time = max+1;
                        //L2 LRU update finish
                    }
                    else {
                        L2AcceState = WM;
                        //update L2
                        min = 40000;
                        L2updateway = -1;
                        for (int i = 0; i < L2way; i++){
                            if (cache_L2[L2Index.to_ulong()][i].time < min) {
                                min = cache_L2[L2Index.to_ulong()][i].time;
                                L2updateway = i;
                            }
                        }
                        cache_L2[L2Index.to_ulong()][L2updateway].tag = L2Tag;
                        cache_L2[L2Index.to_ulong()][L2updateway].valid = 1;
                        //update L2 finish
                    }
                }
                else{
                    L1AcceState = WM;
                    //L2 hit?
                    L2hitway = -1;
                    for (int i = 0; i < L2way; i++) {
                        if (cache_L2[L2Index.to_ulong()][i].tag.to_string() == L2Tag.to_string()) {
                            L2hitway = i;
                            L2hit = true;
                            break;
                        }
                        else L2hit = false;
                    }
                    //L2 hit finish
                    if (L2hit && cache_L2[L2Index.to_ulong()][L2hitway].valid == 1) {
                        L2AcceState = WH;
                        //L2 LRU update
                        max = -1;
                        for(int i=0;i<L2way;i++){
                            if(cache_L2[L2Index.to_ulong()][i].time > max){
                                max = cache_L2[L2Index.to_ulong()][i].time;
                            }
                        }
                        cache_L2[L2Index.to_ulong()][L2hitway].time = max+1;
                        //L2 LRU update finish
                    }
                    else
                        L2AcceState = WM;


                }



            }



            tracesout<< L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;


        }
        traces.close();
        tracesout.close();
    }
    else cout<< "Unable to open trace or traceout file ";

    return 0;
}
