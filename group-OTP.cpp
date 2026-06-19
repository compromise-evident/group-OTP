//YOUR CONTROLS:                                                                Run it: "apt install g++ geany libgmp-dev". Open the .cpp in Geany.
int digit_length = 200; //50000 max                                             //Append "-lgmp" to Geany's compile & build commands. Hit F9 once. F5 to run.
//digit_length is the length of a (jumping) value, near which
//prime gaps are found, for randomness. Bigger = more secure.



/*Version 6.0.0
Secure One-time pad for unlimited file encryption between n people.
No key is rejected upon decryption--giving false plausible output.
 ______________________________________________________________________________
/                                                                              \
|            you make 5 keys:     1       2       3       4       5            |
|         user 2 gets copies:     1       2       3       4       5            |
|         user 3 gets copies:     1       2       3       4       5            |
|         user 4 gets copies:     1       2       3       4       5            |
|         user 5 gets copies:     1       2       3       4       5            |
|                                                                              |
|                                                                              |
|                                 ^                               ^            |
|                            with which                      with which        |
|                         you encrypt, and                user 5 encrypts,     |
|                          others decrypt                and others decrypt    |
\______________________________________________________________________________/

Send in the order encrypted, and decrypt in the order received! Keys are files
that contain rolling-seeds, which are used to generate pseudorandom bytes 0-255.*/

#include <filesystem>
#include <fstream>
#include <gmp.h> //For primes
#include <iostream>
using namespace std;
int main()
{	int raw_byte;
	char file_byte;
	ifstream in_stream;
	ofstream out_stream;
	
	cout << "\n(1) Get keys"
	     << "\n(2) Encrypt outgoing file"
	     << "\n(3) Decrypt received file"
	
	     << "\n\nOption: ";
	
	int user_option; cin >> user_option;
	if((user_option != 1) && (user_option != 2) && (user_option != 3)) {cout << "\nInvalid.\n"; return 0;}
	
	//______________________________________________________Get_keys__________________________________________________//
	if(user_option == 1)
	{	//Exits if keys already exist.
		in_stream.open("keys"); if(!in_stream.fail()) {cout << "\n\"keys\" folder already exists.\n"; in_stream.close(); return 0;} in_stream.close();
		
		//Gets number of users.
		long long users = 0; cout << "Enter number of users (2 to 10^18): "; cin >> users;
		if((users < 2) || (users > 1000000000000000000)) {cout << "Out of bounds.\n\n"; return 0;}
		
		//Creates temporary seeds file.
		{	//Gets path, fixes it if dropped.
			cout << "Just once, drop/enter any file of " << digit_length << "+ random first bytes:\n";
			string path; getline(cin, path); if(path[0] == '\0') {getline(cin, path);}
			if(path[0] == '\'') {path.erase(0, 1); path.pop_back(); path.pop_back();}
			in_stream.open(path); if(in_stream.fail()) {cout << "\nNo path " << path << "\n"; return 1;} in_stream.close();
			
			//Checks file size.
			long long size = filesystem::file_size(path);
			if(size < digit_length) {cout << "\nFile too small.\n"; return 1;}
			
			//Copies bytes, writes them modified.
			filesystem::create_directories("keys");
			in_stream.open(path);
			out_stream.open("private_seeds");
			for(int a = 0; a < digit_length; a++)
			{	in_stream.get(file_byte); raw_byte = file_byte;
				if(raw_byte < 0) {raw_byte += 256;} out_stream.put((raw_byte % 10) + 48);
			}
			in_stream.close();
			out_stream.close();
		}
		
		//Loads seeds.
		char seeds[50001] = {'\0'}; cout << "Creating keys...\n";
		long long size = filesystem::file_size("private_seeds"); if(size != digit_length) {cout << "\nBad private_seeds.\n"; return 1;} //Checks file size.
		in_stream.open("private_seeds"); for(int a = 0; a < digit_length; a++) {in_stream.get(seeds[a]);} in_stream.close();            //Loads value.
		if(seeds[0] == '0') {seeds[0] = '5';}                                                                                           //Forces its length.
		mpz_t in, out; mpz_init(in); mpz_init(out); mpz_set_str(in, seeds, 10); mpz_nextprime(out, in); mpz_get_str(seeds, 10, out);    //Makes it prime.
		mpz_t prime, two; mpz_init(prime); mpz_init(two); mpz_set_str(prime, seeds, 10); mpz_set_ui(two, 2);                            //Dedicates.
		
		//Generates n keys (randomness). Each key is a value of length digit_length.
		for(long long a = 0; a < users; a++)
		{	//Generates randomness (by concatenating consecutive prime gaps).
			string key; unsigned long long wanted_length = digit_length;
			for(long long gap = 2; key.length() < wanted_length;)
			{	mpz_add(prime, prime, two);
				int primality = mpz_probab_prime_p(prime, 25);
				if(primality == false) {gap += 2;}
				else {key += to_string(gap); gap = 2;}
			}
			key.resize(digit_length);
			
			//Writes to a new key file.
			string file_name = "keys/"; file_name += to_string(a + 1);
			out_stream.open(file_name); out_stream << key; out_stream.close();
		}
		
		//Removes file "private_seeds".
		filesystem::remove_all("private_seeds");
		
		cout << "\nDone! Either share the keys folder in person,"
		     << "\nor they can get the same keys using the same"
		     << "\nfile of " << digit_length << "+ random first bytes.\n";
		
		if(users == 2) {cout << "\nYou are user 1. The other party must be user 2."                                                                   ;}
		else           {cout << "\nYou are user 1. The Other parties must be assigned \na unique user number between 2 and " << users << " incluseve.";}
	}
	
	//______________________________________________________Encrypt___________________________________________________//
	if(user_option == 2)
	{	//Exits if keys not present.
		in_stream.open("keys"); if(in_stream.fail()) {cout << "\nGet keys first.\n"; return 0;} in_stream.close();
		
		//Creates file "user" if not present.
		in_stream.open("user");
		if(in_stream.fail())
		{	long long num = 0; cout << "Just once, enter your user number: "; cin >> num;
			out_stream.open("user"); out_stream << num; out_stream.close();
		}
		in_stream.close();
		
		//Gets path, fixes it if dropped.
		cout << "\nDrop/enter file:\n"; string path; getline(cin, path); if(path[0] == '\0') {getline(cin, path);}
		if(path[0] == '\'') {path.erase(0, 1); path.pop_back(); path.pop_back();}
		in_stream.open(path); if(in_stream.fail()) {cout << "\nNo path " << path << "\n"; return 1;} in_stream.close();
		
		//Gets file size.
		long long input_file_size = filesystem::file_size(path);
		if(input_file_size == 0) {cout << "\nEmpty file.\n"; return 0;}
		
		//Sees which key to use based on file "user".
		string key_file_name = "keys/"; string user_number; in_stream.open("user"); getline(in_stream, user_number); in_stream.close();
		key_file_name += user_number;
		
		//Loads key.
		char seeds[50001] = {'\0'}; cout << "Encrypting...\n";
		long long size = filesystem::file_size(key_file_name); if(size != digit_length) {cout << "\nBad key.\n"; return 1;}          //Checks file size.
		in_stream.open(key_file_name); for(int a = 0; a < digit_length; a++) {in_stream.get(seeds[a]);} in_stream.close();           //Loads value.
		if(seeds[0] == '0') {seeds[0] = '5';}                                                                                        //Forces its length.
		mpz_t in, out; mpz_init(in); mpz_init(out); mpz_set_str(in, seeds, 10); mpz_nextprime(out, in); mpz_get_str(seeds, 10, out); //Makes it prime.
		mpz_t prime, two; mpz_init(prime); mpz_init(two); mpz_set_str(prime, seeds, 10); mpz_set_ui(two, 2);                         //Dedicates.
		
		//Generates randomness.
		in_stream.open(path);
		out_stream.open(user_number);
		for(long long gap = 2, a = 0; a < input_file_size;)
		{	mpz_add(prime, prime, two);
			int primality = mpz_probab_prime_p(prime, 25);
			if(primality == false) {gap += 2;}
			else
			{	mpz_get_str(seeds, 10, prime);
				gap *= 10; gap += seeds[digit_length - 3] - 48;
				gap *= 10; gap += seeds[digit_length - 2] - 48; gap %= 256;
				
				//Reads a byte.
				in_stream.get(file_byte); raw_byte = file_byte; if(raw_byte < 0) {raw_byte += 256;}
				
				//Encrypts byte.
				raw_byte += gap; raw_byte %= 256;
				
				//Writes byte.
				if(raw_byte < 128) {out_stream.put(raw_byte      );}
				else               {out_stream.put(raw_byte - 256);}
				
				gap = 2; a++; cout << a << " of " << input_file_size << " bytes...\n";
			}
		}
		in_stream.close();
		out_stream.close();
		
		//Updates key file.
		out_stream.open(key_file_name);
		for(long long gap = 2, a = 0; a < digit_length;)
		{	mpz_add(prime, prime, two);
			int primality = mpz_probab_prime_p(prime, 25);
			if(primality == false) {gap += 2;}
			else {mpz_get_str(seeds, 10, prime); out_stream << seeds[digit_length - 2]; gap = 2; a++;}
		}
		out_stream.close();
		
		cout << "\nEncrypted! Send file \"" << user_number << "\" to ALL other parties.\n";
	}
	
	//______________________________________________________Decrypt___________________________________________________//
	if(user_option == 3)
	{	//Exits if keys not present.
		in_stream.open("keys"); if(in_stream.fail()) {cout << "\nGet keys first.\n"; return 0;} in_stream.close();
		
		//Gets path, fixes it if dropped.
		cout << "\nDrop/enter file:\n"; string path; getline(cin, path); if(path[0] == '\0') {getline(cin, path);}
		if(path[0] == '\'') {path.erase(0, 1); path.pop_back(); path.pop_back();}
		in_stream.open(path); if(in_stream.fail()) {cout << "\nNo path " << path << "\n"; return 1;} in_stream.close();
		
		//Gets file size.
		long long input_file_size = filesystem::file_size(path);
		if(input_file_size == 0) {cout << "\nEmpty file.\n"; return 0;}
		
		//Sees which key to use based on input file name.
		string key_file_name = "keys/";
		string user_number = path.substr(path.find_last_of("/\\") + 1);
		key_file_name += user_number;
		
		//Loads key.
		char seeds[50001] = {'\0'}; cout << "Decrypting...\n";
		long long size = filesystem::file_size(key_file_name); if(size != digit_length) {cout << "\nBad key.\n"; return 1;}          //Checks file size.
		in_stream.open(key_file_name); for(int a = 0; a < digit_length; a++) {in_stream.get(seeds[a]);} in_stream.close();           //Loads value.
		if(seeds[0] == '0') {seeds[0] = '5';}                                                                                        //Forces its length.
		mpz_t in, out; mpz_init(in); mpz_init(out); mpz_set_str(in, seeds, 10); mpz_nextprime(out, in); mpz_get_str(seeds, 10, out); //Makes it prime.
		mpz_t prime, two; mpz_init(prime); mpz_init(two); mpz_set_str(prime, seeds, 10); mpz_set_ui(two, 2);                         //Dedicates.
		
		//Generates randomness.
		in_stream.open(path);
		string output_file_name = path; output_file_name += "_(decrypted)";
		out_stream.open(output_file_name);
		for(long long gap = 2, a = 0; a < input_file_size;)
		{	mpz_add(prime, prime, two);
			int primality = mpz_probab_prime_p(prime, 25);
			if(primality == false) {gap += 2;}
			else
			{	mpz_get_str(seeds, 10, prime);
				gap *= 10; gap += seeds[digit_length - 3] - 48;
				gap *= 10; gap += seeds[digit_length - 2] - 48; gap %= 256;
				
				//Reads a byte.
				in_stream.get(file_byte); raw_byte = file_byte; if(raw_byte < 0) {raw_byte += 256;}
				
				//Decrypts byte.
				/*   ______________________________________________ ________________________________________________
				    |                                              |                                                |
				    |          if sub-key <= cipherfile            |                     else                       |
				    |   then plainfile = (cipherfile - sub-key)    |    plainfile = ((256 - sub-key) + cipherfile)  |
				    |______________________________________________|________________________________________________|
				*/
				if(gap <= raw_byte) {raw_byte = (raw_byte - gap);        }
				else                {raw_byte = ((256 - gap) + raw_byte);}
				
				//Writes byte.
				if(raw_byte < 128) {out_stream.put(raw_byte      );}
				else               {out_stream.put(raw_byte - 256);}
				
				gap = 2; a++; cout << a << " of " << input_file_size << " bytes...\n";
			}
		}
		in_stream.close();
		out_stream.close();
		
		//Updates key file.
		out_stream.open(key_file_name);
		for(long long gap = 2, a = 0; a < digit_length;)
		{	mpz_add(prime, prime, two);
			int primality = mpz_probab_prime_p(prime, 25);
			if(primality == false) {gap += 2;}
			else {mpz_get_str(seeds, 10, prime); out_stream << seeds[digit_length - 2]; gap = 2; a++;}
		}
		out_stream.close();
		
		cout << "\nDecrypted!\n";
	}
}
