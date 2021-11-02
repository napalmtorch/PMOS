#include <Kernel/Lib/Stream.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    // blank stream constructor
    Stream::Stream()
    {
        Data         = nullptr;
        Size         = 0;
        SeekPosition = 0;
        Access       = (StreamAccess)0;
        Seekable     = false;
    }

    // stream constructor
    Stream::Stream(byte* data, uint size)
    {
        Data         = data;
        Size         = size;
        SeekPosition = 0;
        Access       = StreamAccess::ReadWrite;
        Seekable     = true;
    }

    // stream constructor with specified access
    Stream::Stream(byte* data, uint size, StreamAccess access, bool seekable)
    {
        Data         = data;
        Size         = size;
        SeekPosition = 0;
        Access       = access;
        Seekable     = seekable;
    }

    // stream constructor specifying all properties
    Stream::Stream(byte* data, uint size, StreamAccess access, bool seekable, uint pos)
    {
        Data         = data;
        Size         = size;
        SeekPosition = pos;
        Access       = access;
        Seekable     = seekable;
    }

    // write byte to current position
    void Stream::Write(byte val)
    {
        // check if access is granted
        if (Access == StreamAccess::Read) { return; }
        
        // validate seek position
        if (SeekPosition >= Size) { return; }

        // write value to stream
        Data[SeekPosition] = val;
        SeekPosition++;
    }

    // write specified amount of bytes from buffer to position
    void Stream::Write(byte* buffer, uint buffer_size)
    {
        // write buffer to stream
        for (uint i = 0; i < buffer_size; i++) 
        { 
            Write(buffer[i]); 
        }
    }

    // read byte at current position in stream
    byte Stream::Read()
    {
        // check if access is granted
        if (Access == StreamAccess::Write) { return 0; }

        // validate seek position
        if (SeekPosition >= Size) { return 0; }

        // return value from stream
        return Data[SeekPosition];

    }

    // read specified amount of bytes from position into buffer
    void Stream::Read(byte* buffer, uint buffer_size)
    {
        // check if access is granted
        if (Access == StreamAccess::Write) { return; }

        // read data into buffer
        for (uint i = 0; i < buffer_size; i++) { buffer[i] = Read(); }
    }

    // seek next position
    byte Stream::Seek()
    {
        // check if seeking is enabled
        if (!Seekable) { return SeekPosition; } 

        // increment and return seek position
        SeekPosition++;
        return SeekPosition;
    }

    // seek specified position
    byte Stream::Seek(uint pos)
    {
        // check if seeking is enabled
        if (!Seekable) { return SeekPosition; }

        // set and return seek position
        SeekPosition = pos;
        return SeekPosition;
    }

    // peek value at current position
    byte Stream::Peek()
    {
        // check if access is granted
        if (Access == StreamAccess::Write) { return 0; }

        // validate seek position
        if (SeekPosition >= Size) { return 0; }

        // return value
        return Data[SeekPosition];
    }

    // peek value at specified position
    byte Stream::Peek(uint pos)
    {
        // check if access is granted
        if (Access == StreamAccess::Write) { return 0; }

        // validate seek position
        if (pos >= Size) { return 0; }

        // return value
        return Data[pos];
    }

    // get stream data array
    byte* Stream::ToArray() { return Data; }

    // get stream data size
    uint Stream::GetSize() { return Size; }

    // get current seeking position
    uint Stream::GetPosition() { return SeekPosition; }

    // get stream access level
    StreamAccess Stream::GetAccess() { return Access; }

    // get stream seekability
    bool Stream::IsSeekable() { return Seekable; }
}