#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <list>
#include <time.h>
using namespace std;
#include "datastruct.h"

extern settings s;
list<string> pqueue;
list<string>::iterator it;

bool legacy_rds_init()
{
	system("rm -f /home/pi/rds_ctl");
	system("/usr/bin/mkfifo /home/pi/rds_ctl");
	system("chmod 777 /home/pi/rds_ctl");
	return true;
}

/** 
 * launches pulseaudio, setup bluetooth for connections ecc...
 * this function needs to be launched only once.
 */

bool init()
{
	system("/bin/su pi -c \"pulseaudio -D\"");
	system("hciconfig hci0 piscan");
	//legacy_rds_init();
	return true;
}

/**
 * creates a file list each time is launched 
 * and saves the elements in pqueue (play queue)
 */

void get_list()
{
	FILE *fp;
	const int line_size=200;
	char line[line_size];
	string result;


	string s0="find ";
	string s1=s.storage; 
	string s2="-iname *.";
	string s3=s.format;
	string cmd=s0+" "+s1+" "+s2+s3;
        fp = popen(cmd.c_str(), "r");

	while (fgets(line, line_size, fp))
		pqueue.push_back(line);

	pclose(fp);
}

int play_storage()
{
	while(true){
		get_list();		/**< generate a file list */
		int next;
		int qsize=pqueue.size();
		srand (time(NULL));
	
		init();
		legacy_rds_init();
	
		string sox="sox -t mp3 -v 2 -r 48000 -G";
		string sox_params="-t wav -";
		string pifm1="/home/pi/PiFmRds/src/pi_fm_rds -ctl /home/pi/rds_ctl -ps";
		string pifm2="-rt";
		string pifm3="-audio - -freq";
		string songpath;
	
		while(qsize > 0)
		{
			next=rand() % qsize;		/**< extract a random file from the list */
			it=pqueue.begin();
			advance (it,next);
			songpath=*it;
			songpath.erase(songpath.size()-1);
			pqueue.erase(it);
			qsize--;
	
			size_t found = songpath.find_last_of("/");	/**< extract song name out of the absolute file path */
	  		string songname=songpath.substr(found+1);
	
			string cmdline=sox+" "+songpath+" "+sox_params+" | "+\
				pifm1+" "+songname+" "+pifm2+" "+songname+" "+pifm3+" "+s.freq;
	
			system(cmdline.c_str());
		}
	}
	return 0;
}

int play_bt(string device)
{
	
	string s0="/bin/su pi -c \"parec -d";
	string s1="sox -t raw -v 2.5 -G -b 16 -e signed -c 2 -r 44100 - -t wav - | sudo /home/pi/PiFmRds/src/pi_fm_rds -ps 'BLUETOOTH' -rt 'A2DP BLUETOOTH' -freq";
	string s2="-audio -\"";
	string cmdline=s0+" "+device+" | "+s1+" "+s.freq+" "+s2;

	cout<<cmdline<<endl;
	system(cmdline.c_str());


	return 0;
}

