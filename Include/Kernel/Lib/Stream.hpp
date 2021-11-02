#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    static const char C_NEWLINE   = '\n';
    static const char C_TERMINATE = '\0';
    static const char C_TAB       = '\t';
    static const char C_CARRIAGE  = '\r';
    static const char C_VERTICAL  = '\v';
    static const char C_BLANK     = 0x20;

    enum class StreamAccess
    {
        ReadWrite,
        Read,
        Write,  
    };

    class Stream
    {
        public:
            byte*        Data;
            uint         Size;
            uint         SeekPosition;
            bool         Seekable;
            StreamAccess Access;

        public:
            Stream();
            Stream(byte* data, uint size);
            Stream(byte* data, uint size, StreamAccess access, bool seekable);
            Stream(byte* data, uint size, StreamAccess access, bool seekable, uint pos);

        public:
            virtual void Write(byte val);
            virtual void Write(byte* buffer, uint buffer_size);
            virtual byte Read();
            virtual void Read(byte* buffer, uint buffer_size);

        public:
            byte Seek();
            byte Seek(uint pos);
            byte Peek();
            byte Peek(uint pos);

        public:
            byte*        ToArray();
            uint         GetSize();
            uint         GetPosition();
            StreamAccess GetAccess();
            bool         IsSeekable();
    };
}