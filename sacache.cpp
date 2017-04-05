#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

int ascii2Hex( string& inputStr ) // ascii string to hex
{
	int myInt;
	//do some checks to make sure string has only 4 characters
	// make sure they are all numbers 0-9 || chars a-f
	stringstream ss;
	ss << hex  << inputStr;
	ss >> myInt;
	return myInt;
}

string hexChar2Ascii( unsigned char myChar ) // ascii string to hex
{
	string myString;
	//do some checks to make sure string has only 4 characters
	// make sure they are all numbers 0-9 || chars a-f
	stringstream ss;
	ss << hex << uppercase << setw(2) << setfill('0') << (int)myChar;
	myString = ss.str();
	return myString;
}


/*class memory
{
	private:
		int SIZE;
		unsigned char theMemory[65536]; // [0]-[65535]
	
	public:
		memory();
		unsigned char read ( int address );
		void write( int address, unsigned char data );
	
};

memory::memory()
{
	int SIZE = 65536;
	for ( int i = 0 ; i < SIZE ; i++ )
	{
		theMemory[i] = 0;
	}
}

unsigned char memory::read( int address )
{
	return theMemory[address];
}

void memory::write( int address, unsigned char data )
{
	theMemory[address] = data;
}*/

class sacache
{
	private:
		unsigned char theMem[65536];
		int dirty[8][5]; // [set][cacheLine]
		int tag[8][5]; // [set][cacheLine]
		//unsigned short curAddress;
		//char data[4];
		unsigned char cache[8][5][4]; // [set][cacheLine][offset]
		int LRU[8][5];
	
		ofstream output;
	
	public:
		sacache();
		string read();
		void execute(int, int ,unsigned char);
		int isTagInSet(int, int);
		int getLRUCacheLine( int );
		//void write();

};

sacache::sacache()
{
	output.open("sa-out.txt");
	for ( int i = 0; i < 4  ; i++ )
	{
		for ( int j = 0 ; j < 5 ; j++ )
		{
			for ( int k = 0 ; k < 8 ; k++ )
			{
				cache[k][j][i] = 0;
				//cout << hexChar2Ascii(cache[k][j][i]) << "\n";
			}
		}
	}
	for ( int i = 0; i < 65536 ; i++ )
	{
		theMem[i] = 0;
	}
	for ( int i = 0 ; i < 8 ; i++ )
	{
		for ( int j = 0 ; j < 5 ; j++ )
		{
			tag[i][j] = -1;
			//cout << tag[i][j] << "\n";
			//cout << tag[i] << "\n";
		}
	}
	for ( int i = 0 ; i < 8 ; i++ )
	{
		for ( int j = 0 ; j < 5 ; j++ )
		{
			dirty[i][j] = 0;
			//cout << dirty[i][j] << "\n";
		}
	}
	
	for ( int i = 0 ; i < 8 ; i++ )
	{
		for ( int j = 0 ; j < 5 ; j++ )
		{
			LRU[i][j] = 4 - j;
			//cout << LRU[i][j] << "\n";
		}
	}
}

void sacache::execute( int theAddress, int readWrite, unsigned char chData )
{
	bool HIT = false;
	int set = (theAddress/4)%8; //int cacheLine = (theAddress/4)%64;
	int offset = theAddress%4; // offset
	int newTag = theAddress/32;
	int cacheLine = isTagInSet( set, newTag );
	
	if ( readWrite == 0x00 ) // read
	{
		if( cacheLine != -1 ) // hit
		{
			/*for ( int i = 0 ; i < 5 ; i++ )
			{
				cout << LRU[set][i] << " ";
			}
			cout << "      ";*/
			HIT = true;
			for ( int i = 0 ; i < 5 ; i++ )
			{
				if( LRU[set][i] < LRU[set][cacheLine] )
					(LRU[set][i])++;
			}
			LRU[set][cacheLine] = 0;
			/*for ( int i = 0 ; i < 5 ; i++ )
			{
				cout << LRU[set][i] << " ";
			}
			cout << "\n";*/
		}
		else if( cacheLine == -1 ) // miss
		{
			cacheLine = getLRUCacheLine(set);
			
			if ( dirty[set][cacheLine] == 1 ) // dirty
			{
				for ( int i = 0 ; i < 4 ; i++ )
				{
					//cout << (((tag[cacheLine])*256)+cacheLine+i) << "\n";
					theMem[((tag[set][cacheLine])*32)+(set*4)+i] = cache[set][cacheLine][i]; // back up to memory before evicting
				}
				dirty[set][cacheLine] = 0;
			}
			
			for ( int i = 0 ; i < 4 ; i++ ) 
			{
				cache[set][cacheLine][i] = theMem[(newTag*32)+(set*4)+i]; // evicting from cache working
			}
			for ( int i = 0 ; i < 5 ; i++ ) //update the LRU array // working
			{
				LRU[set][i] = ((LRU[set][i])+1)%5;
			}
			
		}
		
		output << hexChar2Ascii(cache[set][cacheLine][offset]) << " " << HIT  << " " << dirty[set][cacheLine] << "\n";
		tag[set][cacheLine] = newTag;
	}
	
	else if ( readWrite == 0xff ) // write
	{
		if( cacheLine != -1 ) // hit
		{
			HIT = true;
			for ( int i = 0 ; i < 5 ; i++ )
			{
				if( LRU[set][i] < LRU[set][cacheLine] )
					(LRU[set][i])++;
			}
			LRU[set][cacheLine] = 0;
		}
		else if( cacheLine == -1 ) // miss
		{
			cacheLine = getLRUCacheLine(set);
			
			if ( dirty[set][cacheLine] == 1 ) // dirty
			{
				for ( int i = 0 ; i < 4 ; i++ )
				{
					theMem[((tag[set][cacheLine])*32)+(set*4)+i] = cache[set][cacheLine][i]; // back up to memory before evicting
				}
			}
			for ( int i = 0 ; i < 4 ; i++ )
			{
				cache[set][cacheLine][i] = theMem[(newTag*32)+(set*4)+i]; // evicting from cache
			}
			for ( int i = 0 ; i < 5 ; i++ ) //update the LRU array
			{
				LRU[set][i] = ((LRU[set][i])+1)%5;
			}
		}
		cache[set][cacheLine][offset] = chData;
		tag[set][cacheLine] = newTag;
		dirty[set][cacheLine] = 1;
	}
}

int sacache::isTagInSet( int setNum, int newTag )
{
	//bool isInSet = false;
	int cacheLine = -1;
	
	for( int i = 0; i < 5; i++ )
	{
		//cout << tag[setNum][i] << " " <<  newTag << "\n";
		if( tag[setNum][i] == newTag )
		{
			cacheLine = i;
		}
	}
	return cacheLine;
}	


//it will return a cacheLine
int sacache::getLRUCacheLine( int setNum )
{
	for ( int i = 0 ; i < 5 ; i++ )
	{
		if ( LRU[setNum][i] == 4 )
		{
			return i;
		}
	}
	return -1; // this should never happen
}

int main( int argc, char *argv[] )
{
	
	if ( argc != 2 ) // argc should be 2 for correct execution
	{
		cout << "Please provide an input .txt file as an argument.";
	}
	
	else
	{
		sacache theCache;
		string inputFileName(argv[1]);
		
		ifstream inputFile(inputFileName.c_str());
		
		cout << "Opening file " << inputFileName.c_str() << "\n";
		
		if ( !inputFile.is_open() )
		{
			cout << "Could not open file\n";
		}
		else
		{
			cout << "File opened successfully\n";
			string curLine = "ARMEN!!";
			
			string temp;
			
			int theAddress;
			int readWrite;
			//char chReadWrite;
			int data;
			char chData;
			
			for ( int i = 0 ; !(inputFile.eof())  ; i++ )
			{
				getline( inputFile, curLine, '\n' );
						
				if ( curLine.length() < 10 )
				{
					break;
				}
				
				temp = curLine.substr(0,4);
				theAddress = ascii2Hex(temp);
				
				temp = curLine.substr(5,2);
				readWrite = ascii2Hex(temp);
				//chReadWrite = (unsigned char)(readWrite);
				
				temp = curLine.substr(8,2);// make sure u account for the space bar character
				data = ascii2Hex(temp);
				chData = (unsigned char)(data);
				
				
				
				temp = hexChar2Ascii(chData);
				
				//cout << temp << "\n";
				
				theCache.execute( theAddress, readWrite, chData );
				
				//cout << hex << theAddress << " " << hex << readWrite << " " << (unsigned int)chData << "\n";
				
			}
		}
	}
	/*string myString("ffff");
	cout << ascii2Hex(myString);
	*/
	
	return 0;
}