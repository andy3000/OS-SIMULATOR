#include "Job.cpp"
#include <queue>

// configure object
struct Configure {
			
		int version;
		int quantom;
		int processorCycleTime;
		int monitorDisplayTime;
		int hardDriveCycleTime;
		int printerCycleTime;
		int keyboardCycleTime;
		
		string processorScheduling;
		string file;
		string memoryType;
		string logType;
};

class Scheduler {
	
	public: 
		
		// constructor 
		Scheduler() { 
			
			nonPreemptiveFinished = false;
			preemptiveFinished = false;
			nonPreemptiveIO = false;
		}
		
		Configure config;
		
		bool readyQueueEmpty();
		bool waitQueueEmpty();
		
		void moveJobToReadyQueue();
		void moveJobToWaitQueue();
		
		// round robin algorithm to deal with ready queue
		void RoundRobin();
		
		// FIFO or SJF algorithm to deal with ready queue
		void nonPreemptive();
		
		// calculates the time it took to process an operation
		int calculateTimeToProcessOperation( Job, int );
		
		bool moveToNextOperationOfJob( Job & );
		bool checkForIO( Job ); // checks to see if operation is an I or O
		bool runCPU( Job & ); // runs cpu on the operation
		bool finished();
		bool nonPreemptiveFinished;
		bool preemptiveFinished;
		bool nonPreemptiveIO;
		
		void ioManagement(); // algorithm for FIFO or SJF wait queue 
		void ioPreemptive(); // algorithm for round robin wait queue
		void ioPrint( Job ); // prints for IO operations
		
		Job currentJob; // current job of ready queue
		Job jobThreading; // job that is threading currently 
		
		Job *readyQueueForNonPreemptive; // array used to hold ready queue for FIFO or SJF
		
		int numberOfJobs;
		int currentJobIndex; // knows which job is currently processing, used for FIFO or SJF
		int timeQuantom;
		
		queue<Job> readyQueue; // used for round robin
		queue<Job> waitQueue; // used for round robin
};

// if the jobs operation is not finished then it can decrement the cycle
// time by one, other wise it returns false because the operation is complete
bool Scheduler::runCPU( Job &job ) {
	
	if( job.processes[ job.currentOperation ].cycleTime != 0 ) {

		job.processes[ job.currentOperation ].cycleTime--;
		return true;
	}

	return false;
}

bool Scheduler::readyQueueEmpty() {
	
	return( readyQueue.empty() );
}

bool Scheduler::waitQueueEmpty() {
	
	return( waitQueue.empty() );
}

// use these wisely as they only move the "currentJob" object to the 
// ready or wait queue, has no effect on "jobThreading" object
void Scheduler::moveJobToReadyQueue() {
	
	readyQueue.push( currentJob );
}

void Scheduler::moveJobToWaitQueue() {
	
	waitQueue.push( currentJob );
}

bool Scheduler::finished() {
	
	return ( nonPreemptiveFinished || preemptiveFinished );
}

// if the job is not finished then move to the next operation to be 
// processed, if you can't move to the next job that means that the 
// job is done since there are no more operations it needs to process
bool Scheduler::moveToNextOperationOfJob( Job &job ) {
	
	if( !job.jobFinished() ) {
		
		job.currentOperation++;
		return true;
	}
	
	return false;
}

// checks for I or O 
bool Scheduler::checkForIO( Job job ) {

	if( job.processes[ job.currentOperation ].type == 'I' || 
		job.processes[ job.currentOperation ].type == 'O' ) {
		
		return true;
	}
	
	return false;
}

// used to print for io operation, make sure it does console, file or both
void Scheduler::ioPrint( Job job ) {
	
	cout << "  - ";
	
	if( job.processes[ job.currentOperation ].type == 'I' ) {
		
		cout << "Input, ";
	}
	else {
		
		cout << "Output, ";
	}
	
	cout << job.processes[ job.currentOperation ].instruction;
}

// FIFO or SJF io management
void Scheduler::ioManagement() {
	
	// print to console, file or both
	cout << "PID " << currentJob.PID;
	ioPrint( currentJob ); 
	cout << " started" << endl;
		 
	cout << "PID " << currentJob.PID;
	ioPrint( currentJob ); 
	cout << " completed ( " 
		 << calculateTimeToProcessOperation( currentJob, currentJob.processes[ currentJob.currentOperation ].cycleTime ) 
		 << " mSec )" << endl;     
	
	// run down the cycle time to 0
	while( runCPU( currentJob ) );
	
	// if you can move to next operation then do it, if this fails
	// that means the job is done & can be removed from the ready queue
	if( moveToNextOperationOfJob( currentJob ) ) {
		
		// if the job still has operations needed to be processed then
		// we come in here, if the next operation is also IO then manage
		// IO by creating another thread & process job
		if( checkForIO( currentJob ) ) {
			
			cout << "SYSTEM - Managing I/O ( TIME )" << endl;
			nonPreemptiveIO = true;
		}
		// other wise no need to create a thread we can process ready queue
		else {
			
			nonPreemptiveIO = false;
		}
	}
	// else the job is finished & can go to the next job to process
	else {
			
		// print to console, file or both
		cout << "PID " << currentJob.PID << "  - Exit System" << endl;
		cout << "SYSTEM - Ending Process ( TIME )" << endl;
		
		// go to next job to process
		currentJobIndex++;
		
		// if that was the last job then shut down the program
		if( currentJobIndex == numberOfJobs ) {
		
			cout << "SYSTEM - Shutdown Management" << endl;
			nonPreemptiveFinished = true;
		}
		else{
			
			// other wise process the next job on ready queue
			currentJob = readyQueueForNonPreemptive[ currentJobIndex ];
		}
	}	
}

void Scheduler::ioPreemptive() {
	
	// get the front of the ready queue
	jobThreading = waitQueue.front();
	
	// print to console, file or both
	cout << "PID " << jobThreading.PID;
	ioPrint( jobThreading ); 
	cout << " started" << endl;
	
	cout << "PID " << jobThreading.PID;
	ioPrint( jobThreading ); 
	cout << " completed ( " 
		 << calculateTimeToProcessOperation( jobThreading, jobThreading.processes[ jobThreading.currentOperation ].cycleTime ) 
		 << " mSec )" << endl;
	
	// run the cpu cycle time down to 0	 
	while( runCPU( jobThreading ) );

	// if the job that is threading has next operation then move to it
	// if this fails then remove from wait queue
	if( moveToNextOperationOfJob( jobThreading ) ) {
		
		// if the job still has operations it needs to complete then check for
		// IO, if the next operation is IO then put it wait on the wait queue
		if( checkForIO( jobThreading ) ) {
			
			cout << "SYSTEM - Managing I/O ( TIME )" << endl;
			waitQueue.pop();
			waitQueue.push( jobThreading );
		}
		else {
			
			// otherwise pop it off the wait queue & send it back to the ready queue
			waitQueue.pop();
			readyQueue.push( jobThreading );
		}
	}
	// otherwise the job is finished 
	else {
	
		// print to console, file or both
		cout << "PID " << currentJob.PID << "  - Exit System" << endl;
		cout << "SYSTEM - Ending Process ( TIME )" << endl;
		
		// take it off the wait queue
		waitQueue.pop();
		
		// if both the ready queue & wait queue are empty then shutdown
		if( readyQueue.empty() && waitQueue.empty() ) {
			
			cout << "SYSTEM - Shutdown Management" << endl;
			preemptiveFinished = true;
		}
	}
}

// calculate the time it takes to process an operation
int Scheduler::calculateTimeToProcessOperation( Job job, int cycleTime ) {
	
	if( job.processes[ job.currentOperation ].instruction == "run" ) {
					
		return ( cycleTime * config.processorCycleTime );
	}
	if( job.processes[ job.currentOperation ].instruction == "hard drive" ) {
		
		return ( cycleTime * config.hardDriveCycleTime );
	}
	if( job.processes[ job.currentOperation ].instruction == "keyboard" ) {
		
		return ( cycleTime * config.keyboardCycleTime );
	}
	if( job.processes[ job.currentOperation ].instruction == "monitor" ) {
		
		return ( cycleTime * config.monitorDisplayTime );
	}
	if( job.processes[ job.currentOperation ].instruction == "printer" ) {
		
		return ( cycleTime * config.printerCycleTime );
	}	
	
	return -1;
}

// FIFO or SJF ready queue algorithm, both use the same algorithm
// the SJF is already sorted so we can just process all the jobs
void Scheduler::nonPreemptive() {
	
	// print to console, file or both 
	cout << "PID " << currentJob.PID << "  - Processing " 
		 << currentJob.processes[ currentJob.currentOperation ].instruction 
		 << " ( " << calculateTimeToProcessOperation( currentJob, currentJob.processes[ currentJob.currentOperation ].cycleTime ) 
		 << " mSec )" << endl;
	
	// process the operation
	while( runCPU( currentJob ) );

	// if you can move to the next operation then do it, otherwise the
	// job is finished & the next job should be processed
	if( moveToNextOperationOfJob( currentJob ) ) {
		
		// check for IO, if the operation is IO then it will start a thread
		if( checkForIO( currentJob ) ) {
			
			cout << "SYSTEM - Managing I/O ( TIME )" << endl;
			nonPreemptiveIO = true;
		}
	}
	// otherwise the job is done 
	else {
			
		// print to console, file or both
		cout << "PID " << currentJob.PID << "  - Exit System" << endl;
		cout << "SYSTEM - Ending Process ( TIME )" << endl;
		
		// move to the next job on the ready queue
		currentJobIndex++;
		
		// if all the jobs have been processed then shutdown
		if( currentJobIndex == numberOfJobs ) {
		
			cout << "SYSTEM - Shutdown Management ( TIME )" << endl;
			nonPreemptiveFinished = true;
		}
		else{
			
			// other wise start processing the next job
			currentJob = readyQueueForNonPreemptive[ currentJobIndex ];
		}
	}	
}

// round robin scheduling algorithm
void Scheduler::RoundRobin() {
	
	// get time quantom
	timeQuantom = config.quantom;
	int time = 0; // this is how much time has "passed"
	
	if( !currentJob.jobFinished() ) {
		
		// run the operation one time cycle & increment the time
		runCPU( currentJob );
		time++;
		
		// if the current jobs operation isn't finished & the time 
		// quantom isn't reached then keep processing.. this accounts for
		// the scenario when you have a time quantom of say three but your
		// operation only have two cycles left.. it will process it as much 
		// as it needs
		while( !currentJob.operationFinished() && ( time < timeQuantom ) ) {
			
			runCPU( currentJob ); 
			time++;
		}
		
		// print to console, file or both.. here the "time" variable is 
		// sent into the calculateTime function because it only prints
		// how many every cycle we got through, this could be as much
		// as the time quantom or less depending on how many cycles 
		// were left on the jobs operations
		cout << "PID " << currentJob.PID << "  - Processing " 
	         << currentJob.processes[ currentJob.currentOperation ].instruction 
	         << " ( " << calculateTimeToProcessOperation( currentJob, time ) << " mSec )" << endl;		
	
		// if jobs operation is finished then move to next operation,
		// other wise the jobs operation still needs processing
		if( currentJob.operationFinished() ) {
			
			// if the job still has more operations then move to the next one
			if( moveToNextOperationOfJob( currentJob ) ) {
				
				// if the operation is IO then take the job off the
				// ready queue & put it on the wait queue
				if( checkForIO( currentJob ) ) {
					
					cout << "SYSTEM - Managing I/O ( TIME )" << endl;
					readyQueue.pop();
					waitQueue.push( currentJob );
					
					// if the ready queue isn't empty then get the next job
					// on the ready queue & make it the currentJob
					if( !readyQueueEmpty() ) {
					
						cout << "SYSTEM - Swapping Processes ( TIME )" << endl;
						currentJob = readyQueue.front();
					}
				}
				// other wise the next operation is not IO so place it
				// back onto the ready queue & process the job at the
				// front of the ready queue
				else {
					
					cout << "SYSTEM - Swapping Processes ( TIME )" << endl;
					readyQueue.pop();
					moveJobToReadyQueue();
					currentJob = readyQueue.front();
				}
			}
		}
		// other wise the jobs operation is not finsihed & still needs
		// to be processed, so place it back onto the ready queue & 
		// process the job at the front of the ready queue
		else {
			
			cout << "SYSTEM - Swapping Processes ( TIME )" << endl;
			readyQueue.pop();
			moveJobToReadyQueue();
			currentJob = readyQueue.front();
		}
	}
	// other wise the current job is finsished 
	else {
		
		// print to console, file or both
		cout << "PID " << currentJob.PID << "  - Exit System" << endl;
		cout << "SYSTEM - Ending Process ( TIME )" << endl;
		
		// take the current job off the ready queue
		readyQueue.pop();
		
		// if the ready queue & the wait queue is finsihed then we done
		if( readyQueue.empty() && waitQueue.empty() ) {
			
			cout << "SYSTEM - Shutdown Management" << endl;
			preemptiveFinished = true;
		}
		// otherwise if the ready queue is not empty, then process the
		// the front of the ready queue
		else {
			
			if( !readyQueueEmpty() ) {
					
				cout << "SYSTEM - Swapping Processes ( TIME )" << endl;
				currentJob = readyQueue.front();
			}
		}
	}	
}

