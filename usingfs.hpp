
#ifndef USING_FS_H
#define USING_FS_H

//#include <vector>
#include <list>
//#include <stdexcept>
#include <set>
using namespace std;

#include "constypes.hpp"
#include "directories.hpp"

/** This struct is used for sorting the files before storing them - see readDir */
struct Finfo {
	Finfo(const string& n, off_t s): name(n),st_size(s) {};
  string name;
  off_t st_size;
};

/** 
	\brief This class manages the files contained inside the input or output directory
	       It is used when we work with REAL directories
*/
#include <iostream>
class UsingFs: public Directories {
public:
	UsingFs(const Parameters& p):Directories(p){};

	// consolidateOutput may throw an exception (if incompletly initalized) - Ignore it
	~UsingFs() {
		try {
			consolidateOutput();
		} catch (exception& e){};
	}

	//void filesToOutputDb(const vector_of_strings&) {};
	int executeExternalCommand(const string&,const vector_of_strings&) const;
	void makeOutDir(bool,bool);
	string makeTempOutDir();
	string getTempOutDir() const {
		if(temp_output_dir.length()!=0) return temp_output_dir;
		else throw(logic_error("ERROR - temp_output_dir NOT INITIALIZED"));
	}
	string getOutDir() const  {
		if(output_dir.length()!=0) return output_dir;
		else throw(logic_error("ERROR - output_dir NOT INITIALIZED"));
	}
	void buildBlocks(list<Finfo>&, vector_of_strings&) const;
	void consolidateOutput(const string& path="") const;

	friend class ChdbTest_usingFsfindOrCreateDir_Test;

private:
	void readDir(const string &,size_t) const;
	void readDirRecursive(const string &,size_t,list<Finfo>&,bool) const;
	void findOrCreateDir(const string &) const;
	bool isCorrectType(const string &) const;
	void initInputFiles() const;
	virtual void v_readFiles();
	mutable set<string> input_files;
	mutable set<string> found_directories;
	string output_dir;
	string temp_output_dir;
};

#endif

/*
 * Copyright Univ-toulouse/CNRS - xxx@xxx, xxx@xxx
 * This software is a computer program whose purpose is to xxxxxxxxxxxxxxxxxx
 * This software is governed by the CeCILL-C license under French law and abiding by the rules of distribution of free software
 * 
 * Please read the file Licence_CeCILL-C_V1-en.txt which should have been distributed with this file
 * The fact that you are presently reading this, and the aforementioned file, means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
*/
