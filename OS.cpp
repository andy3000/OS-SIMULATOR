#include "Scheduler.cpp"
#include <iostream>
#include <string>
#include <fstream>
#include <queue>

using namespace std;

class OS {
	
	public:
	
		// constructor 
		OS( string );
		
		// configure & scheudler objects
		Configure systemConfig;
		Scheduler scheduler;
		
		// setup / boot function
		void configureSystem( string );
		void loadMetaData( queue<Job> & );
		
		void print(); // do not need this function, just used to see 
					  // if jobs are in the correct order preprocessing
			
		// used to figure out how long a job takes to complete 		  
		void calculateTimeToProcessJob( Job &, Operation );
		
		// sorts the ready queue for shortest job first
		void sortForSJF();
		
		// simulates FIFO, SJF or RR
		void simulate();
};

OS::OS( string filename ) {
		
	// configure system by reading in filename, this should be the args command line parameter
	// configure should return bool if file open correctly, if file did not open stop program
	// here and report the error
	configureSystem( filename );
	
	// else the file opened correctly, so read in the metadata
	loadMetaData( scheduler.readyQueue );
	
	// transfer config file to scheduler
	scheduler.config = systemConfig;
	
	// if shortest job first, then short the jobs
	if( systemConfig.processorScheduling == "SJF" ) {
		
		sortForSJF();
	}
	
	// start simulation
	simulate();
	
	// print to file, console or both
	cout << "SYSTEM - Boot, Setup ( TIME )" << endl;
}

// this function could probably return a bool if the file opened correctly
void OS::configureSystem( string filename ) {
	
	ifstream fin;
	
	fin.clear();
	fin.open( filename.c_str() );
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.version;
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.file;
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.quantom;
	
	fin.ignore( 512, ':' );
	fin.get();
	getline( fin, systemConfig.processorScheduling, '\n' );
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.processorCycleTime;
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.monitorDisplayTime;
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.hardDriveCycleTime;
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.printerCycleTime;
	
	fin.ignore( 512, ':' );
	fin >> systemConfig.keyboardCycleTime;
	
	fin.ignore( 512, ':' );
	fin.get();
	getline( fin, systemConfig.memoryType, '\n' );
	
	fin.ignore( 512, ':' );
	fin.get();
	getline( fin, systemConfig.logType, '\n' );
	
	fin.close();
}

void OS::loadMetaData( queue<Job> &data ) {
	
	ifstream fin;
	char dummy;
	int pidIndex = 0;
	Operation tempOp;
	
	fin.clear();
	fin.open( systemConfig.file.c_str() );
	
	// ready in the beginning of the metadata file
	fin >> tempOp.type;
	
	// ingore the starting, we assume that the metadata should start
	// with an S to signify the starting.. might want to check that all
	// metadata includes the system start S at the beginning
	fin.ignore( 256, ';' );
	
	// read in the first application
	fin >> tempOp.type;
	fin >> dummy;
	getline( fin, tempOp.instruction, ')' );
	fin >> tempOp.cycleTime;
	fin >> dummy;
	
	// while there is still data in the file
	while( fin.good() ) {

		// if an A was hit & the instruction is start then create a
		// new job & fill it with the operations it needs to complete
		if( tempOp.type == 'A' && tempOp.instruction == "start" ) {
			
			// get first operation
			fin >> tempOp.type;
			fin >> dummy;
			getline( fin, tempOp.instruction, ')' );
			fin >> tempOp.cycleTime;
			fin >> dummy;
			
			// create the job & give it a PID
			Job tempJob;
			tempJob.PID = ++pidIndex;
			
			// print to file, console or both
			cout << "PID " << tempJob.PID << "  - Enter System" << endl;
			cout << "SYSTEM - Creating PID " << tempJob.PID << " ( TIME )" << endl;
			
			// while the end of the applicaiton is not reached keep
			// reading in opertions & putting them into the job
			while( tempOp.type != 'A' ) {
				
				// increment the amount of time need to complete job
				// depending on how long the operation takes
				calculateTimeToProcessJob( tempJob, tempOp );
				
				// insert operation into the processes of the job	
				tempJob.processes[ tempJob.currentOperation ] = tempOp;
				
				// increment the current operation, if another operation
				// exists then it will be placed in this index in the processes
				tempJob.currentOperation++;
				
				// get the next operation, this could be another operation
				// or the end of the job signified by an A
				fin >> tempOp.type;
				fin >> dummy;
				getline( fin, tempOp.instruction, ')' );
				fin >> tempOp.cycleTime;
				fin >> dummy;
			}
			
			// once all the operations have been placed into the job
			// record the number of operations this job has
			tempJob.numberOfOperations = tempJob.currentOperation;
			
			// reset the current operation to 0, this will allow you 
			// to start processing the operations from the first operation
			tempJob.currentOperation = 0;
			
			// push the job onto the ready queue.
			data.push( tempJob );
		}
		
		// this reads in the next process which can be the start of 
		// another application siginifed by an A or this could be 
		// an S which signifies the end of the metadata
		fin >> tempOp.type;
		fin >> dummy;
		getline( fin, tempOp.instruction, ')' );
		fin >> tempOp.cycleTime;
		fin >> dummy;
	}
	
	fin.close();
	
	// if the scheduling type is not round robin, that means it is 
	// FIFO or SJF.. so move the jobs over from a "QUEUE" to an "ARRAY"
	// this is done because it is way eaiser to process the jobs with
	// the array
	if( systemConfig.processorScheduling != "Round Robin" ) {
		
		// get the number jobs that will be processes
		scheduler.numberOfJobs = scheduler.readyQueue.size();
		
		// dynamically create an array the size of the number jobs
		// i guess a preallocated array could also be used here as well
		scheduler.readyQueueForNonPreemptive = new Job[ scheduler.numberOfJobs ];
		
		// move the jobs from the ready queue ( QUEUE ) into the ready 
		//queue for non preemptive scheduling ( ARRAY )
		for( int i = 0; i < scheduler.numberOfJobs; i++ ) {
			
			scheduler.readyQueueForNonPreemptive[ i ] = scheduler.readyQueue.front();
			scheduler.readyQueue.pop();
		}
	}
}

// this function can be removed you do not need this, however you can 
// use this before the simulation starts to make sure your ready queue
// for FIFO & SJF is in the correct order & check config file
void OS::print() {

	cout << "::CONFIG FILE::" << endl << endl;
	cout << "Version: " << systemConfig.version << endl;
	cout << "Filepath: " << systemConfig.file << endl;
	cout << "Quantum(cycles): " << systemConfig.quantom << endl;
	cout << "Processor Scheduling: " << systemConfig.processorScheduling << endl;
	cout << "Processor cycle time (msec): " << systemConfig.processorCycleTime << endl;
	cout << "Monitor display time (msec): " << systemConfig.monitorDisplayTime << endl;
	cout << "Hard drive cycle time (msec): " << systemConfig.hardDriveCycleTime << endl;
	cout << "Printer cycle time (msec): " << systemConfig.printerCycleTime << endl;
	cout << "Keyboard cycle time (msec): " << systemConfig.keyboardCycleTime << endl;
	cout << "Memory type: " << systemConfig.memoryType << endl;
	cout << "Log: " << systemConfig.logType << endl << endl;
	
	Operation tempOp;
	
	for( int i = 0; i < scheduler.numberOfJobs; i++ ) {
	
		cout << "JOB IN POSITION #" << i + 1 << endl;
		cout << "PID#" << scheduler.readyQueueForNonPreemptive[ i ].PID << endl;
		
		int count = scheduler.readyQueueForNonPreemptive[ i ].numberOfOperations;
		
		for( int j = 0; j < count; j++ ) {
			
			tempOp = scheduler.readyQueueForNonPreemptive[ i ].processes[ j ];
			
			cout << "PROCESS: " << tempOp.type << " INSTRUCTION: "
				 << tempOp.instruction << " CYCLE TIME: " << tempOp.cycleTime << endl;
		}
	
		cout << endl;
	}
}

// this function increments the running total of the time it takes to 
// complete a job, the time depends on the cycle time of the operation
// & the type of instruction that is to be excuted
void OS::calculateTimeToProcessJob( Job &tempJob, Operation tempOp ) {
	
	if( tempOp.instruction == "run" ) {
					
		tempJob.timeToProcessJob += ( tempOp.cycleTime * systemConfig.processorCycleTime );
	}
	if( tempOp.instruction == "hard drive" ) {
		
		tempJob.timeToProcessJob += ( tempOp.cycleTime * systemConfig.hardDriveCycleTime );
	}
	if( tempOp.instruction == "keyboard" ) {
		
		tempJob.timeToProcessJob += ( tempOp.cycleTime * systemConfig.keyboardCycleTime );
	}
	if( tempOp.instruction == "monitor" ) {
		
		tempJob.timeToProcessJob += ( tempOp.cycleTime * systemConfig.monitorDisplayTime );
	}
	if( tempOp.instruction == "printer" ) {
		
		tempJob.timeToProcessJob += ( tempOp.cycleTime * systemConfig.printerCycleTime );
	}	
}

// this function sorts the ready queue & places the shortest jobs accordingly
// the first job we receieve needs to be processed first no matter what,
// even if the first job is the longest.. with SJF scheduling the first 
// job is processed while any other jobs that may come after it are 
// sorted from shortest to longest 
void OS::sortForSJF() {
	
	Job tempJob;

	// sort for shortest job first excluding first job
	for( int i = 1; i <= scheduler.numberOfJobs; i++ ) {
		
		for( int j = 1; j < ( scheduler.numberOfJobs - 1 ); j++ ) {
			
			if( scheduler.readyQueueForNonPreemptive[ j + 1 ].timeToProcessJob < 
				scheduler.readyQueueForNonPreemptive[ j ].timeToProcessJob ) {

				tempJob = scheduler.readyQueueForNonPreemptive[ j ];
				scheduler.readyQueueForNonPreemptive[ j ] = scheduler.readyQueueForNonPreemptive[ j + 1 ];
				scheduler.readyQueueForNonPreemptive[ j + 1 ] = tempJob;
			}
		}
	}	
}

// this simulates the processing
void OS::simulate() {
	
	// this is some preprocessing stuff, you need to set the current
	// job so the scheduler knows which job to start processing first
	if( systemConfig.processorScheduling == "Round Robin" ) {
			
		scheduler.currentJob = scheduler.readyQueue.front();
	} 
	else
	{
		scheduler.currentJobIndex = 0;
		scheduler.currentJob = scheduler.readyQueueForNonPreemptive[ scheduler.currentJobIndex ];
	}

	// while the scheduler is not finished keep looping, while there are
	// jobs still in the ready queue or in the wait queue
	while( !scheduler.finished() ) {
		
		// if we running round robin go into this algorithm
		if( systemConfig.processorScheduling == "Round Robin" ) {
			
			// if the wait queue is not empty that means we need
			// to create a thread & run the algorithm below
			if( !scheduler.waitQueueEmpty() ) {
				
				scheduler.ioPreemptive();
			}
			
			// if the ready queue not empty then process the ready queue
			if( !scheduler.readyQueueEmpty() ) {
				
				scheduler.RoundRobin();
			}
		}
		else
		{
			// if there is some IO that needs to be performed then
			// create thread & run the algorithm below
			if( scheduler.nonPreemptiveIO ) {
			
				scheduler.ioManagement();
			}
			// otherwise there is not need for IO just process the next
			// operation that needs to be handled
			else {
			
				scheduler.nonPreemptive();
			}
		}
	}
}
