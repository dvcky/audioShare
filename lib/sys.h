#include <sstream>
#include <string>
using namespace std;

void error(string msg) {
	cout << RED << BOLD <<"ERROR: " << RESET << RED << msg << RESET << endl;
}

string exec(string command) {
	char buffer[128];
	string output = "";

	// Open pipe to file
	FILE* pipe = popen(command.c_str(), "r");

	// Read all lines from buffer
	while (fgets(buffer, sizeof buffer, pipe) != NULL) {
		output += buffer;
	}

	// Close pipe and send output
	pclose(pipe);
	return output;
}

void info(string msg) {
	cout << CYAN << BOLD <<"INFO: " << RESET << CYAN << msg << RESET << endl;
}

bool isNumber(string s) {
	for (char c : s) {
		if (isdigit(c) == 0) {
			return false;
		}
	}
	return true;
}

Sound& prompt(int mode, SoundList list) {
	string input;
	bool validInput = false;
	cout << BOLD << "--------" << endl;
	switch(mode) {
		case 0: list.print(YELLOW); break;
		case 1: list.print(GREEN); break;
		case 2: list.print(BLUE); break;
	}
	while(!validInput) {
		switch(mode) {
			case 0: cout << YELLOW << "Which application would you like to route? -> " << RESET; break;
			case 1: cout << GREEN << "Which microphone would you like to route? -> " << RESET; break;
			case 2: cout << BLUE << "Where would you like to send the loopback? -> " << RESET; break;
		}
		getline(cin,input);
		if(isNumber(input) && stoi(input) > -1 && stoi(input) < list.getSize()) { validInput = true; }
		else { error("Invalid input, please enter a number in the list!"); }
	}
	return list.at(stoi(input));
}

SoundList parse(string input) {
	// List to send parsed data to
	SoundList list;

	// Current Sound data
	int soundIndex = -1; // (position in SoundList)
	string soundType; // (sound type)

	stringstream stream(input); // Necessary for scanning through each line of the output from exec
	string line; // Used for temporarily storing each individual line

	// Run through each line of input
	while(getline(stream, line)) {

		// We can shortcut searching data and removing indentation by checking indentation
		int tabCount = line.find_first_not_of("\t");
		if(tabCount != -1) { // Ignore invalid lines
			if(tabCount == 0) {
				// No indent means new object, create new Sound and fetch ID
				++soundIndex;
				list.push(); // Creates new Sound
				soundType = line.substr(0, line.find_first_of("#") - 1); // Oh yeah we also get the sound type here, this is useful for later
				list.at(soundIndex).ID = stoi(line.substr(line.find_first_of("#") + 1)); // fetch ID
			} else {
				// To make things easier to read, we remove indentation on these lines. Don't worry, we stored tabCount already!
				string lineNoIndent = line.substr(tabCount);
				if(tabCount == 1) {

					// I could add a check here for the sound type to make sure it's a Sink or Source, but since they are the 
					// only types that have the Name property anyways, I can just skip that
					if(lineNoIndent.rfind("Name: ") == 0) {
						string tmpName = lineNoIndent.substr(6, lineNoIndent.size() - 6); // Storing in a variable in case it's a Source (will be explained below)
						list.at(soundIndex).name = tmpName;

						if(soundType == "Source") {
							// OK this is a bit of a mess but oh well, pretty much when we fetch sources we only need to get microphones
							// Because of this, we can actually DELETE the element we created earlier if it isn't labeled as an "alsa_input"
							if(tmpName.rfind("alsa_input.") == 0) {
								// This is an input, we can grab the Description from the next line. We do this because the Description is a lot nicer than the actual Name :)
								getline(stream, line);
								list.at(soundIndex).name = line.substr(14, line.size() - 14);
							} else {
								// Not an input, we can delete it!
								list.pull(soundIndex);
								--soundIndex;
							}
						}
					}
				} else if(tabCount == 2) {
					if(soundType == "Sink Input" && lineNoIndent.rfind("application.name = ") == 0) {
						list.at(soundIndex).name = lineNoIndent.substr(20, lineNoIndent.size() - 21);
					}
					if(soundType == "Source Output" && lineNoIndent.rfind("application.process.binary = ") == 0) {
						list.at(soundIndex).name = lineNoIndent.substr(30, lineNoIndent.size() - 31);
					}
				}
			}
		}
	}
	return list;
}