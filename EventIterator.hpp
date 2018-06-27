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

#ifndef JADAQ_EVENTITERATOR_HPP
#define JADAQ_EVENTITERATOR_HPP

#include <iterator>
#include <limits>
#include "caen.hpp"
#include "DataFormat.hpp"

template <typename E> class DPPQDCEventIterator;

class EventIterator
{
private:
    void* ptr;

public:
    template <typename T>
    EventIterator(DPPQDCEventIterator<T>& b): ptr(&b) {}
    template<typename T>
    EventIterator& operator=(DPPQDCEventIterator<T>& b)
    {
        ptr = &b;
        return *this;
    }
    template<typename T>
    DPPQDCEventIterator<T>& base() const
    {
        return *static_cast<DPPQDCEventIterator<T>*>(ptr);
    }
};

/*
 * DPPQDCEventIterator will iterate over a set of Board Aggregates contained in one Data Block
 */
template <typename E>
class DPPQDCEventIterator
{
private:
    const caen::ReadoutBuffer& buffer;
    uint32_t* ptr;
    uint32_t* boardAggregateEnd;
    /*
     * Group Iterator will iterate over a set of Group Aggregates contained in one Board Aggregate
     */
    class GroupIterator
    {
    private:
        uint32_t* ptr;
        uint32_t* end;
        uint8_t groupMask = 0;
        size_t elementSize = 0 ;
        int group = -1;
        bool waveform = false;
        bool extras = false;
        static const uint channelsPerGroup = 8;
    public:
        GroupIterator(void* p) : ptr((uint32_t*)p) {}
        GroupIterator(uint32_t* p, uint16_t gm)
                : ptr(p)
                , groupMask(gm)
                , elementSize(2)
        { nextGroup(); }
        void nextGroup()
        {
            while (!(groupMask & (1<<++group)))
            {
                if (group == sizeof(groupMask)*CHAR_BIT)
                {
                    return; // End of Board Aggregate
                }
            }
            assert(((ptr[0] >> 31) & 1) == 1);
            uint32_t size = ptr[0] & 0x7fffffff;
            uint32_t format = ptr[1];
            assert (((format>>31) & 1) == 0);
            assert (((format>>30) & 1) == 1);
            assert (((format>>29) & 1) == 1);
            extras = ((format>>28) & 1) == 1;
            waveform = ((format>>27) & 1) == 1;
            end = ptr+size;
            ptr += 2; //point to first event
            if (extras)
                elementSize += 1;
            if (waveform)
                elementSize += (format & 0xFFF) << 2;
            assert((size - 2) % elementSize == 0);

        }
        GroupIterator& operator++()
        {
            ptr += elementSize;
            if (ptr == end)
            {
                nextGroup();
            }
            assert(ptr <= end);
            return *this;
        }
        GroupIterator operator++(int)
        {
            GroupIterator tmp(*this);
            ++*this;
            return tmp;
        }
        bool operator==(const GroupIterator& other) const { return ptr == other.ptr; }
        bool operator!=(const GroupIterator& other) const { return (ptr != other.ptr); }
        bool operator>(const GroupIterator& other) const { return ptr > other.ptr; }
        bool operator<(const GroupIterator& other) const { return ptr < other.ptr; }
        bool operator>=(const GroupIterator& other) const { return ptr >= other.ptr; }
        bool operator<=(const GroupIterator& other) const { return ptr <= other.ptr; }
        bool operator==(const void* other) const { return ptr == other; }
        bool operator!=(const void* other) const { return ptr != other; }
        bool operator>(const void* other) const { return ptr > other; }
        bool operator<(const void* other) const { return ptr < other; }
        bool operator>=(const void* other) const { return ptr >= other; }
        bool operator<=(const void* other) const { return ptr <= other; }
        E operator*() const;
    };
    GroupIterator groupIterator;
    GroupIterator nextGroupIterator()
    {
        if ((char*)ptr < buffer.end())
        {
            size_t size = (ptr[0] & 0x0fffffff);
            assert((ptr[0] & 0xf0000000) == 0xa0000000); // Magic value
            boardAggregateEnd = ptr + size;
            uint8_t groupMask = (uint8_t) (ptr[1] & 0xFF);
            ptr += 4; // point to first group aggregate
            return GroupIterator(ptr, groupMask);
        } else
        {
            return GroupIterator(buffer.end());
        }
    }
public:
    DPPQDCEventIterator(const caen::ReadoutBuffer& b)
            : buffer(b)
            , ptr((uint32_t*)buffer.data)
            , groupIterator(nextGroupIterator()) {}
    DPPQDCEventIterator& operator++()
    {
        if (++groupIterator == boardAggregateEnd)
        {
            ptr = boardAggregateEnd;
            groupIterator = nextGroupIterator();
        }
        return *this;
    }
    DPPQDCEventIterator operator++(int)
    {
        DPPQDCEventIterator tmp(*this);
        ++*this;
        return tmp;
    }
    bool operator==(const DPPQDCEventIterator& other) const { return groupIterator == other.groupIterator; }
    bool operator!=(const DPPQDCEventIterator& other) const { return groupIterator != other.groupIterator; }
    bool operator>(const DPPQDCEventIterator& other) const { return groupIterator > other.groupIterator; }
    bool operator<(const DPPQDCEventIterator& other) const { return groupIterator < other.groupIterator; }
    bool operator>=(const DPPQDCEventIterator& other) const { return groupIterator >= other.groupIterator; }
    bool operator<=(const DPPQDCEventIterator& other) const { return groupIterator <= other.groupIterator; }
    bool operator==(const void* other) const { return groupIterator == other; }
    bool operator!=(const void* other) const { return groupIterator != other; }
    bool operator>(const void* other) const { return groupIterator > other; }
    bool operator<(const void* other) const { return groupIterator < other; }
    bool operator>=(const void* other) const { return groupIterator >= other; }
    bool operator<=(const void* other) const { return groupIterator <= other; }
    E operator*() const { return *groupIterator; }
    void* end() const { return buffer.end(); }
};

template <>
inline Data::ListElement422 DPPQDCEventIterator<Data::ListElement422>::GroupIterator::operator*() const
{
    Data::ListElement422 res;
    res.time = ptr[0];
    uint32_t data = ptr[elementSize-1];
    res.channel   = (group*channelsPerGroup) | (uint16_t)(data >> 28);
    res.charge  = (uint16_t)(data & 0x0000ffffu);
    return res;
}

template <>
inline Data::ListElement8222 DPPQDCEventIterator<Data::ListElement8222>::GroupIterator::operator*() const
{
    uint64_t time = ptr[0];
    uint32_t extra = 0;
    if (extras)
        extra = ptr[elementSize-2];
    uint32_t data = ptr[elementSize-1];
    Data::ListElement8222 res;
    res.time = ((uint64_t)(extra & 0x0000ffffu)<<32) | time;
    res.channel   = (group*channelsPerGroup) | (uint16_t)(data >> 28);
    res.charge  = (uint16_t)(data & 0x0000ffffu);
    res.baseline  = (uint16_t)(extra>>16);
    return res;
}

#define DVP(V,S) {                              \
    uint32_t v = (ss & (0x10001000u<<(S)));     \
    if ((V).start == 0xffffu)                   \
    {                                           \
        if (v)                                  \
        {                                       \
            (V).start = (i<<1) | (v>>(28+(S))); \
            if (v == 0x1000u<<(S))              \
                (V).end = (i<<1)|1;             \
        }                                       \
    } else {                                    \
        if (v<(0x10001000u<<(S)))               \
            (V).end = (i<<1) | (v>>(28+(S)));   \
    }                                           \
}


template <>
inline Data::WaveformElement DPPQDCEventIterator<Data::WaveformElement>::GroupIterator::operator*() const
{
    Data::WaveformElement res;
    /*
    size_t n = (elementSize-(2+extras))<<1;
    uint64_t time = ptr[0];
    uint32_t extra = 0;
    if (extras)
        extra = ptr[elementSize-2];
    uint32_t data = ptr[elementSize-1];

    res.time = ((uint64_t)(extra & 0x0000ffffu)<<32) | time;
    res.channel   = (group*channelsPerGroup) | (uint16_t)(data >> 28);
    res.charge  = (uint16_t)(data & 0x0000ffffu);
    res.baseline  = (uint16_t)(extra>>16);
    uint16_t trigger = 0xFFFF;
    Data::Interval gate = {0xffff,0xffff};
    Data::Interval holdoff  = {0xffff,0xffff};
    Data::Interval over = {0xffff,0xffff};
    for (uint16_t i = 0; i < (n>>1); ++i)
    {
        uint32_t ss = ptr[i+1];
      //  res.samples[i<<1] = (uint16_t)(ss & 0x0fff);
      //  res.samples[i<<1|1] = (uint16_t)((ss>>16) & 0x0fff);
        // trigger
        if (uint32_t t = (ss & 0x20002000))
        {
            trigger = (i<<1) | (t>>29);
        }
        DVP(gate,0)
        DVP(holdoff,2)
        DVP(over,3)
    }
    res.trigger = trigger;
    res.gate = gate;
    res.holdoff = holdoff;
    res.overthreshold = over;
    */
    return res;
}

#endif //JADAQ_EVENTITERATOR_HPP