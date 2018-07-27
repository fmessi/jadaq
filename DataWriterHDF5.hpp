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
 * Write data to HDF5 file
 *
 */

#ifndef JADAQ_DATAHANDLERHDF5_HPP
#define JADAQ_DATAHANDLERHDF5_HPP

#include <string>
#include <mutex>
#include <map>
#include <vector>
#include <set>
#include <cassert>
#include <H5Cpp.h>
#include "DataFormat.hpp"
#include "container.hpp"

class DataWriterHDF5
{
private:
    H5::H5File* file = nullptr;
    H5::Group* root = nullptr;
    std::mutex mutex;
    std::map<uint64_t, H5::Group*> digitizerMap;
    H5::Group* addDigitizer_(uint64_t digitizerID)
    {
        std::string name = std::to_string(digitizerID>>32) + "_" + std::to_string(digitizerID & 0xFFFFFFFF);
        H5::Group* digitizerGroup = new H5::Group(file->createGroup(name));
        digitizerMap[digitizerID] = digitizerGroup;
        return digitizerGroup;
    }
  template<typename H5LOC>
  void writeAttribute(std::string name, H5LOC& location, const H5::PredType& type, const void* data) const
    {
        try {
            H5::Attribute a = location.createAttribute(name, type, H5::DataSpace(H5S_SCALAR));
            a.write(type,data);
            a.close();
        } catch (H5::Exception& e)
        {
            std::cerr << "ERROR: DataWriterHDF5 can not writeAttribute \"" << name << "\"." << std::endl;
            throw;
        }
    }


public:
  DataWriterHDF5(std::string pathname, std::string runID)
    {
        if (!pathname.empty() && *pathname.rbegin() != '/')
          pathname += '/';
        std::string filename = pathname + "jadaq-run-" + runID + ".h5";
        try
        {
            file = new H5::H5File(filename, H5F_ACC_TRUNC);
            root = new H5::Group(file->openGroup("/"));
        } catch (H5::Exception& e)
        {
            std::cerr << "ERROR: could not open/create HDF5-file \"" << filename <<  "\":" << e.getDetailMsg() << std::endl;
            throw;
        }
    }

    ~DataWriterHDF5()
    {
        assert(file);
        mutex.lock(); // Wait if someone is still writing data
        root->close();
        delete root;
        file->close();
        delete file;
        mutex.unlock();
    }
    void addDigitizer(uint64_t digitizerID)
    {
        //mutex.lock();
        //addDigitizer_(digitizerID);
        //mutex.unlock();
    }

    static bool network() { return false; }

    template <typename E>
    void operator()(const jadaq::buffer<E>* buffer, uint64_t digitizerID, uint64_t globalTimeStamp)
    {
        if (buffer->size() < 1)
            return;
        mutex.lock();
        H5::Group* digitizerGroup;
        auto itr = digitizerMap.find(digitizerID);
        if (itr != digitizerMap.end())
        {
            digitizerGroup = itr->second;
        } else
        {
            digitizerGroup = addDigitizer_(digitizerID);
            uint16_t format = E::type();
            writeAttribute("JADAQ_DATA_TYPE", *digitizerGroup, H5::PredType::NATIVE_UINT16, &format);
        }
        const hsize_t size[1] = {buffer->size()};
        try
        {
            H5::DataSpace dataspace(1, size);
            H5::DataSet dataset = digitizerGroup->createDataSet(std::to_string(globalTimeStamp), buffer->begin()->h5type(),
                                                                dataspace);
            writeAttribute("globalTimestamp", dataset, H5::PredType::NATIVE_UINT64, &globalTimeStamp);
            dataset.write(buffer->data(), buffer->begin()->h5type());
        } catch (H5::Exception& e)
        {
            std::cerr << "Error while writing to HDF5 file: " << e.getDetailMsg() <<
                      "\n\t " << "HDF5::write( " << digitizerID << ", " << globalTimeStamp <<
                      ", " << buffer->size() << " )" << std::endl;
            for(const E& element: *buffer)
            {
                std::cerr << element << "\n";
            }

        }
        mutex.unlock();
    }
};


#endif //JADAQ_DATAHANDLERHDF5_HPP
