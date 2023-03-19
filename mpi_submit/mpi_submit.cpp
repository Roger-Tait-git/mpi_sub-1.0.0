

//   File name: mpi_submit
//   Authors: Roger Tait (roger.tait@bcu.ac.uk)
//   Date created: 10/7/2018

//   Part of the mpi_sub-1.0.0 package
//   The Software is distributed 'as is' and is solely for non-commercial use

//   Developed at Faculty of Computing, Engineering and the Built Environment
//   Birmingham City Univerity
//   University House
//   Birmingham



#include <mpi.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>


#define WORKTAG 1
#define DIETAG 2


int commandCounter = 0;
const int MESSAGESIZE = 100000;
int verbose = 0;


void master(std::string pTaskFileName,std::string pMode);
void slave(void);
bool do_work( char message[],char title[],int rank,int size );




///////////////////////////////////////////////////////////////////////////
//
// main: Program entry point.
//
// Arguments:
// <argv[0]> task file name
// <argv[1]> mode of operation (serial/parallel/script)
//
// Return:
// Exit status of program
//
///////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

	std::string taskfilename = argv[1];
	std::string mode = argv[2];

	if (verbose)
	{
		std::ofstream logfile;
		logfile.open("mpi_sub_log.txt", std::ios::app );
		logfile << "\ntaskfilename: " << taskfilename;
		logfile << "\nmode: " << mode;
		logfile.close();
	}


	int myrank;
	
  	// Initialize MPI
  	MPI_Init(&argc, &argv);

  	// Find out my identity in the default communicator
  	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  	if (myrank == 0) 
	{
    		master(taskfilename,mode);
  	} 
	else 
	{
    		slave();
  	}

  	// Shut down MPI
  	MPI_Finalize();

  	return 0;
}


///////////////////////////////////////////////////////////////////////////
//
// master: Master process of master/slave model
//
// Arguments:
// <pTaskFileName> task file name
// <pMode> mode of operation (serial/parallel/script)
//
// Return:
// none
//
///////////////////////////////////////////////////////////////////////////
void master(std::string pTaskFileName,std::string pMode)
{
	int ntasks;
	int rank;
	bool work = true;
	int result;
	char message[MESSAGESIZE] = "";
	long int commandcomplete = 0;

  	MPI_Status status;
	std::vector<std::string> commands; 
	std::string line;

	// Print some messages to the output file
	time_t start_time, stop_time;
	time( &start_time );
	printf("Start-time(sec): %d\n",(int)start_time);  fflush(stdout);


  	// Find out how many processes there are in the default communicator
  	MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

	if (verbose)
	{
		std::ofstream logfile;
		logfile.open("mpi_sub_log.txt", std::ios::out );
		logfile << "\nntasks master: " << ntasks;
		logfile.close();
	}


	// Parallel mode is to be employed
	if( pMode == "Parallel" )
	{
		// Open the task file for processing
		std::ifstream inFile;
		inFile.open( pTaskFileName.c_str() );
	
		if(!inFile)
		{
			std::cout<<"Could not open file: "<<pTaskFileName<<std::endl;
			return;
		}

		// Process in Parallel
		std::cout<<"Working on task file: "<<pTaskFileName<<" in Parallel"<<std::endl;

		// Seed the slaves by sending one unit of work to each slave
		for (rank = 1; rank < ntasks; ++rank) 
		{

			// Get next item of work to do
			std::getline(inFile,line);
			std::memset( message,0,MESSAGESIZE);
			std::copy( line.begin(),line.end(), message);

			// Send it to each rank
			MPI_Send(&message,             
     			MESSAGESIZE,                 
     			MPI_CHAR,           
     			rank,              
     			WORKTAG,           
     			MPI_COMM_WORLD); 

			line.clear();
			commandcomplete++;

		}	

		// Loop over getting new work requests until there is no more work to be done
		line.clear();

		while ( std::getline(inFile,line) ) 
		{
			std::memset( message,0,MESSAGESIZE);
			std::copy( line.begin(),line.end(), message);

			// Receive results from a slave
			MPI_Recv(&result,           
     			1,                 
     			MPI_INT,        
     			MPI_ANY_SOURCE,    
     			MPI_ANY_TAG,       
     			MPI_COMM_WORLD,    
     			&status);          

			// Send the slave new work
			MPI_Send(&message,             
     			MESSAGESIZE,                 
     			MPI_CHAR,           
     			status.MPI_SOURCE, 
     			WORKTAG,           
     			MPI_COMM_WORLD);   


			line.clear();
			commandcomplete++;

		}

		// No more work so receive all outstanding results from the slaves
		for (rank = 1; rank < ntasks; ++rank) 
		{
			MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE,MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		}

		inFile.close();



	}
	// Serial mode is to be employed
	else if( pMode == "Serial" )
	{

		// Open the task file for processing
		std::ifstream inFile;
		inFile.open( pTaskFileName.c_str() );
	
		if(!inFile)
		{
			std::cout<<"Could not open file: "<<pTaskFileName<<std::endl;
			return;
		}

		// Process in Serial
		std::cout<<"Working on task file: "<<pTaskFileName<<" in Serial"<<std::endl;


		//THIS COULD BE TAKEN OUT OF THE WHILE LOOP!
		// Loop over getting new work requests until there is no more work to be done
		while ( std::getline(inFile,line) ) 
		{

			std::memset( message,0,MESSAGESIZE);
			std::copy( line.begin(),line.end(), message);

			bool ok = do_work( message,"manager",1,1 );

			line.clear();
			commandcomplete++;
		}

		inFile.close();

	}
	// Script mode is to be employed
	else if( pMode == "Script" ) 
	{

		// Process as script
		std::cout<<"Working on task file: "<<pTaskFileName<<" as Script"<<std::endl;

		std::memset( message,0,MESSAGESIZE);
		std::copy( pTaskFileName.begin(),pTaskFileName.end(), message);
		
		bool ok = do_work( message,"manager",1,1 );

		commandcomplete++;

	}
	// Undefined mode
	else
	{

		std::cout<<"!!!!!!!!Undefined mode!!!!!!!!"<<std::endl;
	}


  	// Tell all the slaves to exit by sending an empty message with the DIETAG
  	for (rank = 1; rank < ntasks; ++rank) 
	{
    		MPI_Send(0, 0, MPI_INT, rank, DIETAG, MPI_COMM_WORLD);
  	}

	message[0]='\0';


	// Print some messages to the output file
	time( &stop_time );
	printf("Start-time(sec): %d stop-time(sec): %d run-time(sec): %d\n",(int)start_time,(int)stop_time,(int)stop_time-(int)start_time); 
	printf("No masters: %d no slaves: %d\n",1,ntasks-1);
	printf("Commands completed: %ld\n",commandcomplete);

}


///////////////////////////////////////////////////////////////////////////
//
// slave: Slave process of master/slave model just listen for work
//
// Arguments:
// none
//
// Return:
// none
//
///////////////////////////////////////////////////////////////////////////
void slave(void)
{

	int result=1;
	int size;
	int rank;
	char recMessage[MESSAGESIZE] = "";

  	MPI_Status status;

  	while (1) 
	{

    		// Receive a message from the master
    		MPI_Recv(&recMessage, MESSAGESIZE, MPI_CHAR, 0, MPI_ANY_TAG,MPI_COMM_WORLD, &status);

		MPI_Comm_size( MPI_COMM_WORLD, &size );
		MPI_Comm_rank( MPI_COMM_WORLD, &rank );
		
    		// Check if the tag of the received message is DIETAG
    		if (status.MPI_TAG == DIETAG) 
		{
      			return;
    		}

		if (verbose)
		{
			std::ofstream logfile;
			logfile.open("mpi_sub_log.txt", std::ios::out );
			logfile << "\nSlave process " << rank << " of " << size << " recieved: " << recMessage;
			logfile.close();
		}

		// Do some work
		bool ok = do_work( recMessage,"slave",rank,size );

    		// Send the result back to the master
    		MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

  	}
}



///////////////////////////////////////////////////////////////////////////
//
// do_work: Actually do something rather than managing messages
//
// Arguments:
// <message> the task which needs to be executed
// <title> who master/slave sent the task
// <rank> the id of the sender
// <size> the size of the world
//
// Return:
// <bool> was the forked pipe successfully created?
//
///////////////////////////////////////////////////////////////////////////
bool do_work(char message[],char title[],int rank,int size)
{

	char cmd[MESSAGESIZE];
	char result[MESSAGESIZE];
	int pos = sprintf(cmd,"%s",message);
	cmd[pos]='\0';


	if (verbose)
	{
		std::ofstream logfile;
		logfile.open("mpi_sub_log.txt", std::ios::out );
		
		time_t call_time;
		time( &call_time );

		logfile << (int)call_time << " " << title << " id " << rank << " of " << size << " doing " << cmd;
		logfile.close();
	}


	// Fork a pipe which executes the cmd string as a process
	FILE *ptr;
	if ((ptr = popen(cmd, "r")) != NULL)
	{
		
		while (fgets(result,MESSAGESIZE, ptr) != NULL)
		{
                 	printf("%s\n",result); fflush(stdout);    
		}
		(void) pclose(ptr);

		return true;
	}
	else
	{
		return false;
	}

	return false;
}
