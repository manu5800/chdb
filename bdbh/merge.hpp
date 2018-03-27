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

#ifndef BDBH_MERGE_H
#define BDBH_MERGE_H

#include <vector>
#include <string>
using namespace std;

#include "command.hpp"
#include <db_cxx.h>

namespace bdbh {
/** \brief This object is used with the merge command
 */
	class Merge: public Command {
    public:
		Merge (const Parameters& p, BerkeleyDb& d, Merge* to=NULL);
		virtual void Exec();
    
    private:
		void __Exec(const Fkey&);
		void __ExecFrom();
		void __ExecFromKey(const string&);
		void __ExecFromDir(const string&, Mdata);
		void __ExecFromFile(const string&, Mdata&);

		Merge* destination;
	};
}
#endif

