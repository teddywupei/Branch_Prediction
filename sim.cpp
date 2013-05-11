
#include <iostream>
#include <string>
#include <fstream>
#include <bitset>
#include <vector>
#include <iomanip> 
#include <stdlib.h> 
#include <math.h>

using namespace std;

class Branch{
protected:
	int M, N;
	int* prediction_table;
	size_t size; // table size
	int mispredictions;

public:
	string make_prediction(int index, string actual_outcome);
    void update_prediction_table(int index, string actual_outcome);
    void print_miss_rate(int sum);
    void print_contents();
	Branch(int m, int n, int ini):M(m), N(n), mispredictions(0)
	   {  
           size = (size_t)pow(double(2),double(M));  
           prediction_table = new int[size];
            for(size_t i=0; i!=size; ++i){
             prediction_table[i] =ini;
            }
	    }

};

class Bimodal: public Branch {
public:

	Bimodal(int m, int n, int ini): Branch(m,n,ini) {}
	int get_b_index(int M, unsigned long branch_PC);

};

class Gshare: public Branch {
public:

	Gshare(int m, int n, int ini): Branch(m,n, ini),BHR(0){	}
	int get_g_index(int M, int N, unsigned long branch_PC);
	void update_BHR(int N, string actual_outcome);
private:
	bitset<24> BHR_bit;
	int BHR;
};

class Hybrid: public Branch{
public:
	string dirty;
	Hybrid(int K, int n, int ini): Branch(K, n, ini){}
	int get_h_index(int K, unsigned long branch_PC); 
	string make_h_prediction(int index, string b_prediction, string g_prediction, string actual_outcome);
	void update_h_table(string b_prediction, string g_prediction, string actual_outcome, int h_index);

};


// return the least M bits of branch_PC as index
int Bimodal::get_b_index(int M, unsigned long branch_PC){
	bitset<24> bitvec(branch_PC>>2);

	for (int index=23; index!=M-1; index--){
     bitvec[index]=0;
    }
	int index = (int)bitvec.to_ulong();
//	cout<<"PT index: " <<dec<< index<<endl;
	return index;
}

// get index of gshare
int Gshare::get_g_index(int M, int N, unsigned long branch_PC){
	bitset<24> M_PC(branch_PC>>2);
	bitset<24> M_N_PC;
  //get the M-N bits of PC
	for (int index=M-1; index!=N-1; index--){
	 M_N_PC[index]=M_PC[index];
	}
  //get the lowest N bits of PC
	for (int index=23; index!=N-1; index--){
     M_PC[index]=0;
	}

   // cout<<"N_bit: "<<M_PC<<endl; 
  //get the lowest N bits of BHR
	for (int index=23; index!=N-1; index--){
	 BHR_bit[index]=0;
	}

   // cout<<"BHT_bit: "<<BHR_bit<<endl;
    
  //lowest N bits of M_PC Xor with BHR_Bit
  //	for (int index=0; index!=N+1; index++){
	M_PC^=BHR_bit;
  //	}
	
//	cout<<"XOR: "<<M_PC<<endl;
  //get M bits for index
	for (int index=M-1; index!=N-1; index--){
	 M_PC[index]=M_N_PC[index];
	}
  
  //  cout<<"MPC: "<<M_PC<<endl;
	int index = (int)M_PC.to_ulong();
//	cout<<"PT index: " <<dec<< index<<endl;
	return index;
}

// return the K bits of branch_PC as index
int Hybrid::get_h_index(int K, unsigned long branch_PC){
	bitset<24> bitvec(branch_PC>>2);
	for (int index=23; index!=K-1; index--){
     bitvec[index]=0;
	}
	int index = (int)bitvec.to_ulong();
//	cout<<"CT index: " <<dec<< index<<endl;
//	cout<<"CT value: " <<dec<<prediction_table[index] <<endl;
	return index;
}

// if index value is >>2, choose gshare predictor, otherwise choose bimodal predictor
string Hybrid::make_h_prediction(int index, string b_prediction, string g_prediction, string actual_outcome){
  string prediction; 
  if (prediction_table[index]>=2) {prediction=g_prediction;dirty="g";}
  else {prediction=b_prediction;dirty="b";}
//  if (prediction=="t") cout<<"prediction:  true"<<endl;
//  else                            cout<<"prediction:  false"<<endl;
  if (prediction!=actual_outcome)   mispredictions++; 
  return prediction;
}

void Gshare::update_BHR(int N, string actual_outcome){
	BHR_bit=(BHR>>1);
	if (actual_outcome=="t")
	  BHR_bit[N-1]=1;
	else
      BHR_bit[N-1]=0;
 
//    cout<<"BHR now set to: "; 
//    for(int index=N-1; index>=0;index--){
//      cout<<"["<<BHR_bit[index]<<"]";
//    }     
//    cout<<endl;
	BHR=(int)BHR_bit.to_ulong();
}

//use index to get the branch's counter from prediction table , if the counter value is greater than or equal to 4, then predition is taken, else is not-taken
string Branch::make_prediction(int index, string actual_outcome){
   string prediction;
   //find counter
   int counter = prediction_table[index];
 //  cout<<"PT value: "<<counter<<endl;
   //make a decision
   if (counter>=4) {
                   prediction="t";
   //                cout<<"prediction: true"<<endl;
                   }
   else            {
                   prediction="n";
   //                cout<<"prediction: false"<<endl; 
         }
   if (prediction!= actual_outcome)   mispredictions++;
   return prediction;       

}

//update the branch predictor based on the branch's actual outcome
void Branch::update_prediction_table(int index, string actual_outcome){
    
   if (actual_outcome=="t")
      { 
       if(prediction_table[index]<7) prediction_table[index]+=1;                          
      }
   else
      { 
       if(prediction_table[index]>0) prediction_table[index]-=1;
      }   
      
   //cout<<"new PT value: "<< prediction_table[index]<<endl;                                            
}       

void Hybrid::update_h_table(string b_prediction, string g_prediction, string actual_outcome, int h_index)
{ if (g_prediction==actual_outcome && b_prediction!=actual_outcome) 
        {if (prediction_table[h_index]!=3) prediction_table[h_index]++;
        }
else if (g_prediction!=actual_outcome && b_prediction==actual_outcome)
        {if (prediction_table[h_index]!=0) prediction_table[h_index]--;
        }
// cout<<"New CT value:  "<< prediction_table[h_index]<<endl;
}

//print miss rate
void Branch::print_miss_rate(int sum){  
 double mis_rate = (double)mispredictions/sum*100;
 cout<<fixed;
 cout<<"number of mispredictions:     "<<dec<<setprecision(2)<<mispredictions<<endl;
 cout<<"misprediction rate:           "<<mis_rate<<"%"<<endl;
}

//print table contents
void Branch::print_contents(){
 for(int i=0; i<size; i++){ 
    cout<<dec<<i<<"	";
    cout<<prediction_table[i]<<endl; 
    }    
}

int main (int argc, char* argv[]){
 //input from command line
 int M2=0, M1=0, N=0, K=0;
 const char* tracefile;

 //input from file reading
 unsigned long branch_PC;
 string file_PC_in;
 string actual_outcome;

 //intermediate variable
 int b_index, g_index, h_index;
 string b_prediction, g_prediction, h_prediction;

 cout << "COMMAND" << endl;
// while(*argv){
// cout << *argv++<<" ";
// }
// int trace_index;
 int mode;  // 0 for bimodal, 1 for gshare, 2 for hybrid 
 if (argc == 4) 
    { mode = 0;
      M2 = atoi(argv[2]);  // # of PC bits used to index the bimodal table
      N = 0;               // n=0
      tracefile=argv[3];
      cout<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<" "<<argv[3]<<endl;
    }     
 else if (argc == 5)
    { mode =1;
      M1 = atoi(argv[2]);  // # of PC bits 
      N = atoi(argv[3]);   // global branch history register bits used to index the gshare table, n>0
      tracefile= argv[4];
      cout<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<" "<<argv[3]<<" "<<argv[4]<<endl;
    }
 else if (argc == 7)    
    { mode =2;
      K = atoi(argv[2]);   // # of PC bits used to index the chooser table
      M1 = atoi(argv[3]);  // # of PC bits 
      N = atoi(argv[4]);   // global branch history register bits used to index the gshare table
      M2 = atoi(argv[5]);  // # of PC bits used to index the bimodal table
      tracefile= argv[6];
      cout<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<" "<<argv[3]<<" "<<argv[4]<<" "<<argv[5]<<" "<<argv[6]<<endl;
    }  

    
/*
   size_t b_n = (size_t)pow(double(2),double(M2));  
   int* bimodal_prediction_table = new int[b_n];

   for(size_t i=0; i!=b_n; ++i){
      bimodal_prediction_table[i] =4;
   }
*/
 //  size_t g_n = (size_t)pow(double(2),double(M1));
 //  int* gshare_prediction_table= new int[g_n];
  int hybrid_n=0;
     Bimodal* bimodal = new Bimodal (M2, N, 4); 
     Gshare* gshare = new Gshare (M1, N, 4); 
	 Hybrid* hybrid = new Hybrid (K, hybrid_n, 1);

 ifstream fin(tracefile);

// if(fin.is_open()) cout<<"file open"<<endl;
 int line_count=0;
 while(fin>>file_PC_in && fin>>actual_outcome){                  
  	 branch_PC= strtoul(file_PC_in.c_str(), NULL, 16);
 //cout<<"<Line #"<<line_count<<">       "<<hex<<branch_PC<<"  "<<actual_outcome<<endl;
	 if(mode==0) {
                 b_index = bimodal->get_b_index(M2, branch_PC);
                 b_prediction = bimodal->make_prediction(b_index, actual_outcome);
                 bimodal->update_prediction_table(b_index, actual_outcome);                 

	             }
	 if(mode==1) {
                 g_index = gshare->get_g_index(M1, N, branch_PC);
                 g_prediction = gshare->make_prediction(g_index, actual_outcome);
                 gshare->update_prediction_table(g_index, actual_outcome);                 
				 gshare->update_BHR(N, actual_outcome);
	             }
     if(mode==2) {
	               //get index and make prediction
		         h_index= hybrid->get_h_index(K, branch_PC);
				 b_index = bimodal->get_b_index(M2, branch_PC);
				 b_prediction = bimodal-> make_prediction(b_index, actual_outcome);
				 g_index = gshare->get_g_index(M1, N, branch_PC);
                 g_prediction = gshare->make_prediction(g_index, actual_outcome);
				 h_prediction = hybrid->make_h_prediction(h_index, b_prediction, g_prediction, actual_outcome);
				  //update prediction table and BHR
				 if (hybrid->dirty =="b") bimodal->update_prediction_table(b_index,actual_outcome);
				 else                     gshare->update_prediction_table(g_index, actual_outcome);      
				 gshare->update_BHR(N, actual_outcome);
				 hybrid->update_h_table(b_prediction, g_prediction, actual_outcome, h_index);
	 }
//    cout<<endl;
	line_count++;
     }    

 cout<<"OUTPUT"<<endl;
 cout<<"number of predictions:        "<<dec<<line_count<<endl;
 if(mode==0){
  bimodal->print_miss_rate(line_count);
  cout<<"FINAL BIMODAL CONTENTS"<<endl;
  bimodal->print_contents();
  }     
 if(mode==1){
  gshare->print_miss_rate(line_count);
  cout<<"FINAL GSHARE CONTENTS"<<endl;
  gshare->print_contents();
  }
 if(mode==2){
  hybrid->print_miss_rate(line_count);
  cout<<"FINAL CHOOSER CONTENTS"<<endl; 
  hybrid->print_contents();    
  cout<<"FINAL GSHARE CONTENTS"<<endl;
  gshare->print_contents();    
  cout<<"FINAL BIMODAL CONTENTS"<<endl;
  bimodal->print_contents(); 
  }
 
 fin.close();
                
}
