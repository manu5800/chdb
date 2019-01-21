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

#ifndef BDBH_EXCEPTION_H
#define BDBH_EXCEPTION_H

using namespace std;

#include <stdexcept>
#include <string.h>
#include <string>

namespace bdbh {
/** \brief The exceptions generated by this application
	The error message is passed to the constructor
*/
	class BdbhException: public runtime_error {
    public:
		BdbhException(const string& msg) throw(): runtime_error(msg),_message(msg) {};
		BdbhException(const string name, int err);
		virtual ~BdbhException() throw() {};
		virtual const char * what() const throw() { 
			return _message.c_str();
		};
    protected:
		string _message;
	};

/** \brief Derives from BdbhException. Called when a stupid switch is specified
 */
	class BdbhUsageException: public BdbhException {
    public:
		BdbhUsageException() throw(): BdbhException("") {};
		virtual ~BdbhUsageException() throw() {};
	};

/** An alternate constructor for BdbhException: give him a file/directory name and an errno, 
    the constructor builds the message from strerror
*/

	inline BdbhException::BdbhException(const string name, int err):runtime_error(name),_message(name + " - " + strerror(err)){}
}
#endif
