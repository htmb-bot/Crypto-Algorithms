#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cstdlib>

unsigned long long message[5][5];
unsigned long long newmessage[5][5];
//Round constants, first taken from the XKCP github, to be precalculated using another program
unsigned long long RC[24] = {0x0000000000000001,0x0000000000008082,0x800000000000808A,0x8000000080008000,0x000000000000808B,0x0000000080000001,
0x8000000080008081,0x8000000000008009,0x000000000000008A,0x0000000000000088,0x0000000080008009,0x000000008000000A,0x000000008000808B,
0x800000000000008B,0x8000000000008089,0x8000000000008003,0x8000000000008002,0x8000000000000080,0x000000000000800A,0x800000008000000A,
0x8000000080008081,0x8000000000008080,0x0000000080000001,0x8000000080008008};

//debug function for printing the message
void printMessage(){
	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			std::cout << std::hex << message[j][i] << " ";
		}
		std::cout << std::endl;
	}
}

//util function that copies newmessage into message and erases newmessage
void reset(){
	for(int i=0;i<5;i++){
		for(int j=0;j<5;j++){
			message[i][j] = newmessage[i][j];
			newmessage[i][j] = 0;
		}
	}
}

//padding function using 64-bit int
long long pads(int x, int m){
	int j = (x+((-m-2)%x)) %x;
	long long res = 1;
	res ^= (1<<j+1);
	return res;
}

//padding function using string
std::string padString(int x, int m){
	int j = (x+((-m-2)%x)) %x;
	std::string res = "1";
	for(int i=0;i<j;i++)
		res += "0";
	return res+"1";
}

//theta
void theta(){
	long long C[5] = {0,0,0,0,0};
	for(int x=0;x<5;x++){
		C[x] = message[x][0]^message[x][1]^message[x][2]^message[x][3]^message[x][4]; 
		//std::cout << C[x] << std::endl;
	}
	for(int x=0;x<5;x++){
		for(int y=0;y<5;y++){
			newmessage[x][y] = message[x][y];
			newmessage[x][y] ^= C[(x+4)%5];
			for(int z=0;z<64;z++){
				long long temp = 0;
				if((1LL<<((z+63)%64))&C[(x+1)%5]){
					temp = 1LL<<z;
				}
				newmessage[x][y] ^= temp;
			}
		}
	}
	reset();
}

//rho
void rho(){
	int x = 1; int y = 0; int temp;
	newmessage[0][0] = message[0][0];
	for(int t=0; t<24;t++){
		for(int z=0;z<64;z++){
			long long temp2 = 0;
			if((1LL<<((64+(z-(t+1)*(t+2)/2)%64)%64))& message[x][y]){
				temp2 = 1LL<<z;
			}
			newmessage[x][y] ^= temp2;  
		}
		temp = x;
		x = y;
		y = (2*temp+3*y)%5;
	}	
	reset();
}

//pi
void pi(){
	for(int x=0;x<5;x++){
		for(int y=0;y<5;y++){
			newmessage[x][y] = message[(x+3*y)%5][x];
		}
	}
	reset();
}

//chi
void chi(){
	for(int x=0;x<5;x++){
		for(int y=0;y<5;y++){
			newmessage[x][y] = message[x][y];
			newmessage[x][y] ^= ((~message[(x+1)%5][y]) & message[(x+2)%5][y]);
		}
	}	
	reset();
}

//iota
void iota(int round){
	for(int x=0;x<5;x++){
		for(int y=0;y<5;y++){
			newmessage[x][y] = message[x][y];
		}
	}		
	newmessage[0][0] ^= RC[round];
	reset();
}

//applying f to message
void f(){
	for(int round=0;round<24;round++){
		//std::cout << "-------Round " << round << "------" << std::endl << std::endl;
		theta();
		//std::cout << std::endl << "After theta" << std::endl;
		//printMessage();
		rho();
		//std::cout << std::endl << "After rho" << std::endl;
		//printMessage();
		pi();
		//std::cout << std::endl << "After pi" << std::endl;
		//printMessage();
		chi();
		//std::cout << std::endl << "After chi" << std::endl;
		//printMessage();
		iota(round);
		//std::cout << std::endl << "After iota" << std::endl;		
		//printMessage();
	}
}

//utils for bit detection
long long convert(char c){
	if(c == '1') return 1LL;
	return 0;
}

//same but reversed
std::string anticonvert(int a){
	if(a == 0) return "0";
	return "1";
}

//computes lane(i,j)
std::string lane(int i, int j){
	//std::cout << std::dec << message[i][j] << std::endl;
	std::string s = "";
	for(int z=0;z<32;z++)
		s += anticonvert((1LL<<z)&message[i][j]);
	for(int z=0;z<32;z++)
		s+= anticonvert((1<<z)&(message[i][j]>>32));
	return s;
}

//computes Plane(j)
std::string Plane(int j){
	std::string s = "";
	for(int x=0;x<5;x++)
		s += lane(x,j);
	return s;
}

//applying f directly to a string of size b=1600
std::string fString(std::string s){
	//initialize message
	for(int x=0;x<5;x++){
		for(int y=0;y<5;y++){
			message[x][y] = 0;
			newmessage[x][y] = 0;
		}
	}

	//convert s to message
	for(int x=0;x<5;x++){
		for(int y=0;y<5;y++){
			for(int z=0;z<64;z++){
				long long temp = convert(s[64*(5*y+x)+z]);
				message[x][y] ^= temp<<z;
			}
		}
	}

	//use f
	f();

	//convert msg to s
	s = Plane(0) + Plane(1) + Plane(2) + Plane(3) + Plane(4);

	return s;
}

//absorb permutation+truncated xor
std::string absorbBox(std::string s, std::string p){
	//concatenate and xor
	for(int i=0;i<1600-256;i++) p += "0";
	std::string newS = "";
	for(int i=0;i<1600;i++){
		if(s[i] == p[i]) newS += "0";
		else newS += "1";
	}
	//apply f
	return fString(newS);
}

//to print string as hex using little endian encoding
void printHex(std::string s){
	int n = s.size()/8;
	for(int i=0;i<n;i++){
		int m = 0;
		for(int j=0;j<8;j++){
			m += (convert(s[8*i+j])<<j);
		}
		std::cout << std::setfill('0') << std::setw(2) << std::hex << m;
	}
	std::cout << std::endl;
}

//function to debug the permutation (comparing with data from the website)
void testf(){
	for(int x=0;x<5;x++){
		for(int y=0;y<5;y++){
			newmessage[x][y] = 0;
			message[x][y] = 0;
		}
	}		
	printMessage();
	for(int round=0;round<24;round++){
		std::cout << "-------Round " << std::dec << round << "------" << std::endl << std::endl;
		theta();
		std::cout << std::endl << "After theta" << std::endl;
		printMessage();
		rho();
		std::cout << std::endl << "After rho" << std::endl;
		printMessage();
		pi();
		std::cout << std::endl << "After pi" << std::endl;
		printMessage();
		chi();
		std::cout << std::endl << "After chi" << std::endl;
		printMessage();
		iota(round);
		std::cout << std::endl << "After iota" << std::endl;		
		printMessage();
	}	

	std::string s = Plane(0) + Plane(1) + Plane(2) + Plane(3) + Plane(4);
	printHex(lane(0,0));
	printHex(s);		
}

int main(int argc, char *argv[]){

	int b = 1600;
	int c = 256;
	int r = b-c; 

	//Input message and required size
	int d = atoi(argv[1]);

	std::string N=""; //std::cin >> N;

  char buf;    
  while(fread(&buf, sizeof(char), 1, stdin)){
        for(int j = 0; j < 8; j++){
            N += anticonvert((1 << j)&buf);
        }
  }

	//add 1111
	std::string P = N+"1111";

	//add padding
	P = P+padString(r,P.size());
	
	int n = P.size()/r;

	//strings that will be absorbed
	std::string Water[n];
	for(int i=0; i<n;i++){
		Water[i] = P.substr(r*i,r);
	}

	//Initial state
	std::string S = "";
	for(int i=0;i<b;i++) S+="0";

	//std::cout << "----Absorbing----" << std::endl;
	//printHex(S);

	//absorbtion time !
	for(int i=0;i<n;i++){
		S = absorbBox(S,Water[i]);
		//printHex(S);
		//std::cout <<"------Next round-----" << std::endl;
	}


	std::string Z = "";
	int cnt = 0;

	//std::cout << "-----And now we squeeeze------" << std::endl;

	//Squeeze now
	do {
		if(cnt>0){
			S = fString(S);
			//std::cout << "----Permuting-----"<< std::endl;	
			//printHex(S);
		} 
		cnt++;
		Z = Z + S.substr(0,r);
	}
	while(d>8*Z.size());

	//printHex(Z);
	//std::cout << "------Trunc------" << std::endl;

	printHex(Z.substr(0,8*d));

	//testf();

	return 0;
}