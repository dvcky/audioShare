#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "lib/color.h"
#include "lib/Sound.h"
#include "lib/sys.h"
using namespace std;

string nullModule = "NULL";
string combModule = "NULL";

void handler(int num) {
	if(nullModule != "NULL") {
		exec("pactl unload-module " + nullModule);
		cout << "Unloaded null\n";
	}
	if(combModule != "NULL") {
		exec("pactl unload-module " + combModule);
		cout << "Unloaded comb\n";
	}
	cout << endl << YELLOW << "Exitting..." << RESET << endl;
	exit(-1);
}

int main() {
	
	signal(SIGINT, handler);

	// Fill lists with respective Sound types, then check to make sure they aren't empty.
	SoundList sinkInputs = parse(exec("pactl list sink-inputs"));
	if(sinkInputs.empty()) {
		error("No applications playing audio!");
		return 0;
	}
	SoundList sources = parse(exec("pactl list sources"));
	if(sources.empty()) {
		error("No input devices detected!");
		return 1;
	}
	SoundList sourceOutputs = parse(exec("pactl list source-outputs"));
	if(sourceOutputs.empty()) {
		error("No applications recording audio!");
		return 2;
	}

	Sound input1 = prompt(0, sinkInputs);
	Sound input2 = prompt(1, sources);
	Sound output = prompt(2, sourceOutputs);

	cout << BOLD << "--------" << endl << RESET << MAGENTA;

	cout << "Creating null sink...........................";
	nullModule = exec("pactl load-module module-null-sink sink_name=\"aLnull\" sink_properties=device.description=\"aL[App+Mic]\"");
	nullModule = nullModule.erase(nullModule.size() - 1);
	Sound nullSink = parse(exec("pactl list sinks")).atName("aLnull");
	cout << "done!\n";

	cout << "Creating combined sink.......................";
	combModule = exec("pactl load-module module-combine-sink sink_name=\"aLcomb\" slaves=\"aLnull,@DEFAULT_SINK@\" sink_properties=device.description=\"aL[App]\"");
	combModule = combModule.erase(combModule.size() - 1);
	Sound combSink = parse(exec("pactl list sinks")).atName("aLcomb");
	cout << "done!\n";

	cout << "Sending application audio to combined sink...";
	exec("pactl move-sink-input " + to_string(input1.ID) + " " + to_string(combSink.ID));
	cout << "done!\n";

	cout << "Sending microphone audio to null sink........";
	exec("pactl load-module module-loopback source=" + to_string(input2.ID) + " sink=" + to_string(nullSink.ID) + " sink_dont_move=true source_dont_move=true");
	cout << "done!\n";

	cout << "Sending null sink to output application......";
	exec("pactl move-source-output " + to_string(output.ID) + " aLnull.monitor");
	cout << "done!\n";
	while(true) {
		sleep(128);
	}

}