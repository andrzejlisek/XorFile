#include "hashcalcmd5.h"

namespace EdenClass
{

    HashCalcMD5::HashCalcMD5()
    {
        ChunkCurrent = 0;
        ChunkAll = 0;
    }

    inline unsigned int HashCalcMD5::LEFTROTATE(unsigned int x, unsigned int c)
    {
        return ((x << c) | (x >> (32 - c)));
    }


    inline void HashCalcMD5::to_bytes(unsigned int val, unsigned char * bytes)
    {
        bytes[0] = (unsigned char)val;
        bytes[1] = (unsigned char)(val >> 8);
        bytes[2] = (unsigned char)(val >> 16);
        bytes[3] = (unsigned char)(val >> 24);
    }

    inline unsigned int HashCalcMD5::to_int32(unsigned char * bytes)
    {
        return (unsigned int)bytes[0] | ((unsigned int)bytes[1] << 8) | ((unsigned int)bytes[2] << 16) | ((unsigned int)bytes[3] << 24);
    }

    void HashCalcMD5::md5_array(unsigned char * initial_msg, unsigned int initial_len, unsigned char * digest)
    {
        // These vars will contain the hash
        unsigned int h0, h1, h2, h3;

        // Message (to prepare)
        unsigned char *msg = NULL;

        unsigned int new_len, offset;
        unsigned int w[16];
        unsigned int a, b, c, d, i, f, g, temp;

        // Initialize variables - simple count in nibbles:
        h0 = 0x67452301;
        h1 = 0xefcdab89;
        h2 = 0x98badcfe;
        h3 = 0x10325476;

        //Pre-processing:
        //append "1" bit to message
        //append "0" bits until message length in bits ≡ 448 (mod 512)
        //append length mod (2^64) to message

        for (new_len = initial_len + 1; new_len % (512/8) != 448/8; new_len++);

        ChunkAll = (new_len + 8) / 64;

        msg = (unsigned char*)malloc(new_len + 8);
        memcpy(msg, initial_msg, initial_len);
        msg[initial_len] = 0x80; // append the "1" bit; most significant bit is "first"
        for (offset = initial_len + 1; offset < new_len; offset++)
        {
            msg[offset] = 0; // append "0" bits
        }

        // append the len in bits at the end of the buffer.
        to_bytes(initial_len*8, msg + new_len);
        // initial_len>>29 == initial_len*8>>32, but avoids overflow.
        to_bytes(initial_len>>29, msg + new_len + 4);

        // Process the message in successive 512-bit chunks:
        //for each 512-bit chunk of message:
        for(offset=0; offset<new_len; offset += (512/8))
        {

            // break chunk into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
            for (i = 0; i < 16; i++)
                w[i] = to_int32(msg + offset + i*4);

            // Initialize hash value for this chunk:
            a = h0;
            b = h1;
            c = h2;
            d = h3;

            // Main loop:
            for(i = 0; i<64; i++)
            {

                if (i < 16) {
                    f = (b & c) | ((~b) & d);
                    g = i;
                } else if (i < 32) {
                    f = (d & b) | ((~d) & c);
                    g = (5*i + 1) % 16;
                } else if (i < 48) {
                    f = b ^ c ^ d;
                    g = (3*i + 5) % 16;
                } else {
                    f = c ^ (b | (~d));
                    g = (7*i) % 16;
                }

                temp = d;
                d = c;
                c = b;
                b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
                a = temp;

            }

            // Add this chunk's hash to result so far:
            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;

            ChunkCurrent++;
        }

        // cleanup
        free(msg);

        //var char digest[16] := h0 append h1 append h2 append h3 //(Output is in little-endian)
        to_bytes(h0, digest);
        to_bytes(h1, digest + 4);
        to_bytes(h2, digest + 8);
        to_bytes(h3, digest + 12);
    }

    void HashCalcMD5::md5_file(const char * filename, unsigned char * digest)
    {
        std::fstream RAW(filename, std::ios::in|std::ios::binary);

        if (RAW.is_open())
        {
            RAW.seekg(0, std::ios_base::end);
            unsigned long long initial_len = RAW.tellg();
            RAW.seekg(0);

            // Calculating the MD5 as the same way as for the array
            // #####################################################################




            // These vars will contain the hash
            unsigned int h0, h1, h2, h3;

            // Message (to prepare)
            unsigned char *msg = NULL;

            unsigned long long new_len, offset;
            unsigned int w[16];
            unsigned int a, b, c, d, i, f, g, temp;

            // Initialize variables - simple count in nibbles:
            h0 = 0x67452301;
            h1 = 0xefcdab89;
            h2 = 0x98badcfe;
            h3 = 0x10325476;

            //Pre-processing:
            //append "1" bit to message
            //append "0" bits until message length in bits ≡ 448 (mod 512)
            //append length mod (2^64) to message

            for (new_len = initial_len + 1; new_len % (512/8) != 448/8; new_len++);

            ChunkAll = (new_len + 8) / 64;

            // Dwa ostatnie segmenty po rozszerzeniu
            unsigned char* AdditionalMsg = (unsigned char*)malloc(128);

            // FileBufLen - liczba bajtow do odczytu z pliku
            unsigned long long FileBufLen = initial_len;
            FileBufLen /= 64;
            FileBufLen *= 64;

            // Doczytanie koncowki pliku do bufora
            if (initial_len > FileBufLen)
            {
                RAW.seekg(FileBufLen);
                RAW.read((char*)AdditionalMsg, initial_len - FileBufLen);
                RAW.seekg(0);
            }

            // msg - czesc pliku
            msg = (unsigned char*)malloc(64);


            AdditionalMsg[initial_len - FileBufLen] = 0x80; // append the "1" bit; most significant bit is "first"
            for (offset = initial_len + 1; offset < new_len; offset++)
            {
                AdditionalMsg[offset - FileBufLen] = 0; // append "0" bits
            }

            // append the len in bits at the end of the buffer.
            to_bytes(initial_len*8, AdditionalMsg + new_len - FileBufLen);
            // initial_len>>29 == initial_len*8>>32, but avoids overflow.
            to_bytes(initial_len>>29, AdditionalMsg + new_len + 4 - FileBufLen);

            // Process the message in successive 512-bit chunks:
            //for each 512-bit chunk of message:
            for(offset=0; offset<new_len; offset += (512/8))
            {
                // Pobieranie porcji 64 bajtow z pliku
                if ((offset + 64) <= FileBufLen)
                {
                    RAW.read((char*)msg, 64);
                }
                else
                {
                    memcpy(msg, AdditionalMsg + (offset - FileBufLen), 64);
                }

                // break chunk into sixteen 32-bit words w[j], 0 ≤ j ≤ 15
                for (i = 0; i < 16; i++)
                {
                    w[i] = to_int32(msg + i*4);
                }

                // Initialize hash value for this chunk:
                a = h0;
                b = h1;
                c = h2;
                d = h3;

                // Main loop:
                for(i = 0; i<64; i++)
                {
                    if (i < 16) {
                        f = (b & c) | ((~b) & d);
                        g = i;
                    } else if (i < 32) {
                        f = (d & b) | ((~d) & c);
                        g = (5*i + 1) % 16;
                    } else if (i < 48) {
                        f = b ^ c ^ d;
                        g = (3*i + 5) % 16;
                    } else {
                        f = c ^ (b | (~d));
                        g = (7*i) % 16;
                    }

                    temp = d;
                    d = c;
                    c = b;
                    b = b + LEFTROTATE((a + f + k[i] + w[g]), r[i]);
                    a = temp;

                }

                // Add this chunk's hash to result so far:
                h0 += a;
                h1 += b;
                h2 += c;
                h3 += d;

                ChunkCurrent++;
            }

            // cleanup
            free(msg);
            free(AdditionalMsg);

            //var char digest[16] := h0 append h1 append h2 append h3 //(Output is in little-endian)
            to_bytes(h0, digest);
            to_bytes(h1, digest + 4);
            to_bytes(h2, digest + 8);
            to_bytes(h3, digest + 12);



            // #####################################################################



            RAW.close();
        }
        else
        {
            to_bytes(0, digest);
            to_bytes(0, digest + 4);
            to_bytes(0, digest + 8);
            to_bytes(0, digest + 12);
        }
    }

    unsigned char * HashCalcMD5::Calc(unsigned char * RAW, unsigned int Length)
    {
        unsigned char * X = new unsigned char[16];
        HashCalcMD5 Temp;
        Temp.md5_array(RAW, Length, X);
        return X;
    }

    unsigned char * HashCalcMD5::Calc(char *FileName)
    {
        unsigned char * X = new unsigned char[16];
        HashCalcMD5 Temp;
        Temp.md5_file(FileName, X);
        return X;
    }

    unsigned char * HashCalcMD5::CalcObj(unsigned char * RAW, unsigned int Length)
    {
        ChunkAll = 1000;
        ChunkCurrent = 0;
        unsigned char * X = new unsigned char[16];
        md5_array(RAW, Length, X);
        return X;
    }

    unsigned char * HashCalcMD5::CalcObj(const char *FileName)
    {
        ChunkAll = 1000;
        ChunkCurrent = 0;
        unsigned char * X = new unsigned char[16];
        md5_file(FileName, X);
        return X;
    }
}
