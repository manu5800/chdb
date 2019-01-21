/** This file is part of bdbh software
 * 
 * bdbh helps users to store little files in an embedded database
 *
 * bdbh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  Copyright (C) 2010-2014    L I P M
 *  Copyright (C) 2015-2018    C A L M I P
 *  bdbh is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with bdbh.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:
 *        Emmanuel Courcelle - C.N.R.S. - UMS 3667 - CALMIP
 *        Nicolas Renon - Université Paul Sabatier - University of Toulouse)
 */

#ifndef BDBH_COMMAND_H
#define BDBH_COMMAND_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <memory>
using namespace std;

#include "exception.hpp"
#include "parameters.hpp"
#include <db_cxx.h>

namespace bdbh {

/** \brief Some default parameters
   
   MAX_FILE_SIZE         ==> The initial max size for a file to be added to the database, see parameters.h
   COMPRESSION_THRESHOLD ==> Only a file bigger than this threshold will be compressed
*/

#define COMPRESSION_THRESHOLD  1024

// Will be passed to the constructor of Command
#define BDBH_OCREATE 0
#define BDBH_OREAD   1
#define BDBH_OWRITE  2
#define BDBH_OINFO   3  // Consolidate the database info
#define BDBH_OSHELL  4  // Used by the shell command
#define BDBH_OCONVERT 5 // Used by the convert command

// Exit error codes
#define BDBH_ERR_OK 0
#define BDBH_ERR_US 1   // Usage pb
#define BDBH_ERR_PB 2   // Something wrong
#define BDBH_ERR_DB 3   // Error generated by Berkeley Db
#define BDBH_ERR_OW 10  // Overwrite not specified
#define BDBH_ERR_DR 11  // Cannot do this on a directory
#define BDBH_ERR_NF 12  // Key not found
#define BDBH_ERR_RE 13  // Recursive not specified

// The program version
#define BDBH_VERSION "2.0.0"

// The version number (not the program version, just the data file version)
#define V_MAJOR 1
#define V_MINOR 3

// Some special files and directories
// DIRECTORY_MARKER = MUST start with / !!!
#define DIRECTORY_MARKER "/ "
#define INFO_KEY    " "
#define DATA_NAME   "data"
#define METADATA_NAME "metadata"

/* 
   Some helper classes
*/

/* 
// just for debug
class Coucou {
    public:
    Coucou(const string& msg){cerr << "hello " << msg << "\n";level++;};
    ~Coucou(){cerr << "bye " << --level << "\n";};
    private:
    static int level;
};
*/

	/** \brief We alloc some memory in the contructor, free it in the destructor
	   
		GetData() returns the buffer's base address
		SetSize() is used to set the size of the buffer (must be < the memory allocated)
		Getsize() returns the current size
	*/
	class Buffer {
    public:
    Buffer(u_int32_t s=MAX_FILE_SIZE);
    ~Buffer();

    void SetCapacity(u_int32_t);
    void SetSize(int32_t);
    void* GetData() const {return bfr;};
    u_int32_t GetSize() const {return size;};
    
    private:
    void* bfr;
    u_int32_t size;
    u_int32_t capacity;
};

/** \brief Open the file in the constructor, close it  in the destructor
 */

	class File {
    public:
		File(const char*, int);
		File(const char*, int, mode_t);
		~File() {close(fd);};
		int GetFd() const {return fd;};
		int GetErr() const {return er;};
    
    private:
		int fd;
		int er;
	};

/** \brief Open the dir in the constructor, close it in the destructor
 */

	class Dir {
    public:
		Dir(const char*);
		~Dir() {closedir(d);};
		DIR* GetDir() const {return d;};
		int GetErr() const {return er;};
    
    private:
		DIR* d;
		int er;
	};

/** \brief   The metadata of each file/directory/symlink
 */

	typedef u_int32_t bdbh_ino_t;
	struct Mdata {
		Mdata():ino(0),mode(0),uid(0),gid(0),size(0),csize(0),cmpred(false),atime(0),mtime(0){};
		bdbh_ino_t ino;   /* inode (in bdbh) */
		mode_t mode;    /* protection and type of object (dir,link,file) */
		uid_t  uid;
		gid_t  gid;
		off_t  size;    /* total size, in bytes */
		off_t  csize;   /* size of compressed data */
		bool   cmpred;  /* If true, those data are compressed */
		time_t    atime;   /* time of last access */
		time_t    mtime;   /* time of last modification */
	};

/** \brief  Global information about the database, 
 */

	class InfoData {
	public:
		InfoData(bool compression_status=false) : v_major(V_MAJOR),v_minor(V_MINOR), data_size_uncompressed(0),
												  data_size_compressed(0),max_data_size_uncompressed(0),key_size(0),max_key_size(0),nb_of_files(0),nb_of_dir(0),nb_of_inodes(0),next_inode(0),date_created(0),date_modified(0),
												  data_compressed(compression_status) {};

    private:
		int64_t NextInode() { return ++next_inode; };
		void Update(int size_uncompressed,int size_compressed,int key,int files,int dir,int inodes);
		int v_major;
		int v_minor;
		int64_t data_size_uncompressed;
		int64_t data_size_compressed;
		u_int32_t max_data_size_uncompressed;
		int64_t key_size;
		u_int32_t max_key_size;
		int64_t nb_of_files;
		int64_t nb_of_dir;
		int64_t nb_of_inodes;
		int64_t next_inode;
		time_t date_created;
		time_t date_modified;
		bool data_compressed;

		friend class BerkeleyDb;
		friend class Info;
		//friend class Convert;
	};

	class Command;

/** \brief TriBuff groups 3 objects Buffer 

    There is no need for a public section, because Command, the only object to use TriBuff, is a friend
*/
	class TriBuff
	{
    public:
		TriBuff():tribuff_size(MAX_FILE_SIZE),data_bfr(tribuff_size),key_bfr(100000),c_data_bfr(tribuff_size+tribuff_size/10){};
		friend class Command;
    
    private:
		u_int32_t tribuff_size;

		/// A buffer used to deposit the data
		Buffer data_bfr;           
    
		/// A buffer used to deposit the key
		Buffer key_bfr;             

		/// compression buffer
		Buffer c_data_bfr;
	};

/** \brief BerkeleyDb: All db related variables and methods are grouped in a BerkeleyDb object
 */

	typedef auto_ptr<Db> Db_aptr;
	typedef auto_ptr<DbEnv> DbEnv_aptr;
	class BerkeleyDb
	{
    public:
		BerkeleyDb(const char* name,int o_mode,bool verb=false, bool in_memory=false);
		~BerkeleyDb();

		int GetOpenMode() const { return open_mode; };

		/// Sync the database
		void Sync(bool with_consolidate_info=false);
    
		/// Accessor: what is the max data size stored inside info_data ?
		size_t GetMaxDataSize() const { return info_data.max_data_size_uncompressed; };
    
		/// Accessor: Are data compressed ?
		bool GetCompressionFlag() const { return info_data.data_compressed;}; 
    
		/// Accessor: Get InfoData for display purpose
		const InfoData& GetInfoData() const {return info_data;};
    
		/// Mutator: Ignore any compression status
		void IgnoreCompressionFlag() {ignore_comp=true;};
    
		/// Throw an exception if the db has the same dev,inode pair as the parameters
		void IsDbItself(const char *file) const;
		void IsDbItself(const struct stat&) const; 

		/// Generate and return the next inode number
		bdbh_ino_t NextInode() { return info_data.NextInode(); };

		/// Update the data_info from the parameters
		void UpdateDbSize(int size_uncompressed, int size_compressed, int size_key, int files, int dir, int inodes);
		
		/// Update the compression status in data_info (it works because we are friend !)
		void UpdateComp(bool comp) { info_data.data_compressed = comp ; };

		/// Read/Write info_data
		void __ReadInfoData();
		void __WriteInfoData();
    
		/// Read/write data
		int ReadKeyData(const string& key, Mdata& metadata, Buffer& data_bfr, Buffer& c_data_bfr, bool with_data);
		int ReadKeyData(const string& key, string& rtn_key, Mdata& metadata, Buffer& key_bfr, Buffer& data_bfr, Buffer& c_data_bfr, int lvl, bool first=true, bool with_data=false, bool reverse=false );
		int WriteKeyData(const string& key, Mdata& metadata, Buffer& data_bfr, Buffer& c_data_bfr, bool with_data, bool to_db, bool overwrite );
    
		/// Remove metadata and eventually data, using a correctly positioned cursor
		int RemoveKeyDataCursor(int lvl,Mdata*);
   
    protected:
    
    private:
    
		void __InitDb(const char* name,int flg);
		void __OpenCreate(const char*);
		void __OpenRead(const char*);
		void __OpenWrite(const char*);
		void __Open(const char*, uint32_t);

		void __InMemory(const string&); // Put the whole database in memory to increase performance
    	bool __aligned_p(void*);
    	void __path2dbt_key(const string &);
		uint32_t __bytes2pages(uint64_t);
		bool __is_mincore_page_resident(char p);
		bool verbose;               // The switch --verbose was specified
		bool in_memory;				// the switch --in_memory was specified
    
		// Metadata for info_data
		Mdata minfo_data;
    
		// Data for info_data
		InfoData info_data;
		int update_cnt;

		bool ignore_comp;
    
		/// The (device, inode) of the database file
		struct stat bd_stat;
    
		/// Dbt objects for read-write data
		Dbt dbt_key;
		Dbt dbt_inode;
		Dbt dbt_data;
		Dbt dbt_mdata;
		bdbh_ino_t c_inode; // the current inode

		/// The db itself: dbi for the metadata (i=inodes), db for the data
		Db_aptr dbi;
		Db_aptr db;
		
		/// The pagesize on our platform
		long pagesize;

		/// A vector of cursors: 1 cursor / level
		vector<Dbc*> cursors;
		Dbc* __SelectCurrentCursor(unsigned int lvl);
    
		int open_mode;
		Buffer data_bfr;
		Buffer key_bfr;
	};

	class Merge;

/** \brief This is an abstract class - When the user specifies a command, an object deriving from tihs class will be created
    by the main program
*/
	class Command {
	public:
	
		/** 
			The constructor:
			The command line is analyzed by Parameters (see parameters.cpp)
			The database is initialized by the constructor
			We pass a Parameters object, plus a flag, used to open the database
		*/       
		Command(const bdbh::Parameters& p, BerkeleyDb& d);
	
		/**
		   The virtual destructor
		*/
		virtual ~Command(){};
    
		/** \brief Exec: Something will be executed, each derived object implements the thing
			This results in an exit status, retrieved with GetExitStatus
		*/
	
		virtual void Exec() = 0;
		int GetExitStatus() const {return exit_status;};

		// Signal handling: the signal handler should call SetSignal. 
		// The bdbh operations are interrupted when signal_received is > 0, but the database should stay coherent

   		void SetSignal(int s) {signal_received=s;};
		void ResetSignal() {SetSignal(0); };
	
		friend class Merge;
		friend void Initialize();
		friend void Terminate();
    
		/// The exit status, set by the Exec() functions
		int exit_status;           
    
		/// Accessing data_bfr
		Buffer& GetDataBfr() {return bfr3.data_bfr;};

	protected:
		/// The parameters, analyzed by the command line
		const bdbh::Parameters& prm; 

		/// Accessing the BerkeleyDb object (sometimes useful, see Mv)
		BerkeleyDb& _GetBerkeleyDb() { return bdb;};
		
	private:
		/// Accessing c_data_bfr (private)
		Buffer& GetCDataBfr() {return bfr3.key_bfr;};
    
		/// Accessing key_bfr (private)
		Buffer& GetKeyBfr() {return bfr3.key_bfr;};
	
		/*
		  Protected functions
		*/
	
	protected:
		const InfoData& _GetInfoData() const {return bdb.GetInfoData();};
	
		// Make a directory inside the database (2 versions)
		void _Mkdir (const Fkey& fkey);
		void _Mkdir (const Fkey& fkey, Mdata & mdata);
	
		// Generate and return the next inode number
		bdbh_ino_t _NextInode() { return bdb.NextInode(); };
		
		// Write a pair (key, data) to the database
		int _WriteKeyData(const string& key,Mdata& metadata, bool with_data=true, bool overwrite=true){
			return bdb.WriteKeyData(key,metadata,GetDataBfr(),GetCDataBfr(),with_data,true,overwrite);
		};
	
		// _ReadKeyData: Read a pair (key, data) from the database
		int _ReadKeyData(const string& key,Mdata&metadata,bool with_data=false){
			return bdb.ReadKeyData(key,metadata,GetDataBfr(),GetCDataBfr(),with_data);
		};
	
		// The cursor
		int _ReadKeyDataCursor(const string& key,string& rtn_key,Mdata&metadata,int lvl, bool first, bool with_data=false,bool reverse=false) {
			return bdb.ReadKeyData(key,rtn_key,metadata,GetKeyBfr(),GetDataBfr(),GetCDataBfr(),lvl,first,with_data,reverse);
		};
	
		// Remove metadata and eventually data, using a cursor
		int _RemoveUsingCursor(int lvl, Mdata* mdata_ptr=nullptr) { 
			return bdb.RemoveKeyDataCursor(lvl,mdata_ptr);
		};
    
		/// Throw an exception if the db has the same dev,inode pair as the parameters
		void _IsDbItself(const char *file) const {bdb.IsDbItself(file);};
		void _IsDbItself(const struct stat& st) const {bdb.IsDbItself(st);};
	
		/// Return true if the key is found in the database, and if true return the metadata
		bool _IsInDb(const char* key, Mdata& mdata) {return _ReadKeyData(key,mdata)==0;};
		
		/// Return true if the key is found in the database, the metadata are lost
		bool _IsInDb(const char* key) {Mdata mdata; return _ReadKeyData(key,mdata)==0;};
    
		/// Return true if the file is found in the file system
		bool _IsInFs(const char* file);

		// Return a cursor
		///Dbc* _GetCursor() {return bdb.GetCursor();};
    
		// Update the data_info from the parameters
		void _UpdateDbSize(int size_uncompressed, int size_compressed, int key, int files, int dir, int inodes);
		void _UpdateComp(bool comp) { bdb.UpdateComp(comp); };
	
		/// Return true if a signal was received recently
		bool _IsSignalReceived() {return signal_received!=0;};
	
		/// Compare the level passed by parameter with the --level switch
		/// Returns 0  if level higher than the switch (too deep)
		/// Returns 1  if level lower  than the switch (not too deep)
		/// Returns -1 if no switch specified (not too deep, because there is no limit)
		int _IsNotTooDeep(int lvl) const { 
			if (prm.GetLevel()==-1)
				return -1;
			else
				return (lvl <= prm.GetLevel()) ? 1 : 0;
		}
    
		/// Expand the wilcard: /toto/titi/*/tutu
		vector<string> _ExpandWildcard(const string& k);
	
		/// Adjust the databuffers' capacity
		void _AdjustBufferCapacity();
    
	private: 
	
		/// The database-related objects
		BerkeleyDb& bdb;

		/// 3 buffers used to deposit the data, the key, the compressed data
		TriBuff& bfr3;

		/// bfr3_ptr is COMMON TO ALL Command objects
		/// This is good for performance for some derived objects (ie Merge)
		static TriBuff* bfr3_ptr;

		TriBuff& __InitBfr3() {
			if (bfr3_ptr==NULL) {
				throw (logic_error("Command: You must call bdbh::Initialize before creating a Command object"));
			} else {
				return *bfr3_ptr;
			}
		}
    
		/*
		  Last received signal (0 if none)
		*/
    
		static int signal_received;
	};

/** \brief Initialize bdbh, ie allocate data for a TriBuf
           Do not do anything if already done (!)
 */
	inline void Initialize() { if (Command::bfr3_ptr==NULL) Command::bfr3_ptr = new TriBuff(); }


/** \brief Terminate bdbh, ie free memory taken by Initialize
           Do not anything if already done, or if not initialized
*/
	inline void Terminate() { if (Command::bfr3_ptr!=NULL) {delete Command::bfr3_ptr; Command::bfr3_ptr = NULL;} }

/** \brief Print the time in a human-readable way
 */
	struct Time {
		Time(time_t t):tme(t){};
		time_t tme;
	};
	ostream& operator<<(ostream&,const Time&);

/* Inline functions */
/** Return true if the file is in the file system
 */
	inline bool Command::_IsInFs(const char* file)
	{
		int rvl=open(file,O_RDONLY);
		if(rvl!=-1) 
		{
			close(rvl);
			return true;
		}
		else
		{
			return false;
		};
	}

/**
   Update an InfoData struct (only in memory)
*/
	inline void InfoData::Update(int size_uncompressed,int size_compressed,int key,int files,int dir, int inodes)
	{
		data_size_uncompressed += size_uncompressed;
		if (size_uncompressed > 0 && size_uncompressed > (int) max_data_size_uncompressed)
			max_data_size_uncompressed = size_uncompressed;
    
		data_size_compressed   += size_compressed;
		key_size += key;
		if (key > (int) max_key_size)
			max_key_size = key;
		nb_of_files  += files;
		nb_of_dir    += dir;
		nb_of_inodes += inodes;
	}

/**
   Update info_data in memory, write it to the database from time to time 
*/
	inline void Command::_UpdateDbSize(int size_uncompressed, int size_compressed, int key, int files, int dir, int ino) {
		bdb.UpdateDbSize(size_uncompressed, size_compressed, key, files, dir, ino);
	}

	inline void BerkeleyDb::UpdateDbSize(int size_uncompressed, int size_compressed, int key, int files, int dir, int ino) {
		info_data.Update(size_uncompressed,size_compressed,key,files,dir,ino);
		update_cnt++;
		if (update_cnt==1000)
		{
			__WriteInfoData();
			db->sync(0);      // Calling sync here to get a real checkpoint 
			update_cnt=0;
		};
	}

	inline File::File(const char* file_name, int flags)
	{
		fd = open(file_name,flags);
		if (fd==-1)
			er = errno;
	}

	inline File::File(const char* file_name, int flags,mode_t mode)
	{
		fd = open(file_name,flags,mode);
		if (fd==-1)
			er = errno;
	}

	inline Dir::Dir(const char* dir_name)
	{
		d = opendir(dir_name);
		if (d==NULL)
			er = errno;
	}
}

#endif
