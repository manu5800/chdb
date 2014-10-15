
/**
   unit tests for the class Directories, and its subclasses
*/


#include "constypes_unittest.hpp"
#include "../basicscheduler.hpp"
#include <fstream>
using namespace std;

class SchedTestStr : public ChdbTest {
public:
	SchedTestStr() {
		int n = 5;
		string tmp((char*) &n,sizeof(int));
		expected_bfr  = tmp;
		expected_bfr += "B.txt";
		expected_bfr += '\0';
		expected_bfr += "C/C.txt";
		expected_bfr += '\0';
		expected_bfr += "C/C/C.txt";
		expected_bfr += '\0';
		expected_bfr += "D/C.txt";
		expected_bfr += '\0';
		expected_bfr += "A.txt";
		expected_bfr += '\0';

		bfr_len = 5;
		bfr_len += sizeof(int) + 5 + 7 + 9 + 7 + 5;
		bfr = malloc(bfr_len);

	};
	~SchedTestStr() { free(bfr); };

protected:
	string expected_bfr;
	void*  bfr;
	size_t bfr_len;
};

TEST_F(SchedTestStr,vctToBfrStrings) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);
	vector_of_strings files=dir.getFiles();

	size_t data_len;
	sched.vctToBfr(files,bfr,bfr_len,data_len);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),bfr_len));
	EXPECT_EQ(bfr_len,data_len);

	FREE_ARGV(7);
};

TEST_F(SchedTestStr,bfrToVctStrings) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	char const* bfr = expected_bfr.c_str();

	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);

	vector_of_strings file_pathes;
	size_t  data_size;
	sched.bfrToVct((const void*)bfr,data_size,file_pathes);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_file_pathes,file_pathes);

	// 2nd time
	sched.bfrToVct((const void*)bfr,data_size,file_pathes);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_file_pathes,file_pathes);

	FREE_ARGV(7);
};

class SchedTestInt : public ChdbTest {
public:
	SchedTestInt() {
		expected_values.push_back(3);
		expected_values.push_back(5);
		expected_values.push_back(7);
		int v[]={3,3,5,7};
		string b((char*)v,4*sizeof(int));
		expected_bfr = b;

		bfr_len  = 4*sizeof(int);
		bfr = malloc(bfr_len);
	};
	~SchedTestInt() { free(bfr); };

protected:
	vector_of_int expected_values;
	
	string expected_bfr;
	void*  bfr;
	size_t bfr_len;
};

TEST_F(SchedTestInt,vctToBfrInt) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);
	vector_of_strings files=dir.getFiles();

	vector_of_int values;
	values.push_back(3);
	values.push_back(5);
	values.push_back(7);

	size_t data_len;
	sched.vctToBfr(values,bfr,bfr_len,data_len);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),bfr_len));
	EXPECT_EQ(bfr_len,data_len);

	FREE_ARGV(7);
};

TEST_F(SchedTestInt,bfrToVctInt) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	
	char const* bfr = expected_bfr.c_str();

	Parameters prms(7,argv);
	UsingFs dir(prms);
	BasicScheduler sched(prms,dir);

	vector_of_int values;
	size_t data_size;
	sched.bfrToVct(bfr,data_size,values);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_values,values);

	// 2nd time
	sched.bfrToVct(bfr,data_size,values);
	EXPECT_EQ(data_size,expected_bfr.length());
	EXPECT_EQ(expected_values,values);

	FREE_ARGV(7);
};

class SchedTestStrInt : public ChdbTest {
public:
	SchedTestStrInt() {
		expected_file_pathes.push_back(input_dir + '/' + "B.txt");
		expected_file_pathes.push_back(input_dir + '/' + "C/C.txt");
		expected_file_pathes.push_back(input_dir + '/' + "C/C/C.txt");
		expected_file_pathes.push_back(input_dir + '/' + "D/C.txt");
		expected_file_pathes.push_back(input_dir + '/' + "A.txt");

		// no value, 5 files
		int v[2] = {0,5};
		expected_bfr = string((char*) &v,2*sizeof(int));
		expected_bfr += input_dir + '/' + "B.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "C/C/C.txt" + '\0';
		expected_bfr += input_dir + '/' + "D/C.txt"   + '\0';
		expected_bfr += input_dir + '/' + "A.txt"     + '\0';

		// 5 values, 5 files
		int v_1[7] = {5,0,1,2,3,4,5};
		expected_values_1.push_back(0);
		expected_values_1.push_back(1);
		expected_values_1.push_back(2);
		expected_values_1.push_back(3);
		expected_values_1.push_back(4);

		expected_bfr_1 = string((char*) &v_1,7*sizeof(int));
		expected_bfr_1 += input_dir + '/' + "B.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "C/C.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "C/C/C.txt" + '\0';
		expected_bfr_1 += input_dir + '/' + "D/C.txt"   + '\0';
		expected_bfr_1 += input_dir + '/' + "A.txt"     + '\0';
		
		bfr_len  = sizeof(int);
		bfr_len += 5 * input_dir.length();
		bfr_len += 10;
		bfr_len += sizeof(int) + 5 + 7 + 9 + 7 + 5;
		bfr = malloc(bfr_len);
	};
	~SchedTestStrInt() { free(bfr); };

protected:
	vector_of_strings expected_file_pathes;
	vector_of_int expected_values_1;
	string expected_bfr;
	string expected_bfr_1;
	void*  bfr;
	size_t bfr_len;
};

TEST_F(SchedTestStrInt,readwriteToSndBfr) {

	// Init prms
	char* argv[10];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"coucou");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--block-size");
	INIT_ARGV(8,"5");
	
	Parameters prms(9,argv);
	UsingFs dir(prms);

	// Load (empty) values and file_pathes from expected_bfr
	BasicScheduler sched(prms,dir);
	sched.readFrmRecvBfr(expected_bfr.c_str());
	EXPECT_EQ(0,sched.return_values.size());
	EXPECT_EQ(expected_file_pathes,sched.file_pathes);

	// Create a bfr from the (empty) values and file_pathes and compare it to expected_bfr
	size_t data_size;
	size_t bfr_size = expected_bfr.length() + 50;
	void * bfr = malloc(bfr_size);
	sched.writeToSndBfr(bfr,bfr_size,data_size);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr.c_str(),data_size));
	EXPECT_EQ(data_size,expected_bfr.length());
	
	// clear file_pathes and reload values and file_pathes from  expected_bfr_1
	sched.file_pathes.clear();
	sched.readFrmRecvBfr(expected_bfr_1.c_str());
	EXPECT_EQ(expected_values_1,sched.return_values);
	EXPECT_EQ(expected_file_pathes,sched.file_pathes);

	// write again to bfr and compare value to expected_bfr_1
	sched.writeToSndBfr(bfr,bfr_size,data_size);
	EXPECT_EQ(0,memcmp(bfr,expected_bfr_1.c_str(),data_size));
	EXPECT_EQ(data_size,expected_bfr_1.length());

	FREE_ARGV(9);
};

TEST_F(ChdbTest1,ExecuteCommand) {

	// Init prms
	char* argv[11];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");

	Parameters prms(11,argv);
	UsingFs dir(prms);
	dir.makeOutputDir();

	// Read the files
	dir.readFiles();

	// execute command, initializing return_values and file_pathes
	// An exception is generated for the file D/C.txt
	BasicScheduler sched(prms,dir);
	sched.return_values.clear();
	sched.file_pathes = dir.nextBlock();
	EXPECT_THROW(sched.executeCommand(),runtime_error);

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));

	FREE_ARGV(11);
};

TEST_F(ChdbTest1,ExecuteCommandWithErr) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--on-error");
	INIT_ARGV(12,"errors.txt");

	Parameters prms(13,argv);
	UsingFs dir(prms);
	dir.makeOutputDir();

	// Read the files
	dir.readFiles();

	// execute command, initializing return_values and file_pathes
	// no exception, thanks to --on-error
	BasicScheduler sched(prms,dir);
	sched.return_values.clear();
	sched.file_pathes = dir.nextBlock();
	EXPECT_NO_THROW(sched.executeCommand());

	// call errorHandle, generating errors.txt
	string e_out = prms.getErrFile();
	{
		ofstream e(e_out.c_str());
		sched.errorHandle(e);
		e.close();
	}

	ifstream e(e_out.c_str());
	EXPECT_EQ(true,e.good());

	EXPECT_EQ(expected_file_contents["B.txt"],readFile("inputdir.out/B.txt"));
	EXPECT_EQ(expected_file_contents["C/C.txt"],readFile("inputdir.out/C/C.txt"));
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));

	FREE_ARGV(13);
};

TEST_F(ChdbTest1,ExecuteCommandFrmList1) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path% 0");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--in-file");
	INIT_ARGV(12,"errors.txt");

	Parameters prms(13,argv);
	UsingFs dir(prms);
	dir.makeOutputDir();

	// Read the files
	dir.readFiles();

	// execute command, initializing return_values and file_pathes
	// The files used are read from errors.txt, generated by the test, ExecuteCommandWithErr
	// This test should have been run juste before
	// no exception, thanks to the last parameter of ext_cmd.sh
	BasicScheduler sched(prms,dir);
	sched.return_values.clear();
	sched.file_pathes = dir.nextBlock();
	EXPECT_NO_THROW(sched.executeCommand());

	// only ONE result
	EXPECT_EQ(1,sched.return_values.size());
	EXPECT_EQ(1,sched.file_pathes.size());
	EXPECT_EQ(expected_file_contents["D/C.txt"],readFile("inputdir.out/D/C.txt"));

	// Remove errors.txt
	system("rm errors.txt");

	FREE_ARGV(13);
};

TEST_F(ChdbTest1,ExecuteCommandFrmList2) {

	// Init prms
	char* argv[13];
	INIT_ARGV(0,"directories_unittest");
	INIT_ARGV(1,"--command-line");
	INIT_ARGV(2,"./ext_cmd.sh %in-dir%/%path% %out-dir%/%path%");
	INIT_ARGV(3,"--in-dir");
	INIT_ARGV(4,input_dir.c_str());
	INIT_ARGV(5,"--in-type");
	INIT_ARGV(6,"txt");
	INIT_ARGV(7,"--out-files");
	INIT_ARGV(8,"%out-dir%/%path%");
	INIT_ARGV(9,"--block-size");
	INIT_ARGV(10,"5");
	INIT_ARGV(11,"--in-file");
	INIT_ARGV(12,"files.txt");

	Parameters prms(13,argv);
	UsingFs dir(prms);
	dir.makeOutputDir();

	// Prepare files.txt
	ofstream f("files.txt");
	f << "C/C/C.txt\n";
	f << "# some comment\n";
	f << "A.txt\n";
	f.close();

	// Read the files
	dir.readFiles();

	// execute command, initializing return_values and file_pathes
	// The files used are read from files.txt (2 files only)
	BasicScheduler sched(prms,dir);
	sched.return_values.clear();
	sched.file_pathes = dir.nextBlock();
	EXPECT_NO_THROW(sched.executeCommand());

	// only TWO results
	EXPECT_EQ(2,sched.return_values.size());
	EXPECT_EQ(2,sched.file_pathes.size());
	EXPECT_EQ(expected_file_contents["C/C/C.txt"],readFile("inputdir.out/C/C/C.txt"));
	EXPECT_EQ(expected_file_contents["A.txt"],readFile("inputdir.out/A.txt"));

	// Remove files.txt
	system("rm files.txt");

	FREE_ARGV(13);
};

// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?

