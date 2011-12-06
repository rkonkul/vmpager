/*
 * vmpager.cpp
 * Ryan Konkul
 * rkonku2
 * CS 385
 * Homework 4: vmpager
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

const bool LEAST = false;

class Frame {
public:
	int pid;
    unsigned char page_number;
    int number_times_used;
    Frame(int pid_, unsigned char page_number_);
};
Frame::Frame(int pid_, unsigned char page_number_) {
	pid = pid_;
    page_number = page_number_;
    number_times_used = 0;
}

class PTableEntry {
public:
    bool valid;
    bool reference;
    bool dirty;
    int num_valid;
    PTableEntry();
};
PTableEntry::PTableEntry() {
    valid = false; reference = false; dirty = false;
    num_valid = 0;
}

class Process {
public:
    unsigned short pid;
    unsigned short num_reads;
    vector<unsigned char> mem_accesses;
    vector<PTableEntry> process_table;

    Process(unsigned short pid_,
    unsigned short num_reads_,
    vector<unsigned char> mem_accesses_) {
        pid = pid_;
        num_reads = num_reads_;
        mem_accesses = mem_accesses_;
        for(int i=0; i<256; ++i) {
            PTableEntry p;
            process_table.push_back(p);
        }
    }
};

class FrameTable {
public:
	list<Frame> frame_table;
    int max_size;
	FrameTable(int size);
    bool find(int pid, unsigned char mem_access);
    int size() {
        return frame_table.size();
    }
    Frame front() {
        return frame_table.front();
    }
    void pop_front() {
        frame_table.pop_front();
    }
    void push_back(Frame f) {
        frame_table.push_back(f);
    }
    list<Frame>::iterator fifo() {
    	return frame_table.begin();
    }
    //picks a victim frame by analyzing all processes memory accesses and checking
    //what pid,page will ever be used the least
    list<Frame>::iterator least_used(vector<Process> &processes);
};
FrameTable::FrameTable(int size) {
	max_size = size;
}
bool FrameTable::find(int pid, unsigned char mem_access) {
    for(list<Frame>::iterator f = frame_table.begin(); f != frame_table.end(); f++) {
        if(f->pid == pid && f->page_number == mem_access) {
            return true;
        }
    }
    return false;
}

list<Frame>::iterator FrameTable::least_used(vector<Process> &processes) {
	//check all frames
	for(list<Frame>::iterator it = frame_table.begin(); it != frame_table.end(); ++it) {
		int pid = it->pid;
		int page = it->page_number;
		int number_of_access = 0;
		//count number of times frame is used ever in any processes accesses
		for(unsigned int i=0; i<processes.size(); ++i) {
			//if process matches pid of frame
			if((int)processes[i].pid == (int)pid) {
				//count the number of access the process makes
				for(unsigned int j=0; j<processes[i].mem_accesses.size(); ++j) {
					if((int)processes[i].mem_accesses[j] == (int)page) {
						number_of_access++;
					}
				}
			}
		}
		//save number of accesses the process does for the frame
		it->number_times_used = number_of_access;
	}
	//locate lowest number of accesses
	int lowest_num_access = frame_table.begin()->number_times_used;
	for(list<Frame>::iterator it = frame_table.begin(); it != frame_table.end(); ++it) {
		if(lowest_num_access > it->number_times_used) {
			lowest_num_access = it->number_times_used;
		}
	}
	for(list<Frame>::iterator it = frame_table.begin(); it != frame_table.end(); ++it) {
		if(lowest_num_access == it->number_times_used) {
			return it;
		}
	}
	cout << "Error in good()" << endl;
	return frame_table.begin();
}


void read_processes(FILE* file, int num_processes, int maxReads, vector<Process>& processes) {
    //loop until end of file or max processes is reached
    int i=0;
    bool complete = true;
    while(i < num_processes && complete) {
        complete = true;
        ++i;
        unsigned short pid;
        unsigned short num_reads;
        unsigned char mem_access;
        vector<unsigned char> mem_accesses;
        //read in pid
        if(fread(&pid, sizeof(unsigned short), 1, file) != 1) {
            complete = false;
        }
        pid = pid % num_processes;
        //read in number of reads
        if(fread(&num_reads, sizeof(unsigned short), 1, file) != 1) {
            complete = false;
        }
        num_reads = num_reads % maxReads;
        //read in memory accesses
        for(unsigned short j=0; j<num_reads; ++j) {
            if(fread(&mem_access, sizeof(unsigned char), 1, file) != 1) {
                //allow the mem reads to go through
                //so don't complete=false;
                break;
            }
            mem_accesses.push_back(mem_access);
        }
        //if didn't hit end of file, create a process
        if(complete) {
            processes.push_back(Process(pid, num_reads, mem_accesses));
        }
        else {
            cout << "\nEnd of file occurred.";
        }
    }
}

void print_processes(vector<Process> &processes) {
    for(unsigned int f=0; f<processes.size(); ++f) {
        cout << "\n\npid: " << processes[f].pid;
        cout <<" - " << processes[f].mem_accesses.size() << " mem_accesses\n";
        for(unsigned int k=0; k<processes[f].mem_accesses.size(); k++) {
            if(k%18==17) cout << "\n";
            cout << (int)processes[f].mem_accesses[k] << " ";
        }
    }
}

int main( int argc, char ** argv) {
	cout << "Ryan Konkul\nrkonku2\nCS 385\nHomework 4: vmpager\n";
	int num_processes = 64;
	int MB_RAM = 16;
	int maxReads = 512;
	int fractionWrite = 2;

    vector<Process> processes;

	if(argc < 2) {
		cout << "Must enter a file as argument" << endl;
		return -1;
	}
	char* infile = argv[1];
	FILE* file;
	file = fopen(infile, "r");
    if(file == NULL) {
      cout << "Could not open file: " << infile << endl;
      return -2;
    }
    cout << "\nReading from file: " << infile << endl;
    //check arguments and initialize them
 	if(argc > 2) num_processes = atoi(argv[2]);
	if(argc > 3) MB_RAM = atoi(argv[3]);
	if(argc > 4) maxReads = atoi(argv[4]);
	if(argc > 5) fractionWrite = atoi(argv[5]);
	cout << "\nnum_processes " << num_processes << "\nmbram " << MB_RAM <<
    "\nmaxReads " << maxReads << "\nfractionWrite " << fractionWrite << endl;
    //find size of file
	long file_size;
	fseek (file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	//only offset if filesize is large enough
	if(file_size > 5000) {
		fseek(file, 5000, SEEK_SET);
	}
    //read in processes from file
    read_processes(file, num_processes, maxReads, processes);
    print_processes(processes);

    FrameTable f_table(MB_RAM*4);
    //if infinite memory
    if(MB_RAM == 0) {
        cout << "\nInfinite memory\n";
        f_table.max_size = 256*num_processes;
    }

    int page_hits = 0;
    int page_misses = 0;
    bool write = false;
    int clock = 0;
    for(unsigned int i=0; i<processes.size(); ++i) {
        for(unsigned int j=0; j<processes[i].mem_accesses.size(); ++j) {
            int pid = processes[i].pid;
            unsigned char mem_access = processes[i].mem_accesses[j];
            //if access already in frame table
            if(f_table.find(pid, mem_access)) {
                page_hits++;
                clock += 1;
            }
            //else not in frame table
            else {
                //if frame is max size, need to kick one out
                if(f_table.max_size <= f_table.size()) {
                    //if fifo, take first frame placed in FrameTable
                	list<Frame>::iterator del = f_table.fifo();
                	if(LEAST) {
                		del = f_table.least_used(processes);
                	}
                   int pid_to_remove = del->pid;
               	   int mem_access_to_remove = del->page_number;
                   unsigned int k = 0;
                   //find process with given pid
                   while(k < processes.size()) {
                       if(processes[k].pid == pid_to_remove) {
                           break;
                       }
                       k++;
                   }
                   //if dirty and need to write out to disk
                   if(processes[k].process_table[mem_access_to_remove].dirty == true) {
                       clock += 40000;
                       //not dirty anymore
                       processes[k].process_table[mem_access_to_remove].dirty = false;
                   }
                   //removed from frame table so no longer valid
                   processes[k].process_table[mem_access_to_remove].valid = false;
                   f_table.frame_table.erase(del);
                }
                //add new mem access
                f_table.push_back(Frame(pid, mem_access));
                clock += 40001;
                page_misses++;
            }
            processes[i].process_table[(int)mem_access].valid = true;
            processes[i].process_table[(int)mem_access].reference = true;
            processes[i].process_table[(int)mem_access].dirty = false;
            //if not first access and memaccess % fwrite is zero then
            //next page is a write
            if(write) {
                processes[i].process_table[(int)mem_access].dirty = true;
                write = false;
            }
            if(((int) mem_access) % fractionWrite == 0) {
                write = true;
            }
        }
    }
    int num_access = 0;
    for(unsigned int i=0; i<processes.size(); ++i) {
        num_access += processes[i].mem_accesses.size();
    }
    cout << "\n\nFinal frame table: " << f_table.size() << "\n";
    cout << "(pid, page)\n";

    list<Frame> f = f_table.frame_table;
    int i=0;
    for(list<Frame>::iterator it = f.begin(); it != f.end(); ++it) {
        cout << "(" << it->pid << ": " << (int)it->page_number << ") ";
        if(i%8 == 7) cout << "\n";
        ++i;
    }

    cout << "\n\nTotal number of reads: " << num_access;
    cout << "\nNumber of page misses: " << page_misses;
    cout << "\nNumber of page hits  : " << page_hits;
    cout << "\nClock                : " << clock << endl;
    int sdf; cin >> sdf;
	return 0;
}
