/*
  RS485 protocol library - non-blocking.

  Devised and written by Nick Gammon.
  Date: 4 December 2012
  Version: 1.0

  Licence: Released for public use.

*/

#include "Arduino.h"

class IWriteByte {
  public:
    virtual size_t writeByte(const int what) = 0;
};

class IBytesAvailable {
  public:
    virtual int bytesAvailable() = 0;
};

class IReadByte {
  public:
    virtual int readByte() = 0;
};

class LowLevelNonBlockingRS485Connector
{

    typedef size_t (*WriteCallback)  (const byte what);    // send a byte to serial port
    typedef int  (*AvailableCallback)  ();    // return number of bytes available
    typedef int  (*ReadCallback)  ();    // read a byte from serial port

    enum {
      STX = '\2',   // start of text
      ETX = '\3'    // end of text
    };  // end of enum

    // callback functions to do reading/writing
    ReadCallback fReadCallback_;
    AvailableCallback fAvailableCallback_;
    WriteCallback fWriteCallback_;

    // where we save incoming stuff
    byte * data_;

    // how much data is in the buffer
    const int bufferSize_;

    // this is true once we have valid data in buf
    bool available_;

    // an STX (start of text) signals a packet start
    bool haveSTX_;

    // count of errors
    unsigned long errorCount_;

    // variables below are set when we get an STX
    bool haveETX_;
    byte inputPos_;
    byte currentByte_;
    bool firstNibble_;
    unsigned long startTime_;
    IWriteByte* writeByteInstance;
    IReadByte* readByteInstance;
    IBytesAvailable* bytesAvailableInstance;

    // helper private functions

    // calculate 8-bit CRC
    byte crc8 (const byte *addr, byte len)
    {
      byte crc = 0;
      while (len--)
      {
        byte inbyte = *addr++;
        for (byte i = 8; i; i--)
        {
          byte mix = (crc ^ inbyte) & 0x01;
          crc >>= 1;
          if (mix)
            crc ^= 0x8C;
          inbyte >>= 1;
        }  // end of for
      }  // end of while
      return crc;
    }  // end of RS485::crc8


    // send a byte complemented, repeated
    // only values sent would be (in hex):
    //   0F, 1E, 2D, 3C, 4B, 5A, 69, 78, 87, 96, A5, B4, C3, D2, E1, F0
    void sendComplemented (const byte what)
    {
      byte c;
      // first nibble
      c = what >> 4;
      writeByteInstance->writeByte((c << 4) | (c ^ 0x0F));
      // second nibble
      c = what & 0x0F;
      writeByteInstance->writeByte((c << 4) | (c ^ 0x0F));
    }  // end of RS485::sendComplemented

  public:

    // constructor
    LowLevelNonBlockingRS485Connector (IReadByte*  readByteInstance_,
           IBytesAvailable*  bytesAvailableInstance_,
           IWriteByte*  writeByteInstance_,
           const byte bufferSize) :
      readByteInstance(readByteInstance_),
      bytesAvailableInstance(bytesAvailableInstance_),
      writeByteInstance(writeByteInstance_),
      data_ (NULL),
      bufferSize_ (bufferSize)
    {}

    // destructor - frees memory used
    ~LowLevelNonBlockingRS485Connector () {
      stop ();
    }

    // allocate the requested buffer size : allocate memory for buf_
    void begin () {
      data_ = (byte *) malloc (bufferSize_);
      reset ();
      errorCount_ = 0;
    }

    // get rid of the buffer : free memory in buf_
    void stop () {
      reset ();
      free (data_);
      data_ = NULL;
    }

    // handle incoming data, return true if packet ready
    // called periodically from main loop to process data and
    // assemble the finished packet in 'data_'
    // You could implement a timeout by seeing if isPacketStarted() returns
    // true, and if too much time has passed since getPacketStartTime() time.
    bool update ()
    {
      // no data? can't go ahead (eg. begin() not called)
      if (data_ == NULL)
        return false;

      // no callbacks? Can't read
      if (bytesAvailableInstance == NULL || readByteInstance == NULL)
        return false;

      while (bytesAvailableInstance->bytesAvailable() > 0)
      {
        byte inByte = readByteInstance->readByte();

        switch (inByte)
        {

          case STX:   // start of text
            haveSTX_ = true;
            haveETX_ = false;
            inputPos_ = 0;
            firstNibble_ = true;
            startTime_ = millis ();
            break;

          case ETX:   // end of text (now expect the CRC check)
            haveETX_ = true;
            break;

          default:
            // wait until packet officially starts
            if (!haveSTX_)
              break;

            // check byte is in valid form (4 bits followed by 4 bits complemented)
            if ((inByte >> 4) != ((inByte & 0x0F) ^ 0x0F) )
            {
              reset ();
              errorCount_++;
              break;  // bad character
            } // end if bad byte

            // convert back
            inByte >>= 4;

            // high-order nibble?
            if (firstNibble_)
            {
              currentByte_ = inByte;
              firstNibble_ = false;
              break;
            }  // end of first nibble

            // low-order nibble
            currentByte_ <<= 4;
            currentByte_ |= inByte;
            firstNibble_ = true;

            // if we have the ETX this must be the CRC
            if (haveETX_)
            {
              if (crc8 (data_, inputPos_) != currentByte_)
              {
                reset ();
                errorCount_++;
                break;  // bad crc
              } // end of bad CRC

              available_ = true;
              return true;  // show data ready
            }  // end if have ETX already

            // keep adding if not full
            if (inputPos_ < bufferSize_)
              data_ [inputPos_++] = currentByte_;
            else
            {
              reset (); // overflow, start again
              errorCount_++;
            }

            break;

        }  // end of switch
      }  // end of while incoming data

      return false;  // not ready yet
    } // end of RS485::update

    // called after an error to return to "not in a packet" : reset to no incoming data (eg. after a timeout)
    void reset ()
    {
      haveSTX_ = false;
      available_ = false;
      inputPos_ = 0;
      startTime_ = 0;
    } // end of RS485::reset


    // send a message of "length" bytes (max 255) to other end
    // put STX at start, ETX at end, and add CRC
    void sendMsg (const byte * data, const byte length)
    {
      // no callback? Can't send
      if (writeByteInstance == NULL)
        return;

      writeByteInstance->writeByte(STX);  // STX
      for (byte i = 0; i < length; i++)
        sendComplemented (data [i]);
      writeByteInstance->writeByte(ETX);  // ETX
      sendComplemented (crc8 (data, length));
    }  // end of RS485::sendMsg


    // returns true if packet available
    bool available () const {
      return available_;
    };

    // once available, returns the address of the current message
    byte * getData ()   const {
      return data_;
    }
    byte   getLength () const {
      return inputPos_;
    }

    // return how many errors we have had
    unsigned long getErrorCount () const {
      return errorCount_;
    }

    // return when last packet started
    unsigned long getPacketStartTime () const {
      return startTime_;
    }

    // return true if a packet has started to be received
    bool isPacketStarted () const {
      return haveSTX_;
    }

}; // end of class RS485

