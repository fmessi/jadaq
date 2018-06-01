/**
 * jadaq (Just Another DAQ)
 * Copyright (C) 2018  Troels Blum <troels@blum.dk>
 *
 * @author Troels Blum <troels@blum.dk>
 * @section LICENSE
 * This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *         but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * Data handler that simply throws data away - for performance testing purposes
 *
 */

#ifndef JADAQ_DATAWRITERNULL_HPP
#define JADAQ_DATAWRITERNULL_HPP


#include <cstdint>
#include "DataFormat.hpp"
#include "container.hpp"

class DataWriter
{
public:
    virtual void addDigitizer(uint32_t digitizerID) = 0;
    virtual void operator()(const jadaq::vector<Data::ListElement422>* buffer, uint32_t digitizerID, uint64_t globalTimeStamp) = 0;
    virtual void operator()(const jadaq::set<Data::ListElement422>* buffer, uint32_t digitizerID, uint64_t globalTimeStamp) = 0;
    virtual void operator()(const jadaq::buffer<Data::ListElement422>* buffer, uint32_t digitizerID, uint64_t globalTimeStamp) = 0;
};

class DataWriterNull: public DataWriter
{
public:
    DataWriterNull(uuid runID) {}
    void addDigitizer(uint32_t digitizerID) override {}
    void operator()(const jadaq::vector<Data::ListElement422>* buffer, uint32_t digitizerID, uint64_t globalTimeStamp) override {}
    void operator()(const jadaq::set<Data::ListElement422>* buffer, uint32_t digitizerID, uint64_t globalTimeStamp) override {}
    void operator()(const jadaq::buffer<Data::ListElement422>* buffer, uint32_t digitizerID, uint64_t globalTimeStamp) override {}
};


#endif //JADAQ_DATAWRITERNULL_HPP