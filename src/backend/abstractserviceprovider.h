// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef ABSTRACTSERVICEPROVIDER_H
#define ABSTRACTSERVICEPROVIDER_H

#include <string>
#include <vector>
#include <chrono>
#include <QIcon>

/**************************************************************************//**
 * @brief The AbstractServiceProvider class
 */
class AbstractServiceProvider
{
public:
	class Item;
	enum class Action { Enter, Alt, Ctrl };

	AbstractServiceProvider(){}
	virtual ~AbstractServiceProvider(){}
	virtual void query(const std::string&, std::vector<Item*>*) = 0;
};


/**************************************************************************//**
 * @brief The AbstractServiceProvider::Item class
 */
class AbstractServiceProvider::Item
{
public:
	explicit Item(): _lastAccess(0) {}
	virtual ~Item(){}
	virtual std::string title()            const = 0;
	virtual QIcon       icon()             const = 0;
	virtual std::string complete()         const = 0;
	virtual void        action(Action)           = 0;
	virtual std::string actionText(Action) const = 0;
	virtual std::string infoText()         const = 0;
	inline  u_int64_t   lastAccess()       const {return _lastAccess;}

protected:
	u_int64_t _lastAccess; // secs since epoch
};

#endif // ABSTRACTSERVICEPROVIDER_H