/**
 * @file   basicscheduler.cpp
 * @author Emmanuel Courcelle <manu@eoslogin1.bullx>
 * @date   Tue Sep 30 09:57:49 2014
 * 
 * @brief  
 * 
 * 
 */

#include <mpi.h>
#include <iostream>
//#include <iterator>
//#include <set>
using namespace std;

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <algorithm>
//#include <cstdlib>
#include <cerrno>

//#include "command.h"
#include "basicscheduler.hpp"
//#include "exception.h"
//#include <unistd.h>
//#include <errno.h>
//#include <libgen.h>
//#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

// The tags MPI defined by CHDB
#define CHDB_TAG_READY 1000
#define CHDB_TAG_GO    1010
#define CHDB_TAG_END   1020

void BasicScheduler::mainLoop() {
	if (isMaster()) {
		mainLoopMaster();
	} else {
		mainLoopSlave();
	}
}

/** 
 * @brief The main loop for the master
 * 
 */
void BasicScheduler::mainLoopMaster() {
	MPI_Status sts;
/*	
	{
		pid_t pid = getpid();
		ostringstream s_tmp;
		s_tmp << "gdbserver host:2345 --attach " << pid << "&";
		cerr << "launching gdbserver as:\n" << s_tmp.str() << "\n";
		system(s_tmp.str().c_str());
	}
*/
	// read the file names
	dir.readFiles();

	// check the number of blocks versus number of slaves and throw an exception if too many slaves
	size_t slaves_max = dir.getNbOfFiles()/prms.getBlockSize();
	if (dir.getNbOfFiles() % prms.getBlockSize() != 0) {
		slaves_max += 1;
	};
//	size_t slaves_max=100;
	if (slaves_max<getNbOfSlaves()) {
		ostringstream out;
		out << "ERROR - You should NOT use more than " << slaves_max << " slaves";
		throw(runtime_error(out.str()));
	}

	// Create the output directory
	dir.makeOutputDir();

	// loop over the file blocks
	// Listen to the slaves, the message is Status + File names
	size_t bfr_size=0;
	void* send_bfr=NULL;
	void* recv_bfr=NULL;

	allocBfr(send_bfr,bfr_size);
	allocBfr(recv_bfr,bfr_size);

	// Open the error file, if any
	ofstream err_file;
	if (!prms.isAbrtOnErr()) {
		string err_name = prms.getErrFile();
		err_file.open(err_name.c_str());
		if (!err_file.good()) {
			string msg = "ERROR - File could not be opened: ";
			msg += err_name;
			throw(runtime_error(msg));
		}
	}
	
	return_values.clear();
	file_pathes = dir.nextBlock();

	while(!file_pathes.empty()) {

		// Prepare the send buffer for the next message
		size_t send_msg_len=0;
		writeToSndBfr(send_bfr,bfr_size,send_msg_len);
		
		// Listen to the slaves
		MPI_Recv((char*)recv_bfr,(int)bfr_size, MPI_BYTE, MPI_ANY_SOURCE, CHDB_TAG_READY, MPI_COMM_WORLD, &sts);
		int talking_slave = sts.MPI_SOURCE;
		//size_t recv_msg_len;
		//MPI_Get_count(&sts, MPI_BYTE, (int*) &recv_msg_len);

        // Init return_values and file_pathes with the message
		readFrmRecvBfr(recv_bfr);
		
		// counting the files
		treated_files += count_if(file_pathes.begin(),file_pathes.end(),isNotNullStr);

		// Handle the error
		errorHandle(err_file);

		// Send the block to the slave
		MPI_Send(send_bfr,send_msg_len,MPI_BYTE,talking_slave,CHDB_TAG_GO,MPI_COMM_WORLD);

		// Init return_values and file_pathes for next iteration
		return_values.clear();
		file_pathes = dir.nextBlock();
	}

	// loop over the slaves: when each slave is ready, send it a msg END
	int working_slaves = getNbOfSlaves(); // The master is not a slave
	while(working_slaves>0) {
		MPI_Recv(recv_bfr, bfr_size, MPI_BYTE, MPI_ANY_SOURCE, CHDB_TAG_READY, MPI_COMM_WORLD, &sts);
		int talking_slave = sts.MPI_SOURCE;
		//int msg_len;
		//MPI_Get_count(&sts, MPI_BYTE, &msg_len);

        // Init return_values and file_pathes with the message
		readFrmRecvBfr(recv_bfr);
		
		// counting the files
		treated_files += count_if(file_pathes.begin(),file_pathes.end(),isNotNullStr);

		// Handle the error
		errorHandle(err_file);
		
		// Send an empty message tagged END to the slave
		MPI_Send(send_bfr, 0, MPI_BYTE, talking_slave, CHDB_TAG_END, MPI_COMM_WORLD);
		working_slaves--;
	}

	// close err_file
	if (err_file.is_open()) err_file.close();

	// free memory
	free(send_bfr);
	free(recv_bfr);
}

/** 
 * @brief The main loop for the slaves
 * 
 */
void BasicScheduler::mainLoopSlave() {
	vector_of_strings blk;
	MPI_Status sts;

	size_t bfr_size=0;
	void* bfr=NULL;
	allocBfr(bfr,bfr_size);

	// all msgs are sent/received to/from the master
	const int master = 0;
	int tag    = CHDB_TAG_GO;
	while(tag==CHDB_TAG_GO) {

		// Prepare the send buffer for the next message
		size_t send_msg_len=0;
		writeToSndBfr(bfr,bfr_size,send_msg_len);

		// Send the report+ready message to the master, receive a list of files to treat
		MPI_Sendrecv_replace((char*)bfr,(int)bfr_size,MPI_BYTE,master,CHDB_TAG_READY,master,MPI_ANY_TAG,MPI_COMM_WORLD,&sts);
		tag = sts.MPI_TAG;
		
        // Init file_pathes with the message
		readFrmRecvBfr(bfr);

		if (tag==CHDB_TAG_GO) {
			executeCommand();
		}
	}

	// free memory
	free(bfr);
}

/** 
 * @brief Execute the command on all files of file_pathes, and store the result in return_values
 * 
 */
void BasicScheduler::executeCommand() {
	string command = prms.getExternalCommand();
	int zero = 0;
	return_values.assign (file_pathes.size(),zero);

	for (size_t i=0; i<file_pathes.size(); ++i) {
		string cmd = command;
		string in_path = file_pathes[i];

		// skip input files with empty names
		if (in_path.size()==0) continue;

		dir.completeFilePath(in_path,cmd);
		vector_of_strings out_files = prms.getOutFiles();
		for (size_t j=0; j<out_files.size(); ++j) {
			dir.completeFilePath(in_path,out_files[j]);
		}
		int sts = dir.executeExternalCommand(cmd,out_files);
		// If abort on Error, throw an exception if status != 0
		if (sts!=0) {
			if (prms.isAbrtOnErr()) {
				ostringstream msg;
				msg << "ERROR with external command\n";
				msg << "command    : " << cmd << '\n';
				msg << "exit status: " << sts << '\n';
				msg << "ABORTING - Try again with --on-error\n";
				throw(runtime_error(msg.str()));
			} else {
				return_values[i] = sts;
			}
		}
	}
}

/** 
 * @brief Called by the master when error mode is on
 * 
 * @param err_file 
 */
void BasicScheduler::errorHandle(ofstream& err_file) {

	// If Abort On Error, just return. Abort already called if there was an error !
	if (prms.isAbrtOnErr()) return;

	// find the first value in error and return if none
	vector_of_int::iterator it = find_if(return_values.begin(),return_values.end(),isNotNull);
	if (it == return_values.end()) return;

	// loop from it and write the files in error
	for ( size_t i=it-return_values.begin(); i<return_values.size(); ++i) {
		if (return_values[i]==0) continue;
		err_file << *it << '\t' << file_pathes[i] << '\n';
	}
}

/** 
 * @brief Alloc a buffer big enough to send/receive return_values and file_pathes
 *        For block_size 4 and FILEPATH_MAXLENGTH 5 we may have at most:
 *          4000AAAABBBBCCCCDDDD4000xxxxx\0xxxxx\0xxxxx\0xxxxx\0
 *          4000 is the integer representation of 4 in little endian machines
 *          A,B,C,D are integers representing the status retrieved by the slaves for each file name
 *
 * @param[out] bfr The allocated buffer
 * @param[out] bfr_sze The size of the allocated buffer
 *
 */
void BasicScheduler::allocBfr(void*& bfr,size_t& bfr_size) {
	size_t vct_size = prms.getBlockSize();
	bfr_size  = sizeof(int) + vct_size*sizeof(int);
	bfr_size += sizeof(int) + vct_size*(FILEPATH_MAXLENGTH+1);
	bfr       = malloc(bfr_size);
	if (bfr==NULL) {
		throw(runtime_error("ERROR - Could not allocate memory"));
	}
}

/** 
 * @brief Write to a send buffer the vectors return_values and file_pathes
 * 
 * @pre The bfr should be already allocated with allocBfr

 * @param[in]  bfr       The buffer (should be already allocated)
 * @param[in]  bfr_size  The buffer length
 * @param[out] data_size The data stored, should be <= bfr_size
 * 
 */
void BasicScheduler::writeToSndBfr(void* bfr, size_t bfr_size, size_t& data_size) {
	checkInvariant();
	size_t int_data_size=0;
	size_t str_data_size=0;

	// Fill the buffer with the data from return_values, then from file_pathes
	//      iiiiiiiiifffffffffffffffffffffffffff00000000000000000000000000000
	//      ^        ^                          ^
	//      0        int_data_size              int_data_size+str_data_size  ^bfr_size
	//                               
	
	vctToBfr(return_values,bfr,bfr_size,int_data_size);
	bfr = (void*) ((char*) bfr + int_data_size);
	bfr_size = bfr_size - int_data_size;
	vctToBfr(file_pathes,bfr,bfr_size,str_data_size);
	data_size = int_data_size + str_data_size;
}

void BasicScheduler::readFrmRecvBfr	(const void* bfr) {
	size_t data_size;
	bfrToVct(bfr,data_size,return_values);
	bfr = (void*) ((char*) bfr + data_size);
	bfrToVct(bfr, data_size, file_pathes);
	checkInvariant();
}

/** 
 * @brief The invariant is: return_values empty, OR same size as file_pathes
 *
 */
void BasicScheduler::checkInvariant() {
	assert(return_values.empty() || return_values.size()==file_pathes.size());
}
	
/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/
