#include <iostream>
#include <string>
using namespace std;

// Used to store minimal data on PulseAudio sinks, sources, sink inputs, and source outputs
struct Sound {
	int ID;
	string name;
};

// Basic dynamic array for storing Sound types, with minimal features to conserve memory
class SoundList {
	public:
		SoundList() {
			list = new Sound[8];
			size = 0;
		}
		Sound& at(int index) {
			return list[index];
		}
		Sound& atName(string name) {
			for(int i = 0; i < size; ++i) {
				if(list[i].name == name) {
					return list[i];
				}
			}
		}
		bool empty() {
			return size == 0;
		}
		int getSize() {
			return size;
		}
		void print(const char* color) {
			for(int i = 0; i < size; ++i) {
				cout << color << BOLD << i << ": " << RESET << list[i].name << endl;
			}
		}
		void pull(int index) {
			for(int i = 0; i < size - 1; ++i) {
				if (i >= index) {
					list[i] = list[i + 1];
				}
			}
			--size;
		}
		void push() {
			if(size == sizeof(list)) {
				Sound* newList = new Sound[size + 8];
				for(int i = 0; i < size; ++i) {
					newList[i] = list[i];
				}
				delete[] list;
				list = newList;
			}
			Sound newSound;
			list[size] = newSound;
			++size;
		}
	private:
		Sound* list;
		int size;
};