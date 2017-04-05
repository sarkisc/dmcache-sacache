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

class dmcache
{
	private:
		unsigned char theMem[65536];
		int dirty[64];
		int tag[64];
		//unsigned short curAddress;
		//char data[4];
		unsigned char cache[4][64]; 
		ofstream output;
	
	public:
		dmcache();
		string read();
		void execute(int, int ,unsigned char);
		//void write();

};


dmcache::dmcache()
{
	output.open("dm-out.txt");
	for ( int i = 0; i < 64  ; i++ )
	{
		for ( int j = 0 ; j < 4 ; j++ )
		{
			cache[j][i] = 0;
			//cout << hexChar2Ascii(cache[j][i]) << "\n";
		}
	}
	for ( int i = 0; i < 65536 ; i++ )
	{
		theMem[i] = 0;
	}
	for ( int i = 0 ; i < 64 ; i++ )
	{
		tag[i] = -1;
		//cout << tag[i] << "\n";
	}
	for ( int i = 0 ; i < 64 ; i++ )
	{
		dirty[i] = 0;
		//cout << dirty[i] << "\n";
	}
}

void dmcache::execute( int theAddress, int readWrite, unsigned char chData )
{
	
	bool HIT = false;
	int cacheLine = (theAddress/4)%64;
	int offset = theAddress%4; // offset
	int newTag = theAddress/256; //theAddress - offset;
	
	if ( readWrite == 0x00 ) // read
	{
		if ( tag[cacheLine] == newTag ) // hit
		{
			HIT = true;
			//cache[offset][cacheLine] = theMem[theAddress];
		}
		
		else if ( tag[cacheLine] != newTag ) // miss
		{
			//HIT = false;
			
			if ( dirty[cacheLine] == 1 ) // dirty
			{
				for ( int i = 0 ; i < 4 ; i++ )
				{
					//cout << (((tag[cacheLine])*256)+cacheLine+i) << "\n";
					theMem[((tag[cacheLine])*256)+(cacheLine*4)+i] = cache[i][cacheLine]; // back up to memory before evicting
				}
				dirty[cacheLine] = 0;
			}
			
			for ( int i = 0 ; i < 4 ; i++ ) // evicting from cache
			{
				cache[i][cacheLine] = theMem[(newTag*256)+(cacheLine*4)+i];
			}
		}
		output << hexChar2Ascii(cache[offset][cacheLine]) << " " << HIT  << " " << dirty[cacheLine] << "\n";
		tag[cacheLine] = newTag;
	}
	
	else if ( readWrite == 0xff ) // write
	{
		//cout << hexChar2Ascii(cache[offset][cacheLine]) << "\n";
		if ( tag[cacheLine] == newTag ) // hit
		{
			HIT = true;
		}
		
		else if ( tag[cacheLine] != newTag ) // miss
		{
			//HIT = false;
			if ( dirty[cacheLine] == 1 ) // dirty
			{
				for ( int i = 0 ; i < 4 ; i++ )
				{
					theMem[((tag[cacheLine])*256)+(cacheLine*4)+i] = cache[i][cacheLine]; // back up to memory before evicting
				}
			}
			for ( int i = 0 ; i < 4 ; i++ )
			{
				cache[i][cacheLine] = theMem[(newTag*256)+(cacheLine*4)+ i]; // evicting from cache
			}
		}
		cache[offset][cacheLine] = chData;
		tag[cacheLine] = newTag;
		dirty[cacheLine] = 1;
	}
	
	/*
	if (dirty && !HIT)
	{
		theMem.write( theAddress, chData );
	}
	if ( readWrite == 0x00 && etc.. )
	{
		output << hexChar2Ascii(chData) << HIT << dirty;
		HIT = false;
		HIT = true;
	}*/
}


int main( int argc, char *argv[] )
{
	
	if ( argc != 2 ) // argc should be 2 for correct execution
	{
		cout << "Please provide an input .txt file as an argument.";
	}
	
	else
	{
		dmcache theCache;
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