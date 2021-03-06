/**
 * jadaq (Just Another DAQ)
 * Copyright (C) 2017  Troels Blum <troels@blum.dk>
 * Contributions from  Jonas Bardino <bardino@nbi.ku.dk>
 *
 * @file
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
 *
 */

#ifndef JADAQ_EVENTACCESSOR_HPP
#define JADAQ_EVENTACCESSOR_HPP

#include "caen.hpp"
#include "DataFormat.hpp"

template <typename E>
class DPPEventAccessor
{
public:
    virtual uint16_t channels() const = 0;
    virtual uint32_t events(uint16_t channel) const = 0;
    virtual E operator()(uint16_t channel, size_t i) const = 0;
};

struct AnyDPPEventAccessor
{
private:
    void* ptr;

public:
    template <typename T>
    AnyDPPEventAccessor(DPPEventAccessor<T>& b): ptr(&b) {}
    template<typename T>
    AnyDPPEventAccessor& operator=(DPPEventAccessor<T>& b)
    {
        ptr = &b;
        return *this;
    }
    template<typename T>
    DPPEventAccessor<T>& base() const
    {
        return *static_cast<DPPEventAccessor<T>*>(ptr);
    }
    template<typename T>
    T operator()(uint16_t channel, size_t i) const
    {
        return base<T>().operator()(channel,i);
    }
};


template <typename E>
class DPPQDCEventAccessor : public DPPEventAccessor<E>
{
private:
    const caen::DPPEvents<_CAEN_DGTZ_DPP_QDC_Event_t>& container;
    const uint16_t numChannels;
public:
    DPPQDCEventAccessor(const caen::DPPEvents<_CAEN_DGTZ_DPP_QDC_Event_t>& events, uint16_t channels)
            : container(events)
            , numChannels(channels) {}
    uint16_t channels() const override { return numChannels; }
    uint32_t events(uint16_t channel) const override { return container.nEvents[channel]; }
    E operator()(uint16_t channel, size_t i) const override;
};

template <>
inline Data::ListElement422 DPPQDCEventAccessor<Data::ListElement422>::operator()(uint16_t channel, size_t i) const
{
    Data::ListElement422 res;
    res.time = container.ptr[channel][i].TimeTag;
    res.channel = channel;
    res.charge = container.ptr[channel][i].Charge;
    return res;
}


#endif //JADAQ_ACCESSOR_HPP
