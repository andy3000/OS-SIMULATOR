#include <iostream>

using namespace std;

// an operation contains the following elements
struct Operation {
	
	char type;	// the type of operation it is, can be P for process I for input O for output
	string instruction;	// instruction type can be run, keyboard, monitor, hard drive etc.
	int cycleTime;	// amount of time it takes to complete
};

// a job represents "A" from start to end in the meta data
// every process, input or output operations that happen from 
// the beginning to the end make up the job, once all operations 
// have been completed the job is completed & taked off the ready queue
class Job {
	
	public:
	
		// constructor
		Job() {
			// intialiaze 
			timeToProcessJob = 0;
			currentOperation = 0;
		}
		
		// returns if the job is finished
		bool jobFinished() {
			
			return ( currentOperation == numberOfOperations );
		}
		
		// returns if the current operation is finished
		bool operationFinished() {
			
			return ( processes[ currentOperation ].cycleTime == 0 );
		}
	
		int PID;	// PID of the job
		
		int timeToProcessJob;	// time it takes to process the job,
								// this is used to sort for shortest job first
								
		int numberOfOperations; // number of operations that make up this job
								// all operations must be complete for job to be done
								
		int currentOperation;	// the current operation in action can be process or I/O
		
		Operation processes[ 256 ];	// all operations that make up the job 
};
